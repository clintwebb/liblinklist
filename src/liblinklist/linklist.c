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
// delete a particular node from the list.
static void ll_delete_node(list_t *list, _list_node_t *node)
{
	assert(list);
	assert(node);
	assert(list->head);
	assert(list->tail);

	if (node->prev) node->prev->next = node->next;
	if (node->next) node->next->prev = node->prev;
	if (list->head == node) list->head = node->next;
	if (list->tail == node) list->tail = node->prev;
	node->prev = NULL;
	node->next = NULL;
	node->data = NULL;
	ll_return_node(list, node);
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

		ll_delete_node(list, list->head);

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
	
	assert(list);

	data = NULL;
	if (list->tail) {
		assert(list->tail->data);
		data = list->tail->data;

		ll_delete_node(list, list->tail);
		list->items --;
		assert(list->items >= 0);
	}
	
	return(data);
}


//-----------------------------------------------------------------------------
// Start the iterations through the list.  If there are no entries in the list,
// will return NULL.
void * ll_start(list_t *list)
{
	assert(list);
	return(list->head);
}

//-----------------------------------------------------------------------------
// return the data pointer for the next node.
void * ll_next(list_t *list, void **next)
{
	void *ptr;
	
	assert(list);
	assert(next);

	if (*next == NULL) {
		return(NULL);
	}
	else {
		ptr = ((_list_node_t *)*next)->data;
		assert(ptr);
		*next = ((_list_node_t *)*next)->next;
		return(ptr);
	}
}

//-----------------------------------------------------------------------------
// Remove a particular pointer from the list.  The 'next' is used to give a
// hint to where in the list the item is (normally provided as part of
// ll_start/ll_next).  If next is NULL, then the tail of the list is checked
// first.
void ll_remove(list_t *list, void *ptr, void *next)
{
	int found;
	_list_node_t *node;

	assert(list);
	assert(ptr);

	assert(list->head);
	assert(list->tail);

	// first check the 'next' hint that we were given.   It should be either
	// pointing to the one we want, or pointing to the next one in the list from
	// it.
	found = 0;
	if (next) {
		node = next;
		if (node->data == ptr) {
			ll_delete_node(list, node);
			found ++;
		}
		else if (node->prev) {
			if (node->prev->data == ptr) {
				ll_delete_node(list, node->prev);
				found ++;
			}
		}
	}

	if (found == 0) {
		node = list->tail;
		while (node) {
			if (node->data == ptr) {
				ll_delete_node(list, node);
				found++;
			}
			else {
				node = node->prev;
			}
		}
	}

	assert(found > 0);
}

