9250 // Boot loader.
9251 //
9252 // Part of the boot block, along with bootasm.S, which calls bootmain().
9253 // bootasm.S has put the processor into protected 32-bit mode.
9254 // bootmain() loads an ELF kernel image from the disk starting at
9255 // sector 1 and then jumps to the kernel entry routine.
9256 
9257 #include "types.h"
9258 #include "elf.h"
9259 #include "x86.h"
9260 #include "memlayout.h"
9261 
9262 #define SECTSIZE  512
9263 
9264 void readseg(uchar*, uint, uint);
9265 
9266 void
9267 bootmain(void)
9268 {
9269   struct elfhdr *elf;
9270   struct proghdr *ph, *eph;
9271   void (*entry)(void);
9272   uchar* pa;
9273 
9274   elf = (struct elfhdr*)0x10000;  // scratch space
9275 
9276   // Read 1st page off disk
9277   readseg((uchar*)elf, 4096, 0);
9278 
9279   // Is this an ELF executable?
9280   if(elf->magic != ELF_MAGIC)
9281     return;  // let bootasm.S handle error
9282 
9283   // Load each program segment (ignores ph flags).
9284   ph = (struct proghdr*)((uchar*)elf + elf->phoff);
9285   eph = ph + elf->phnum;
9286   for(; ph < eph; ph++){
9287     pa = (uchar*)ph->paddr;
9288     readseg(pa, ph->filesz, ph->off);
9289     if(ph->memsz > ph->filesz)
9290       stosb(pa + ph->filesz, 0, ph->memsz - ph->filesz);
9291   }
9292 
9293   // Call the entry point from the ELF header.
9294   // Does not return!
9295   entry = (void(*)(void))(elf->entry);
9296   entry();
9297 }
9298 
9299 
9300 void
9301 waitdisk(void)
9302 {
9303   // Wait for disk ready.
9304   while((inb(0x1F7) & 0xC0) != 0x40)
9305     ;
9306 }
9307 
9308 // Read a single sector at offset into dst.
9309 void
9310 readsect(void *dst, uint offset)
9311 {
9312   // Issue command.
9313   waitdisk();
9314   outb(0x1F2, 1);   // count = 1
9315   outb(0x1F3, offset);
9316   outb(0x1F4, offset >> 8);
9317   outb(0x1F5, offset >> 16);
9318   outb(0x1F6, (offset >> 24) | 0xE0);
9319   outb(0x1F7, 0x20);  // cmd 0x20 - read sectors
9320 
9321   // Read data.
9322   waitdisk();
9323   insl(0x1F0, dst, SECTSIZE/4);
9324 }
9325 
9326 // Read 'count' bytes at 'offset' from kernel into physical address 'pa'.
9327 // Might copy more than asked.
9328 void
9329 readseg(uchar* pa, uint count, uint offset)
9330 {
9331   uchar* epa;
9332 
9333   epa = pa + count;
9334 
9335   // Round down to sector boundary.
9336   pa -= offset % SECTSIZE;
9337 
9338   // Translate from bytes to sectors; kernel starts at sector 1.
9339   offset = (offset / SECTSIZE) + 1;
9340 
9341   // If this is too slow, we could read lots of sectors at a time.
9342   // We'd write more to memory than asked, but it doesn't matter --
9343   // we load in increasing order.
9344   for(; pa < epa; pa += SECTSIZE, offset++)
9345     readsect(pa, offset);
9346 }
9347 
9348 
9349 
