// Necula Mihail 323CAa 2024 - 2025
#include "trie.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

trie_node_t *trie_node_create(trie_t *trie)
{
    /* Allocate memory for the structure of the node. */
    trie_node_t *node = (trie_node_t *)malloc(sizeof(trie_node_t));
    DIE(!node, "malloc() failed\n");

    /* Allocate memory for the children array.*/
    node->children = (trie_node_t **)calloc(trie->alphabet_size,
                      sizeof(trie_node_t));
    DIE(!node->children, "calloc() failed\n");

    /* Initialize the other fields from structure with 0.*/
    node->end_of_word = false;
    node->n_children = 0;
    node->value = NULL;

    /* Return the created node. */
    return node;
}

trie_t* trie_create(u_int data_size, u_int alphabet_size, char *alphabet,
                    void (*free_value)(void*))
{
    /* Allocate memory for the structure of the trie. */
	trie_t *trie = (trie_t *)malloc(sizeof(trie_t));
	DIE(!trie, "malloc() failed\n");

    /* Allocate memory for alphabet. */
    trie->alphabet = (char *)malloc(strlen(alphabet) + 1);
    DIE(!trie->alphabet, "malloc() failed\n");

    /* Initialize the fields of the structure with the given values. */
    trie->data_size = data_size;
    trie->alphabet_size = alphabet_size;
    trie->free_value = free_value;
    strcpy(trie->alphabet, alphabet);

    /* Initialize the other fields. */
    trie->root = trie_node_create(trie);
    trie->root->end_of_word = false;
    trie->n_Nodes = 1;
    trie->size = 0;
    
    /* Return the created trie. */
    return trie;

}

int get_index_from_alphabet(char *alphabet, char letter)
{
    for (int i = 0; i < strlen(alphabet); ++i)
        if (alphabet[i] == letter)
            return i;

    return -1;
}

/******************************
 * @brief Intermediate the insertion of a new par in a trie.
 *
 * @param trie with which we work
 * @param trie_node at which we got
 * @param key rest
 * @param value which will be added at final of key
*******************************/
static void trie_insert_helper(trie_t *trie, trie_node_t *trie_node,
                               const char *const key, void *value)
{
    /* Verify if we get at the end of the key. */
    if (!strlen(key)) {
        /* Allocate memory for value.*/
        trie_node->value = malloc(trie->data_size);
        DIE(!trie_node->value, "malloc() failed\n");
        
        /* Update the fields of the node. */
        memcpy(trie_node->value, value, trie->data_size);
        trie_node->end_of_word = true;
        trie->size++;
    } else {
        /* Find the index of the child at which we will get. */
        int child_idx = get_index_from_alphabet(trie->alphabet, key[0]);
        DIE(child_idx == -1, "Current letter from key is not in alphabet");

        /* Allocate memory for child if does not have already.*/
        if (!trie_node->children[child_idx]) {
            trie_node->children[child_idx] = trie_node_create(trie);
            trie_node->n_children++;
            trie->n_Nodes++;
        }

        /* Go at next letter from key. */
        trie_insert_helper(trie, trie_node->children[child_idx], key + 1,
                           value);
    }
}

void trie_insert(trie_t *trie, const char *const key, void *value)
{
    trie_insert_helper(trie, trie->root, key, value);
}

/******************************
 * @brief Intermediate the search of a key in a trie.
 *
 * @param trie with which we work
 * @param trie_node at which we got
 * @param key rest
 *
 * @return: NULL, if the key is not a valid word AND a trie word
 *			value of key, else
*******************************/
static void *trie_search_helper(trie_t *trie, trie_node_t *trie_node,
                                const char *const key)
{
    /* Verify if we get at the end of the key. */
    if (!strlen(key)) {
        if (trie_node->end_of_word)
            return trie_node->value;
        return NULL;
    }

    /* Verify first letter from key. */
    int child_idx = get_index_from_alphabet(trie->alphabet, key[0]);
    if (!trie_node->children[child_idx])
        return NULL;
    
    /* Go at next letter from key. */
    return trie_search_helper(trie, trie_node->children[child_idx], key + 1);
}

void *trie_search(trie_t *trie, const char *const key)
{
    return trie_search_helper(trie, trie->root, key);
}

void trie_node_free(trie_t *trie, trie_node_t *node)
{
    if (!node)
        return;

    if (node->value)
        trie->free_value(node->value);
    free(node->children);
    free(node);
}

/******************************
 * @brief: Intermediate the remove of a key from a trie.
 *
 * @param trie with which we work
 * @param trie_node at which we got
 * @param key rest
 *
 * @return 0, if the children of current node corresponding with
 *			  first letter from key must not be deleted
 *		   something else, else
*******************************/
static int trie_remove_helper(trie_t *trie, trie_node_t *trie_node,
                              const char *const key)
{
    /* Verify if we get at the end of the key. */
    if (!strlen(key)) {
        if (trie_node->end_of_word) {
            trie->size--;
            trie_node->end_of_word = false;
            trie->free_value(trie_node->value);
            trie_node->value = NULL;
        }
        return (trie_node->n_children == 0);
    }

    /* Find the index of the child at which we will get. */
    int child_idx = get_index_from_alphabet(trie->alphabet, key[0]);
    if (child_idx == -1 || !trie_node->children[child_idx])
        return 0;

    /* Verify if the children corresponding with the first letter from key
     * must be deleted. */
    int delete = trie_remove_helper(trie, trie_node->children[child_idx],
                                    key + 1);
    if (delete) {
        trie_node_free(trie, trie_node->children[child_idx]);
        trie_node->children[child_idx] = NULL;
        trie_node->n_children--;
        trie->n_Nodes--;
    }

    /* Return if the current node must be removed / deleted. */
    return (trie_node->n_children == 0 && !trie_node->end_of_word);
}

void trie_remove(trie_t *trie, const char *const key)
{
    trie_remove_helper(trie, trie->root, key);
}

/******************************
 * @brief Free the memory of a trie node and all his children.
 *
 * @param trie from which is the node (we need the function to
 *             free a key's value)
 * @param node which will be freed
*******************************/
static void trie_node_and_children_free(trie_t *trie, trie_node_t *trie_node)
{
    if (!trie || !trie_node)
        return;

    /* Free the memory of the children. */
    for (int i = 0; i < trie->alphabet_size; ++i)
        if (trie_node->children[i])
            trie_node_and_children_free(trie, trie_node->children[i]);

    /* Free the memory of the node. */
    trie_node_free(trie, trie_node);
}

void trie_free(trie_t **pTrie)
{
    trie_t *trie = *pTrie;

    trie_node_and_children_free(trie, trie->root);
    free(trie->alphabet);
    free(trie);

    *pTrie = NULL;
}   
