7750 // PC keyboard interface constants
7751 
7752 #define KBSTATP         0x64    // kbd controller status port(I)
7753 #define KBS_DIB         0x01    // kbd data in buffer
7754 #define KBDATAP         0x60    // kbd data port(I)
7755 
7756 #define NO              0
7757 
7758 #define SHIFT           (1<<0)
7759 #define CTL             (1<<1)
7760 #define ALT             (1<<2)
7761 
7762 #define CAPSLOCK        (1<<3)
7763 #define NUMLOCK         (1<<4)
7764 #define SCROLLLOCK      (1<<5)
7765 
7766 #define E0ESC           (1<<6)
7767 
7768 // Special keycodes
7769 #define KEY_HOME        0xE0
7770 #define KEY_END         0xE1
7771 #define KEY_UP          0xE2
7772 #define KEY_DN          0xE3
7773 #define KEY_LF          0xE4
7774 #define KEY_RT          0xE5
7775 #define KEY_PGUP        0xE6
7776 #define KEY_PGDN        0xE7
7777 #define KEY_INS         0xE8
7778 #define KEY_DEL         0xE9
7779 
7780 // C('A') == Control-A
7781 #define C(x) (x - '@')
7782 
7783 static uchar shiftcode[256] =
7784 {
7785   [0x1D] CTL,
7786   [0x2A] SHIFT,
7787   [0x36] SHIFT,
7788   [0x38] ALT,
7789   [0x9D] CTL,
7790   [0xB8] ALT
7791 };
7792 
7793 static uchar togglecode[256] =
7794 {
7795   [0x3A] CAPSLOCK,
7796   [0x45] NUMLOCK,
7797   [0x46] SCROLLLOCK
7798 };
7799 
7800 static uchar normalmap[256] =
7801 {
7802   NO,   0x1B, '1',  '2',  '3',  '4',  '5',  '6',  // 0x00
7803   '7',  '8',  '9',  '0',  '-',  '=',  '\b', '\t',
7804   'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  // 0x10
7805   'o',  'p',  '[',  ']',  '\n', NO,   'a',  's',
7806   'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',  // 0x20
7807   '\'', '`',  NO,   '\\', 'z',  'x',  'c',  'v',
7808   'b',  'n',  'm',  ',',  '.',  '/',  NO,   '*',  // 0x30
7809   NO,   ' ',  NO,   NO,   NO,   NO,   NO,   NO,
7810   NO,   NO,   NO,   NO,   NO,   NO,   NO,   '7',  // 0x40
7811   '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
7812   '2',  '3',  '0',  '.',  NO,   NO,   NO,   NO,   // 0x50
7813   [0x9C] '\n',      // KP_Enter
7814   [0xB5] '/',       // KP_Div
7815   [0xC8] KEY_UP,    [0xD0] KEY_DN,
7816   [0xC9] KEY_PGUP,  [0xD1] KEY_PGDN,
7817   [0xCB] KEY_LF,    [0xCD] KEY_RT,
7818   [0x97] KEY_HOME,  [0xCF] KEY_END,
7819   [0xD2] KEY_INS,   [0xD3] KEY_DEL
7820 };
7821 
7822 static uchar shiftmap[256] =
7823 {
7824   NO,   033,  '!',  '@',  '#',  '$',  '%',  '^',  // 0x00
7825   '&',  '*',  '(',  ')',  '_',  '+',  '\b', '\t',
7826   'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  // 0x10
7827   'O',  'P',  '{',  '}',  '\n', NO,   'A',  'S',
7828   'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',  // 0x20
7829   '"',  '~',  NO,   '|',  'Z',  'X',  'C',  'V',
7830   'B',  'N',  'M',  '<',  '>',  '?',  NO,   '*',  // 0x30
7831   NO,   ' ',  NO,   NO,   NO,   NO,   NO,   NO,
7832   NO,   NO,   NO,   NO,   NO,   NO,   NO,   '7',  // 0x40
7833   '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
7834   '2',  '3',  '0',  '.',  NO,   NO,   NO,   NO,   // 0x50
7835   [0x9C] '\n',      // KP_Enter
7836   [0xB5] '/',       // KP_Div
7837   [0xC8] KEY_UP,    [0xD0] KEY_DN,
7838   [0xC9] KEY_PGUP,  [0xD1] KEY_PGDN,
7839   [0xCB] KEY_LF,    [0xCD] KEY_RT,
7840   [0x97] KEY_HOME,  [0xCF] KEY_END,
7841   [0xD2] KEY_INS,   [0xD3] KEY_DEL
7842 };
7843 
7844 
7845 
7846 
7847 
7848 
7849 
7850 static uchar ctlmap[256] =
7851 {
7852   NO,      NO,      NO,      NO,      NO,      NO,      NO,      NO,
7853   NO,      NO,      NO,      NO,      NO,      NO,      NO,      NO,
7854   C('Q'),  C('W'),  C('E'),  C('R'),  C('T'),  C('Y'),  C('U'),  C('I'),
7855   C('O'),  C('P'),  NO,      NO,      '\r',    NO,      C('A'),  C('S'),
7856   C('D'),  C('F'),  C('G'),  C('H'),  C('J'),  C('K'),  C('L'),  NO,
7857   NO,      NO,      NO,      C('\\'), C('Z'),  C('X'),  C('C'),  C('V'),
7858   C('B'),  C('N'),  C('M'),  NO,      NO,      C('/'),  NO,      NO,
7859   [0x9C] '\r',      // KP_Enter
7860   [0xB5] C('/'),    // KP_Div
7861   [0xC8] KEY_UP,    [0xD0] KEY_DN,
7862   [0xC9] KEY_PGUP,  [0xD1] KEY_PGDN,
7863   [0xCB] KEY_LF,    [0xCD] KEY_RT,
7864   [0x97] KEY_HOME,  [0xCF] KEY_END,
7865   [0xD2] KEY_INS,   [0xD3] KEY_DEL
7866 };
7867 
7868 
7869 
7870 
7871 
7872 
7873 
7874 
7875 
7876 
7877 
7878 
7879 
7880 
7881 
7882 
7883 
7884 
7885 
7886 
7887 
7888 
7889 
7890 
7891 
7892 
7893 
7894 
7895 
7896 
7897 
7898 
7899 
