2400 #include "types.h"
2401 #include "defs.h"
2402 #include "param.h"
2403 #include "memlayout.h"
2404 #include "mmu.h"
2405 #include "x86.h"
2406 #include "proc.h"
2407 #include "spinlock.h"
2408 
2409 struct {
2410   struct spinlock lock;
2411   struct proc proc[NPROC];
2412 } ptable;
2413 
2414 static struct proc *initproc;
2415 
2416 int nextpid = 1;
2417 extern void forkret(void);
2418 extern void trapret(void);
2419 
2420 static void wakeup1(void *chan);
2421 
2422 void
2423 pinit(void)
2424 {
2425   initlock(&ptable.lock, "ptable");
2426 }
2427 
2428 
2429 
2430 
2431 
2432 
2433 
2434 
2435 
2436 
2437 
2438 
2439 
2440 
2441 
2442 
2443 
2444 
2445 
2446 
2447 
2448 
2449 
2450 // Look in the process table for an UNUSED proc.
2451 // If found, change state to EMBRYO and initialize
2452 // state required to run in the kernel.
2453 // Otherwise return 0.
2454 // Must hold ptable.lock.
2455 static struct proc*
2456 allocproc(void)
2457 {
2458   struct proc *p;
2459   char *sp;
2460 
2461   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
2462     if(p->state == UNUSED)
2463       goto found;
2464   return 0;
2465 
2466 found:
2467   p->state = EMBRYO;
2468   p->pid = nextpid++;
2469 
2470   // Allocate kernel stack.
2471   if((p->kstack = kalloc()) == 0){
2472     p->state = UNUSED;
2473     return 0;
2474   }
2475   sp = p->kstack + KSTACKSIZE;
2476 
2477   // Leave room for trap frame.
2478   sp -= sizeof *p->tf;
2479   p->tf = (struct trapframe*)sp;
2480 
2481   // Set up new context to start executing at forkret,
2482   // which returns to trapret.
2483   sp -= 4;
2484   *(uint*)sp = (uint)trapret;
2485 
2486   sp -= sizeof *p->context;
2487   p->context = (struct context*)sp;
2488   memset(p->context, 0, sizeof *p->context);
2489   p->context->eip = (uint)forkret;
2490 
2491   return p;
2492 }
2493 
2494 
2495 
2496 
2497 
2498 
2499 
2500 // Set up first user process.
2501 void
2502 userinit(void)
2503 {
2504   struct proc *p;
2505   extern char _binary_initcode_start[], _binary_initcode_size[];
2506 
2507   acquire(&ptable.lock);
2508 
2509   p = allocproc();
2510 
2511   // release the lock in case namei() sleeps.
2512   // the lock isn't needed because no other
2513   // thread will look at an EMBRYO proc.
2514   release(&ptable.lock);
2515 
2516   initproc = p;
2517   if((p->pgdir = setupkvm()) == 0)
2518     panic("userinit: out of memory?");
2519   inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
2520   p->sz = PGSIZE;
2521   memset(p->tf, 0, sizeof(*p->tf));
2522   p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
2523   p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
2524   p->tf->es = p->tf->ds;
2525   p->tf->ss = p->tf->ds;
2526   p->tf->eflags = FL_IF;
2527   p->tf->esp = PGSIZE;
2528   p->tf->eip = 0;  // beginning of initcode.S
2529 
2530   safestrcpy(p->name, "initcode", sizeof(p->name));
2531   p->cwd = namei("/");
2532 
2533   // this assignment to p->state lets other cores
2534   // run this process. the acquire forces the above
2535   // writes to be visible, and the lock is also needed
2536   // because the assignment might not be atomic.
2537   acquire(&ptable.lock);
2538 
2539   p->state = RUNNABLE;
2540 
2541   release(&ptable.lock);
2542 }
2543 
2544 
2545 
2546 
2547 
2548 
2549 
2550 // Grow current process's memory by n bytes.
2551 // Return 0 on success, -1 on failure.
2552 int
2553 growproc(int n)
2554 {
2555   uint sz;
2556 
2557   sz = proc->sz;
2558   if(n > 0){
2559     if((sz = allocuvm(proc->pgdir, sz, sz + n)) == 0)
2560       return -1;
2561   } else if(n < 0){
2562     if((sz = deallocuvm(proc->pgdir, sz, sz + n)) == 0)
2563       return -1;
2564   }
2565   proc->sz = sz;
2566   switchuvm(proc);
2567   return 0;
2568 }
2569 
2570 // Create a new process copying p as the parent.
2571 // Sets up stack to return as if from system call.
2572 // Caller must set state of returned proc to RUNNABLE.
2573 int
2574 fork(void)
2575 {
2576   int i, pid;
2577   struct proc *np;
2578 
2579   acquire(&ptable.lock);
2580 
2581   // Allocate process.
2582   if((np = allocproc()) == 0){
2583     release(&ptable.lock);
2584     return -1;
2585   }
2586 
2587   release(&ptable.lock);
2588 
2589   // Copy process state from p.
2590   if((np->pgdir = copyuvm(proc->pgdir, proc->sz)) == 0){
2591     kfree(np->kstack);
2592     np->kstack = 0;
2593     np->state = UNUSED;
2594     release(&ptable.lock);
2595     return -1;
2596   }
2597   np->sz = proc->sz;
2598   np->parent = proc;
2599   *np->tf = *proc->tf;
2600   // Clear %eax so that fork returns 0 in the child.
2601   np->tf->eax = 0;
2602 
2603   for(i = 0; i < NOFILE; i++)
2604     if(proc->ofile[i])
2605       np->ofile[i] = filedup(proc->ofile[i]);
2606   np->cwd = idup(proc->cwd);
2607 
2608   safestrcpy(np->name, proc->name, sizeof(proc->name));
2609 
2610   pid = np->pid;
2611 
2612   acquire(&ptable.lock);
2613 
2614   np->state = RUNNABLE;
2615 
2616   release(&ptable.lock);
2617 
2618   return pid;
2619 }
2620 
2621 // Exit the current process.  Does not return.
2622 // An exited process remains in the zombie state
2623 // until its parent calls wait() to find out it exited.
2624 void
2625 exit(void)
2626 {
2627   struct proc *p;
2628   int fd;
2629 
2630   if(proc == initproc)
2631     panic("init exiting");
2632 
2633   // Close all open files.
2634   for(fd = 0; fd < NOFILE; fd++){
2635     if(proc->ofile[fd]){
2636       fileclose(proc->ofile[fd]);
2637       proc->ofile[fd] = 0;
2638     }
2639   }
2640 
2641   begin_op();
2642   iput(proc->cwd);
2643   end_op();
2644   proc->cwd = 0;
2645 
2646   acquire(&ptable.lock);
2647 
2648   // Parent might be sleeping in wait().
2649   wakeup1(proc->parent);
2650   // Pass abandoned children to init.
2651   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
2652     if(p->parent == proc){
2653       p->parent = initproc;
2654       if(p->state == ZOMBIE)
2655         wakeup1(initproc);
2656     }
2657   }
2658 
2659   // Jump into the scheduler, never to return.
2660   proc->state = ZOMBIE;
2661   sched();
2662   panic("zombie exit");
2663 }
2664 
2665 // Wait for a child process to exit and return its pid.
2666 // Return -1 if this process has no children.
2667 int
2668 wait(void)
2669 {
2670   struct proc *p;
2671   int havekids, pid;
2672 
2673   acquire(&ptable.lock);
2674   for(;;){
2675     // Scan through table looking for exited children.
2676     havekids = 0;
2677     for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
2678       if(p->parent != proc)
2679         continue;
2680       havekids = 1;
2681       if(p->state == ZOMBIE){
2682         // Found one.
2683         pid = p->pid;
2684         kfree(p->kstack);
2685         p->kstack = 0;
2686         freevm(p->pgdir);
2687         p->pid = 0;
2688         p->parent = 0;
2689         p->name[0] = 0;
2690         p->killed = 0;
2691         p->state = UNUSED;
2692         release(&ptable.lock);
2693         return pid;
2694       }
2695     }
2696 
2697 
2698 
2699 
2700     // No point waiting if we don't have any children.
2701     if(!havekids || proc->killed){
2702       release(&ptable.lock);
2703       return -1;
2704     }
2705 
2706     // Wait for children to exit.  (See wakeup1 call in proc_exit.)
2707     sleep(proc, &ptable.lock);  //DOC: wait-sleep
2708   }
2709 }
2710 
2711 
2712 
2713 
2714 
2715 
2716 
2717 
2718 
2719 
2720 
2721 
2722 
2723 
2724 
2725 
2726 
2727 
2728 
2729 
2730 
2731 
2732 
2733 
2734 
2735 
2736 
2737 
2738 
2739 
2740 
2741 
2742 
2743 
2744 
2745 
2746 
2747 
2748 
2749 
2750 // Per-CPU process scheduler.
2751 // Each CPU calls scheduler() after setting itself up.
2752 // Scheduler never returns.  It loops, doing:
2753 //  - choose a process to run
2754 //  - swtch to start running that process
2755 //  - eventually that process transfers control
2756 //      via swtch back to the scheduler.
2757 void
2758 scheduler(void)
2759 {
2760   struct proc *p;
2761 
2762   for(;;){
2763     // Enable interrupts on this processor.
2764     sti();
2765 
2766     // Loop over process table looking for process to run.
2767     acquire(&ptable.lock);
2768     for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
2769       if(p->state != RUNNABLE)
2770         continue;
2771 
2772       // Switch to chosen process.  It is the process's job
2773       // to release ptable.lock and then reacquire it
2774       // before jumping back to us.
2775       proc = p;
2776       switchuvm(p);
2777       p->state = RUNNING;
2778       swtch(&cpu->scheduler, p->context);
2779       switchkvm();
2780 
2781       // Process is done running for now.
2782       // It should have changed its p->state before coming back.
2783       proc = 0;
2784     }
2785     release(&ptable.lock);
2786 
2787   }
2788 }
2789 
2790 
2791 
2792 
2793 
2794 
2795 
2796 
2797 
2798 
2799 
2800 // Enter scheduler.  Must hold only ptable.lock
2801 // and have changed proc->state. Saves and restores
2802 // intena because intena is a property of this
2803 // kernel thread, not this CPU. It should
2804 // be proc->intena and proc->ncli, but that would
2805 // break in the few places where a lock is held but
2806 // there's no process.
2807 void
2808 sched(void)
2809 {
2810   int intena;
2811 
2812   if(!holding(&ptable.lock))
2813     panic("sched ptable.lock");
2814   if(cpu->ncli != 1)
2815     panic("sched locks");
2816   if(proc->state == RUNNING)
2817     panic("sched running");
2818   if(readeflags()&FL_IF)
2819     panic("sched interruptible");
2820   intena = cpu->intena;
2821   swtch(&proc->context, cpu->scheduler);
2822   cpu->intena = intena;
2823 }
2824 
2825 // Give up the CPU for one scheduling round.
2826 void
2827 yield(void)
2828 {
2829   acquire(&ptable.lock);  //DOC: yieldlock
2830   proc->state = RUNNABLE;
2831   sched();
2832   release(&ptable.lock);
2833 }
2834 
2835 // A fork child's very first scheduling by scheduler()
2836 // will swtch here.  "Return" to user space.
2837 void
2838 forkret(void)
2839 {
2840   static int first = 1;
2841   // Still holding ptable.lock from scheduler.
2842   release(&ptable.lock);
2843 
2844   if (first) {
2845     // Some initialization functions must be run in the context
2846     // of a regular process (e.g., they call sleep), and thus cannot
2847     // be run from main().
2848     first = 0;
2849     iinit(ROOTDEV);
2850     initlog(ROOTDEV);
2851   }
2852 
2853   // Return to "caller", actually trapret (see allocproc).
2854 }
2855 
2856 // Atomically release lock and sleep on chan.
2857 // Reacquires lock when awakened.
2858 void
2859 sleep(void *chan, struct spinlock *lk)
2860 {
2861   if(proc == 0)
2862     panic("sleep");
2863 
2864   if(lk == 0)
2865     panic("sleep without lk");
2866 
2867   // Must acquire ptable.lock in order to
2868   // change p->state and then call sched.
2869   // Once we hold ptable.lock, we can be
2870   // guaranteed that we won't miss any wakeup
2871   // (wakeup runs with ptable.lock locked),
2872   // so it's okay to release lk.
2873   if(lk != &ptable.lock){  //DOC: sleeplock0
2874     acquire(&ptable.lock);  //DOC: sleeplock1
2875     release(lk);
2876   }
2877 
2878   // Go to sleep.
2879   proc->chan = chan;
2880   proc->state = SLEEPING;
2881   sched();
2882 
2883   // Tidy up.
2884   proc->chan = 0;
2885 
2886   // Reacquire original lock.
2887   if(lk != &ptable.lock){  //DOC: sleeplock2
2888     release(&ptable.lock);
2889     acquire(lk);
2890   }
2891 }
2892 
2893 
2894 
2895 
2896 
2897 
2898 
2899 
2900 // Wake up all processes sleeping on chan.
2901 // The ptable lock must be held.
2902 static void
2903 wakeup1(void *chan)
2904 {
2905   struct proc *p;
2906 
2907   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
2908     if(p->state == SLEEPING && p->chan == chan)
2909       p->state = RUNNABLE;
2910 }
2911 
2912 // Wake up all processes sleeping on chan.
2913 void
2914 wakeup(void *chan)
2915 {
2916   acquire(&ptable.lock);
2917   wakeup1(chan);
2918   release(&ptable.lock);
2919 }
2920 
2921 // Kill the process with the given pid.
2922 // Process won't exit until it returns
2923 // to user space (see trap in trap.c).
2924 int
2925 kill(int pid)
2926 {
2927   struct proc *p;
2928 
2929   acquire(&ptable.lock);
2930   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
2931     if(p->pid == pid){
2932       p->killed = 1;
2933       // Wake process from sleep if necessary.
2934       if(p->state == SLEEPING)
2935         p->state = RUNNABLE;
2936       release(&ptable.lock);
2937       return 0;
2938     }
2939   }
2940   release(&ptable.lock);
2941   return -1;
2942 }
2943 
2944 
2945 
2946 
2947 
2948 
2949 
2950 // Print a process listing to console.  For debugging.
2951 // Runs when user types ^P on console.
2952 // No lock to avoid wedging a stuck machine further.
2953 void
2954 procdump(void)
2955 {
2956   static char *states[] = {
2957   [UNUSED]    "unused",
2958   [EMBRYO]    "embryo",
2959   [SLEEPING]  "sleep ",
2960   [RUNNABLE]  "runble",
2961   [RUNNING]   "run   ",
2962   [ZOMBIE]    "zombie"
2963   };
2964   int i;
2965   struct proc *p;
2966   char *state;
2967   uint pc[10];
2968 
2969   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
2970     if(p->state == UNUSED)
2971       continue;
2972     if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
2973       state = states[p->state];
2974     else
2975       state = "???";
2976     cprintf("%d %s %s", p->pid, state, p->name);
2977     if(p->state == SLEEPING){
2978       getcallerpcs((uint*)p->context->ebp+2, pc);
2979       for(i=0; i<10 && pc[i] != 0; i++)
2980         cprintf(" %p", pc[i]);
2981     }
2982     cprintf("\n");
2983   }
2984 }
2985 
2986 
2987 
2988 
2989 
2990 
2991 
2992 
2993 
2994 
2995 
2996 
2997 
2998 
2999 
