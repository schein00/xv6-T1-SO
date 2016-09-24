3700 #include "types.h"
3701 #include "x86.h"
3702 #include "defs.h"
3703 #include "date.h"
3704 #include "param.h"
3705 #include "memlayout.h"
3706 #include "mmu.h"
3707 #include "proc.h"
3708 
3709 int
3710 sys_fork(void)
3711 {
3712   return fork();
3713 }
3714 
3715 int
3716 sys_exit(void)
3717 {
3718   exit();
3719   return 0;  // not reached
3720 }
3721 
3722 int
3723 sys_wait(void)
3724 {
3725   return wait();
3726 }
3727 
3728 int
3729 sys_kill(void)
3730 {
3731   int pid;
3732 
3733   if(argint(0, &pid) < 0)
3734     return -1;
3735   return kill(pid);
3736 }
3737 
3738 int
3739 sys_getpid(void)
3740 {
3741   return proc->pid;
3742 }
3743 
3744 
3745 
3746 
3747 
3748 
3749 
3750 int
3751 sys_sbrk(void)
3752 {
3753   int addr;
3754   int n;
3755 
3756   if(argint(0, &n) < 0)
3757     return -1;
3758   addr = proc->sz;
3759   if(growproc(n) < 0)
3760     return -1;
3761   return addr;
3762 }
3763 
3764 int
3765 sys_sleep(void)
3766 {
3767   int n;
3768   uint ticks0;
3769 
3770   if(argint(0, &n) < 0)
3771     return -1;
3772   acquire(&tickslock);
3773   ticks0 = ticks;
3774   while(ticks - ticks0 < n){
3775     if(proc->killed){
3776       release(&tickslock);
3777       return -1;
3778     }
3779     sleep(&ticks, &tickslock);
3780   }
3781   release(&tickslock);
3782   return 0;
3783 }
3784 
3785 // return how many clock tick interrupts have occurred
3786 // since start.
3787 int
3788 sys_uptime(void)
3789 {
3790   uint xticks;
3791 
3792   acquire(&tickslock);
3793   xticks = ticks;
3794   release(&tickslock);
3795   return xticks;
3796 }
3797 
3798 
3799 
