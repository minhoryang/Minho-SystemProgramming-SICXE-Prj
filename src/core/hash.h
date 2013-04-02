#ifndef src_core_hash
	#define src_core_hash
	
	#include <stdbool.h>
	#include <stdint.h>
	#include <stddef.h>
	#include "core/list.h"

	typedef struct hash_elem{
		struct list_elem e;
	}HElem;

	typedef unsigned int hash_hash_func (
			const struct hash_elem *e, void *aux);

	typedef bool hash_less_func(
			const struct hash_elem *a,
			const struct hash_elem *b,
			void *aux);

	typedef void hash_action_func(
			const struct hash_elem *e, void *aux);

	typedef struct hash{
		size_t elem_cnt;
		size_t bucket_cnt;
		List *buckets;
		hash_hash_func *hash;
		hash_less_func *less;
		void *aux;
	}Hash;
	typedef struct hash_iterator{
		struct hash *hash;
		List *bucket;
		struct hash_elem *elem;
	}HashIter;

	#define hash_entry(HASH_ELEM, STRUCT, MEMBER) ((STRUCT *) ((uint8_t *) &(HASH_ELEM)->e - offsetof (STRUCT, MEMBER.e)))

	#define Elem2HElem(LIST_ELEM) list_entry(LIST_ELEM, struct hash_elem, e)

	struct hash *hash_init(hash_hash_func *hash, hash_less_func *less, void *aux);
	void hash_clear(struct hash *h, hash_action_func *destructor);
	void hash_destroy(struct hash *h, hash_action_func *destructor);

	struct hash_elem *hash_insert(struct hash *h, struct hash_elem *new);
	struct hash_elem *hash_find(struct hash *h, struct hash_elem *e);
	void hash_apply(struct hash *h, hash_action_func *action);
	struct list *find_bucket(struct hash *h, struct hash_elem *e);
	struct hash_elem *find_elem(struct hash *h, struct list *bucket, struct hash_elem *e);
	void insert_elem(struct hash *h, struct list *bucket, struct hash_elem *e);

	#define BEST_ELEMS_PER_BUCKET 2
	void rehash(struct hash *h);
	size_t turn_off_least_1bit(size_t x);
	size_t is_power_of_2(size_t x);
#endif
