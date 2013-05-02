#ifndef src_core_list
	#define src_core_list
	
	#include <stdbool.h>
	#include <stddef.h>
	#include <stdint.h>

	typedef struct list_elem{
		struct list_elem *prev;
		struct list_elem *next;
	}Elem;
	typedef struct list{
		struct list_elem head;
		struct list_elem tail;
	}List;

	typedef bool list_less_func (
			const struct list_elem *a,
			const struct list_elem *b,
			void *aux);

	#define list_entry(LIST_ELEM, STRUCT, MEMBER) ((STRUCT *) ((uint8_t *) &(LIST_ELEM)->next - offsetof (STRUCT, MEMBER.next)))

	void list_init(struct list *list);
	struct list_elem *list_begin(struct list *list);
	struct list_elem *list_end(struct list *list);
	struct list_elem *list_back(struct list *list);
	struct list_elem *list_next(struct list_elem *elem);
	void list_insert(struct list_elem *before, struct list_elem *elem);
	void list_push_front(struct list *list, struct list_elem *elem);
	void list_push_back(struct list *list, struct list_elem *elem);
	struct list_elem *list_remove(struct list_elem *elem);
	struct list_elem *list_pop_front(struct list *list);
	struct list_elem *list_pop_back(struct list *list);
	bool list_empty(struct list *list);
	void list_sort (struct list *list, list_less_func *less, void *aux);
	struct list_elem *find_end_of_run (struct list_elem *a, struct list_elem *b, list_less_func *less, void *aux);
	void inplace_merge (struct list_elem *a0, struct list_elem *a1b0, struct list_elem *b1, list_less_func *less, void *aux);
	void list_splice (struct list_elem *before, struct list_elem *first, struct list_elem *last);
#endif
