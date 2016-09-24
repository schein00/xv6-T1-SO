1100 # The xv6 kernel starts executing in this file. This file is linked with
1101 # the kernel C code, so it can refer to kernel symbols such as main().
1102 # The boot block (bootasm.S and bootmain.c) jumps to entry below.
1103 
1104 # Multiboot header, for multiboot boot loaders like GNU Grub.
1105 # http://www.gnu.org/software/grub/manual/multiboot/multiboot.html
1106 #
1107 # Using GRUB 2, you can boot xv6 from a file stored in a
1108 # Linux file system by copying kernel or kernelmemfs to /boot
1109 # and then adding this menu entry:
1110 #
1111 # menuentry "xv6" {
1112 # 	insmod ext2
1113 # 	set root='(hd0,msdos1)'
1114 # 	set kernel='/boot/kernel'
1115 # 	echo "Loading ${kernel}..."
1116 # 	multiboot ${kernel} ${kernel}
1117 # 	boot
1118 # }
1119 
1120 #include "asm.h"
1121 #include "memlayout.h"
1122 #include "mmu.h"
1123 #include "param.h"
1124 
1125 # Multiboot header.  Data to direct multiboot loader.
1126 .p2align 2
1127 .text
1128 .globl multiboot_header
1129 multiboot_header:
1130   #define magic 0x1badb002
1131   #define flags 0
1132   .long magic
1133   .long flags
1134   .long (-magic-flags)
1135 
1136 # By convention, the _start symbol specifies the ELF entry point.
1137 # Since we haven't set up virtual memory yet, our entry point is
1138 # the physical address of 'entry'.
1139 .globl _start
1140 _start = V2P_WO(entry)
1141 
1142 # Entering xv6 on boot processor, with paging off.
1143 .globl entry
1144 entry:
1145   # Turn on page size extension for 4Mbyte pages
1146   movl    %cr4, %eax
1147   orl     $(CR4_PSE), %eax
1148   movl    %eax, %cr4
1149   # Set page directory
1150   movl    $(V2P_WO(entrypgdir)), %eax
1151   movl    %eax, %cr3
1152   # Turn on paging.
1153   movl    %cr0, %eax
1154   orl     $(CR0_PG|CR0_WP), %eax
1155   movl    %eax, %cr0
1156 
1157   # Set up the stack pointer.
1158   movl $(stack + KSTACKSIZE), %esp
1159 
1160   # Jump to main(), and switch to executing at
1161   # high addresses. The indirect call is needed because
1162   # the assembler produces a PC-relative instruction
1163   # for a direct jump.
1164   mov $main, %eax
1165   jmp *%eax
1166 
1167 .comm stack, KSTACKSIZE
1168 
1169 
1170 
1171 
1172 
1173 
1174 
1175 
1176 
1177 
1178 
1179 
1180 
1181 
1182 
1183 
1184 
1185 
1186 
1187 
1188 
1189 
1190 
1191 
1192 
1193 
1194 
1195 
1196 
1197 
1198 
1199 
