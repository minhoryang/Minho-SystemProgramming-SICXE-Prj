#ifndef src_modules_linking_loader
	#define src_modules_linking_loader

	#include <stdio.h>
	#include <stdlib.h>
	#include <stdbool.h>
	#include "core/hex.h"
	#include "core/list.h"

	typedef enum {ctrlsec = 0, symbol} loader_type;
	typedef struct {
		Elem elem;
		loader_type Type;
		char *Name;
		size_t Addr;
		size_t Length;
	}Load;

	bool linking_loader_set_progaddr(char *, size_t *);
	bool linking_loader_loader(char *, void *, List *, size_t *);
	void linking_loader_printer(List *);
	void linking_loader_dealloc(List *);
#endif
