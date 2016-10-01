4350 // Buffer cache.
4351 //
4352 // The buffer cache is a linked list of buf structures holding
4353 // cached copies of disk block contents.  Caching disk blocks
4354 // in memory reduces the number of disk reads and also provides
4355 // a synchronization point for disk blocks used by multiple processes.
4356 //
4357 // Interface:
4358 // * To get a buffer for a particular disk block, call bread.
4359 // * After changing buffer data, call bwrite to write it to disk.
4360 // * When done with the buffer, call brelse.
4361 // * Do not use the buffer after calling brelse.
4362 // * Only one process at a time can use a buffer,
4363 //     so do not keep them longer than necessary.
4364 //
4365 // The implementation uses three state flags internally:
4366 // * B_BUSY: the block has been returned from bread
4367 //     and has not been passed back to brelse.
4368 // * B_VALID: the buffer data has been read from the disk.
4369 // * B_DIRTY: the buffer data has been modified
4370 //     and needs to be written to disk.
4371 
4372 #include "types.h"
4373 #include "defs.h"
4374 #include "param.h"
4375 #include "spinlock.h"
4376 #include "fs.h"
4377 #include "buf.h"
4378 
4379 struct {
4380   struct spinlock lock;
4381   struct buf buf[NBUF];
4382 
4383   // Linked list of all buffers, through prev/next.
4384   // head.next is most recently used.
4385   struct buf head;
4386 } bcache;
4387 
4388 void
4389 binit(void)
4390 {
4391   struct buf *b;
4392 
4393   initlock(&bcache.lock, "bcache");
4394 
4395 
4396 
4397 
4398 
4399 
4400   // Create linked list of buffers
4401   bcache.head.prev = &bcache.head;
4402   bcache.head.next = &bcache.head;
4403   for(b = bcache.buf; b < bcache.buf+NBUF; b++){
4404     b->next = bcache.head.next;
4405     b->prev = &bcache.head;
4406     b->dev = -1;
4407     bcache.head.next->prev = b;
4408     bcache.head.next = b;
4409   }
4410 }
4411 
4412 // Look through buffer cache for block on device dev.
4413 // If not found, allocate a buffer.
4414 // In either case, return B_BUSY buffer.
4415 static struct buf*
4416 bget(uint dev, uint blockno)
4417 {
4418   struct buf *b;
4419 
4420   acquire(&bcache.lock);
4421 
4422  loop:
4423   // Is the block already cached?
4424   for(b = bcache.head.next; b != &bcache.head; b = b->next){
4425     if(b->dev == dev && b->blockno == blockno){
4426       if(!(b->flags & B_BUSY)){
4427         b->flags |= B_BUSY;
4428         release(&bcache.lock);
4429         return b;
4430       }
4431       sleep(b, &bcache.lock);
4432       goto loop;
4433     }
4434   }
4435 
4436   // Not cached; recycle some non-busy and clean buffer.
4437   // "clean" because B_DIRTY and !B_BUSY means log.c
4438   // hasn't yet committed the changes to the buffer.
4439   for(b = bcache.head.prev; b != &bcache.head; b = b->prev){
4440     if((b->flags & B_BUSY) == 0 && (b->flags & B_DIRTY) == 0){
4441       b->dev = dev;
4442       b->blockno = blockno;
4443       b->flags = B_BUSY;
4444       release(&bcache.lock);
4445       return b;
4446     }
4447   }
4448   panic("bget: no buffers");
4449 }
4450 // Return a B_BUSY buf with the contents of the indicated block.
4451 struct buf*
4452 bread(uint dev, uint blockno)
4453 {
4454   struct buf *b;
4455 
4456   b = bget(dev, blockno);
4457   if(!(b->flags & B_VALID)) {
4458     iderw(b);
4459   }
4460   return b;
4461 }
4462 
4463 // Write b's contents to disk.  Must be B_BUSY.
4464 void
4465 bwrite(struct buf *b)
4466 {
4467   if((b->flags & B_BUSY) == 0)
4468     panic("bwrite");
4469   b->flags |= B_DIRTY;
4470   iderw(b);
4471 }
4472 
4473 // Release a B_BUSY buffer.
4474 // Move to the head of the MRU list.
4475 void
4476 brelse(struct buf *b)
4477 {
4478   if((b->flags & B_BUSY) == 0)
4479     panic("brelse");
4480 
4481   acquire(&bcache.lock);
4482 
4483   b->next->prev = b->prev;
4484   b->prev->next = b->next;
4485   b->next = bcache.head.next;
4486   b->prev = &bcache.head;
4487   bcache.head.next->prev = b;
4488   bcache.head.next = b;
4489 
4490   b->flags &= ~B_BUSY;
4491   wakeup(b);
4492 
4493   release(&bcache.lock);
4494 }
4495 
4496 
4497 
4498 
4499 
4500 // Blank page.
4501 
4502 
4503 
4504 
4505 
4506 
4507 
4508 
4509 
4510 
4511 
4512 
4513 
4514 
4515 
4516 
4517 
4518 
4519 
4520 
4521 
4522 
4523 
4524 
4525 
4526 
4527 
4528 
4529 
4530 
4531 
4532 
4533 
4534 
4535 
4536 
4537 
4538 
4539 
4540 
4541 
4542 
4543 
4544 
4545 
4546 
4547 
4548 
4549 
