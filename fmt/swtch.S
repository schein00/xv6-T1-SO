3000 # Context switch
3001 #
3002 #   void swtch(struct context **old, struct context *new);
3003 #
3004 # Save current register context in old
3005 # and then load register context from new.
3006 
3007 .globl swtch
3008 swtch:
3009   movl 4(%esp), %eax
3010   movl 8(%esp), %edx
3011 
3012   # Save old callee-save registers
3013   pushl %ebp
3014   pushl %ebx
3015   pushl %esi
3016   pushl %edi
3017 
3018   # Switch stacks
3019   movl %esp, (%eax)
3020   movl %edx, %esp
3021 
3022   # Load new callee-save registers
3023   popl %edi
3024   popl %esi
3025   popl %ebx
3026   popl %ebp
3027   ret
3028 
3029 
3030 
3031 
3032 
3033 
3034 
3035 
3036 
3037 
3038 
3039 
3040 
3041 
3042 
3043 
3044 
3045 
3046 
3047 
3048 
3049 
