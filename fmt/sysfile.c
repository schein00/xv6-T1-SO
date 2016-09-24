5850 //
5851 // File-system system calls.
5852 // Mostly argument checking, since we don't trust
5853 // user code, and calls into file.c and fs.c.
5854 //
5855 
5856 #include "types.h"
5857 #include "defs.h"
5858 #include "param.h"
5859 #include "stat.h"
5860 #include "mmu.h"
5861 #include "proc.h"
5862 #include "fs.h"
5863 #include "file.h"
5864 #include "fcntl.h"
5865 
5866 // Fetch the nth word-sized system call argument as a file descriptor
5867 // and return both the descriptor and the corresponding struct file.
5868 static int
5869 argfd(int n, int *pfd, struct file **pf)
5870 {
5871   int fd;
5872   struct file *f;
5873 
5874   if(argint(n, &fd) < 0)
5875     return -1;
5876   if(fd < 0 || fd >= NOFILE || (f=proc->ofile[fd]) == 0)
5877     return -1;
5878   if(pfd)
5879     *pfd = fd;
5880   if(pf)
5881     *pf = f;
5882   return 0;
5883 }
5884 
5885 // Allocate a file descriptor for the given file.
5886 // Takes over file reference from caller on success.
5887 static int
5888 fdalloc(struct file *f)
5889 {
5890   int fd;
5891 
5892   for(fd = 0; fd < NOFILE; fd++){
5893     if(proc->ofile[fd] == 0){
5894       proc->ofile[fd] = f;
5895       return fd;
5896     }
5897   }
5898   return -1;
5899 }
5900 int
5901 sys_dup(void)
5902 {
5903   struct file *f;
5904   int fd;
5905 
5906   if(argfd(0, 0, &f) < 0)
5907     return -1;
5908   if((fd=fdalloc(f)) < 0)
5909     return -1;
5910   filedup(f);
5911   return fd;
5912 }
5913 
5914 int
5915 sys_read(void)
5916 {
5917   struct file *f;
5918   int n;
5919   char *p;
5920 
5921   if(argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0)
5922     return -1;
5923   return fileread(f, p, n);
5924 }
5925 
5926 int
5927 sys_write(void)
5928 {
5929   struct file *f;
5930   int n;
5931   char *p;
5932 
5933   if(argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0)
5934     return -1;
5935   return filewrite(f, p, n);
5936 }
5937 
5938 int
5939 sys_close(void)
5940 {
5941   int fd;
5942   struct file *f;
5943 
5944   if(argfd(0, &fd, &f) < 0)
5945     return -1;
5946   proc->ofile[fd] = 0;
5947   fileclose(f);
5948   return 0;
5949 }
5950 int
5951 sys_fstat(void)
5952 {
5953   struct file *f;
5954   struct stat *st;
5955 
5956   if(argfd(0, 0, &f) < 0 || argptr(1, (void*)&st, sizeof(*st)) < 0)
5957     return -1;
5958   return filestat(f, st);
5959 }
5960 
5961 // Create the path new as a link to the same inode as old.
5962 int
5963 sys_link(void)
5964 {
5965   char name[DIRSIZ], *new, *old;
5966   struct inode *dp, *ip;
5967 
5968   if(argstr(0, &old) < 0 || argstr(1, &new) < 0)
5969     return -1;
5970 
5971   begin_op();
5972   if((ip = namei(old)) == 0){
5973     end_op();
5974     return -1;
5975   }
5976 
5977   ilock(ip);
5978   if(ip->type == T_DIR){
5979     iunlockput(ip);
5980     end_op();
5981     return -1;
5982   }
5983 
5984   ip->nlink++;
5985   iupdate(ip);
5986   iunlock(ip);
5987 
5988   if((dp = nameiparent(new, name)) == 0)
5989     goto bad;
5990   ilock(dp);
5991   if(dp->dev != ip->dev || dirlink(dp, name, ip->inum) < 0){
5992     iunlockput(dp);
5993     goto bad;
5994   }
5995   iunlockput(dp);
5996   iput(ip);
5997 
5998   end_op();
5999 
6000   return 0;
6001 
6002 bad:
6003   ilock(ip);
6004   ip->nlink--;
6005   iupdate(ip);
6006   iunlockput(ip);
6007   end_op();
6008   return -1;
6009 }
6010 
6011 // Is the directory dp empty except for "." and ".." ?
6012 static int
6013 isdirempty(struct inode *dp)
6014 {
6015   int off;
6016   struct dirent de;
6017 
6018   for(off=2*sizeof(de); off<dp->size; off+=sizeof(de)){
6019     if(readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
6020       panic("isdirempty: readi");
6021     if(de.inum != 0)
6022       return 0;
6023   }
6024   return 1;
6025 }
6026 
6027 
6028 
6029 
6030 
6031 
6032 
6033 
6034 
6035 
6036 
6037 
6038 
6039 
6040 
6041 
6042 
6043 
6044 
6045 
6046 
6047 
6048 
6049 
6050 int
6051 sys_unlink(void)
6052 {
6053   struct inode *ip, *dp;
6054   struct dirent de;
6055   char name[DIRSIZ], *path;
6056   uint off;
6057 
6058   if(argstr(0, &path) < 0)
6059     return -1;
6060 
6061   begin_op();
6062   if((dp = nameiparent(path, name)) == 0){
6063     end_op();
6064     return -1;
6065   }
6066 
6067   ilock(dp);
6068 
6069   // Cannot unlink "." or "..".
6070   if(namecmp(name, ".") == 0 || namecmp(name, "..") == 0)
6071     goto bad;
6072 
6073   if((ip = dirlookup(dp, name, &off)) == 0)
6074     goto bad;
6075   ilock(ip);
6076 
6077   if(ip->nlink < 1)
6078     panic("unlink: nlink < 1");
6079   if(ip->type == T_DIR && !isdirempty(ip)){
6080     iunlockput(ip);
6081     goto bad;
6082   }
6083 
6084   memset(&de, 0, sizeof(de));
6085   if(writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
6086     panic("unlink: writei");
6087   if(ip->type == T_DIR){
6088     dp->nlink--;
6089     iupdate(dp);
6090   }
6091   iunlockput(dp);
6092 
6093   ip->nlink--;
6094   iupdate(ip);
6095   iunlockput(ip);
6096 
6097   end_op();
6098 
6099   return 0;
6100 bad:
6101   iunlockput(dp);
6102   end_op();
6103   return -1;
6104 }
6105 
6106 static struct inode*
6107 create(char *path, short type, short major, short minor)
6108 {
6109   uint off;
6110   struct inode *ip, *dp;
6111   char name[DIRSIZ];
6112 
6113   if((dp = nameiparent(path, name)) == 0)
6114     return 0;
6115   ilock(dp);
6116 
6117   if((ip = dirlookup(dp, name, &off)) != 0){
6118     iunlockput(dp);
6119     ilock(ip);
6120     if(type == T_FILE && ip->type == T_FILE)
6121       return ip;
6122     iunlockput(ip);
6123     return 0;
6124   }
6125 
6126   if((ip = ialloc(dp->dev, type)) == 0)
6127     panic("create: ialloc");
6128 
6129   ilock(ip);
6130   ip->major = major;
6131   ip->minor = minor;
6132   ip->nlink = 1;
6133   iupdate(ip);
6134 
6135   if(type == T_DIR){  // Create . and .. entries.
6136     dp->nlink++;  // for ".."
6137     iupdate(dp);
6138     // No ip->nlink++ for ".": avoid cyclic ref count.
6139     if(dirlink(ip, ".", ip->inum) < 0 || dirlink(ip, "..", dp->inum) < 0)
6140       panic("create dots");
6141   }
6142 
6143   if(dirlink(dp, name, ip->inum) < 0)
6144     panic("create: dirlink");
6145 
6146   iunlockput(dp);
6147 
6148   return ip;
6149 }
6150 int
6151 sys_open(void)
6152 {
6153   char *path;
6154   int fd, omode;
6155   struct file *f;
6156   struct inode *ip;
6157 
6158   if(argstr(0, &path) < 0 || argint(1, &omode) < 0)
6159     return -1;
6160 
6161   begin_op();
6162 
6163   if(omode & O_CREATE){
6164     ip = create(path, T_FILE, 0, 0);
6165     if(ip == 0){
6166       end_op();
6167       return -1;
6168     }
6169   } else {
6170     if((ip = namei(path)) == 0){
6171       end_op();
6172       return -1;
6173     }
6174     ilock(ip);
6175     if(ip->type == T_DIR && omode != O_RDONLY){
6176       iunlockput(ip);
6177       end_op();
6178       return -1;
6179     }
6180   }
6181 
6182   if((f = filealloc()) == 0 || (fd = fdalloc(f)) < 0){
6183     if(f)
6184       fileclose(f);
6185     iunlockput(ip);
6186     end_op();
6187     return -1;
6188   }
6189   iunlock(ip);
6190   end_op();
6191 
6192   f->type = FD_INODE;
6193   f->ip = ip;
6194   f->off = 0;
6195   f->readable = !(omode & O_WRONLY);
6196   f->writable = (omode & O_WRONLY) || (omode & O_RDWR);
6197   return fd;
6198 }
6199 
6200 int
6201 sys_mkdir(void)
6202 {
6203   char *path;
6204   struct inode *ip;
6205 
6206   begin_op();
6207   if(argstr(0, &path) < 0 || (ip = create(path, T_DIR, 0, 0)) == 0){
6208     end_op();
6209     return -1;
6210   }
6211   iunlockput(ip);
6212   end_op();
6213   return 0;
6214 }
6215 
6216 int
6217 sys_mknod(void)
6218 {
6219   struct inode *ip;
6220   char *path;
6221   int major, minor;
6222 
6223   begin_op();
6224   if((argstr(0, &path)) < 0 ||
6225      argint(1, &major) < 0 ||
6226      argint(2, &minor) < 0 ||
6227      (ip = create(path, T_DEV, major, minor)) == 0){
6228     end_op();
6229     return -1;
6230   }
6231   iunlockput(ip);
6232   end_op();
6233   return 0;
6234 }
6235 
6236 
6237 
6238 
6239 
6240 
6241 
6242 
6243 
6244 
6245 
6246 
6247 
6248 
6249 
6250 int
6251 sys_chdir(void)
6252 {
6253   char *path;
6254   struct inode *ip;
6255 
6256   begin_op();
6257   if(argstr(0, &path) < 0 || (ip = namei(path)) == 0){
6258     end_op();
6259     return -1;
6260   }
6261   ilock(ip);
6262   if(ip->type != T_DIR){
6263     iunlockput(ip);
6264     end_op();
6265     return -1;
6266   }
6267   iunlock(ip);
6268   iput(proc->cwd);
6269   end_op();
6270   proc->cwd = ip;
6271   return 0;
6272 }
6273 
6274 int
6275 sys_exec(void)
6276 {
6277   char *path, *argv[MAXARG];
6278   int i;
6279   uint uargv, uarg;
6280 
6281   if(argstr(0, &path) < 0 || argint(1, (int*)&uargv) < 0){
6282     return -1;
6283   }
6284   memset(argv, 0, sizeof(argv));
6285   for(i=0;; i++){
6286     if(i >= NELEM(argv))
6287       return -1;
6288     if(fetchint(uargv+4*i, (int*)&uarg) < 0)
6289       return -1;
6290     if(uarg == 0){
6291       argv[i] = 0;
6292       break;
6293     }
6294     if(fetchstr(uarg, &argv[i]) < 0)
6295       return -1;
6296   }
6297   return exec(path, argv);
6298 }
6299 
6300 int
6301 sys_pipe(void)
6302 {
6303   int *fd;
6304   struct file *rf, *wf;
6305   int fd0, fd1;
6306 
6307   if(argptr(0, (void*)&fd, 2*sizeof(fd[0])) < 0)
6308     return -1;
6309   if(pipealloc(&rf, &wf) < 0)
6310     return -1;
6311   fd0 = -1;
6312   if((fd0 = fdalloc(rf)) < 0 || (fd1 = fdalloc(wf)) < 0){
6313     if(fd0 >= 0)
6314       proc->ofile[fd0] = 0;
6315     fileclose(rf);
6316     fileclose(wf);
6317     return -1;
6318   }
6319   fd[0] = fd0;
6320   fd[1] = fd1;
6321   return 0;
6322 }
6323 
6324 
6325 
6326 
6327 
6328 
6329 
6330 
6331 
6332 
6333 
6334 
6335 
6336 
6337 
6338 
6339 
6340 
6341 
6342 
6343 
6344 
6345 
6346 
6347 
6348 
6349 
