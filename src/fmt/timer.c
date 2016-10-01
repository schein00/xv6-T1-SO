8300 // Intel 8253/8254/82C54 Programmable Interval Timer (PIT).
8301 // Only used on uniprocessors;
8302 // SMP machines use the local APIC timer.
8303 
8304 #include "types.h"
8305 #include "defs.h"
8306 #include "traps.h"
8307 #include "x86.h"
8308 
8309 #define IO_TIMER1       0x040           // 8253 Timer #1
8310 
8311 // Frequency of all three count-down timers;
8312 // (TIMER_FREQ/freq) is the appropriate count
8313 // to generate a frequency of freq Hz.
8314 
8315 #define TIMER_FREQ      1193182
8316 #define TIMER_DIV(x)    ((TIMER_FREQ+(x)/2)/(x))
8317 
8318 #define TIMER_MODE      (IO_TIMER1 + 3) // timer mode port
8319 #define TIMER_SEL0      0x00    // select counter 0
8320 #define TIMER_RATEGEN   0x04    // mode 2, rate generator
8321 #define TIMER_16BIT     0x30    // r/w counter 16 bits, LSB first
8322 
8323 void
8324 timerinit(void)
8325 {
8326   // Interrupt 100 times/sec.
8327   outb(TIMER_MODE, TIMER_SEL0 | TIMER_RATEGEN | TIMER_16BIT);
8328   outb(IO_TIMER1, TIMER_DIV(100) % 256);
8329   outb(IO_TIMER1, TIMER_DIV(100) / 256);
8330   picenable(IRQ_TIMER);
8331 }
8332 
8333 
8334 
8335 
8336 
8337 
8338 
8339 
8340 
8341 
8342 
8343 
8344 
8345 
8346 
8347 
8348 
8349 
