4050 struct file {
4051   enum { FD_NONE, FD_PIPE, FD_INODE } type;
4052   int ref; // reference count
4053   char readable;
4054   char writable;
4055   struct pipe *pipe;
4056   struct inode *ip;
4057   uint off;
4058 };
4059 
4060 
4061 // in-memory copy of an inode
4062 struct inode {
4063   uint dev;           // Device number
4064   uint inum;          // Inode number
4065   int ref;            // Reference count
4066   int flags;          // I_BUSY, I_VALID
4067 
4068   short type;         // copy of disk inode
4069   short major;
4070   short minor;
4071   short nlink;
4072   uint size;
4073   uint addrs[NDIRECT+1];
4074 };
4075 #define I_BUSY 0x1
4076 #define I_VALID 0x2
4077 
4078 // table mapping major device number to
4079 // device functions
4080 struct devsw {
4081   int (*read)(struct inode*, char*, int);
4082   int (*write)(struct inode*, char*, int);
4083 };
4084 
4085 extern struct devsw devsw[];
4086 
4087 #define CONSOLE 1
4088 
4089 
4090 
4091 
4092 
4093 
4094 
4095 
4096 
4097 
4098 
4099 
4100 // Blank page.
4101 
4102 
4103 
4104 
4105 
4106 
4107 
4108 
4109 
4110 
4111 
4112 
4113 
4114 
4115 
4116 
4117 
4118 
4119 
4120 
4121 
4122 
4123 
4124 
4125 
4126 
4127 
4128 
4129 
4130 
4131 
4132 
4133 
4134 
4135 
4136 
4137 
4138 
4139 
4140 
4141 
4142 
4143 
4144 
4145 
4146 
4147 
4148 
4149 
