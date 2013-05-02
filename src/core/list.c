#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include "core/list.h"

#ifdef list_test
int main(){
	struct list *l = (struct list *)calloc(1, sizeof(struct list));
	list_init(l);
	return 0;
}
#endif

void list_init(struct list *list){
	list->head.prev = NULL;
	list->head.next = &list->tail;
	list->tail.prev = &list->head;
	list->tail.next = NULL;
}

struct list_elem *list_begin(struct list *list){
	return list->head.next;
}

struct list_elem *list_end(struct list *list){
	return &list->tail;
}

struct list_elem *list_back(struct list *list){
	return list->tail.prev;
}

struct list_elem *list_next(struct list_elem *elem){
	return elem->next;
}

void list_insert(struct list_elem *before, struct list_elem *elem){
	elem->prev = before->prev;
	elem->next = before;
	before->prev->next = elem;
	before->prev = elem;
}

void list_push_front(struct list *list, struct list_elem *elem){
	list_insert(list_begin(list), elem);
}

void list_push_back(struct list *list, struct list_elem *elem){
	list_insert(list_end(list), elem);
}

struct list_elem *list_remove(struct list_elem *elem){
	elem->prev->next = elem->next;
	elem->next->prev = elem->prev;
	return elem->next;
}

struct list_elem *list_pop_front(struct list *list){
	struct list_elem *front = list_begin(list);
	list_remove(front);
	return front;
}

struct list_elem *list_pop_back(struct list *list){
	struct list_elem *back = list_back(list);
	list_remove(back);
	return back;
}

bool list_empty(struct list *list){
	return list_begin(list) == list_end(list);
}

void list_sort (struct list *list, list_less_func *less, void *aux){
	size_t output_run_cnt;        /* Number of runs output in current pass. */

	/* Pass over the list repeatedly, merging adjacent runs of
	   nondecreasing elements, until only one run is left. */
	do
	{
		struct list_elem *a0;     /* Start of first run. */
		struct list_elem *a1b0;   /* End of first run, start of second. */
		struct list_elem *b1;     /* End of second run. */

		output_run_cnt = 0;
		for (a0 = list_begin (list); a0 != list_end (list); a0 = b1)
		{
			/* Each iteration produces one output run. */
			output_run_cnt++;

			/* Locate two adjacent runs of nondecreasing elements
			   A0...A1B0 and A1B0...B1. */
			a1b0 = find_end_of_run (a0, list_end (list), less, aux);
			if (a1b0 == list_end (list))
				break;
			b1 = find_end_of_run (a1b0, list_end (list), less, aux);

			/* Merge the runs. */
			inplace_merge (a0, a1b0, b1, less, aux);
		}
	}
	while (output_run_cnt > 1);
}

struct list_elem *find_end_of_run (struct list_elem *a, struct list_elem *b, list_less_func *less, void *aux){
	do 
	{
		a = list_next (a);
	}
	while (a != b && !less (a, (a)->prev, aux));
	return a;
}

void inplace_merge (struct list_elem *a0, struct list_elem *a1b0, struct list_elem *b1, list_less_func *less, void *aux)
{
	while (a0 != a1b0 && a1b0 != b1)
		if (!less (a1b0, a0, aux)) 
			a0 = list_next (a0);
		else 
		{
			a1b0 = list_next (a1b0);
			list_splice (a0, (a1b0)->prev, a1b0);
		}
}

void list_splice (struct list_elem *before, struct list_elem *first, struct list_elem *last)
{
	if (first == last)
		return;
	last = (last)->prev;

	/* Cleanly remove FIRST...LAST from its current list. */
	first->prev->next = last->next;
	last->next->prev = first->prev;

	/* Splice FIRST...LAST into new list. */
	first->prev = before->prev;
	last->next = before;
	before->prev->next = first;
	before->prev = last;
}
