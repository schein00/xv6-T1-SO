7600 // Intel 8259A programmable interrupt controllers.
7601 
7602 #include "types.h"
7603 #include "x86.h"
7604 #include "traps.h"
7605 
7606 // I/O Addresses of the two programmable interrupt controllers
7607 #define IO_PIC1         0x20    // Master (IRQs 0-7)
7608 #define IO_PIC2         0xA0    // Slave (IRQs 8-15)
7609 
7610 #define IRQ_SLAVE       2       // IRQ at which slave connects to master
7611 
7612 // Current IRQ mask.
7613 // Initial IRQ mask has interrupt 2 enabled (for slave 8259A).
7614 static ushort irqmask = 0xFFFF & ~(1<<IRQ_SLAVE);
7615 
7616 static void
7617 picsetmask(ushort mask)
7618 {
7619   irqmask = mask;
7620   outb(IO_PIC1+1, mask);
7621   outb(IO_PIC2+1, mask >> 8);
7622 }
7623 
7624 void
7625 picenable(int irq)
7626 {
7627   picsetmask(irqmask & ~(1<<irq));
7628 }
7629 
7630 // Initialize the 8259A interrupt controllers.
7631 void
7632 picinit(void)
7633 {
7634   // mask all interrupts
7635   outb(IO_PIC1+1, 0xFF);
7636   outb(IO_PIC2+1, 0xFF);
7637 
7638   // Set up master (8259A-1)
7639 
7640   // ICW1:  0001g0hi
7641   //    g:  0 = edge triggering, 1 = level triggering
7642   //    h:  0 = cascaded PICs, 1 = master only
7643   //    i:  0 = no ICW4, 1 = ICW4 required
7644   outb(IO_PIC1, 0x11);
7645 
7646   // ICW2:  Vector offset
7647   outb(IO_PIC1+1, T_IRQ0);
7648 
7649 
7650   // ICW3:  (master PIC) bit mask of IR lines connected to slaves
7651   //        (slave PIC) 3-bit # of slave's connection to master
7652   outb(IO_PIC1+1, 1<<IRQ_SLAVE);
7653 
7654   // ICW4:  000nbmap
7655   //    n:  1 = special fully nested mode
7656   //    b:  1 = buffered mode
7657   //    m:  0 = slave PIC, 1 = master PIC
7658   //      (ignored when b is 0, as the master/slave role
7659   //      can be hardwired).
7660   //    a:  1 = Automatic EOI mode
7661   //    p:  0 = MCS-80/85 mode, 1 = intel x86 mode
7662   outb(IO_PIC1+1, 0x3);
7663 
7664   // Set up slave (8259A-2)
7665   outb(IO_PIC2, 0x11);                  // ICW1
7666   outb(IO_PIC2+1, T_IRQ0 + 8);      // ICW2
7667   outb(IO_PIC2+1, IRQ_SLAVE);           // ICW3
7668   // NB Automatic EOI mode doesn't tend to work on the slave.
7669   // Linux source code says it's "to be investigated".
7670   outb(IO_PIC2+1, 0x3);                 // ICW4
7671 
7672   // OCW3:  0ef01prs
7673   //   ef:  0x = NOP, 10 = clear specific mask, 11 = set specific mask
7674   //    p:  0 = no polling, 1 = polling mode
7675   //   rs:  0x = NOP, 10 = read IRR, 11 = read ISR
7676   outb(IO_PIC1, 0x68);             // clear specific mask
7677   outb(IO_PIC1, 0x0a);             // read IRR by default
7678 
7679   outb(IO_PIC2, 0x68);             // OCW3
7680   outb(IO_PIC2, 0x0a);             // OCW3
7681 
7682   if(irqmask != 0xFFFF)
7683     picsetmask(irqmask);
7684 }
7685 
7686 
7687 
7688 
7689 
7690 
7691 
7692 
7693 
7694 
7695 
7696 
7697 
7698 
7699 
7700 // Blank page.
7701 
7702 
7703 
7704 
7705 
7706 
7707 
7708 
7709 
7710 
7711 
7712 
7713 
7714 
7715 
7716 
7717 
7718 
7719 
7720 
7721 
7722 
7723 
7724 
7725 
7726 
7727 
7728 
7729 
7730 
7731 
7732 
7733 
7734 
7735 
7736 
7737 
7738 
7739 
7740 
7741 
7742 
7743 
7744 
7745 
7746 
7747 
7748 
7749 
