4550 #include "types.h"
4551 #include "defs.h"
4552 #include "param.h"
4553 #include "spinlock.h"
4554 #include "fs.h"
4555 #include "buf.h"
4556 
4557 // Simple logging that allows concurrent FS system calls.
4558 //
4559 // A log transaction contains the updates of multiple FS system
4560 // calls. The logging system only commits when there are
4561 // no FS system calls active. Thus there is never
4562 // any reasoning required about whether a commit might
4563 // write an uncommitted system call's updates to disk.
4564 //
4565 // A system call should call begin_op()/end_op() to mark
4566 // its start and end. Usually begin_op() just increments
4567 // the count of in-progress FS system calls and returns.
4568 // But if it thinks the log is close to running out, it
4569 // sleeps until the last outstanding end_op() commits.
4570 //
4571 // The log is a physical re-do log containing disk blocks.
4572 // The on-disk log format:
4573 //   header block, containing block #s for block A, B, C, ...
4574 //   block A
4575 //   block B
4576 //   block C
4577 //   ...
4578 // Log appends are synchronous.
4579 
4580 // Contents of the header block, used for both the on-disk header block
4581 // and to keep track in memory of logged block# before commit.
4582 struct logheader {
4583   int n;
4584   int block[LOGSIZE];
4585 };
4586 
4587 struct log {
4588   struct spinlock lock;
4589   int start;
4590   int size;
4591   int outstanding; // how many FS sys calls are executing.
4592   int committing;  // in commit(), please wait.
4593   int dev;
4594   struct logheader lh;
4595 };
4596 
4597 
4598 
4599 
4600 struct log log;
4601 
4602 static void recover_from_log(void);
4603 static void commit();
4604 
4605 void
4606 initlog(int dev)
4607 {
4608   if (sizeof(struct logheader) >= BSIZE)
4609     panic("initlog: too big logheader");
4610 
4611   struct superblock sb;
4612   initlock(&log.lock, "log");
4613   readsb(dev, &sb);
4614   log.start = sb.logstart;
4615   log.size = sb.nlog;
4616   log.dev = dev;
4617   recover_from_log();
4618 }
4619 
4620 // Copy committed blocks from log to their home location
4621 static void
4622 install_trans(void)
4623 {
4624   int tail;
4625 
4626   for (tail = 0; tail < log.lh.n; tail++) {
4627     struct buf *lbuf = bread(log.dev, log.start+tail+1); // read log block
4628     struct buf *dbuf = bread(log.dev, log.lh.block[tail]); // read dst
4629     memmove(dbuf->data, lbuf->data, BSIZE);  // copy block to dst
4630     bwrite(dbuf);  // write dst to disk
4631     brelse(lbuf);
4632     brelse(dbuf);
4633   }
4634 }
4635 
4636 // Read the log header from disk into the in-memory log header
4637 static void
4638 read_head(void)
4639 {
4640   struct buf *buf = bread(log.dev, log.start);
4641   struct logheader *lh = (struct logheader *) (buf->data);
4642   int i;
4643   log.lh.n = lh->n;
4644   for (i = 0; i < log.lh.n; i++) {
4645     log.lh.block[i] = lh->block[i];
4646   }
4647   brelse(buf);
4648 }
4649 
4650 // Write in-memory log header to disk.
4651 // This is the true point at which the
4652 // current transaction commits.
4653 static void
4654 write_head(void)
4655 {
4656   struct buf *buf = bread(log.dev, log.start);
4657   struct logheader *hb = (struct logheader *) (buf->data);
4658   int i;
4659   hb->n = log.lh.n;
4660   for (i = 0; i < log.lh.n; i++) {
4661     hb->block[i] = log.lh.block[i];
4662   }
4663   bwrite(buf);
4664   brelse(buf);
4665 }
4666 
4667 static void
4668 recover_from_log(void)
4669 {
4670   read_head();
4671   install_trans(); // if committed, copy from log to disk
4672   log.lh.n = 0;
4673   write_head(); // clear the log
4674 }
4675 
4676 // called at the start of each FS system call.
4677 void
4678 begin_op(void)
4679 {
4680   acquire(&log.lock);
4681   while(1){
4682     if(log.committing){
4683       sleep(&log, &log.lock);
4684     } else if(log.lh.n + (log.outstanding+1)*MAXOPBLOCKS > LOGSIZE){
4685       // this op might exhaust log space; wait for commit.
4686       sleep(&log, &log.lock);
4687     } else {
4688       log.outstanding += 1;
4689       release(&log.lock);
4690       break;
4691     }
4692   }
4693 }
4694 
4695 
4696 
4697 
4698 
4699 
4700 // called at the end of each FS system call.
4701 // commits if this was the last outstanding operation.
4702 void
4703 end_op(void)
4704 {
4705   int do_commit = 0;
4706 
4707   acquire(&log.lock);
4708   log.outstanding -= 1;
4709   if(log.committing)
4710     panic("log.committing");
4711   if(log.outstanding == 0){
4712     do_commit = 1;
4713     log.committing = 1;
4714   } else {
4715     // begin_op() may be waiting for log space.
4716     wakeup(&log);
4717   }
4718   release(&log.lock);
4719 
4720   if(do_commit){
4721     // call commit w/o holding locks, since not allowed
4722     // to sleep with locks.
4723     commit();
4724     acquire(&log.lock);
4725     log.committing = 0;
4726     wakeup(&log);
4727     release(&log.lock);
4728   }
4729 }
4730 
4731 // Copy modified blocks from cache to log.
4732 static void
4733 write_log(void)
4734 {
4735   int tail;
4736 
4737   for (tail = 0; tail < log.lh.n; tail++) {
4738     struct buf *to = bread(log.dev, log.start+tail+1); // log block
4739     struct buf *from = bread(log.dev, log.lh.block[tail]); // cache block
4740     memmove(to->data, from->data, BSIZE);
4741     bwrite(to);  // write the log
4742     brelse(from);
4743     brelse(to);
4744   }
4745 }
4746 
4747 
4748 
4749 
4750 static void
4751 commit()
4752 {
4753   if (log.lh.n > 0) {
4754     write_log();     // Write modified blocks from cache to log
4755     write_head();    // Write header to disk -- the real commit
4756     install_trans(); // Now install writes to home locations
4757     log.lh.n = 0;
4758     write_head();    // Erase the transaction from the log
4759   }
4760 }
4761 
4762 // Caller has modified b->data and is done with the buffer.
4763 // Record the block number and pin in the cache with B_DIRTY.
4764 // commit()/write_log() will do the disk write.
4765 //
4766 // log_write() replaces bwrite(); a typical use is:
4767 //   bp = bread(...)
4768 //   modify bp->data[]
4769 //   log_write(bp)
4770 //   brelse(bp)
4771 void
4772 log_write(struct buf *b)
4773 {
4774   int i;
4775 
4776   if (log.lh.n >= LOGSIZE || log.lh.n >= log.size - 1)
4777     panic("too big a transaction");
4778   if (log.outstanding < 1)
4779     panic("log_write outside of trans");
4780 
4781   acquire(&log.lock);
4782   for (i = 0; i < log.lh.n; i++) {
4783     if (log.lh.block[i] == b->blockno)   // log absorbtion
4784       break;
4785   }
4786   log.lh.block[i] = b->blockno;
4787   if (i == log.lh.n)
4788     log.lh.n++;
4789   b->flags |= B_DIRTY; // prevent eviction
4790   release(&log.lock);
4791 }
4792 
4793 
4794 
4795 
4796 
4797 
4798 
4799 
