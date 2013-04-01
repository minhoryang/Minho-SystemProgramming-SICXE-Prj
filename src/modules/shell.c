#include <stdio.h>
#include <stdlib.h>

#include "core/stringswitch.h"
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
	char *cmds[] = {"quit", "q"};
	Environment *env = (Environment *)calloc(1, sizeof(Environment));
	env->line = (char *)calloc(Tokenizer_Max_Length, sizeof(char));
	env->tokens = AllocToken();
	env->cmds = AllocStringSwitchSet(cmds, 2);
	return env;
}

void Shell_DeAllocateEnvironment(Environment *env){
	DeAllocStringSwitchSet(env->cmds);
	DeAllocToken(env->tokens);
	free(env->line);
	free(env);
}

int Shell_MainLoop(Environment *env){
	printf("sicsim> ");
	{
		fgets(env->line, Tokenizer_Max_Length, env->fin);
		env->len_token = Tokenizer(env->line, env->tokens);
		StringSwitch(env->cmds, env->tokens[0]){
			case 0:  // "quit"
			case 1:  // "q"
				return 0;
				break;
			default:
				break;
		}
	}
	return !0;
}

