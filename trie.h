// Necula Mihail 323CAa 2024 - 2025
#ifndef _TRIE_H_
#define _TRIE_H_

#include <sys/types.h>
#include <stdbool.h>

#define DIE(assertion, call_description)                                      \
	do {                                                                      \
		if (assertion) {                                                      \
			fprintf(stderr, "(%s, %d): ", __FILE__, __LINE__);                \
			perror(call_description);                                         \
			exit(errno);                                                      \
		}                                                                     \
	} while (0)                                                               \

/******************************
 * Node of a trie saves 2 important things:
 * 		-> a value, if it's the end of a word
 *		-> links to its children
*******************************/
typedef struct trie_node_t {
	void *value;				/* Value associated with key (set if end_of_word = true) */
	bool end_of_word;			/* true if current node marks the end of a word, false otherwise */
	struct trie_node_t **children; 
	u_int n_children; 			/* n_children <= size of alphabet */
} trie_node_t;

/******************************
 * The most important things which are saved in trie are:
 *		-> a link to its root
 *		-> the used alphabet
 *		-> value's size of a key (because the trie is generic) 
*******************************/
typedef struct trie_t {
	trie_node_t *root;
	u_int size;					/* Number of keys */
	u_int data_size;			/* Generic Data Structure */
	u_int alphabet_size;		/* Trie-Specific, alphabet properties */
	char *alphabet;
	void (*free_value)(void*);	/* Fuction to free value associated with key. */
	u_int n_Nodes;				/* Total number of nodes from trie. */
} trie_t;

/******************************
 * @brief Allocate memory for a trie node and his array of children.
 *		  All the other fields of the node ar initiated with 0.
 *
 * @param trie: the trie in which will be added (we need alphabet properties)
 *
 * @return The created trie node.
*******************************/
trie_node_t *trie_node_create(trie_t *trie);

/******************************
 * @brief Allocate memory for a trie, his alphabet and his root which is neutral.
 *		  Also, initiate the other fields with the given and corresponding values.
 *
 * @param data_size: the number of needed bytes for a value from node
 * @param alphabet_size: number of chars from alphabet
 * @param alphabet: array with all allowed characters
 * @param free_value: function which free the memory of value from a node
 *
 * @return The created trie.
*******************************/
trie_t* trie_create(u_int data_size, u_int alphabet_size, char *alphabet, void (*free_value)(void*));

/******************************
 * @brief Find the index from the alphabet of a letter.
 *
 * @param alphabet: array of different characters
 * @param letter: the character which we search it
 *
 * @return: -1, if the letter is not in alphabet
 *			index of letter in alphabet, else		
*******************************/
int get_index_from_alphabet(char *alphabet, char letter);

/******************************
 * @brief Add a new pair (key, value) in a trie.
 *
 * @param trie which will be used
 * @param key which is formed from trie's alphabet
 * @param value from the final of key
*******************************/
void trie_insert(trie_t *trie, const char *const key, void *value);

/******************************
 * @brief Return the value of key from trie
 *
 * @param trie with which we work
 * @param key in which are interested
 *
 * @return NULL, if the key is not a valid word AND a trie word
 *		   value of key, else
*******************************/
void *trie_search(trie_t *trie, const char *const key);

/******************************
 * @brief Fre all memory allocated for a trie node.
 *
 * @param trie from which is the node (we need the function to free a key's value)
 * @param node which will be freed
*******************************/
void trie_node_free(trie_t *trie, trie_node_t *node);

/******************************
 * @brief Remove a key from a trie.
 *
 * @param trie which will be used
 * @param key which is formed from trie's alphabet
 * @param value from the final of key
*******************************/
void trie_remove(trie_t *trie, const char *const key);

/******************************
 * @brief Free the memory of a trie and *pTrie becomes NULL.
 *
 * @param pTrie: pointer to trie
*******************************/
void trie_free(trie_t **pTrie);

#endif /* _TRIE_H_ */
