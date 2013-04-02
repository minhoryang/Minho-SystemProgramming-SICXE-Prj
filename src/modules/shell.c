#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/hex.h"
#include "modules/shell.h"
#include "modules/tokenizer.h"
#include "modules/directory.h"
#include "modules/memory.h"
#include "modules/history.h"

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
	{
		env->line = (char *)calloc(Tokenizer_Max_Length, sizeof(char));
		env->tokens = AllocToken();
		env->len_token = 0;
	}
	{
		char *cmds[] = {
			"help", "h",
			"dir", "d",
			"quit", "q",
			"history", "hi",
			"dump", "du",
			"edit", "e",
			"fill", "f",
			"reset",
			"opcode",
			"mnemonic",
			"opcodelist",
			"mnemoniclist"};
		env->cmds = AllocStringSwitchSet(cmds, 19);
	}
	{
		env->memory = (unsigned char *)calloc(MEM_MAX, sizeof(unsigned char));
		env->loc_memory = 0;
	}
	{
		env->history = (struct list *)calloc(1, sizeof(struct list));
		list_init((env->history));
	}
	return env;
}

void Shell_DeAllocateEnvironment(Environment *env){
	History_DeAlloc(env->history);
	DeAllocStringSwitchSet(env->cmds);
	DeAllocToken(env->tokens);
	if(env->fin)
		fclose(env->fin);
	free(env->history);
	free(env->memory);
	free(env->line);
	free(env);
}

int Shell_MainLoop(Environment *env){
	// Init
	env->was_clear = true;
	printf("sicsim> ");

	// Handler
	{
		fgets(env->line, Tokenizer_Max_Length, env->fin);
		env->len_token = Tokenizer(env->line, env->tokens);
		StringSwitch(env->cmds, env->tokens[0]){
			case 0:  // "help"
			case 1:  // "h"
				if(env->len_token == 1)
					Shell_Help();
				else
					Shell_Exception(env);
				break;
			case 2:  // "dir"
			case 3:  // "d"
				if(env->len_token == 1)
					Directory_Print(".");
				//else if(env->len_token == 2)
				//	Directory_Print(env->tokens[1]);  // TODO : Fix this!
				else
					Shell_Exception(env);
				break;
			case 4:  // "quit"
			case 5:  // "q"
				if(env->len_token == 1)
					return 0;
				else
					Shell_Exception(env);
				break;
			case 6:  // "history"
			case 7:  // "hi"
				if(env->len_token == 1)
					History_View(env->history);
				else
					Shell_Exception(env);
				break;
			case 8:  // "dump"
			case 9:  // "du"
				if(env->len_token == 1){
					env->loc_memory = memory_dump(env->memory, env->loc_memory, env->loc_memory + TEN_LINES) + 1;
				}else if(env->len_token == 2){
					size_t from = hex2int(env->tokens[1]);
					env->loc_memory = memory_dump(env->memory, from, from + TEN_LINES) + 1;
				}else if(env->len_token == 3){
					size_t from = hex2int(env->tokens[1]),
						   to = hex2int(env->tokens[2]);
					env->loc_memory = memory_dump(env->memory, from, to) + 1;
				}else
					Shell_Exception(env);
				break;
			case 10:  // "edit"
			case 11:  // "e"
				if(env->len_token == 3){
					size_t addr = hex2int(env->tokens[1]),
						   value = hex2int(env->tokens[2]);
					memory_edit(env->memory, addr, value);
				}else
					Shell_Exception(env);
				break;
			case 12:  // "fill"
			case 13:  // "f"
				if(env->len_token == 4){
					size_t from = hex2int(env->tokens[1]),
						   to = hex2int(env->tokens[2]),
						   value = hex2int(env->tokens[3]);
					memory_fill(env->memory, from, to, value);
				}else
					Shell_Exception(env);
				break;
			case 14:  // "reset"
				if(env->len_token == 1){
					memory_reset(env->memory);
				}else
					Shell_Exception(env);
				break;
			case 15:  // "opcode"
				break;
			case 16:  // "mnemonic"
				break;
			case 17:  // "opcodelist"
				break;
			case 18:  // "mnemoniclist"
				break;
			default:
				Shell_Exception(env);
				break;
		}
	}

	// done.
	if(env->was_clear){
		History_Add(env);
	}
	return !0;
}

void Shell_Help(){
	printf("h[elp]\n");
	printf("d[ir]\n");
	printf("q[uit]\n");
	printf("hi[story]\n");
	printf("du[mp] [start, end]\n");
	printf("e[dir] address, value\n");
	printf("f[ill] start, end, value\n");
	printf("reset\n");
	printf("opcode mnemonic\n");
	printf("opcodelist\n");
	printf("mnemoniclist\n");
}

void Shell_Exception(Environment *env){
	env->was_clear = false;
	printf("Sorry, I can't handle you.\n");
}
