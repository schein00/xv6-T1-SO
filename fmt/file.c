5650 //
5651 // File descriptors
5652 //
5653 
5654 #include "types.h"
5655 #include "defs.h"
5656 #include "param.h"
5657 #include "fs.h"
5658 #include "file.h"
5659 #include "spinlock.h"
5660 
5661 struct devsw devsw[NDEV];
5662 struct {
5663   struct spinlock lock;
5664   struct file file[NFILE];
5665 } ftable;
5666 
5667 void
5668 fileinit(void)
5669 {
5670   initlock(&ftable.lock, "ftable");
5671 }
5672 
5673 // Allocate a file structure.
5674 struct file*
5675 filealloc(void)
5676 {
5677   struct file *f;
5678 
5679   acquire(&ftable.lock);
5680   for(f = ftable.file; f < ftable.file + NFILE; f++){
5681     if(f->ref == 0){
5682       f->ref = 1;
5683       release(&ftable.lock);
5684       return f;
5685     }
5686   }
5687   release(&ftable.lock);
5688   return 0;
5689 }
5690 
5691 
5692 
5693 
5694 
5695 
5696 
5697 
5698 
5699 
5700 // Increment ref count for file f.
5701 struct file*
5702 filedup(struct file *f)
5703 {
5704   acquire(&ftable.lock);
5705   if(f->ref < 1)
5706     panic("filedup");
5707   f->ref++;
5708   release(&ftable.lock);
5709   return f;
5710 }
5711 
5712 // Close file f.  (Decrement ref count, close when reaches 0.)
5713 void
5714 fileclose(struct file *f)
5715 {
5716   struct file ff;
5717 
5718   acquire(&ftable.lock);
5719   if(f->ref < 1)
5720     panic("fileclose");
5721   if(--f->ref > 0){
5722     release(&ftable.lock);
5723     return;
5724   }
5725   ff = *f;
5726   f->ref = 0;
5727   f->type = FD_NONE;
5728   release(&ftable.lock);
5729 
5730   if(ff.type == FD_PIPE)
5731     pipeclose(ff.pipe, ff.writable);
5732   else if(ff.type == FD_INODE){
5733     begin_op();
5734     iput(ff.ip);
5735     end_op();
5736   }
5737 }
5738 
5739 
5740 
5741 
5742 
5743 
5744 
5745 
5746 
5747 
5748 
5749 
5750 // Get metadata about file f.
5751 int
5752 filestat(struct file *f, struct stat *st)
5753 {
5754   if(f->type == FD_INODE){
5755     ilock(f->ip);
5756     stati(f->ip, st);
5757     iunlock(f->ip);
5758     return 0;
5759   }
5760   return -1;
5761 }
5762 
5763 // Read from file f.
5764 int
5765 fileread(struct file *f, char *addr, int n)
5766 {
5767   int r;
5768 
5769   if(f->readable == 0)
5770     return -1;
5771   if(f->type == FD_PIPE)
5772     return piperead(f->pipe, addr, n);
5773   if(f->type == FD_INODE){
5774     ilock(f->ip);
5775     if((r = readi(f->ip, addr, f->off, n)) > 0)
5776       f->off += r;
5777     iunlock(f->ip);
5778     return r;
5779   }
5780   panic("fileread");
5781 }
5782 
5783 
5784 
5785 
5786 
5787 
5788 
5789 
5790 
5791 
5792 
5793 
5794 
5795 
5796 
5797 
5798 
5799 
5800 // Write to file f.
5801 int
5802 filewrite(struct file *f, char *addr, int n)
5803 {
5804   int r;
5805 
5806   if(f->writable == 0)
5807     return -1;
5808   if(f->type == FD_PIPE)
5809     return pipewrite(f->pipe, addr, n);
5810   if(f->type == FD_INODE){
5811     // write a few blocks at a time to avoid exceeding
5812     // the maximum log transaction size, including
5813     // i-node, indirect block, allocation blocks,
5814     // and 2 blocks of slop for non-aligned writes.
5815     // this really belongs lower down, since writei()
5816     // might be writing a device like the console.
5817     int max = ((LOGSIZE-1-1-2) / 2) * 512;
5818     int i = 0;
5819     while(i < n){
5820       int n1 = n - i;
5821       if(n1 > max)
5822         n1 = max;
5823 
5824       begin_op();
5825       ilock(f->ip);
5826       if ((r = writei(f->ip, addr + i, f->off, n1)) > 0)
5827         f->off += r;
5828       iunlock(f->ip);
5829       end_op();
5830 
5831       if(r < 0)
5832         break;
5833       if(r != n1)
5834         panic("short filewrite");
5835       i += r;
5836     }
5837     return i == n ? n : -1;
5838   }
5839   panic("filewrite");
5840 }
5841 
5842 
5843 
5844 
5845 
5846 
5847 
5848 
5849 
