6500 #include "types.h"
6501 #include "defs.h"
6502 #include "param.h"
6503 #include "mmu.h"
6504 #include "proc.h"
6505 #include "fs.h"
6506 #include "file.h"
6507 #include "spinlock.h"
6508 
6509 #define PIPESIZE 512
6510 
6511 struct pipe {
6512   struct spinlock lock;
6513   char data[PIPESIZE];
6514   uint nread;     // number of bytes read
6515   uint nwrite;    // number of bytes written
6516   int readopen;   // read fd is still open
6517   int writeopen;  // write fd is still open
6518 };
6519 
6520 int
6521 pipealloc(struct file **f0, struct file **f1)
6522 {
6523   struct pipe *p;
6524 
6525   p = 0;
6526   *f0 = *f1 = 0;
6527   if((*f0 = filealloc()) == 0 || (*f1 = filealloc()) == 0)
6528     goto bad;
6529   if((p = (struct pipe*)kalloc()) == 0)
6530     goto bad;
6531   p->readopen = 1;
6532   p->writeopen = 1;
6533   p->nwrite = 0;
6534   p->nread = 0;
6535   initlock(&p->lock, "pipe");
6536   (*f0)->type = FD_PIPE;
6537   (*f0)->readable = 1;
6538   (*f0)->writable = 0;
6539   (*f0)->pipe = p;
6540   (*f1)->type = FD_PIPE;
6541   (*f1)->readable = 0;
6542   (*f1)->writable = 1;
6543   (*f1)->pipe = p;
6544   return 0;
6545 
6546 
6547 
6548 
6549 
6550  bad:
6551   if(p)
6552     kfree((char*)p);
6553   if(*f0)
6554     fileclose(*f0);
6555   if(*f1)
6556     fileclose(*f1);
6557   return -1;
6558 }
6559 
6560 void
6561 pipeclose(struct pipe *p, int writable)
6562 {
6563   acquire(&p->lock);
6564   if(writable){
6565     p->writeopen = 0;
6566     wakeup(&p->nread);
6567   } else {
6568     p->readopen = 0;
6569     wakeup(&p->nwrite);
6570   }
6571   if(p->readopen == 0 && p->writeopen == 0){
6572     release(&p->lock);
6573     kfree((char*)p);
6574   } else
6575     release(&p->lock);
6576 }
6577 
6578 
6579 int
6580 pipewrite(struct pipe *p, char *addr, int n)
6581 {
6582   int i;
6583 
6584   acquire(&p->lock);
6585   for(i = 0; i < n; i++){
6586     while(p->nwrite == p->nread + PIPESIZE){  //DOC: pipewrite-full
6587       if(p->readopen == 0 || proc->killed){
6588         release(&p->lock);
6589         return -1;
6590       }
6591       wakeup(&p->nread);
6592       sleep(&p->nwrite, &p->lock);  //DOC: pipewrite-sleep
6593     }
6594     p->data[p->nwrite++ % PIPESIZE] = addr[i];
6595   }
6596   wakeup(&p->nread);  //DOC: pipewrite-wakeup1
6597   release(&p->lock);
6598   return n;
6599 }
6600 int
6601 piperead(struct pipe *p, char *addr, int n)
6602 {
6603   int i;
6604 
6605   acquire(&p->lock);
6606   while(p->nread == p->nwrite && p->writeopen){  //DOC: pipe-empty
6607     if(proc->killed){
6608       release(&p->lock);
6609       return -1;
6610     }
6611     sleep(&p->nread, &p->lock); //DOC: piperead-sleep
6612   }
6613   for(i = 0; i < n; i++){  //DOC: piperead-copy
6614     if(p->nread == p->nwrite)
6615       break;
6616     addr[i] = p->data[p->nread++ % PIPESIZE];
6617   }
6618   wakeup(&p->nwrite);  //DOC: piperead-wakeup
6619   release(&p->lock);
6620   return i;
6621 }
6622 
6623 
6624 
6625 
6626 
6627 
6628 
6629 
6630 
6631 
6632 
6633 
6634 
6635 
6636 
6637 
6638 
6639 
6640 
6641 
6642 
6643 
6644 
6645 
6646 
6647 
6648 
6649 
