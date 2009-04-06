/*

	liblinklist
	(c) Copyright Hyper-Active Systems, Australia

	Contact:
		Clinton Webb
		webb.clint@gmail.com

	This is a small, simplistic library designed to handle a linked list of generic objects.

	It is released under GPL v2 or later license.  Details are included in the LICENSE file.

*/

#ifndef __LINKLIST_H
#define __LINKLIST_H


typedef struct __llist {
	void *data;
	struct __llist *prev, *next;
} _list_node_t;

typedef struct {
	_list_node_t *head;
	_list_node_t *tail;
	_list_node_t *pool;
	int items;
} list_t;

void ll_init(list_t *list);
void ll_free(list_t *list);

void ll_push_head(list_t *list, void *data);
void ll_push_tail(list_t *list, void *data);

void * ll_get_head(list_t *list);
void * ll_get_tail(list_t *list);

void * ll_pop_head(list_t *list);
void * ll_pop_tail(list_t *list);




#endif
