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


//-----------------------------------------------------------------------------
// Initialise the list structure.  
void ll_init(list_t *list)
{
	assert(list);
	
	list->head = NULL;
	list->tail = NULL;
	list->pool = NULL;
	list->items = 0;
}

//-----------------------------------------------------------------------------
// Free the resources created by the list.  At this point there should not be
// any nodes in the list.  Will not free the actual list itself... as it could
// have been malloc'd, or it could be embedded in another structure, or an
// auto-var.
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

//-----------------------------------------------------------------------------
// Internal function to return a new node wrapper from the pool.  If there
// isn't any left in the pool, then create a new node wrapper, and return that
// instead.
static _list_node_t * ll_new_node(list_t *list)
{
	_list_node_t *node;

	assert(list);
	
	if (list->pool) {
		// we have a node already available in the pool, so we will use that.
		node = list->pool;
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
	assert(node->data == NULL);
	assert(node->prev == NULL);
	assert(node->next == NULL);
	return(node);
}

//-----------------------------------------------------------------------------
// Internal function to return a node-wrapper back to the pool.
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

//-----------------------------------------------------------------------------
// push an object to the head of the list.  Will add the data pointer to a
// node-wrapper that contains all the actual list control mechanisms.
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

//-----------------------------------------------------------------------------
// same as ll_push_head, except pushing to the bottom of the list instead.
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

//-----------------------------------------------------------------------------
// return a pointer to the data object in the first entry in the list.  Does
// not remove the entry from the list.  This is useful for examining an item
// before determining to remove it.
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

//-----------------------------------------------------------------------------
// same as ll_get_head except it gets the node from the tail.
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

//-----------------------------------------------------------------------------
// Pop an entry from the top of the list.  Which means that it returns the data
// pointer if there is a node, and removes the node from the list.
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

//-----------------------------------------------------------------------------
// Same as ll_pop_head except pops from the bottom of the list.
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




