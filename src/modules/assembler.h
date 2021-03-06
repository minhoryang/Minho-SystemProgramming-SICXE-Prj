#define DEBUG_PRINT false  // TODO : DO NOT COMMIT WITH 'true'!!!!!!!!!!!!!

#ifndef src_modules_assembler
	#define src_modules_assembler
	
	#include "core/list.h"
	#include "modules/optab.h"

	typedef struct _FLAG FLAG;
	typedef struct _NODE NODE;
	typedef struct _CSECT CSECT;
	typedef struct _DATA DATA;
	typedef struct _MODIFY MODIFY;
	typedef struct _SYMBOL SYMBOL;
	typedef struct _BLOCK BLOCK;
	typedef struct _LITERAL LITERAL;
	typedef struct _LITERAL_USER LITUSER;
	typedef struct _ASMDir ASMDir;
	typedef enum { NotSet = 0, Set, UseSymbol } ThreeStates;
	typedef enum { FAILED = -1, Integer, Symbol, Literal} From;
	typedef void (*ASMDirFunc)(CSECT *);
	#define CUR2(A, B) (A->token_pass[A->cur_token + B])
	#define CUR(A) CUR2(A, 0)

	struct _FLAG{
		bool _N_,
			 _I_,
			 _X_,
			 _B_,
			 _P_,
			 _E_;
		bool RESERVED_SO_JMP_OBJ;  // NO NEED TO PRINT!
		bool COMMENTED_SO_JMP_LST;
	};

	struct _NODE{
		Elem elem;

		// for Tokenizer.
		char *token_orig,
			 **token_pass;
		size_t token_cnt,
			   cur_token;

		// for PASS 1
		size_t LINE_NUM,
			   LOCATION_CNT,
			   _size;
		
		// for PASS 2
		char *STORED_DATA,
			 *OBJECTCODE;

		BLOCK *_PARENT;
		SYMBOL *Symbol;
		LITERAL *Literal;
		OPMNNode *OPCODE;
		FLAG FLAGS;

	};

	struct _SYMBOL{
		Elem elem;
		char *symbol;
		NODE *link;
		// for EQU.
		bool is_equ;
		int equ;
	};

	struct _BLOCK{
		CSECT *_PARENT;
		Elem elem;
		char *NAME;
		size_t ID;
		size_t SIZE;
		size_t BASE;

		List *nodes;
		NODE *cur_node;

		size_t prev_locctr;
	};

	struct _CSECT{
		// for Base.
		size_t base;
		ThreeStates is_base;
		NODE *to_base;

		char *filename,
			 *progname;
		size_t start_addr, end_addr;

		List *blocks,
			 *symtab,
			 *datas,
			 *modtab,
			 *littab;

		// for Current.
		BLOCK *cur_block;
		size_t prev_base;
	};

	struct _DATA{
		Elem elem;
		size_t wanted;
		From where;
	};

	struct _MODIFY_DATA{
		bool is_plus;
	};
	struct _MODIFY{
		Elem elem;
		NODE *target;
		struct _MODIFY_DATA *more;
	};

	struct _LITERAL{
		Elem elem;
		char *name;
		bool done;
		NODE *where;
	};

	struct _ASMDir{
		Elem elem;
		char name[7];
		ASMDirFunc apply;
	};

	bool assembler_readline(char *, CSECT *);
	bool IsNumberOnly(char * input);
	bool assembler_pass1(CSECT *, Hash *, List *);
	bool assembler_pass2(CSECT *csect);
	size_t assembler_pass2_set_flag(NODE *now);
	void assembler_pass2_object_print(NODE *now, int data);
	void assembler_pass2_debug_print(NODE *now);
	int assembler_get_value_from_register(char *this);
	DATA *assembler_get_value_from_symbol_or_not(CSECT *, char *);
	bool assembler_make_lst(CSECT *, char *);
	void assembler_make_obj(CSECT *, char *);
	void assembler_obj_range_print(FILE *fp, Elem *start, Elem *end, size_t cnt);

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

	void assembler_directives_START(CSECT *);
	void assembler_directives_END(CSECT *);
	void assembler_directives_BYTE(CSECT *);
	void assembler_directives_WORD(CSECT *);
	void assembler_directives_RESB(CSECT *);
	void assembler_directives_RESW(CSECT *);
	void assembler_directives_BASE(CSECT *);
	bool assembler_directives_BASE_TO_BE(CSECT *, bool);
	void assembler_directives_EQU(CSECT *);
	void assembler_directives_LTORG(CSECT *);
	void assembler_directives_USE(CSECT *);
	void assembler_directives_ORG(CSECT *);
	int plus_minus_shit_parade(CSECT *);  // TODO MOVE!
#endif
#ifndef src_modules_assembler_symbol
	#define src_modules_assembler_symbol
	SYMBOL *symbol_add(List *, char *, NODE *);
	SYMBOL *symbol_search(List *, char *);
	bool symbol_less_func(const struct list_elem *a, const struct list_elem *b, void *aux);
	void symbol_view(List *symtab);
#endif
#ifndef src_modules_disassembler
	#define src_modules_disassembler
	bool disassembler(char *filename, Hash *mn);
#endif
#ifndef src_modules_assembler_literal
	#define src_modules_assembler_literal
	LITERAL *literal_add(List *littab, char *query);
	LITERAL *literal_search(List *littab, char *query);
	LITERAL *literal_detect(CSECT *csect);
	void literal_flush(CSECT *csect);
	LITERAL *literal_dealloc(CSECT *csect);
#endif
#ifndef src_modules_assembler_block
	#define src_modules_assembler_block
	BLOCK *block_alloc(size_t ID, char *name);
	char *block_detect(NODE *node);
	BLOCK *block_search(List *blocks, char *query);
	void block_dealloc(BLOCK *block);
#endif
#ifndef src_modules_assembler_csect
	#define src_modules_assembler_csect
	CSECT *csect_alloc();
	void csect_dealloc(CSECT *);
#endif
