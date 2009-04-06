/*

	liblinklist
	(c) Copyright Hyper-Active Systems, Australia

	Contact:
		Clinton Webb
		webb.clint@gmail.com

*/


#include "linklist.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>




void ll_init(list_t *list)
{
	assert(list);
	
	list->head = NULL;
	list->tail = NULL;
	list->pool = NULL;
	list->items = 0;
}

void ll_free(list_t *list)
{
	_list_node_t *node, *tmp;
	assert(list);

	assert(list->head == NULL);
	assert(list->tail == NULL);

	node = list->pool;
	while (node) {
		assert(node->data == NULL);
		tmp = node;
		node = node->next;
		free(tmp);
	}
	list->pool = NULL;
}

static _list_node_t * ll_new_node(list_t *list)
{
	_list_node_t *node;

	assert(list);
	
	if (list->pool) {
		// we have a node already available in the pool, so we will use that.
		node = list->pool;
		assert(node->data == NULL);
		assert(node->prev == NULL);
		list->pool = node->next;
		node->next = NULL;
	}
	else {
		// There aren't anymore nodes in the pool, so we need to create one.
		node = (_list_node_t *) malloc(sizeof(_list_node_t));
		assert(node);
		node->data = NULL;
		node->next = NULL;
		node->prev = NULL;
	}

	assert(node);
	return(node);
}

static void ll_return_node(list_t *list, _list_node_t *node)
{
	assert(list);
	assert(node);

	assert(node != list->tail);
	assert(node != list->head);

	assert(node->data == NULL);
	assert(node->next == NULL);
	assert(node->prev == NULL);

	node->next = list->pool;
	list->pool = node;
}


void ll_push_head(list_t *list, void *data)
{
	_list_node_t *node;
	
	assert(list);
	assert(data);

	node = ll_new_node(list);
	node->data = data;
	assert(node->prev == NULL);
	node->next = list->head;
	if (node->next) node->next->prev = node;
	list->head = node;
	if (list->tail == NULL) list->tail = node;
	list->items ++;
}

void ll_push_tail(list_t *list, void *data)
{
	_list_node_t *node;
	
	assert(list);
	assert(data);

	node = ll_new_node(list);
	node->data = data;
	assert(node->next == NULL);
	node->prev = list->tail;
	if (node->prev) node->prev->next = node;
	list->tail = node;
	if (list->head == NULL) list->head = node;
	list->items ++;
}

void * ll_get_head(list_t *list)
{
	void *data;
	
	assert(list);

	if (list->head) {
		assert(list->head->data);
		return(list->head->data);
	}
	else {
		return(NULL);
	}
}

void * ll_get_tail(list_t *list)
{
	assert(list);

	if (list->tail) {
		assert(list->tail->data);
		return(list->tail->data);
	}
	else {
		return(NULL);
	}
}

void * ll_pop_head(list_t *list)
{
	void *data;
	_list_node_t *node;
	
	assert(list);

	data = NULL;
	if (list->head) {
		assert(list->head->data);
		data = list->head->data;

		node = list->head;
		if (node->next) node->next->prev = NULL;
		list->head = node->next;
		if (list->head == NULL) list->tail = NULL;

		node->data = NULL;
		node->prev = NULL;
		node->next = NULL;
		ll_return_node(list, node);

		list->items --;
		assert(list->items >= 0);
	}
	
	return(data);
}

void * ll_pop_tail(list_t *list)
{
	void *data;
	_list_node_t *node;
	
	assert(list);

	data = NULL;
	if (list->tail) {
		assert(list->tail->data);
		data = list->tail->data;

		node = list->tail;
		if (node->prev) node->prev->next = NULL;
		list->tail = node->prev;
		if (list->tail == NULL) list->head = NULL;

		node->data = NULL;
		node->prev = NULL;
		node->next = NULL;
		ll_return_node(list, node);

		list->items --;
		assert(list->items >= 0);
	}
	
	return(data);
}




