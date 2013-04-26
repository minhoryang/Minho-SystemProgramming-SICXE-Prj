#include <stdio.h>
#include <stdlib.h>

#include "core/hash.h"

#ifdef hash_test
	#include "core/list.c"

	int main(){
		return 0;
	}
#endif

struct hash *hash_init(hash_hash_func *hash, hash_less_func *less, void *aux){
	struct hash *h = (struct hash *)calloc(1, sizeof(struct hash));
	h->elem_cnt = 0;
	h->bucket_cnt = 20;
	h->buckets = (List *)calloc(h->bucket_cnt, sizeof(List));
	h->hash = hash;
	h->less = less;
	h->aux = aux;

	if(h->buckets != NULL){
		hash_clear(h, NULL);
		return h;
	}else{
		free(h->buckets);
		free(h);
		return NULL;
	}
}

void hash_clear(struct hash *h, hash_action_func *destructor){
	size_t i;
	for(i = 0; i < h->bucket_cnt; i++){
		struct list *bucket = &h->buckets[i];
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

void hash_destroy(struct hash *h, hash_action_func *destructor){
	if (destructor != NULL)
		hash_clear (h, destructor);
	free(h->buckets);
	free(h);
}

struct hash_elem *hash_insert(struct hash *h, struct hash_elem *new){
	struct list *bucket = find_bucket(h, new);
	struct hash_elem *old = find_elem (h, bucket, new);
	if(old == NULL)
		insert_elem(h, bucket, new);

	rehash(h);

	return old;
}

struct hash_elem *hash_find(struct hash *h, struct hash_elem *e){
	return find_elem(h, find_bucket(h, e), e);
}

void hash_apply(struct hash *h, hash_action_func *action){
	size_t i;
	for(i = 0; i < h->bucket_cnt; i++){
		struct list *bucket = &h->buckets[i];
		struct list_elem *elem, *next;
		for(elem = list_begin(bucket); elem != list_end(bucket); elem = next){
			next = list_next(elem);
			action(Elem2HElem(elem), h->aux);
		}
	}
}

struct list *find_bucket(struct hash *h, struct hash_elem *e){
	size_t bucket_idx = h->hash(e, h->aux) & (h->bucket_cnt -1);
	return &h->buckets[bucket_idx];
}

struct hash_elem *find_elem(struct hash *h, struct list *bucket, struct hash_elem *e){
	struct list_elem *i;
	for (i=list_begin(bucket); i != list_end(bucket); i = list_next(i)){
		struct hash_elem *hi = Elem2HElem(i);
		if(!h->less(hi, e, h->aux) && !h->less(e, hi, h->aux))
			return hi;
	}
	return NULL;
}

void insert_elem(struct hash *h, struct list *bucket, struct hash_elem *e){
	h->elem_cnt++;
	list_push_front(bucket, &e->e);
}

void rehash(struct hash *h){
  size_t old_bucket_cnt, new_bucket_cnt;
  struct list *new_buckets, *old_buckets;
  size_t i;

  old_buckets = h->buckets;
  old_bucket_cnt = h->bucket_cnt;

  new_bucket_cnt = h->elem_cnt / BEST_ELEMS_PER_BUCKET;
  if (new_bucket_cnt < 4)
    new_bucket_cnt = 4;
  while (!is_power_of_2 (new_bucket_cnt))
    new_bucket_cnt = turn_off_least_1bit (new_bucket_cnt);

  if (new_bucket_cnt == old_bucket_cnt)
    return;

  new_buckets = malloc (sizeof *new_buckets * new_bucket_cnt);
  if (new_buckets == NULL) 
    {
      return;
    }
  for (i = 0; i < new_bucket_cnt; i++) 
    list_init (&new_buckets[i]);

  h->buckets = new_buckets;
  h->bucket_cnt = new_bucket_cnt;

  for (i = 0; i < old_bucket_cnt; i++) 
    {
      struct list *old_bucket;
      struct list_elem *elem, *next;

      old_bucket = &old_buckets[i];
      for (elem = list_begin (old_bucket);
           elem != list_end (old_bucket); elem = next) 
        {
          struct list *new_bucket
            = find_bucket (h, Elem2HElem(elem));
          next = list_next (elem);
          list_remove (elem);
          list_push_front (new_bucket, elem);
        }
    }

  free (old_buckets);
}

size_t turn_off_least_1bit(size_t x){
	return x & (x-1);
}

size_t is_power_of_2(size_t x){
	return x != 0 && turn_off_least_1bit(x) == 0;
}
