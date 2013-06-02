#ifndef src_modules_shell
	#define src_modules_shell

	#include <stdbool.h>
	#include "core/list.h"
	#include "core/hash.h"
	#include "core/stringswitch.h"
	#include "modules/assembler.h"

	typedef struct {
		// Argument.h
		FILE *fin;
		// Tokenizer.h
		char *line, **tokens;
		size_t len_token;
		// StringSwitch.h
		StringSwitchSet *cmds;
		// Memory.h
		unsigned char *memory;
		size_t loc_memory;
		// list.h for history.
		List *history;
		// hash.h for OPTab, MNTab.
		Hash *OP, *MN;
		// ASSEMBLER
		CSECT *csect;
		List *asmdir;
		// LINKING LOADER
		size_t progaddr;
		List *loads;
		//
		bool was_clear;
	}Environment;

	Environment *Shell_AllocateEnvironment();
	void Shell_DeAllocateEnvironment(Environment *);
	int Shell_MainLoop(Environment *);
	void Shell_Help();
	void Shell_Exception(Environment *);
#endif
