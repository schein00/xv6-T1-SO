7500 // The I/O APIC manages hardware interrupts for an SMP system.
7501 // http://www.intel.com/design/chipsets/datashts/29056601.pdf
7502 // See also picirq.c.
7503 
7504 #include "types.h"
7505 #include "defs.h"
7506 #include "traps.h"
7507 
7508 #define IOAPIC  0xFEC00000   // Default physical address of IO APIC
7509 
7510 #define REG_ID     0x00  // Register index: ID
7511 #define REG_VER    0x01  // Register index: version
7512 #define REG_TABLE  0x10  // Redirection table base
7513 
7514 // The redirection table starts at REG_TABLE and uses
7515 // two registers to configure each interrupt.
7516 // The first (low) register in a pair contains configuration bits.
7517 // The second (high) register contains a bitmask telling which
7518 // CPUs can serve that interrupt.
7519 #define INT_DISABLED   0x00010000  // Interrupt disabled
7520 #define INT_LEVEL      0x00008000  // Level-triggered (vs edge-)
7521 #define INT_ACTIVELOW  0x00002000  // Active low (vs high)
7522 #define INT_LOGICAL    0x00000800  // Destination is CPU id (vs APIC ID)
7523 
7524 volatile struct ioapic *ioapic;
7525 
7526 // IO APIC MMIO structure: write reg, then read or write data.
7527 struct ioapic {
7528   uint reg;
7529   uint pad[3];
7530   uint data;
7531 };
7532 
7533 static uint
7534 ioapicread(int reg)
7535 {
7536   ioapic->reg = reg;
7537   return ioapic->data;
7538 }
7539 
7540 static void
7541 ioapicwrite(int reg, uint data)
7542 {
7543   ioapic->reg = reg;
7544   ioapic->data = data;
7545 }
7546 
7547 
7548 
7549 
7550 void
7551 ioapicinit(void)
7552 {
7553   int i, id, maxintr;
7554 
7555   if(!ismp)
7556     return;
7557 
7558   ioapic = (volatile struct ioapic*)IOAPIC;
7559   maxintr = (ioapicread(REG_VER) >> 16) & 0xFF;
7560   id = ioapicread(REG_ID) >> 24;
7561   if(id != ioapicid)
7562     cprintf("ioapicinit: id isn't equal to ioapicid; not a MP\n");
7563 
7564   // Mark all interrupts edge-triggered, active high, disabled,
7565   // and not routed to any CPUs.
7566   for(i = 0; i <= maxintr; i++){
7567     ioapicwrite(REG_TABLE+2*i, INT_DISABLED | (T_IRQ0 + i));
7568     ioapicwrite(REG_TABLE+2*i+1, 0);
7569   }
7570 }
7571 
7572 void
7573 ioapicenable(int irq, int cpunum)
7574 {
7575   if(!ismp)
7576     return;
7577 
7578   // Mark interrupt edge-triggered, active high,
7579   // enabled, and routed to the given cpunum,
7580   // which happens to be that cpu's APIC ID.
7581   ioapicwrite(REG_TABLE+2*irq, T_IRQ0 + irq);
7582   ioapicwrite(REG_TABLE+2*irq+1, cpunum << 24);
7583 }
7584 
7585 
7586 
7587 
7588 
7589 
7590 
7591 
7592 
7593 
7594 
7595 
7596 
7597 
7598 
7599 
