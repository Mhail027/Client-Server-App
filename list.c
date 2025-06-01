// Necula Mihail 323CAa 2024 - 2025
#include "list.h"

void free_ll_node(ll_node_t *node, void (*free_data)(void *))
{
	free_data(node->data);
	free(node);
}

ll_t *ll_create(u_int data_size, void (*free_data)(void *))
{
	/* Allocate memory for the list./ Create the list. */
	ll_t *list = (ll_t *)malloc(sizeof(ll_t));
	DIE(!list, "malloc() failed\n");

	/* Initialize the list. */
	list->head = NULL;
	list->tail = NULL;
	list->size = 0;
	list->data_size = data_size;
	list->free_data = free_data;

	// Return the list.
	return list;
}

ll_node_t *ll_get_nth_node(ll_t *list, u_int n)
{
	/* Verify the arguments. */
	if (!list) {
		fprintf(stderr, "ll_get_nth_node() - list can't be NULL\n");
		return NULL;
	}
	if (!list->head) {
		fprintf(stderr, "ll_get_nth_node() - list doesn't have nodes\n");
		return NULL;
	}
	if (n >= list->size) {
		fprintf(stderr, "ll_get_nth_node() - the index n is not valid\n");
		return NULL;
	}

	/* If the nth node is at the end of the list, we return it directly
	 * because we have access at it. */
	if (n == list->size - 1) {
		return list->tail;
	}

	/* In contrary case, we go through the list until get to the nth node. */
	ll_node_t *curr_node = list->head;
	for (u_int i = 1; i <= n; ++i) {
		curr_node = curr_node->next;
	}
	return curr_node;
}

ll_node_t *ll_remove_nth_node(ll_t *list, u_int n)
{
	/* Verify the arguments. */
	if (!list) {
		fprintf(stderr, "ll_remove_nth_node() - list can't be NULL\n");
		return NULL;
	}
	if (!list->head) {
		fprintf(stderr, "ll_remove_nth_node() - list doesn't have any node\n");
		return NULL;
	}
	if (n >= list->size) {
		n = list->size - 1;
	}

	/* Find the node which we want to remove and break the links
	 * with the other nodes. */
	ll_node_t *rmv_node;
	if (n == 0) {
		rmv_node = list->head;
		list->head = list->head->next;
	} else {
		ll_node_t *prev_node = ll_get_nth_node(list, n - 1);
		rmv_node = prev_node->next;

		prev_node->next = rmv_node->next;
		if (n == list->size - 1)
			list->tail = prev_node;
	}
	list->size--;

	return rmv_node;
}

void ll_add_nth_node(ll_t *list, u_int n, void *data)
{
	/* Verify the arguments. */
	if (!list) {
		fprintf(stderr, "ll_add_nth_node() - list can't be NULL\n");
		return;
	}
	if (!data) {
		fprintf(stderr, "ll_add_nth_node() - data can't be NULL (nothing)\n");
		return;
	}
	if (n > list->size) {
		n = list->size;
	}

	/* Create the new node. */
	ll_node_t *new_node = (ll_node_t *)malloc(sizeof(ll_node_t));
	DIE(!new_node, "malloc() failed\n");

	/* Initialize the new node. */
	new_node->data = malloc(list->data_size);
	DIE(!new_node->data, "malloc() failed\n");
	memcpy(new_node->data, data, list->data_size);

	/* We have 3 places where we can be asked to place the new node.
	 * To identify the scenario in which we are, will have 3 ifs. */

	/* Insert the new node at the beginning of the list. */
	if (n == 0) {
		new_node->next = list->head;
		list->head = new_node;
	}

	/* Insert the new node in the middle of the list. */
	if (n >= 1 && n < list->size) {
		ll_node_t *prev_node = ll_get_nth_node(list, n - 1);
		ll_node_t *next_node = prev_node->next;
		prev_node->next = new_node;
		new_node->next = next_node;
	}

	/* Insert the new node at the end of the list. */
	if (n == list->size) {
		/* If we have 0 elements in the list before this function, adding
		 * at end is the same thing with adding at beginning. We must verify
		 * this thing, not to add the same node 2 times. */
		if (list->size) {
			ll_node_t *prev_node = list->tail;
			prev_node->next = new_node;
			new_node->next = NULL;
		}

		list->tail = new_node;
	}

	++list->size;
}

void ll_free(ll_t **l)
{
	/* Verify the argument.*/
	if (!l) {
		fprintf(stderr, "ll_free() - argument can not be null\n");
		return;
	} else if (!*l) {
		return;
	}

	/* Get to list. */
	ll_t *list = *l;

	/* Free the memory allocated for every node from list. */
	ll_node_t *curr_node = list->head;
	while (curr_node) {
		ll_node_t *rmv_node = curr_node;
		curr_node = curr_node->next;
		free_ll_node(rmv_node, list->free_data);
	}

	/* Free the memory allocated for the structure of list. */
	free(list);

	/* Lose the address of the list, which doesn't exist anymore. */
	*l = NULL;
}

