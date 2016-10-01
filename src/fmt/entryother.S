1200 #include "asm.h"
1201 #include "memlayout.h"
1202 #include "mmu.h"
1203 
1204 # Each non-boot CPU ("AP") is started up in response to a STARTUP
1205 # IPI from the boot CPU.  Section B.4.2 of the Multi-Processor
1206 # Specification says that the AP will start in real mode with CS:IP
1207 # set to XY00:0000, where XY is an 8-bit value sent with the
1208 # STARTUP. Thus this code must start at a 4096-byte boundary.
1209 #
1210 # Because this code sets DS to zero, it must sit
1211 # at an address in the low 2^16 bytes.
1212 #
1213 # Startothers (in main.c) sends the STARTUPs one at a time.
1214 # It copies this code (start) at 0x7000.  It puts the address of
1215 # a newly allocated per-core stack in start-4,the address of the
1216 # place to jump to (mpenter) in start-8, and the physical address
1217 # of entrypgdir in start-12.
1218 #
1219 # This code combines elements of bootasm.S and entry.S.
1220 
1221 .code16
1222 .globl start
1223 start:
1224   cli
1225 
1226   # Zero data segment registers DS, ES, and SS.
1227   xorw    %ax,%ax
1228   movw    %ax,%ds
1229   movw    %ax,%es
1230   movw    %ax,%ss
1231 
1232   # Switch from real to protected mode.  Use a bootstrap GDT that makes
1233   # virtual addresses map directly to physical addresses so that the
1234   # effective memory map doesn't change during the transition.
1235   lgdt    gdtdesc
1236   movl    %cr0, %eax
1237   orl     $CR0_PE, %eax
1238   movl    %eax, %cr0
1239 
1240   # Complete the transition to 32-bit protected mode by using a long jmp
1241   # to reload %cs and %eip.  The segment descriptors are set up with no
1242   # translation, so that the mapping is still the identity mapping.
1243   ljmpl    $(SEG_KCODE<<3), $(start32)
1244 
1245 
1246 
1247 
1248 
1249 
1250 .code32  # Tell assembler to generate 32-bit code now.
1251 start32:
1252   # Set up the protected-mode data segment registers
1253   movw    $(SEG_KDATA<<3), %ax    # Our data segment selector
1254   movw    %ax, %ds                # -> DS: Data Segment
1255   movw    %ax, %es                # -> ES: Extra Segment
1256   movw    %ax, %ss                # -> SS: Stack Segment
1257   movw    $0, %ax                 # Zero segments not ready for use
1258   movw    %ax, %fs                # -> FS
1259   movw    %ax, %gs                # -> GS
1260 
1261   # Turn on page size extension for 4Mbyte pages
1262   movl    %cr4, %eax
1263   orl     $(CR4_PSE), %eax
1264   movl    %eax, %cr4
1265   # Use entrypgdir as our initial page table
1266   movl    (start-12), %eax
1267   movl    %eax, %cr3
1268   # Turn on paging.
1269   movl    %cr0, %eax
1270   orl     $(CR0_PE|CR0_PG|CR0_WP), %eax
1271   movl    %eax, %cr0
1272 
1273   # Switch to the stack allocated by startothers()
1274   movl    (start-4), %esp
1275   # Call mpenter()
1276   call	 *(start-8)
1277 
1278   movw    $0x8a00, %ax
1279   movw    %ax, %dx
1280   outw    %ax, %dx
1281   movw    $0x8ae0, %ax
1282   outw    %ax, %dx
1283 spin:
1284   jmp     spin
1285 
1286 .p2align 2
1287 gdt:
1288   SEG_NULLASM
1289   SEG_ASM(STA_X|STA_R, 0, 0xffffffff)
1290   SEG_ASM(STA_W, 0, 0xffffffff)
1291 
1292 
1293 gdtdesc:
1294   .word   (gdtdesc - gdt - 1)
1295   .long   gdt
1296 
1297 
1298 
1299 
