8450 # Initial process execs /init.
8451 # This code runs in user space.
8452 
8453 #include "syscall.h"
8454 #include "traps.h"
8455 
8456 
8457 # exec(init, argv)
8458 .globl start
8459 start:
8460   pushl $argv
8461   pushl $init
8462   pushl $0  // where caller pc would be
8463   movl $SYS_exec, %eax
8464   int $T_SYSCALL
8465 
8466 # for(;;) exit();
8467 exit:
8468   movl $SYS_exit, %eax
8469   int $T_SYSCALL
8470   jmp exit
8471 
8472 # char init[] = "/init\0";
8473 init:
8474   .string "/init\0"
8475 
8476 # char *argv[] = { init, 0 };
8477 .p2align 2
8478 argv:
8479   .long init
8480   .long 0
8481 
8482 
8483 
8484 
8485 
8486 
8487 
8488 
8489 
8490 
8491 
8492 
8493 
8494 
8495 
8496 
8497 
8498 
8499 
