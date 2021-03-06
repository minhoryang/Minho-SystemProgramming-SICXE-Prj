#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/hex.h"
#include "modules/shell.h"
#include "modules/tokenizer.h"
#include "modules/directory.h"
#include "modules/memory.h"
#include "modules/history.h"
#include "modules/optab.h"
#include "modules/linking_loader.h"

#ifdef shell_test
	#include "core/argument.h"
	#include "modules/tokenizer.c"
	#include "modules/directory.c"
	#include "modules/memory.c"
	#include "modules/history.c"
	#include "modules/optab.c"
	#include "modules/assembler.c"
	#include "modules/linking_loader.c"

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
			"mnemoniclist",
			"assemble",
			"type",
			"symbol",
			"disassemble",
			"progaddr",
			"loader",
			"run",
			"bp"};
		env->cmds = AllocStringSwitchSet(cmds, 27);
	}
	{
		env->memory = (unsigned char *)calloc(MEM_MAX, sizeof(unsigned char));
		env->loc_memory = 0;
	}
	{
		env->history = (struct list *)calloc(1, sizeof(struct list));
		list_init((env->history));
	}
	{
		env->OP = OP_Alloc();
		env->MN = MN_Alloc();
		OPMN_Load(env->OP, env->MN, NULL);
	}
	{
		env->csect = NULL;
		env->asmdir = assembler_directives_load();
	}
	{
		env->loads = (struct list *)calloc(1, sizeof(struct list));
		list_init(env->loads);
	}
	return env;
}

void Shell_DeAllocateEnvironment(Environment *env){
	linking_loader_dealloc(env->loads);
	free(env->loads);
	if(env->csect)
		csect_dealloc(env->csect);
	assembler_directives_unload(env->asmdir);
	hash_destroy(env->OP, both_hash_destructor);
	hash_destroy(env->MN, NULL);
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
		env->len_token = Tokenizer(env->line, env->tokens, NONE);
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
			case 15:  // "opcode want-to-find-mnemonic"
				if(env->len_token == 2){
					OPMNNode *find = MN_Search(env->OP, env->tokens[1]);
					if(find != NULL)
						printf("opcode is %02X.\n", find->opcode);
					else
						Shell_Exception(env);
				}else
					Shell_Exception(env);
				break;
			case 16:  // "mnemonic want-to-find-opcode"
				if(env->len_token == 2){
					OPMNNode *find = OP_Search(env->MN, env->tokens[1]);
					if(find != NULL)
						printf("mnemonic is %s.\n", find->mnemonic);
					else
						Shell_Exception(env);
				}else
					Shell_Exception(env);
				break;
			case 17:  // "opcodelist"
				OP_List(env->OP);
				break;
			case 18:  // "mnemoniclist"
				MN_List(env->MN);
				break;
			case 19:  // "assemble"
				if(env->len_token == 2){
					if(env->csect)
						csect_dealloc(env->csect);
					if(!assembler_readline(env->tokens[1], (env->csect = csect_alloc()))){
						if(!assembler_pass1(env->csect, env->OP, env->asmdir)){
							if(!assembler_pass2(env->csect)){
								char *filename1 = strdup(env->csect->filename),
									 *filename2 = strdup(env->csect->filename);
								strcat(filename1, ".lst");
								strcat(filename2, ".obj");
								if(!assembler_make_lst(env->csect, filename1)){
									assembler_make_obj(env->csect, filename2);
									printf("\toutput file : [%s], [%s]\n", filename1, filename2);
								}else{
									remove(filename1);
									remove(filename2);
									Shell_Exception(env);
								}
							}else
								Shell_Exception(env);
						}else
							Shell_Exception(env);
					}else
						Shell_Exception(env);
				}else
					Shell_Exception(env);
				break;
			case 20:  // "type"
				if(env->len_token == 2){
					Directory_File_Type(env->tokens[1]);
				}else
					Shell_Exception(env);
				break;
			case 21:  // "symbol"
				if(env->csect){
					if(env->len_token == 1){
						symbol_view(env->csect->symtab);
					}else
						Shell_Exception(env);
				}else
					Shell_Exception(env);
				break;
			case 22:  // "disassemble"
				if(env->len_token == 2){
					if(disassembler(env->tokens[1], env->MN)){
						Shell_Exception(env);
					}
				}else
					Shell_Exception(env);
				break;
			case 23:  // "progaddr"
				if(env->len_token == 2){
					if(!linking_loader_set_progaddr(
								env->tokens[1], &env->progaddr))
						Shell_Exception(env);
				}else
					Shell_Exception(env);
				break;
			case 24:  // "loader"
				{
					linking_loader_dealloc(env->loads);
					bool isDone = false;
					{
						List *modq = (List *)calloc(1, sizeof(List));
						list_init(modq);
						size_t startAddr = env->progaddr;
						{
							size_t now;
								for(now = 1; now < env->len_token; now++){
								isDone = linking_loader_loader(
										env->tokens[now],
										env->memory,
										env->loads,
										&startAddr,
										modq);
								if(!isDone){
									printf("LOAD ERROR @ %s\n",
											env->tokens[now]);
									break;
								}
							}
						}
						linking_loader_fill_modify(env->memory, env->loads, modq);
						free(modq);  // TODO
					}
					if(!isDone)
						Shell_Exception(env);
					else
						linking_loader_printer(env->loads);
				}
				break;
			case 25:  // "run"
				printf("Not Implemented!\n");
				Shell_Exception(env);
				break;
			case 26:  // "bp"
				printf("Not Implemented!\n");
				Shell_Exception(env);
				break;
			default:
				Shell_Exception(env);
				break;
		}
	}

	// done.
	if(env->was_clear){
		History_Add(env->line, env->history);
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
	printf("opcode\n");
	printf("mnemonic\n");
	printf("opcodelist\n");
	printf("mnemoniclist\n");
	printf("assemble filename\n");
	printf("type filename\n");
	printf("symbol\n");
	printf("disassemble filename\n");
	printf("progaddr [address]\n");
	printf("loader [object filename1] [object filename2] [...]\n");
	printf("run\n");
	printf("bp [address]\n");
}

void Shell_Exception(Environment *env){
	env->was_clear = false;
	printf("Sorry, I can't handle you.\n");
}
