#ifndef src_modules_shell
	#define src_modules_shell

	typedef struct {
		char *line;
		char **tokens;
		size_t len_token;
		FILE *fin;
		StringSwitchSet *cmds;
	}Environment;

	Environment *Shell_AllocateEnvironment();
	void Shell_DeAllocateEnvironment(Environment *);
	int Shell_MainLoop(Environment *);
#endif
