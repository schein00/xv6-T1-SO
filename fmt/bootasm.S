9150 #include "asm.h"
9151 #include "memlayout.h"
9152 #include "mmu.h"
9153 
9154 # Start the first CPU: switch to 32-bit protected mode, jump into C.
9155 # The BIOS loads this code from the first sector of the hard disk into
9156 # memory at physical address 0x7c00 and starts executing in real mode
9157 # with %cs=0 %ip=7c00.
9158 
9159 .code16                       # Assemble for 16-bit mode
9160 .globl start
9161 start:
9162   cli                         # BIOS enabled interrupts; disable
9163 
9164   # Zero data segment registers DS, ES, and SS.
9165   xorw    %ax,%ax             # Set %ax to zero
9166   movw    %ax,%ds             # -> Data Segment
9167   movw    %ax,%es             # -> Extra Segment
9168   movw    %ax,%ss             # -> Stack Segment
9169 
9170   # Physical address line A20 is tied to zero so that the first PCs
9171   # with 2 MB would run software that assumed 1 MB.  Undo that.
9172 seta20.1:
9173   inb     $0x64,%al               # Wait for not busy
9174   testb   $0x2,%al
9175   jnz     seta20.1
9176 
9177   movb    $0xd1,%al               # 0xd1 -> port 0x64
9178   outb    %al,$0x64
9179 
9180 seta20.2:
9181   inb     $0x64,%al               # Wait for not busy
9182   testb   $0x2,%al
9183   jnz     seta20.2
9184 
9185   movb    $0xdf,%al               # 0xdf -> port 0x60
9186   outb    %al,$0x60
9187 
9188   # Switch from real to protected mode.  Use a bootstrap GDT that makes
9189   # virtual addresses map directly to physical addresses so that the
9190   # effective memory map doesn't change during the transition.
9191   lgdt    gdtdesc
9192   movl    %cr0, %eax
9193   orl     $CR0_PE, %eax
9194   movl    %eax, %cr0
9195 
9196 
9197 
9198 
9199 
9200   # Complete the transition to 32-bit protected mode by using a long jmp
9201   # to reload %cs and %eip.  The segment descriptors are set up with no
9202   # translation, so that the mapping is still the identity mapping.
9203   ljmp    $(SEG_KCODE<<3), $start32
9204 
9205 .code32  # Tell assembler to generate 32-bit code now.
9206 start32:
9207   # Set up the protected-mode data segment registers
9208   movw    $(SEG_KDATA<<3), %ax    # Our data segment selector
9209   movw    %ax, %ds                # -> DS: Data Segment
9210   movw    %ax, %es                # -> ES: Extra Segment
9211   movw    %ax, %ss                # -> SS: Stack Segment
9212   movw    $0, %ax                 # Zero segments not ready for use
9213   movw    %ax, %fs                # -> FS
9214   movw    %ax, %gs                # -> GS
9215 
9216   # Set up the stack pointer and call into C.
9217   movl    $start, %esp
9218   call    bootmain
9219 
9220   # If bootmain returns (it shouldn't), trigger a Bochs
9221   # breakpoint if running under Bochs, then loop.
9222   movw    $0x8a00, %ax            # 0x8a00 -> port 0x8a00
9223   movw    %ax, %dx
9224   outw    %ax, %dx
9225   movw    $0x8ae0, %ax            # 0x8ae0 -> port 0x8a00
9226   outw    %ax, %dx
9227 spin:
9228   jmp     spin
9229 
9230 # Bootstrap GDT
9231 .p2align 2                                # force 4 byte alignment
9232 gdt:
9233   SEG_NULLASM                             # null seg
9234   SEG_ASM(STA_X|STA_R, 0x0, 0xffffffff)   # code seg
9235   SEG_ASM(STA_W, 0x0, 0xffffffff)         # data seg
9236 
9237 gdtdesc:
9238   .word   (gdtdesc - gdt - 1)             # sizeof(gdt) - 1
9239   .long   gdt                             # address gdt
9240 
9241 
9242 
9243 
9244 
9245 
9246 
9247 
9248 
9249 
