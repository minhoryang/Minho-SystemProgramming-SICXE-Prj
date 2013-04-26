#ifndef src_modules_history
	#define src_modules_history

	#include "core/list.h"
	#include "modules/shell.h"

	typedef struct {
		char *line;
		Elem elem;
	}History;

	void History_Add(char *, List *);
	void History_View(List *);
	void History_DeAlloc(List *);
#endif
