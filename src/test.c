#include "param.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"
#include "memlayout.h"

#define OUT 1

int main(){
	int i;
	for(i = 0 ; i < 100; i++){
		fork((i % 10) + 1);
	}
	//while(1){
        //	p = wait();
        //	if(p < 0)
	//		break;
        //      	printf(OUT,"Filho %d finalisou\n",pid ); 
	//}
	//for(i = 0 ; i < 100; i++)	
//		exit();
	return 1;
}
