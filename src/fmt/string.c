6650 #include "types.h"
6651 #include "x86.h"
6652 
6653 void*
6654 memset(void *dst, int c, uint n)
6655 {
6656   if ((int)dst%4 == 0 && n%4 == 0){
6657     c &= 0xFF;
6658     stosl(dst, (c<<24)|(c<<16)|(c<<8)|c, n/4);
6659   } else
6660     stosb(dst, c, n);
6661   return dst;
6662 }
6663 
6664 int
6665 memcmp(const void *v1, const void *v2, uint n)
6666 {
6667   const uchar *s1, *s2;
6668 
6669   s1 = v1;
6670   s2 = v2;
6671   while(n-- > 0){
6672     if(*s1 != *s2)
6673       return *s1 - *s2;
6674     s1++, s2++;
6675   }
6676 
6677   return 0;
6678 }
6679 
6680 void*
6681 memmove(void *dst, const void *src, uint n)
6682 {
6683   const char *s;
6684   char *d;
6685 
6686   s = src;
6687   d = dst;
6688   if(s < d && s + n > d){
6689     s += n;
6690     d += n;
6691     while(n-- > 0)
6692       *--d = *--s;
6693   } else
6694     while(n-- > 0)
6695       *d++ = *s++;
6696 
6697   return dst;
6698 }
6699 
6700 // memcpy exists to placate GCC.  Use memmove.
6701 void*
6702 memcpy(void *dst, const void *src, uint n)
6703 {
6704   return memmove(dst, src, n);
6705 }
6706 
6707 int
6708 strncmp(const char *p, const char *q, uint n)
6709 {
6710   while(n > 0 && *p && *p == *q)
6711     n--, p++, q++;
6712   if(n == 0)
6713     return 0;
6714   return (uchar)*p - (uchar)*q;
6715 }
6716 
6717 char*
6718 strncpy(char *s, const char *t, int n)
6719 {
6720   char *os;
6721 
6722   os = s;
6723   while(n-- > 0 && (*s++ = *t++) != 0)
6724     ;
6725   while(n-- > 0)
6726     *s++ = 0;
6727   return os;
6728 }
6729 
6730 // Like strncpy but guaranteed to NUL-terminate.
6731 char*
6732 safestrcpy(char *s, const char *t, int n)
6733 {
6734   char *os;
6735 
6736   os = s;
6737   if(n <= 0)
6738     return os;
6739   while(--n > 0 && (*s++ = *t++) != 0)
6740     ;
6741   *s = 0;
6742   return os;
6743 }
6744 
6745 
6746 
6747 
6748 
6749 
6750 int
6751 strlen(const char *s)
6752 {
6753   int n;
6754 
6755   for(n = 0; s[n]; n++)
6756     ;
6757   return n;
6758 }
6759 
6760 
6761 
6762 
6763 
6764 
6765 
6766 
6767 
6768 
6769 
6770 
6771 
6772 
6773 
6774 
6775 
6776 
6777 
6778 
6779 
6780 
6781 
6782 
6783 
6784 
6785 
6786 
6787 
6788 
6789 
6790 
6791 
6792 
6793 
6794 
6795 
6796 
6797 
6798 
6799 
