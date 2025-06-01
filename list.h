// Necula Mihail 323CAa 2024 - 2025
#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define DIE(assertion, call_description)                                      \
	do {                                                                      \
		if (assertion) {                                                      \
			fprintf(stderr, "(%s, %d): ", __FILE__, __LINE__);                \
			perror(call_description);                                         \
			exit(errno);                                                      \
		}                                                                     \
	} while (0)

/******************************
 * ll_node = node from a (singly) linked list
*******************************/
typedef struct ll_node_t {
	struct ll_node_t *next;
	void *data;
} ll_node_t;

/******************************
 * ll = (singly) linked list
*******************************/
typedef struct ll_t {
	ll_node_t *head;
	ll_node_t *tail;
	u_int size; 		/* The number of nodes from list */
	u_int data_size;	/* The dimension of information from every node, in bytes. */
	void (*free_data)(void *); 
} ll_t;

/******************************
 * @brief Free the memory allocated for a node from a singly linked list
 *		and his data.
*******************************/
void free_ll_node(ll_node_t *node, void (*free_data)(void *));

/******************************
 * ll_create() - Create and initialize a singly linked list.
 *
 * @param data_size: The length, in bytes, of the information
 *		from a node.
 * @param free_data: Function to free the memory allocated for
 *		the data of a node.
 *
 * @return - The created list.
*******************************/
ll_t *ll_create(u_int data_size, void (*free_data)(void *));

/******************************
 * @brief Return the address of nth node from the given singly
 *		linked list. The count starts from 0, so n must be in the
 *		next interval: [0, size of the list - 1].
*******************************/
ll_node_t *ll_get_nth_node(ll_t *list, u_int n);

/******************************
 * @brief Remove the nth node from a singly linked list and return
 *		the address of that node. It is the responsibility of user
 *		to free its memory. The position is indexed from 0. If the
 *		position is bigger or equal with the size of list, we will
 *		remove the last node.
*******************************/
ll_node_t *ll_remove_nth_node(ll_t *list, u_int n);

/******************************
 * @brief Create, initialize and put a node on nth postion in the given
 *		singly linked list. The position is indexed from 0. If the position
 *		n is bigger than the size of the list, the node is added at end.
*******************************/
void ll_add_nth_node(ll_t *list, u_int n, void *data);

/******************************
 * ll_free() - Free the memory allocated a singly linked list.
 *
 * @param l: Pointer to the address of the list.
*******************************/
void ll_free(ll_t **l);

#endif

