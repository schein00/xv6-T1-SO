3800 struct buf {
3801   int flags;
3802   uint dev;
3803   uint blockno;
3804   struct buf *prev; // LRU cache list
3805   struct buf *next;
3806   struct buf *qnext; // disk queue
3807   uchar data[BSIZE];
3808 };
3809 #define B_BUSY  0x1  // buffer is locked by some process
3810 #define B_VALID 0x2  // buffer has been read from disk
3811 #define B_DIRTY 0x4  // buffer needs to be written to disk
3812 
3813 
3814 
3815 
3816 
3817 
3818 
3819 
3820 
3821 
3822 
3823 
3824 
3825 
3826 
3827 
3828 
3829 
3830 
3831 
3832 
3833 
3834 
3835 
3836 
3837 
3838 
3839 
3840 
3841 
3842 
3843 
3844 
3845 
3846 
3847 
3848 
3849 
