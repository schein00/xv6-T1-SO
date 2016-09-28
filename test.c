#include "param.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"
#include "memlayout.h"

int main(){
	int i;
	for(i = 0 ; i < 100; i++){
		fork((i % 10) + 1);
	}

}
