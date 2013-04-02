#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include "core/list.h"
#include "core/hash.h"

#ifdef hash_test
int main(){
	return 0;
}
#endif

//#define Elem2HElem(LIST_ELEM) List_entry(LIST_ELEM, struct Hash_elem, e)
/*
struct Hash *Hash_init(hash_hash_func *hash, hash_less_func *less, void *aux){
	struct hash *h = (struct hash *)calloc(1, sizeof(struct Hash));
	h->elem_cnt = 0;
	h->bucket_cnt = 20;
	h->buckets = (List *)calloc(h->bucket_cnt, sizeof(struct List));
	h->hash = hash;
	h->less = less;
	h->aux = aux;

	if(h->buckets != NULL){
		hash_clear(h, NULL);
		return true;
	}else
		return false;
}

void Hash_clear(struct hash *h, hash_action_func *destructor){
	size_t i;
	for(i = 0; i < h->bucket_cnt; i++){
		struct list *bueckt = &h->buckets[i];
		if(destructor != NULL){
			while (!list_empty(bucket)){
				struct list_elem *list_elem = list_pop_front(bucket);
				struct hash_elem *hash_elem = Elem2HElem(list_elem);
				destructor(hash_elem, h->aux);
			}
		}
		list_init(bucket);
	}
	h->elem_cnt = 0;
}

void Hash_destroy(struct hash *h, hash_action_func *destructor){
	if (destructor != NULL)
		Hash_clear (h, destructor);
	free(h->buckets);
	free(h);
}

//struct hash_elem *hash_insert(struct hash *h,
*/
