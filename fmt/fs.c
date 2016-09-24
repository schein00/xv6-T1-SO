4800 // File system implementation.  Five layers:
4801 //   + Blocks: allocator for raw disk blocks.
4802 //   + Log: crash recovery for multi-step updates.
4803 //   + Files: inode allocator, reading, writing, metadata.
4804 //   + Directories: inode with special contents (list of other inodes!)
4805 //   + Names: paths like /usr/rtm/xv6/fs.c for convenient naming.
4806 //
4807 // This file contains the low-level file system manipulation
4808 // routines.  The (higher-level) system call implementations
4809 // are in sysfile.c.
4810 
4811 #include "types.h"
4812 #include "defs.h"
4813 #include "param.h"
4814 #include "stat.h"
4815 #include "mmu.h"
4816 #include "proc.h"
4817 #include "spinlock.h"
4818 #include "fs.h"
4819 #include "buf.h"
4820 #include "file.h"
4821 
4822 #define min(a, b) ((a) < (b) ? (a) : (b))
4823 static void itrunc(struct inode*);
4824 // there should be one superblock per disk device, but we run with
4825 // only one device
4826 struct superblock sb;
4827 
4828 // Read the super block.
4829 void
4830 readsb(int dev, struct superblock *sb)
4831 {
4832   struct buf *bp;
4833 
4834   bp = bread(dev, 1);
4835   memmove(sb, bp->data, sizeof(*sb));
4836   brelse(bp);
4837 }
4838 
4839 // Zero a block.
4840 static void
4841 bzero(int dev, int bno)
4842 {
4843   struct buf *bp;
4844 
4845   bp = bread(dev, bno);
4846   memset(bp->data, 0, BSIZE);
4847   log_write(bp);
4848   brelse(bp);
4849 }
4850 // Blocks.
4851 
4852 // Allocate a zeroed disk block.
4853 static uint
4854 balloc(uint dev)
4855 {
4856   int b, bi, m;
4857   struct buf *bp;
4858 
4859   bp = 0;
4860   for(b = 0; b < sb.size; b += BPB){
4861     bp = bread(dev, BBLOCK(b, sb));
4862     for(bi = 0; bi < BPB && b + bi < sb.size; bi++){
4863       m = 1 << (bi % 8);
4864       if((bp->data[bi/8] & m) == 0){  // Is block free?
4865         bp->data[bi/8] |= m;  // Mark block in use.
4866         log_write(bp);
4867         brelse(bp);
4868         bzero(dev, b + bi);
4869         return b + bi;
4870       }
4871     }
4872     brelse(bp);
4873   }
4874   panic("balloc: out of blocks");
4875 }
4876 
4877 // Free a disk block.
4878 static void
4879 bfree(int dev, uint b)
4880 {
4881   struct buf *bp;
4882   int bi, m;
4883 
4884   readsb(dev, &sb);
4885   bp = bread(dev, BBLOCK(b, sb));
4886   bi = b % BPB;
4887   m = 1 << (bi % 8);
4888   if((bp->data[bi/8] & m) == 0)
4889     panic("freeing free block");
4890   bp->data[bi/8] &= ~m;
4891   log_write(bp);
4892   brelse(bp);
4893 }
4894 
4895 
4896 
4897 
4898 
4899 
4900 // Inodes.
4901 //
4902 // An inode describes a single unnamed file.
4903 // The inode disk structure holds metadata: the file's type,
4904 // its size, the number of links referring to it, and the
4905 // list of blocks holding the file's content.
4906 //
4907 // The inodes are laid out sequentially on disk at
4908 // sb.startinode. Each inode has a number, indicating its
4909 // position on the disk.
4910 //
4911 // The kernel keeps a cache of in-use inodes in memory
4912 // to provide a place for synchronizing access
4913 // to inodes used by multiple processes. The cached
4914 // inodes include book-keeping information that is
4915 // not stored on disk: ip->ref and ip->flags.
4916 //
4917 // An inode and its in-memory represtative go through a
4918 // sequence of states before they can be used by the
4919 // rest of the file system code.
4920 //
4921 // * Allocation: an inode is allocated if its type (on disk)
4922 //   is non-zero. ialloc() allocates, iput() frees if
4923 //   the link count has fallen to zero.
4924 //
4925 // * Referencing in cache: an entry in the inode cache
4926 //   is free if ip->ref is zero. Otherwise ip->ref tracks
4927 //   the number of in-memory pointers to the entry (open
4928 //   files and current directories). iget() to find or
4929 //   create a cache entry and increment its ref, iput()
4930 //   to decrement ref.
4931 //
4932 // * Valid: the information (type, size, &c) in an inode
4933 //   cache entry is only correct when the I_VALID bit
4934 //   is set in ip->flags. ilock() reads the inode from
4935 //   the disk and sets I_VALID, while iput() clears
4936 //   I_VALID if ip->ref has fallen to zero.
4937 //
4938 // * Locked: file system code may only examine and modify
4939 //   the information in an inode and its content if it
4940 //   has first locked the inode. The I_BUSY flag indicates
4941 //   that the inode is locked. ilock() sets I_BUSY,
4942 //   while iunlock clears it.
4943 //
4944 // Thus a typical sequence is:
4945 //   ip = iget(dev, inum)
4946 //   ilock(ip)
4947 //   ... examine and modify ip->xxx ...
4948 //   iunlock(ip)
4949 //   iput(ip)
4950 //
4951 // ilock() is separate from iget() so that system calls can
4952 // get a long-term reference to an inode (as for an open file)
4953 // and only lock it for short periods (e.g., in read()).
4954 // The separation also helps avoid deadlock and races during
4955 // pathname lookup. iget() increments ip->ref so that the inode
4956 // stays cached and pointers to it remain valid.
4957 //
4958 // Many internal file system functions expect the caller to
4959 // have locked the inodes involved; this lets callers create
4960 // multi-step atomic operations.
4961 
4962 struct {
4963   struct spinlock lock;
4964   struct inode inode[NINODE];
4965 } icache;
4966 
4967 void
4968 iinit(int dev)
4969 {
4970   initlock(&icache.lock, "icache");
4971   readsb(dev, &sb);
4972   cprintf("sb: size %d nblocks %d ninodes %d nlog %d logstart %d\
4973           inodestart %d bmap start %d\n", sb.size, sb.nblocks,
4974           sb.ninodes, sb.nlog, sb.logstart, sb.inodestart,
4975           sb.bmapstart);
4976 }
4977 
4978 static struct inode* iget(uint dev, uint inum);
4979 
4980 
4981 
4982 
4983 
4984 
4985 
4986 
4987 
4988 
4989 
4990 
4991 
4992 
4993 
4994 
4995 
4996 
4997 
4998 
4999 
5000 // Allocate a new inode with the given type on device dev.
5001 // A free inode has a type of zero.
5002 struct inode*
5003 ialloc(uint dev, short type)
5004 {
5005   int inum;
5006   struct buf *bp;
5007   struct dinode *dip;
5008 
5009   for(inum = 1; inum < sb.ninodes; inum++){
5010     bp = bread(dev, IBLOCK(inum, sb));
5011     dip = (struct dinode*)bp->data + inum%IPB;
5012     if(dip->type == 0){  // a free inode
5013       memset(dip, 0, sizeof(*dip));
5014       dip->type = type;
5015       log_write(bp);   // mark it allocated on the disk
5016       brelse(bp);
5017       return iget(dev, inum);
5018     }
5019     brelse(bp);
5020   }
5021   panic("ialloc: no inodes");
5022 }
5023 
5024 // Copy a modified in-memory inode to disk.
5025 void
5026 iupdate(struct inode *ip)
5027 {
5028   struct buf *bp;
5029   struct dinode *dip;
5030 
5031   bp = bread(ip->dev, IBLOCK(ip->inum, sb));
5032   dip = (struct dinode*)bp->data + ip->inum%IPB;
5033   dip->type = ip->type;
5034   dip->major = ip->major;
5035   dip->minor = ip->minor;
5036   dip->nlink = ip->nlink;
5037   dip->size = ip->size;
5038   memmove(dip->addrs, ip->addrs, sizeof(ip->addrs));
5039   log_write(bp);
5040   brelse(bp);
5041 }
5042 
5043 
5044 
5045 
5046 
5047 
5048 
5049 
5050 // Find the inode with number inum on device dev
5051 // and return the in-memory copy. Does not lock
5052 // the inode and does not read it from disk.
5053 static struct inode*
5054 iget(uint dev, uint inum)
5055 {
5056   struct inode *ip, *empty;
5057 
5058   acquire(&icache.lock);
5059 
5060   // Is the inode already cached?
5061   empty = 0;
5062   for(ip = &icache.inode[0]; ip < &icache.inode[NINODE]; ip++){
5063     if(ip->ref > 0 && ip->dev == dev && ip->inum == inum){
5064       ip->ref++;
5065       release(&icache.lock);
5066       return ip;
5067     }
5068     if(empty == 0 && ip->ref == 0)    // Remember empty slot.
5069       empty = ip;
5070   }
5071 
5072   // Recycle an inode cache entry.
5073   if(empty == 0)
5074     panic("iget: no inodes");
5075 
5076   ip = empty;
5077   ip->dev = dev;
5078   ip->inum = inum;
5079   ip->ref = 1;
5080   ip->flags = 0;
5081   release(&icache.lock);
5082 
5083   return ip;
5084 }
5085 
5086 // Increment reference count for ip.
5087 // Returns ip to enable ip = idup(ip1) idiom.
5088 struct inode*
5089 idup(struct inode *ip)
5090 {
5091   acquire(&icache.lock);
5092   ip->ref++;
5093   release(&icache.lock);
5094   return ip;
5095 }
5096 
5097 
5098 
5099 
5100 // Lock the given inode.
5101 // Reads the inode from disk if necessary.
5102 void
5103 ilock(struct inode *ip)
5104 {
5105   struct buf *bp;
5106   struct dinode *dip;
5107 
5108   if(ip == 0 || ip->ref < 1)
5109     panic("ilock");
5110 
5111   acquire(&icache.lock);
5112   while(ip->flags & I_BUSY)
5113     sleep(ip, &icache.lock);
5114   ip->flags |= I_BUSY;
5115   release(&icache.lock);
5116 
5117   if(!(ip->flags & I_VALID)){
5118     bp = bread(ip->dev, IBLOCK(ip->inum, sb));
5119     dip = (struct dinode*)bp->data + ip->inum%IPB;
5120     ip->type = dip->type;
5121     ip->major = dip->major;
5122     ip->minor = dip->minor;
5123     ip->nlink = dip->nlink;
5124     ip->size = dip->size;
5125     memmove(ip->addrs, dip->addrs, sizeof(ip->addrs));
5126     brelse(bp);
5127     ip->flags |= I_VALID;
5128     if(ip->type == 0)
5129       panic("ilock: no type");
5130   }
5131 }
5132 
5133 // Unlock the given inode.
5134 void
5135 iunlock(struct inode *ip)
5136 {
5137   if(ip == 0 || !(ip->flags & I_BUSY) || ip->ref < 1)
5138     panic("iunlock");
5139 
5140   acquire(&icache.lock);
5141   ip->flags &= ~I_BUSY;
5142   wakeup(ip);
5143   release(&icache.lock);
5144 }
5145 
5146 
5147 
5148 
5149 
5150 // Drop a reference to an in-memory inode.
5151 // If that was the last reference, the inode cache entry can
5152 // be recycled.
5153 // If that was the last reference and the inode has no links
5154 // to it, free the inode (and its content) on disk.
5155 // All calls to iput() must be inside a transaction in
5156 // case it has to free the inode.
5157 void
5158 iput(struct inode *ip)
5159 {
5160   acquire(&icache.lock);
5161   if(ip->ref == 1 && (ip->flags & I_VALID) && ip->nlink == 0){
5162     // inode has no links and no other references: truncate and free.
5163     if(ip->flags & I_BUSY)
5164       panic("iput busy");
5165     ip->flags |= I_BUSY;
5166     release(&icache.lock);
5167     itrunc(ip);
5168     ip->type = 0;
5169     iupdate(ip);
5170     acquire(&icache.lock);
5171     ip->flags = 0;
5172     wakeup(ip);
5173   }
5174   ip->ref--;
5175   release(&icache.lock);
5176 }
5177 
5178 // Common idiom: unlock, then put.
5179 void
5180 iunlockput(struct inode *ip)
5181 {
5182   iunlock(ip);
5183   iput(ip);
5184 }
5185 
5186 
5187 
5188 
5189 
5190 
5191 
5192 
5193 
5194 
5195 
5196 
5197 
5198 
5199 
5200 // Inode content
5201 //
5202 // The content (data) associated with each inode is stored
5203 // in blocks on the disk. The first NDIRECT block numbers
5204 // are listed in ip->addrs[].  The next NINDIRECT blocks are
5205 // listed in block ip->addrs[NDIRECT].
5206 
5207 // Return the disk block address of the nth block in inode ip.
5208 // If there is no such block, bmap allocates one.
5209 static uint
5210 bmap(struct inode *ip, uint bn)
5211 {
5212   uint addr, *a;
5213   struct buf *bp;
5214 
5215   if(bn < NDIRECT){
5216     if((addr = ip->addrs[bn]) == 0)
5217       ip->addrs[bn] = addr = balloc(ip->dev);
5218     return addr;
5219   }
5220   bn -= NDIRECT;
5221 
5222   if(bn < NINDIRECT){
5223     // Load indirect block, allocating if necessary.
5224     if((addr = ip->addrs[NDIRECT]) == 0)
5225       ip->addrs[NDIRECT] = addr = balloc(ip->dev);
5226     bp = bread(ip->dev, addr);
5227     a = (uint*)bp->data;
5228     if((addr = a[bn]) == 0){
5229       a[bn] = addr = balloc(ip->dev);
5230       log_write(bp);
5231     }
5232     brelse(bp);
5233     return addr;
5234   }
5235 
5236   panic("bmap: out of range");
5237 }
5238 
5239 
5240 
5241 
5242 
5243 
5244 
5245 
5246 
5247 
5248 
5249 
5250 // Truncate inode (discard contents).
5251 // Only called when the inode has no links
5252 // to it (no directory entries referring to it)
5253 // and has no in-memory reference to it (is
5254 // not an open file or current directory).
5255 static void
5256 itrunc(struct inode *ip)
5257 {
5258   int i, j;
5259   struct buf *bp;
5260   uint *a;
5261 
5262   for(i = 0; i < NDIRECT; i++){
5263     if(ip->addrs[i]){
5264       bfree(ip->dev, ip->addrs[i]);
5265       ip->addrs[i] = 0;
5266     }
5267   }
5268 
5269   if(ip->addrs[NDIRECT]){
5270     bp = bread(ip->dev, ip->addrs[NDIRECT]);
5271     a = (uint*)bp->data;
5272     for(j = 0; j < NINDIRECT; j++){
5273       if(a[j])
5274         bfree(ip->dev, a[j]);
5275     }
5276     brelse(bp);
5277     bfree(ip->dev, ip->addrs[NDIRECT]);
5278     ip->addrs[NDIRECT] = 0;
5279   }
5280 
5281   ip->size = 0;
5282   iupdate(ip);
5283 }
5284 
5285 // Copy stat information from inode.
5286 void
5287 stati(struct inode *ip, struct stat *st)
5288 {
5289   st->dev = ip->dev;
5290   st->ino = ip->inum;
5291   st->type = ip->type;
5292   st->nlink = ip->nlink;
5293   st->size = ip->size;
5294 }
5295 
5296 
5297 
5298 
5299 
5300 // Read data from inode.
5301 int
5302 readi(struct inode *ip, char *dst, uint off, uint n)
5303 {
5304   uint tot, m;
5305   struct buf *bp;
5306 
5307   if(ip->type == T_DEV){
5308     if(ip->major < 0 || ip->major >= NDEV || !devsw[ip->major].read)
5309       return -1;
5310     return devsw[ip->major].read(ip, dst, n);
5311   }
5312 
5313   if(off > ip->size || off + n < off)
5314     return -1;
5315   if(off + n > ip->size)
5316     n = ip->size - off;
5317 
5318   for(tot=0; tot<n; tot+=m, off+=m, dst+=m){
5319     bp = bread(ip->dev, bmap(ip, off/BSIZE));
5320     m = min(n - tot, BSIZE - off%BSIZE);
5321     memmove(dst, bp->data + off%BSIZE, m);
5322     brelse(bp);
5323   }
5324   return n;
5325 }
5326 
5327 
5328 
5329 
5330 
5331 
5332 
5333 
5334 
5335 
5336 
5337 
5338 
5339 
5340 
5341 
5342 
5343 
5344 
5345 
5346 
5347 
5348 
5349 
5350 // Write data to inode.
5351 int
5352 writei(struct inode *ip, char *src, uint off, uint n)
5353 {
5354   uint tot, m;
5355   struct buf *bp;
5356 
5357   if(ip->type == T_DEV){
5358     if(ip->major < 0 || ip->major >= NDEV || !devsw[ip->major].write)
5359       return -1;
5360     return devsw[ip->major].write(ip, src, n);
5361   }
5362 
5363   if(off > ip->size || off + n < off)
5364     return -1;
5365   if(off + n > MAXFILE*BSIZE)
5366     return -1;
5367 
5368   for(tot=0; tot<n; tot+=m, off+=m, src+=m){
5369     bp = bread(ip->dev, bmap(ip, off/BSIZE));
5370     m = min(n - tot, BSIZE - off%BSIZE);
5371     memmove(bp->data + off%BSIZE, src, m);
5372     log_write(bp);
5373     brelse(bp);
5374   }
5375 
5376   if(n > 0 && off > ip->size){
5377     ip->size = off;
5378     iupdate(ip);
5379   }
5380   return n;
5381 }
5382 
5383 
5384 
5385 
5386 
5387 
5388 
5389 
5390 
5391 
5392 
5393 
5394 
5395 
5396 
5397 
5398 
5399 
5400 // Directories
5401 
5402 int
5403 namecmp(const char *s, const char *t)
5404 {
5405   return strncmp(s, t, DIRSIZ);
5406 }
5407 
5408 // Look for a directory entry in a directory.
5409 // If found, set *poff to byte offset of entry.
5410 struct inode*
5411 dirlookup(struct inode *dp, char *name, uint *poff)
5412 {
5413   uint off, inum;
5414   struct dirent de;
5415 
5416   if(dp->type != T_DIR)
5417     panic("dirlookup not DIR");
5418 
5419   for(off = 0; off < dp->size; off += sizeof(de)){
5420     if(readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
5421       panic("dirlink read");
5422     if(de.inum == 0)
5423       continue;
5424     if(namecmp(name, de.name) == 0){
5425       // entry matches path element
5426       if(poff)
5427         *poff = off;
5428       inum = de.inum;
5429       return iget(dp->dev, inum);
5430     }
5431   }
5432 
5433   return 0;
5434 }
5435 
5436 
5437 
5438 
5439 
5440 
5441 
5442 
5443 
5444 
5445 
5446 
5447 
5448 
5449 
5450 // Write a new directory entry (name, inum) into the directory dp.
5451 int
5452 dirlink(struct inode *dp, char *name, uint inum)
5453 {
5454   int off;
5455   struct dirent de;
5456   struct inode *ip;
5457 
5458   // Check that name is not present.
5459   if((ip = dirlookup(dp, name, 0)) != 0){
5460     iput(ip);
5461     return -1;
5462   }
5463 
5464   // Look for an empty dirent.
5465   for(off = 0; off < dp->size; off += sizeof(de)){
5466     if(readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
5467       panic("dirlink read");
5468     if(de.inum == 0)
5469       break;
5470   }
5471 
5472   strncpy(de.name, name, DIRSIZ);
5473   de.inum = inum;
5474   if(writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
5475     panic("dirlink");
5476 
5477   return 0;
5478 }
5479 
5480 
5481 
5482 
5483 
5484 
5485 
5486 
5487 
5488 
5489 
5490 
5491 
5492 
5493 
5494 
5495 
5496 
5497 
5498 
5499 
5500 // Paths
5501 
5502 // Copy the next path element from path into name.
5503 // Return a pointer to the element following the copied one.
5504 // The returned path has no leading slashes,
5505 // so the caller can check *path=='\0' to see if the name is the last one.
5506 // If no name to remove, return 0.
5507 //
5508 // Examples:
5509 //   skipelem("a/bb/c", name) = "bb/c", setting name = "a"
5510 //   skipelem("///a//bb", name) = "bb", setting name = "a"
5511 //   skipelem("a", name) = "", setting name = "a"
5512 //   skipelem("", name) = skipelem("////", name) = 0
5513 //
5514 static char*
5515 skipelem(char *path, char *name)
5516 {
5517   char *s;
5518   int len;
5519 
5520   while(*path == '/')
5521     path++;
5522   if(*path == 0)
5523     return 0;
5524   s = path;
5525   while(*path != '/' && *path != 0)
5526     path++;
5527   len = path - s;
5528   if(len >= DIRSIZ)
5529     memmove(name, s, DIRSIZ);
5530   else {
5531     memmove(name, s, len);
5532     name[len] = 0;
5533   }
5534   while(*path == '/')
5535     path++;
5536   return path;
5537 }
5538 
5539 
5540 
5541 
5542 
5543 
5544 
5545 
5546 
5547 
5548 
5549 
5550 // Look up and return the inode for a path name.
5551 // If parent != 0, return the inode for the parent and copy the final
5552 // path element into name, which must have room for DIRSIZ bytes.
5553 // Must be called inside a transaction since it calls iput().
5554 static struct inode*
5555 namex(char *path, int nameiparent, char *name)
5556 {
5557   struct inode *ip, *next;
5558 
5559   if(*path == '/')
5560     ip = iget(ROOTDEV, ROOTINO);
5561   else
5562     ip = idup(proc->cwd);
5563 
5564   while((path = skipelem(path, name)) != 0){
5565     ilock(ip);
5566     if(ip->type != T_DIR){
5567       iunlockput(ip);
5568       return 0;
5569     }
5570     if(nameiparent && *path == '\0'){
5571       // Stop one level early.
5572       iunlock(ip);
5573       return ip;
5574     }
5575     if((next = dirlookup(ip, name, 0)) == 0){
5576       iunlockput(ip);
5577       return 0;
5578     }
5579     iunlockput(ip);
5580     ip = next;
5581   }
5582   if(nameiparent){
5583     iput(ip);
5584     return 0;
5585   }
5586   return ip;
5587 }
5588 
5589 struct inode*
5590 namei(char *path)
5591 {
5592   char name[DIRSIZ];
5593   return namex(path, 0, name);
5594 }
5595 
5596 
5597 
5598 
5599 
5600 struct inode*
5601 nameiparent(char *path, char *name)
5602 {
5603   return namex(path, 1, name);
5604 }
5605 
5606 
5607 
5608 
5609 
5610 
5611 
5612 
5613 
5614 
5615 
5616 
5617 
5618 
5619 
5620 
5621 
5622 
5623 
5624 
5625 
5626 
5627 
5628 
5629 
5630 
5631 
5632 
5633 
5634 
5635 
5636 
5637 
5638 
5639 
5640 
5641 
5642 
5643 
5644 
5645 
5646 
5647 
5648 
5649 
