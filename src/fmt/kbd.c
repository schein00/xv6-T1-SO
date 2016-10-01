7900 #include "types.h"
7901 #include "x86.h"
7902 #include "defs.h"
7903 #include "kbd.h"
7904 
7905 int
7906 kbdgetc(void)
7907 {
7908   static uint shift;
7909   static uchar *charcode[4] = {
7910     normalmap, shiftmap, ctlmap, ctlmap
7911   };
7912   uint st, data, c;
7913 
7914   st = inb(KBSTATP);
7915   if((st & KBS_DIB) == 0)
7916     return -1;
7917   data = inb(KBDATAP);
7918 
7919   if(data == 0xE0){
7920     shift |= E0ESC;
7921     return 0;
7922   } else if(data & 0x80){
7923     // Key released
7924     data = (shift & E0ESC ? data : data & 0x7F);
7925     shift &= ~(shiftcode[data] | E0ESC);
7926     return 0;
7927   } else if(shift & E0ESC){
7928     // Last character was an E0 escape; or with 0x80
7929     data |= 0x80;
7930     shift &= ~E0ESC;
7931   }
7932 
7933   shift |= shiftcode[data];
7934   shift ^= togglecode[data];
7935   c = charcode[shift & (CTL | SHIFT)][data];
7936   if(shift & CAPSLOCK){
7937     if('a' <= c && c <= 'z')
7938       c += 'A' - 'a';
7939     else if('A' <= c && c <= 'Z')
7940       c += 'a' - 'A';
7941   }
7942   return c;
7943 }
7944 
7945 void
7946 kbdintr(void)
7947 {
7948   consoleintr(kbdgetc);
7949 }
