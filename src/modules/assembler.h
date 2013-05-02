#ifndef src_modules_assembler
	#define src_modules_assembler
	
	#include "core/list.h"
	#include "modules/optab.h"

	typedef struct {
  #ifdef SICXE
		bool _N_;
		bool _I_;
  #endif
		bool _X_;
  #ifdef SICXE
		bool _B_;
		bool _P_;
		bool _E_;
  #endif
	}Flag;

	typedef struct _DOCUMENT DOCUMENT;
	typedef struct _SYMBOL SYMBOL;

	typedef struct _NODE{
		DOCUMENT *_PARENT;
		Elem elem;

		char *token_orig;
		char **token_pass;
		size_t token_cnt;
		size_t LINE_NUM;
		size_t LOCATION_CNT;
		size_t _size;

		bool RESERVED;
		bool COMMENTED;
		SYMBOL *Symbol;
		OPMNNode *OPCODE;
		char *STORED_DATA;
		SYMBOL *DISP;
		char *OBJECTCODE;

		Flag FLAGS;
		void (*GetSize)(struct _NODE *this);
	}NODE;

	struct _SYMBOL{
		bool SAVED;
		Elem elem;
		char *symbol;
		NODE *link;
	};

	struct _DOCUMENT{
		size_t base;
		char *filename;
		char *progname;
		size_t prev_locctr;
		size_t end_addr;
		List *nodes;
		List *symtab;
		NODE *cur_node;
	};

	typedef void (*ASMDirFunc)(DOCUMENT *, void *);
	typedef struct {
		Elem elem;
		char name[7];
		ASMDirFunc apply;
	}ASMDir;


	void assembler_readline(char *, DOCUMENT *);
	void assembler_pass1(DOCUMENT *, Hash *, List *);
	void assembler_pass2(DOCUMENT *, char *);
	void assembler_pass3(DOCUMENT *, char *);
	void assembler_pass3_print(FILE *fp, Elem *start, Elem *end, size_t cnt);

	DOCUMENT *document_alloc();
	void document_dealloc(DOCUMENT *);
	NODE *node_alloc();
	void node_dealloc(NODE *);

#endif
#ifndef src_modules_assembler_directives
	#define src_modules_assembler_directives
	
	#include "core/list.h"
	#include "core/hex.h"

	List *assembler_directives_load();
	void assembler_directives_unload(List *);
	ASMDir *assembler_directives_search(List *, char *);

	void assembler_directives_START(DOCUMENT *, void *);
	void assembler_directives_END(DOCUMENT *, void *);
	void assembler_directives_BYTE(DOCUMENT *, void *);
	void assembler_directives_WORD(DOCUMENT *, void *);
	void assembler_directives_RESB(DOCUMENT *, void *);
	void assembler_directives_RESW(DOCUMENT *, void *);
#endif
#ifndef src_modules_assembler_symbol
	#define src_modules_assembler_symbol
	SYMBOL *symbol_add(List *, char *, NODE *, bool);
	SYMBOL *symbol_search(List *, char *);
#endif
