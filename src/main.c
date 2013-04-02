#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "core/argument.h"
#include "core/debug.h"
#include "modules/shell.h"

int main(int argc, char *argv[]){
	Environment *env = Shell_AllocateEnvironment();
	env->fin = argument_file_opener(argc, argv);  // open file.
	while(Shell_MainLoop(env)){}
	Shell_DeAllocateEnvironment(env);
	return 0;
}
