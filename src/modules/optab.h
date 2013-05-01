#ifndef src_modules_optab
	#define src_modules_optab

	#include "core/hash.h"

	typedef struct {
		char mnemonic[10];
		unsigned char opcode;
		HElem op_elem;
		HElem mn_elem;
		//
		bool can_format[4];
	}OPMNNode;

	Hash *OP_Alloc();
	Hash *MN_Alloc();
	unsigned int op_hash_func(const struct hash_elem *e, void *aux);
	unsigned int mn_hash_func(const struct hash_elem *e, void *aux);
	void both_hash_destructor(const struct hash_elem *e, void *aux);
	bool op_less_func(
			const struct hash_elem *a,
			const struct hash_elem *b,
			void *aux);
	bool mn_less_func(
			const struct hash_elem *a,
			const struct hash_elem *b,
			void *aux);
	void OPMN_Load(Hash *, Hash *);
	void OPMN_Insert(Hash *, Hash *, char *opcode, char *mnemonic, bool *);
	void OP_List(Hash *what);
	void MN_List(Hash *what);
	OPMNNode *MN_Search(Hash *what, char *mnemonic);
	OPMNNode *OP_Search(Hash *what, char *opcode);

#endif
