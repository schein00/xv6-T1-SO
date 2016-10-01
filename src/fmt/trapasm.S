3300 #include "mmu.h"
3301 
3302   # vectors.S sends all traps here.
3303 .globl alltraps
3304 alltraps:
3305   # Build trap frame.
3306   pushl %ds
3307   pushl %es
3308   pushl %fs
3309   pushl %gs
3310   pushal
3311 
3312   # Set up data and per-cpu segments.
3313   movw $(SEG_KDATA<<3), %ax
3314   movw %ax, %ds
3315   movw %ax, %es
3316   movw $(SEG_KCPU<<3), %ax
3317   movw %ax, %fs
3318   movw %ax, %gs
3319 
3320   # Call trap(tf), where tf=%esp
3321   pushl %esp
3322   call trap
3323   addl $4, %esp
3324 
3325   # Return falls through to trapret...
3326 .globl trapret
3327 trapret:
3328   popal
3329   popl %gs
3330   popl %fs
3331   popl %es
3332   popl %ds
3333   addl $0x8, %esp  # trapno and errcode
3334   iret
3335 
3336 
3337 
3338 
3339 
3340 
3341 
3342 
3343 
3344 
3345 
3346 
3347 
3348 
3349 
