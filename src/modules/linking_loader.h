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

	typedef struct {
		Elem elem;
		char *name;
		size_t num;
	}Ref;

	typedef struct {
		Elem elem;
		char *target_name;
		size_t addr;
		size_t bit;
		bool is_plus;
	}ModQueue;

	bool linking_loader_set_progaddr(char *, size_t *);
	bool linking_loader_loader(char *, void *, List *, size_t *, List *);
	void linking_loader_printer(List *);
	void linking_loader_dealloc(List *);
	size_t linking_loader_load_header(char *in, List *loads, size_t *base, size_t progaddr, List *ref);
	void linking_loader_load_define(char *in, List *loads, size_t base, size_t progaddr);
	void linking_loader_load_reserve(char *in, List *ref);
	void linking_loader_load_modify(char *in, List *ref, List *modq, size_t base, size_t progaddr);
	char *linking_loader_find_ref(size_t query, List *ref);
	void linking_loader_fill_modify(void *memory, List *loads, List *modq);
	void linking_loader_fill_modify_real(void *memory, ModQueue *nq, Load *nl);
	void linking_loader_load_text(char *in, void *memory, size_t base, size_t progaddr);
#endif
