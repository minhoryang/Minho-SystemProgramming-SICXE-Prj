#ifndef src_modules_shell
	#define src_modules_shell

	void *Shell_AllocateEnvironment();
	void Shell_DeAllocateEnvironment(void *environment);
	int Shell_MainLoop(void *environment);
#endif
