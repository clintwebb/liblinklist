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


#if (LIBLINKLIST_VERSION != 0x00007000)
	#error "This version certified for v0.70 only"
#endif

//-----------------------------------------------------------------------------
// Initialise the list structure.  
void ll_init(list_t *list)
{
	assert(list);
	
	list->head = NULL;
	list->tail = NULL;
	list->pool = NULL;
	list->items = 0;
	list->join = NULL;
	list->loop = NULL;
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
	assert(list->loop == NULL);

	node = list->pool;
	while (node) {
		assert(node->data == NULL);
		tmp = node;
		node = node->next;
		free(tmp);
	}
	list->pool = NULL;

	if (list->join) {
		free(list->join);
		list->join = NULL;
	}
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

	if (list->loop == node) {
		list->loop = node->next;
	}

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
void ll_start(list_t *list)
{
	assert(list);
	assert(list->loop == NULL);
	list->loop = list->head;
}

//-----------------------------------------------------------------------------
// return the data pointer for the next node.
void * ll_next(list_t *list)
{
	void *ptr;
	
	assert(list);

	if (list->loop == NULL) {
		return(NULL);
	}
	else {
		ptr = list->loop->data;
		assert(ptr);
		list->loop = list->loop->next;
		return(ptr);
	}
}

//-----------------------------------------------------------------------------
// This function is used to indicate that the iteration loop is finished.
// Technically it is not necessary, but it allows us to assist developers in
// finding obscure bugs in loops.  So if a loop is started, but not finished,
// and another loop is started, then we generate an assertion.
void ll_finish(list_t *list)
{
	assert(list);
	list->loop = NULL;
}

//-----------------------------------------------------------------------------
// Remove a particular pointer from the list.  The 'next' is used to give a
// hint to where in the list the item is (normally provided as part of
// ll_start/ll_next).  If next is NULL, then the tail of the list is checked
// first.
void ll_remove(list_t *list, void *ptr)
{
	assert(list);
	assert(ptr);

	assert(list->head);
	assert(list->tail);

	// first check the 'next' hint that we were given.   It should be either
	// pointing to the one we want, or pointing to the next one in the list from
	// it.
	if (list->loop) {
		assert(list->loop->data);
		if (list->loop->data == ptr) {
			ll_delete_node(list, list->loop);
			return;
		}
		else if (list->loop->prev) {
			assert(list->loop->prev->data);
			if (list->loop->prev->data == ptr) {
				ll_delete_node(list, list->loop->prev);
				return;
			}
		}
	}

	// we didn't have a hint 'loop' that told us where the entry was, so we need
	// to go through the list.  We will start at the tail.
	_list_node_t *node = list->tail;
	while (node) {
		assert(node->data);
		if (node->data == ptr) {
			ll_delete_node(list, node);
			assert(list->items > 0);
			list->items --;
			return;
		}
		node = node->prev;
	}
}


//-----------------------------------------------------------------------------
// Return the number of items in the list.
int ll_count(list_t *list)
{
	assert(list);
	assert(list->items >= 0);
	return(list->items);
}


//-----------------------------------------------------------------------------
// Assuming that the list contains valid strings, this function will join all
// the elements in the list into a single string
char * ll_join_str(list_t *list, const char *sep)
{
	int max;
	int count;
	char *str;
	
	assert(list);

	// first go thru the list, and calculate the size of the resulting string.
	max = 0;
	count = 0;
	ll_start(list);
	while ((str = ll_next(list))) {
		count ++;
		max += strlen(str);
	}
	ll_finish(list);

	if (sep && count > 1) {
		max += (strlen(sep) * (count-1));
	}

	// max now contains the size of the string we will be building.
	list->join = (char *) realloc(list->join, max+1);
	assert(list->join);
	
	// now build the list.
	count = 0;
	list->join[0] = '\0';
	ll_start(list);
	while ((str = ll_next(list))) {

		// TODO: strcat is not very optimal way to do this.  We should keep track
		// of the end of the string and add it direct to the end, but will bother
		// with that another day.

		if (count > 0 && sep != NULL) {
			strcat(list->join, sep);
		}
		strcat(list->join, str);

		count++;
	}
	ll_finish(list);

	assert(list->join);
	return(list->join);
}
