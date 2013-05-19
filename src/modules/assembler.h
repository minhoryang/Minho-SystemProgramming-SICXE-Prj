#define DEBUG_PRINT false  // TODO : DO NOT COMMIT WITH 'true'!!!!!!!!!!!!!

#ifndef src_modules_assembler
	#define src_modules_assembler
	
	#include "core/list.h"
	#include "modules/optab.h"

	typedef struct _FLAG FLAG;
	typedef struct _NODE NODE;
	typedef struct _DOCUMENT DOCUMENT;
	typedef struct _DATA DATA;
	typedef struct _MODIFY MODIFY;
	typedef struct _SYMBOL SYMBOL;
	typedef struct _LITERAL LITERAL;
	typedef struct _LITERAL_USER LITUSER;
	typedef struct _ASMDir ASMDir;
	typedef enum { NotSet = 0, Set, UseSymbol } ThreeStates;
	typedef enum { FAILED = -1, Integer, Symbol, Literal} From;
	typedef void (*ASMDirFunc)(DOCUMENT *);
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

		DOCUMENT *_PARENT;  // TODO
		SYMBOL *Symbol;
		LITERAL *Literal;
		OPMNNode *OPCODE;
		FLAG FLAGS;

#ifdef DEPRECATED_SIC
		SYMBOL *DISP;
#endif

	};

	struct _SYMBOL{
		Elem elem;
		char *symbol;
		NODE *link;
	};

	struct _DOCUMENT{
		// for Base.
		size_t base;
		ThreeStates is_base;
		NODE *to_base;

		char *filename,
			 *progname;
		size_t start_addr, end_addr;

		List *nodes,
			 *symtab,
			 *datas,
			 *modtab,
			 *littab;

		// for Current.
		NODE *cur_node;
		size_t prev_locctr;
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

	bool assembler_readline(char *, DOCUMENT *);
	bool IsNumberOnly(char * input);
	bool assembler_pass1(DOCUMENT *, Hash *, List *);
#ifdef DEPRECATED_SIC
		//bool assembler_pass1_got_opcode_check_disp(DOCUMENT *doc, NODE *now);
#endif
	bool assembler_pass2(DOCUMENT *doc);
	size_t assembler_pass2_set_flag(NODE *now);
	void assembler_pass2_object_print(NODE *now, int data);
	void assembler_pass2_debug_print(NODE *now);
	int assembler_get_value_from_register(char *this);
	DATA *assembler_get_value_from_symbol_or_not(DOCUMENT *, char *);
	bool assembler_make_lst(DOCUMENT *, char *);
	void assembler_make_obj(DOCUMENT *, char *);
	void assembler_obj_range_print(FILE *fp, Elem *start, Elem *end, size_t cnt);

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

	void assembler_directives_START(DOCUMENT *);
	void assembler_directives_END(DOCUMENT *);
	void assembler_directives_BYTE(DOCUMENT *);
	void assembler_directives_WORD(DOCUMENT *);
	void assembler_directives_RESB(DOCUMENT *);
	void assembler_directives_RESW(DOCUMENT *);
	void assembler_directives_BASE(DOCUMENT *);
	void assembler_directives_BASE_TO_BE(DOCUMENT *, bool);
	void assembler_directives_EQU(DOCUMENT *);
	void assembler_directives_LTORG(DOCUMENT *);
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
	LITERAL *literal_detect(DOCUMENT *doc);
	void literal_flush(DOCUMENT *doc);
	LITERAL *literal_dealloc(DOCUMENT *doc);
#endif

