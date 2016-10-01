7150 // The local APIC manages internal (non-I/O) interrupts.
7151 // See Chapter 8 & Appendix C of Intel processor manual volume 3.
7152 
7153 #include "param.h"
7154 #include "types.h"
7155 #include "defs.h"
7156 #include "date.h"
7157 #include "memlayout.h"
7158 #include "traps.h"
7159 #include "mmu.h"
7160 #include "x86.h"
7161 #include "proc.h"  // ncpu
7162 
7163 // Local APIC registers, divided by 4 for use as uint[] indices.
7164 #define ID      (0x0020/4)   // ID
7165 #define VER     (0x0030/4)   // Version
7166 #define TPR     (0x0080/4)   // Task Priority
7167 #define EOI     (0x00B0/4)   // EOI
7168 #define SVR     (0x00F0/4)   // Spurious Interrupt Vector
7169   #define ENABLE     0x00000100   // Unit Enable
7170 #define ESR     (0x0280/4)   // Error Status
7171 #define ICRLO   (0x0300/4)   // Interrupt Command
7172   #define INIT       0x00000500   // INIT/RESET
7173   #define STARTUP    0x00000600   // Startup IPI
7174   #define DELIVS     0x00001000   // Delivery status
7175   #define ASSERT     0x00004000   // Assert interrupt (vs deassert)
7176   #define DEASSERT   0x00000000
7177   #define LEVEL      0x00008000   // Level triggered
7178   #define BCAST      0x00080000   // Send to all APICs, including self.
7179   #define BUSY       0x00001000
7180   #define FIXED      0x00000000
7181 #define ICRHI   (0x0310/4)   // Interrupt Command [63:32]
7182 #define TIMER   (0x0320/4)   // Local Vector Table 0 (TIMER)
7183   #define X1         0x0000000B   // divide counts by 1
7184   #define PERIODIC   0x00020000   // Periodic
7185 #define PCINT   (0x0340/4)   // Performance Counter LVT
7186 #define LINT0   (0x0350/4)   // Local Vector Table 1 (LINT0)
7187 #define LINT1   (0x0360/4)   // Local Vector Table 2 (LINT1)
7188 #define ERROR   (0x0370/4)   // Local Vector Table 3 (ERROR)
7189   #define MASKED     0x00010000   // Interrupt masked
7190 #define TICR    (0x0380/4)   // Timer Initial Count
7191 #define TCCR    (0x0390/4)   // Timer Current Count
7192 #define TDCR    (0x03E0/4)   // Timer Divide Configuration
7193 
7194 volatile uint *lapic;  // Initialized in mp.c
7195 
7196 
7197 
7198 
7199 
7200 static void
7201 lapicw(int index, int value)
7202 {
7203   lapic[index] = value;
7204   lapic[ID];  // wait for write to finish, by reading
7205 }
7206 
7207 
7208 
7209 
7210 
7211 
7212 
7213 
7214 
7215 
7216 
7217 
7218 
7219 
7220 
7221 
7222 
7223 
7224 
7225 
7226 
7227 
7228 
7229 
7230 
7231 
7232 
7233 
7234 
7235 
7236 
7237 
7238 
7239 
7240 
7241 
7242 
7243 
7244 
7245 
7246 
7247 
7248 
7249 
7250 void
7251 lapicinit(void)
7252 {
7253   if(!lapic)
7254     return;
7255 
7256   // Enable local APIC; set spurious interrupt vector.
7257   lapicw(SVR, ENABLE | (T_IRQ0 + IRQ_SPURIOUS));
7258 
7259   // The timer repeatedly counts down at bus frequency
7260   // from lapic[TICR] and then issues an interrupt.
7261   // If xv6 cared more about precise timekeeping,
7262   // TICR would be calibrated using an external time source.
7263   lapicw(TDCR, X1);
7264   lapicw(TIMER, PERIODIC | (T_IRQ0 + IRQ_TIMER));
7265   lapicw(TICR, 10000000);
7266 
7267   // Disable logical interrupt lines.
7268   lapicw(LINT0, MASKED);
7269   lapicw(LINT1, MASKED);
7270 
7271   // Disable performance counter overflow interrupts
7272   // on machines that provide that interrupt entry.
7273   if(((lapic[VER]>>16) & 0xFF) >= 4)
7274     lapicw(PCINT, MASKED);
7275 
7276   // Map error interrupt to IRQ_ERROR.
7277   lapicw(ERROR, T_IRQ0 + IRQ_ERROR);
7278 
7279   // Clear error status register (requires back-to-back writes).
7280   lapicw(ESR, 0);
7281   lapicw(ESR, 0);
7282 
7283   // Ack any outstanding interrupts.
7284   lapicw(EOI, 0);
7285 
7286   // Send an Init Level De-Assert to synchronise arbitration ID's.
7287   lapicw(ICRHI, 0);
7288   lapicw(ICRLO, BCAST | INIT | LEVEL);
7289   while(lapic[ICRLO] & DELIVS)
7290     ;
7291 
7292   // Enable interrupts on the APIC (but not on the processor).
7293   lapicw(TPR, 0);
7294 }
7295 
7296 
7297 
7298 
7299 
7300 int
7301 cpunum(void)
7302 {
7303   int apicid, i;
7304 
7305   // Cannot call cpu when interrupts are enabled:
7306   // result not guaranteed to last long enough to be used!
7307   // Would prefer to panic but even printing is chancy here:
7308   // almost everything, including cprintf and panic, calls cpu,
7309   // often indirectly through acquire and release.
7310   if(readeflags()&FL_IF){
7311     static int n;
7312     if(n++ == 0)
7313       cprintf("cpu called from %x with interrupts enabled\n",
7314         __builtin_return_address(0));
7315   }
7316 
7317   if (!lapic)
7318     return 0;
7319 
7320   apicid = lapic[ID] >> 24;
7321   for (i = 0; i < ncpu; ++i) {
7322     if (cpus[i].apicid == apicid)
7323       return i;
7324   }
7325   panic("unknown apicid\n");
7326 }
7327 
7328 // Acknowledge interrupt.
7329 void
7330 lapiceoi(void)
7331 {
7332   if(lapic)
7333     lapicw(EOI, 0);
7334 }
7335 
7336 // Spin for a given number of microseconds.
7337 // On real hardware would want to tune this dynamically.
7338 void
7339 microdelay(int us)
7340 {
7341 }
7342 
7343 
7344 
7345 
7346 
7347 
7348 
7349 
7350 #define CMOS_PORT    0x70
7351 #define CMOS_RETURN  0x71
7352 
7353 // Start additional processor running entry code at addr.
7354 // See Appendix B of MultiProcessor Specification.
7355 void
7356 lapicstartap(uchar apicid, uint addr)
7357 {
7358   int i;
7359   ushort *wrv;
7360 
7361   // "The BSP must initialize CMOS shutdown code to 0AH
7362   // and the warm reset vector (DWORD based at 40:67) to point at
7363   // the AP startup code prior to the [universal startup algorithm]."
7364   outb(CMOS_PORT, 0xF);  // offset 0xF is shutdown code
7365   outb(CMOS_PORT+1, 0x0A);
7366   wrv = (ushort*)P2V((0x40<<4 | 0x67));  // Warm reset vector
7367   wrv[0] = 0;
7368   wrv[1] = addr >> 4;
7369 
7370   // "Universal startup algorithm."
7371   // Send INIT (level-triggered) interrupt to reset other CPU.
7372   lapicw(ICRHI, apicid<<24);
7373   lapicw(ICRLO, INIT | LEVEL | ASSERT);
7374   microdelay(200);
7375   lapicw(ICRLO, INIT | LEVEL);
7376   microdelay(100);    // should be 10ms, but too slow in Bochs!
7377 
7378   // Send startup IPI (twice!) to enter code.
7379   // Regular hardware is supposed to only accept a STARTUP
7380   // when it is in the halted state due to an INIT.  So the second
7381   // should be ignored, but it is part of the official Intel algorithm.
7382   // Bochs complains about the second one.  Too bad for Bochs.
7383   for(i = 0; i < 2; i++){
7384     lapicw(ICRHI, apicid<<24);
7385     lapicw(ICRLO, STARTUP | (addr>>12));
7386     microdelay(200);
7387   }
7388 }
7389 
7390 
7391 
7392 
7393 
7394 
7395 
7396 
7397 
7398 
7399 
7400 #define CMOS_STATA   0x0a
7401 #define CMOS_STATB   0x0b
7402 #define CMOS_UIP    (1 << 7)        // RTC update in progress
7403 
7404 #define SECS    0x00
7405 #define MINS    0x02
7406 #define HOURS   0x04
7407 #define DAY     0x07
7408 #define MONTH   0x08
7409 #define YEAR    0x09
7410 
7411 static uint cmos_read(uint reg)
7412 {
7413   outb(CMOS_PORT,  reg);
7414   microdelay(200);
7415 
7416   return inb(CMOS_RETURN);
7417 }
7418 
7419 static void fill_rtcdate(struct rtcdate *r)
7420 {
7421   r->second = cmos_read(SECS);
7422   r->minute = cmos_read(MINS);
7423   r->hour   = cmos_read(HOURS);
7424   r->day    = cmos_read(DAY);
7425   r->month  = cmos_read(MONTH);
7426   r->year   = cmos_read(YEAR);
7427 }
7428 
7429 // qemu seems to use 24-hour GWT and the values are BCD encoded
7430 void cmostime(struct rtcdate *r)
7431 {
7432   struct rtcdate t1, t2;
7433   int sb, bcd;
7434 
7435   sb = cmos_read(CMOS_STATB);
7436 
7437   bcd = (sb & (1 << 2)) == 0;
7438 
7439   // make sure CMOS doesn't modify time while we read it
7440   for(;;) {
7441     fill_rtcdate(&t1);
7442     if(cmos_read(CMOS_STATA) & CMOS_UIP)
7443         continue;
7444     fill_rtcdate(&t2);
7445     if(memcmp(&t1, &t2, sizeof(t1)) == 0)
7446       break;
7447   }
7448 
7449 
7450   // convert
7451   if(bcd) {
7452 #define    CONV(x)     (t1.x = ((t1.x >> 4) * 10) + (t1.x & 0xf))
7453     CONV(second);
7454     CONV(minute);
7455     CONV(hour  );
7456     CONV(day   );
7457     CONV(month );
7458     CONV(year  );
7459 #undef     CONV
7460   }
7461 
7462   *r = t1;
7463   r->year += 2000;
7464 }
7465 
7466 
7467 
7468 
7469 
7470 
7471 
7472 
7473 
7474 
7475 
7476 
7477 
7478 
7479 
7480 
7481 
7482 
7483 
7484 
7485 
7486 
7487 
7488 
7489 
7490 
7491 
7492 
7493 
7494 
7495 
7496 
7497 
7498 
7499 
