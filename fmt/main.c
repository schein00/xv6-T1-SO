1300 #include "types.h"
1301 #include "defs.h"
1302 #include "param.h"
1303 #include "memlayout.h"
1304 #include "mmu.h"
1305 #include "proc.h"
1306 #include "x86.h"
1307 
1308 static void startothers(void);
1309 static void mpmain(void)  __attribute__((noreturn));
1310 extern pde_t *kpgdir;
1311 extern char end[]; // first address after kernel loaded from ELF file
1312 
1313 // Bootstrap processor starts running C code here.
1314 // Allocate a real stack and switch to it, first
1315 // doing some setup required for memory allocator to work.
1316 int
1317 main(void)
1318 {
1319   kinit1(end, P2V(4*1024*1024)); // phys page allocator
1320   kvmalloc();      // kernel page table
1321   mpinit();        // detect other processors
1322   lapicinit();     // interrupt controller
1323   seginit();       // segment descriptors
1324   cprintf("\ncpu%d: starting xv6\n\n", cpunum());
1325   picinit();       // another interrupt controller
1326   ioapicinit();    // another interrupt controller
1327   consoleinit();   // console hardware
1328   uartinit();      // serial port
1329   pinit();         // process table
1330   tvinit();        // trap vectors
1331   binit();         // buffer cache
1332   fileinit();      // file table
1333   ideinit();       // disk
1334   if(!ismp)
1335     timerinit();   // uniprocessor timer
1336   startothers();   // start other processors
1337   kinit2(P2V(4*1024*1024), P2V(PHYSTOP)); // must come after startothers()
1338   userinit();      // first user process
1339   mpmain();        // finish this processor's setup
1340 }
1341 
1342 
1343 
1344 
1345 
1346 
1347 
1348 
1349 
1350 // Other CPUs jump here from entryother.S.
1351 static void
1352 mpenter(void)
1353 {
1354   switchkvm();
1355   seginit();
1356   lapicinit();
1357   mpmain();
1358 }
1359 
1360 // Common CPU setup code.
1361 static void
1362 mpmain(void)
1363 {
1364   cprintf("cpu%d: starting\n", cpunum());
1365   idtinit();       // load idt register
1366   xchg(&cpu->started, 1); // tell startothers() we're up
1367   scheduler();     // start running processes
1368 }
1369 
1370 pde_t entrypgdir[];  // For entry.S
1371 
1372 // Start the non-boot (AP) processors.
1373 static void
1374 startothers(void)
1375 {
1376   extern uchar _binary_entryother_start[], _binary_entryother_size[];
1377   uchar *code;
1378   struct cpu *c;
1379   char *stack;
1380 
1381   // Write entry code to unused memory at 0x7000.
1382   // The linker has placed the image of entryother.S in
1383   // _binary_entryother_start.
1384   code = P2V(0x7000);
1385   memmove(code, _binary_entryother_start, (uint)_binary_entryother_size);
1386 
1387   for(c = cpus; c < cpus+ncpu; c++){
1388     if(c == cpus+cpunum())  // We've started already.
1389       continue;
1390 
1391     // Tell entryother.S what stack to use, where to enter, and what
1392     // pgdir to use. We cannot use kpgdir yet, because the AP processor
1393     // is running in low  memory, so we use entrypgdir for the APs too.
1394     stack = kalloc();
1395     *(void**)(code-4) = stack + KSTACKSIZE;
1396     *(void**)(code-8) = mpenter;
1397     *(int**)(code-12) = (void *) V2P(entrypgdir);
1398 
1399     lapicstartap(c->apicid, V2P(code));
1400     // wait for cpu to finish mpmain()
1401     while(c->started == 0)
1402       ;
1403   }
1404 }
1405 
1406 // The boot page table used in entry.S and entryother.S.
1407 // Page directories (and page tables) must start on page boundaries,
1408 // hence the __aligned__ attribute.
1409 // PTE_PS in a page directory entry enables 4Mbyte pages.
1410 
1411 __attribute__((__aligned__(PGSIZE)))
1412 pde_t entrypgdir[NPDENTRIES] = {
1413   // Map VA's [0, 4MB) to PA's [0, 4MB)
1414   [0] = (0) | PTE_P | PTE_W | PTE_PS,
1415   // Map VA's [KERNBASE, KERNBASE+4MB) to PA's [0, 4MB)
1416   [KERNBASE>>PDXSHIFT] = (0) | PTE_P | PTE_W | PTE_PS,
1417 };
1418 
1419 
1420 
1421 
1422 
1423 
1424 
1425 
1426 
1427 
1428 
1429 
1430 
1431 
1432 
1433 
1434 
1435 
1436 
1437 
1438 
1439 
1440 
1441 
1442 
1443 
1444 
1445 
1446 
1447 
1448 
1449 
1450 // Blank page.
1451 
1452 
1453 
1454 
1455 
1456 
1457 
1458 
1459 
1460 
1461 
1462 
1463 
1464 
1465 
1466 
1467 
1468 
1469 
1470 
1471 
1472 
1473 
1474 
1475 
1476 
1477 
1478 
1479 
1480 
1481 
1482 
1483 
1484 
1485 
1486 
1487 
1488 
1489 
1490 
1491 
1492 
1493 
1494 
1495 
1496 
1497 
1498 
1499 
