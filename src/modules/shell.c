#include <stdio.h>
#include <stdlib.h>

#include "modules/shell.h"
#include "modules/tokenizer.h"

#ifdef shell_test
	#include "core/argument.h"

	int main(int argc, char *argv[]){
		// The main function for Test-driven development (TDD).
		Environment *env = Shell_AllocateEnvironment();
		env->fin = argument_file_opener(argc, argv);  // open file.
		while(Shell_MainLoop(env)){}
		Shell_DeAllocateEnvironment(env);
		return 0;
	}
#endif

Environment *Shell_AllocateEnvironment(){
	Environment *env = (Environment *)calloc(1, sizeof(Environment));
	env->line = (char *)calloc(Tokenizer_Max_Length, sizeof(char));
	env->tokens = AllocToken();
	return env;
}

void Shell_DeAllocateEnvironment(Environment *env){
	DeAllocToken(env->tokens);
	free(env->line);
	free(env);
}

int Shell_MainLoop(Environment *env){
	printf("sicsim> ");
	{
		size_t i;
		fgets(env->line, Tokenizer_Max_Length, env->fin);
		for(i = 0; i < 0; i++){
			;
		}
	}
	return !0;
}
