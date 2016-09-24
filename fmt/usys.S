8500 #include "syscall.h"
8501 #include "traps.h"
8502 
8503 #define SYSCALL(name) \
8504   .globl name; \
8505   name: \
8506     movl $SYS_ ## name, %eax; \
8507     int $T_SYSCALL; \
8508     ret
8509 
8510 SYSCALL(fork)
8511 SYSCALL(exit)
8512 SYSCALL(wait)
8513 SYSCALL(pipe)
8514 SYSCALL(read)
8515 SYSCALL(write)
8516 SYSCALL(close)
8517 SYSCALL(kill)
8518 SYSCALL(exec)
8519 SYSCALL(open)
8520 SYSCALL(mknod)
8521 SYSCALL(unlink)
8522 SYSCALL(fstat)
8523 SYSCALL(link)
8524 SYSCALL(mkdir)
8525 SYSCALL(chdir)
8526 SYSCALL(dup)
8527 SYSCALL(getpid)
8528 SYSCALL(sbrk)
8529 SYSCALL(sleep)
8530 SYSCALL(uptime)
8531 
8532 
8533 
8534 
8535 
8536 
8537 
8538 
8539 
8540 
8541 
8542 
8543 
8544 
8545 
8546 
8547 
8548 
8549 
