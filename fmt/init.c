8550 // init: The initial user-level program
8551 
8552 #include "types.h"
8553 #include "stat.h"
8554 #include "user.h"
8555 #include "fcntl.h"
8556 
8557 char *argv[] = { "sh", 0 };
8558 
8559 int
8560 main(void)
8561 {
8562   int pid, wpid;
8563 
8564   if(open("console", O_RDWR) < 0){
8565     mknod("console", 1, 1);
8566     open("console", O_RDWR);
8567   }
8568   dup(0);  // stdout
8569   dup(0);  // stderr
8570 
8571   for(;;){
8572     printf(1, "init: starting sh\n");
8573     pid = fork();
8574     if(pid < 0){
8575       printf(1, "init: fork failed\n");
8576       exit();
8577     }
8578     if(pid == 0){
8579       exec("sh", argv);
8580       printf(1, "init: exec sh failed\n");
8581       exit();
8582     }
8583     while((wpid=wait()) >= 0 && wpid != pid)
8584       printf(1, "zombie!\n");
8585   }
8586 }
8587 
8588 
8589 
8590 
8591 
8592 
8593 
8594 
8595 
8596 
8597 
8598 
8599 
