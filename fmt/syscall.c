3550 #include "types.h"
3551 #include "defs.h"
3552 #include "param.h"
3553 #include "memlayout.h"
3554 #include "mmu.h"
3555 #include "proc.h"
3556 #include "x86.h"
3557 #include "syscall.h"
3558 
3559 // User code makes a system call with INT T_SYSCALL.
3560 // System call number in %eax.
3561 // Arguments on the stack, from the user call to the C
3562 // library system call function. The saved user %esp points
3563 // to a saved program counter, and then the first argument.
3564 
3565 // Fetch the int at addr from the current process.
3566 int
3567 fetchint(uint addr, int *ip)
3568 {
3569   if(addr >= proc->sz || addr+4 > proc->sz)
3570     return -1;
3571   *ip = *(int*)(addr);
3572   return 0;
3573 }
3574 
3575 // Fetch the nul-terminated string at addr from the current process.
3576 // Doesn't actually copy the string - just sets *pp to point at it.
3577 // Returns length of string, not including nul.
3578 int
3579 fetchstr(uint addr, char **pp)
3580 {
3581   char *s, *ep;
3582 
3583   if(addr >= proc->sz)
3584     return -1;
3585   *pp = (char*)addr;
3586   ep = (char*)proc->sz;
3587   for(s = *pp; s < ep; s++)
3588     if(*s == 0)
3589       return s - *pp;
3590   return -1;
3591 }
3592 
3593 // Fetch the nth 32-bit system call argument.
3594 int
3595 argint(int n, int *ip)
3596 {
3597   return fetchint(proc->tf->esp + 4 + 4*n, ip);
3598 }
3599 
3600 // Fetch the nth word-sized system call argument as a pointer
3601 // to a block of memory of size n bytes.  Check that the pointer
3602 // lies within the process address space.
3603 int
3604 argptr(int n, char **pp, int size)
3605 {
3606   int i;
3607 
3608   if(argint(n, &i) < 0)
3609     return -1;
3610   if((uint)i >= proc->sz || (uint)i+size > proc->sz)
3611     return -1;
3612   *pp = (char*)i;
3613   return 0;
3614 }
3615 
3616 // Fetch the nth word-sized system call argument as a string pointer.
3617 // Check that the pointer is valid and the string is nul-terminated.
3618 // (There is no shared writable memory, so the string can't change
3619 // between this check and being used by the kernel.)
3620 int
3621 argstr(int n, char **pp)
3622 {
3623   int addr;
3624   if(argint(n, &addr) < 0)
3625     return -1;
3626   return fetchstr(addr, pp);
3627 }
3628 
3629 extern int sys_chdir(void);
3630 extern int sys_close(void);
3631 extern int sys_dup(void);
3632 extern int sys_exec(void);
3633 extern int sys_exit(void);
3634 extern int sys_fork(void);
3635 extern int sys_fstat(void);
3636 extern int sys_getpid(void);
3637 extern int sys_kill(void);
3638 extern int sys_link(void);
3639 extern int sys_mkdir(void);
3640 extern int sys_mknod(void);
3641 extern int sys_open(void);
3642 extern int sys_pipe(void);
3643 extern int sys_read(void);
3644 extern int sys_sbrk(void);
3645 extern int sys_sleep(void);
3646 extern int sys_unlink(void);
3647 extern int sys_wait(void);
3648 extern int sys_write(void);
3649 extern int sys_uptime(void);
3650 static int (*syscalls[])(void) = {
3651 [SYS_fork]    sys_fork,
3652 [SYS_exit]    sys_exit,
3653 [SYS_wait]    sys_wait,
3654 [SYS_pipe]    sys_pipe,
3655 [SYS_read]    sys_read,
3656 [SYS_kill]    sys_kill,
3657 [SYS_exec]    sys_exec,
3658 [SYS_fstat]   sys_fstat,
3659 [SYS_chdir]   sys_chdir,
3660 [SYS_dup]     sys_dup,
3661 [SYS_getpid]  sys_getpid,
3662 [SYS_sbrk]    sys_sbrk,
3663 [SYS_sleep]   sys_sleep,
3664 [SYS_uptime]  sys_uptime,
3665 [SYS_open]    sys_open,
3666 [SYS_write]   sys_write,
3667 [SYS_mknod]   sys_mknod,
3668 [SYS_unlink]  sys_unlink,
3669 [SYS_link]    sys_link,
3670 [SYS_mkdir]   sys_mkdir,
3671 [SYS_close]   sys_close,
3672 };
3673 
3674 void
3675 syscall(void)
3676 {
3677   int num;
3678 
3679   num = proc->tf->eax;
3680   if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
3681     proc->tf->eax = syscalls[num]();
3682   } else {
3683     cprintf("%d %s: unknown sys call %d\n",
3684             proc->pid, proc->name, num);
3685     proc->tf->eax = -1;
3686   }
3687 }
3688 
3689 
3690 
3691 
3692 
3693 
3694 
3695 
3696 
3697 
3698 
3699 
