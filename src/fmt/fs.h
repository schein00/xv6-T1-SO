3950 // On-disk file system format.
3951 // Both the kernel and user programs use this header file.
3952 
3953 
3954 #define ROOTINO 1  // root i-number
3955 #define BSIZE 512  // block size
3956 
3957 // Disk layout:
3958 // [ boot block | super block | log | inode blocks |
3959 //                                          free bit map | data blocks]
3960 //
3961 // mkfs computes the super block and builds an initial file system. The
3962 // super block describes the disk layout:
3963 struct superblock {
3964   uint size;         // Size of file system image (blocks)
3965   uint nblocks;      // Number of data blocks
3966   uint ninodes;      // Number of inodes.
3967   uint nlog;         // Number of log blocks
3968   uint logstart;     // Block number of first log block
3969   uint inodestart;   // Block number of first inode block
3970   uint bmapstart;    // Block number of first free map block
3971 };
3972 
3973 #define NDIRECT 12
3974 #define NINDIRECT (BSIZE / sizeof(uint))
3975 #define MAXFILE (NDIRECT + NINDIRECT)
3976 
3977 // On-disk inode structure
3978 struct dinode {
3979   short type;           // File type
3980   short major;          // Major device number (T_DEV only)
3981   short minor;          // Minor device number (T_DEV only)
3982   short nlink;          // Number of links to inode in file system
3983   uint size;            // Size of file (bytes)
3984   uint addrs[NDIRECT+1];   // Data block addresses
3985 };
3986 
3987 
3988 
3989 
3990 
3991 
3992 
3993 
3994 
3995 
3996 
3997 
3998 
3999 
4000 // Inodes per block.
4001 #define IPB           (BSIZE / sizeof(struct dinode))
4002 
4003 // Block containing inode i
4004 #define IBLOCK(i, sb)     ((i) / IPB + sb.inodestart)
4005 
4006 // Bitmap bits per block
4007 #define BPB           (BSIZE*8)
4008 
4009 // Block of free map containing bit for block b
4010 #define BBLOCK(b, sb) (b/BPB + sb.bmapstart)
4011 
4012 // Directory is a file containing a sequence of dirent structures.
4013 #define DIRSIZ 14
4014 
4015 struct dirent {
4016   ushort inum;
4017   char name[DIRSIZ];
4018 };
4019 
4020 
4021 
4022 
4023 
4024 
4025 
4026 
4027 
4028 
4029 
4030 
4031 
4032 
4033 
4034 
4035 
4036 
4037 
4038 
4039 
4040 
4041 
4042 
4043 
4044 
4045 
4046 
4047 
4048 
4049 
