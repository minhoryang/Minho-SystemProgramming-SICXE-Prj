#include <stdio.h>
#include <stdlib.h>

#include "modules/shell.h"

#ifdef shell_test
	int main(){
		void *env = Shell_AllocateEnvironment();
		while(Shell_MainLoop(env)){}
		Shell_DeAllocateEnvironment(env);
		return 0;
	}
#endif

void *Shell_AllocateEnvironment(){
	return NULL;
}

void Shell_DeAllocateEnvironment(void *environment){
	;
}

int Shell_MainLoop(void *environment){
	
	printf("sicsim> \n");
	return !0;
}
