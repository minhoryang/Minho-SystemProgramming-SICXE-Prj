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
