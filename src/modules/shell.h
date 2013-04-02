#ifndef src_modules_shell
	#define src_modules_shell

	#include <stdbool.h>
	#include "core/list.h"
	#include "core/stringswitch.h"

	typedef struct {
		// Argument.h
		FILE *fin;
		// Tokenizer.h
		char *line;
		char **tokens;
		size_t len_token;
		// StringSwitch.h
		StringSwitchSet *cmds;
		// Memory.h
		unsigned char *memory;
		size_t loc_memory;
		// list.h for history.
		List *history;
		bool was_clear;
	}Environment;

	Environment *Shell_AllocateEnvironment();
	void Shell_DeAllocateEnvironment(Environment *);
	int Shell_MainLoop(Environment *);
	void Shell_Help();
	void Shell_Exception(Environment *);
#endif
