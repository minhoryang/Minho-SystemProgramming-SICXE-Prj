#ifndef src_core_hash
	#define src_core_hash
	
	#include <stdbool.h>

	typedef struct Hash_elem{
		struct list_elem e;
	}HElem;

	typedef unsigned int hash_hash_func (const struct Hash_elem *e, void *aux);
	typedef bool hash_less_func(const struct Hash_elem *a, const struct Hash_elem *b, void *aux);

	typedef struct Hash{
		size_t elem_cnt;
		size_t bucket_cnt;
		struct List *buckets;
		hash_hash_func *hash;
		hash_less_func *less;
		void *aux;
	}Hash;
	struct Hash_iterator{
		struct Hash *hash;
		struct List *bucket;
		struct Hash_elem *elem;
	};

	#define Hash_entry(HASH_ELEM, STRUCT, MEMBER) ((STRUCT *) ((uint8_t *) &(HASH_ELEM)->e - offsetof (STRUCT, MEMBER.e)))

#endif
