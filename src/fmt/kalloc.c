3050 // Physical memory allocator, intended to allocate
3051 // memory for user processes, kernel stacks, page table pages,
3052 // and pipe buffers. Allocates 4096-byte pages.
3053 
3054 #include "types.h"
3055 #include "defs.h"
3056 #include "param.h"
3057 #include "memlayout.h"
3058 #include "mmu.h"
3059 #include "spinlock.h"
3060 
3061 void freerange(void *vstart, void *vend);
3062 extern char end[]; // first address after kernel loaded from ELF file
3063 
3064 struct run {
3065   struct run *next;
3066 };
3067 
3068 struct {
3069   struct spinlock lock;
3070   int use_lock;
3071   struct run *freelist;
3072 } kmem;
3073 
3074 // Initialization happens in two phases.
3075 // 1. main() calls kinit1() while still using entrypgdir to place just
3076 // the pages mapped by entrypgdir on free list.
3077 // 2. main() calls kinit2() with the rest of the physical pages
3078 // after installing a full page table that maps them on all cores.
3079 void
3080 kinit1(void *vstart, void *vend)
3081 {
3082   initlock(&kmem.lock, "kmem");
3083   kmem.use_lock = 0;
3084   freerange(vstart, vend);
3085 }
3086 
3087 void
3088 kinit2(void *vstart, void *vend)
3089 {
3090   freerange(vstart, vend);
3091   kmem.use_lock = 1;
3092 }
3093 
3094 
3095 
3096 
3097 
3098 
3099 
3100 void
3101 freerange(void *vstart, void *vend)
3102 {
3103   char *p;
3104   p = (char*)PGROUNDUP((uint)vstart);
3105   for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
3106     kfree(p);
3107 }
3108 
3109 
3110 // Free the page of physical memory pointed at by v,
3111 // which normally should have been returned by a
3112 // call to kalloc().  (The exception is when
3113 // initializing the allocator; see kinit above.)
3114 void
3115 kfree(char *v)
3116 {
3117   struct run *r;
3118 
3119   if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
3120     panic("kfree");
3121 
3122   // Fill with junk to catch dangling refs.
3123   memset(v, 1, PGSIZE);
3124 
3125   if(kmem.use_lock)
3126     acquire(&kmem.lock);
3127   r = (struct run*)v;
3128   r->next = kmem.freelist;
3129   kmem.freelist = r;
3130   if(kmem.use_lock)
3131     release(&kmem.lock);
3132 }
3133 
3134 // Allocate one 4096-byte page of physical memory.
3135 // Returns a pointer that the kernel can use.
3136 // Returns 0 if the memory cannot be allocated.
3137 char*
3138 kalloc(void)
3139 {
3140   struct run *r;
3141 
3142   if(kmem.use_lock)
3143     acquire(&kmem.lock);
3144   r = kmem.freelist;
3145   if(r)
3146     kmem.freelist = r->next;
3147   if(kmem.use_lock)
3148     release(&kmem.lock);
3149   return (char*)r;
3150 }
3151 
3152 
3153 
3154 
3155 
3156 
3157 
3158 
3159 
3160 
3161 
3162 
3163 
3164 
3165 
3166 
3167 
3168 
3169 
3170 
3171 
3172 
3173 
3174 
3175 
3176 
3177 
3178 
3179 
3180 
3181 
3182 
3183 
3184 
3185 
3186 
3187 
3188 
3189 
3190 
3191 
3192 
3193 
3194 
3195 
3196 
3197 
3198 
3199 
