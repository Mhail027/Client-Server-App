// Necula Mihail 323CAa 2024 - 2025
#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "list.h"

#define DIE(assertion, call_description)                                      \
	do {                                                                      \
		if (assertion) {                                                      \
			fprintf(stderr, "(%s, %d): ", __FILE__, __LINE__);                \
			perror(call_description);                                         \
			exit(errno);                                                      \
		}                                                                     \
	} while (0)                                                               \

#define LOAD_FACTOR 0.75

/******************************
 * Structure for the pair key-value.
*******************************/
typedef struct pair_t {
	void *key;
	u_int key_size;

	void *value;
	u_int value_size;
} pair_t;

/******************************
 * The hashtable is implemented using an array of lists.
*******************************/
typedef struct ht_t {
	ll_t **buckets; 	/* Aray of singly linked lists. */
	u_int size; 		/* The total number of nodes from all lists. */
	u_int hmax;			/* The number of buckets / lists. */

	u_int (*hash_function)(void *key);
	int (*compare_function)(void *a, void *b);
	void (*free_pair_function)(void *pair);
} ht_t;

/******************************
 * @brief: Hash an unsigned int.
*******************************/
u_int hash_uint(void *key);

/******************************
 * @brief: Hash a string.
*******************************/
u_int hash_string(void *key);

/******************************
 * compare_uints() - Compare 2 unsigned ints.
 *
 * @param a: Pointer at the first unsigned int.
 * @param b: Pointer at the second unsigned int.
 *
 * @return 0, if the numbers are equal
 *		   1, if the first number is greater than the second one
 *		  -1, if the second number is greater than the first one
*******************************/
int compare_uints(void *a, void *b);

/******************************
 * compare_function_strings() - Compare 2 strings using the function strcmp.
 *
 * @param a: First string.
 * @param b: Second string.
 *
 * @return 0, if the string are identical
 *		   >0, if the first string is greater than the second one
 *		   <0, if the second string is greater than the first one
*******************************/
int compare_strings(void *a, void *b);

/******************************
 * @brief Free the memory allocated for the key and the value of a
 *		pair from an hashtable. It's not taking in consideration that
 *		the key and the value can have in them other things which should
 *		be freed.
*******************************/
void free_simple_pair(void *data);

/******************************
 * ht_create() - Create and initialize a hashtable.
 *
 * @param hmax: The number of lists.
 * @param hash_function: Function to hash a key.
 * @param compare_function: Function to compare 2 values.
 * @param key_val_free_function: Function to free the memory
 *		allocated for a pair key-value.
 *
 * @return - The created hashtable.
*******************************/
ht_t *ht_create(u_int hmax, u_int (*hash_function)(void *),
					   int (*compare_function)(void *, void *),
					   void (*key_val_free_function)(void *));

/******************************
 * ht_has_key() - Verify if a key is in a hashtable.
 *
 * @param ht: The hashtable with which we work.
 * @param key: The key which will be searched.
 *
 * @return 1, if the key is in the hashtable
 *		   0, in contrary case
*******************************/
int ht_has_key(ht_t *ht, void *key);

/******************************
 * ht_get() - Find the value associated with a key.
 *
 * @param ht: The hashtable with which we work.
 * @param key: The key which will be searched.
 *
 * @return - the value associated with the key.
*******************************/
void *ht_get(ht_t *ht, void *key);

/******************************
 * ht_double_hmax: Double the buckets number. After remove every
 *				   key-val pair and put it back again because the
 *				   hash % hmax (position of pair) will change.
 *
 * @param ht: The hashtable with which we work.
*******************************/
void ht_double_hmax(ht_t *ht);

/******************************
 * ht_put() - Put in the given hashtable a pair key-value.
 *
 * @param ht: The hashtable with which we work.
 * @param key: The key's pair.
 * @param key_size: The number of bytes of the key.
 * @param value: The value's pair.
 * @param value_size: The number of bytes of the value.
*******************************/
void ht_put(ht_t *ht, void *key, u_int key_size,
			void *value, u_int value_size);

/******************************
 * @brief Remove the specified key from the given hashtable.
*******************************/
void ht_remove_entry(ht_t *ht, void *key);

/******************************
 *  ht_free() - Free the memory which was allocated for a
 *		hashtable.
 *
 * @param hashtable: Pointer to the address of the hashtable.
*******************************/
void ht_free(ht_t **hashtable);

/******************************
 * @brief Return the number of keys from the given hashtable.
*******************************/
u_int ht_get_size(ht_t *ht);

#endif
