// Necula Mihail 323CAa 2024 - 2025
#include "hashmap.h"

u_int hash_uint(void *key)
{
	u_int x = *((u_int *)key);

	x = ((x >> 16u) ^ x) * 0x45d9f3b;
	x = ((x >> 16u) ^ x) * 0x45d9f3b;
	x = (x >> 16u) ^ x;

	return x;
}

u_int hash_string(void *key)
{
	u_char *key_string = (u_char *)key;
	u_int hash = 5381;
	int c;

	while ((c = *key_string++))
		hash = ((hash << 5u) + hash) + c;

	return hash;
}

int compare_uints(void *a, void *b)
{
	u_int uint_a = *((u_int *)a);
	u_int uint_b = *((u_int *)b);

	if (uint_a == uint_b)
		return 0;
	else if (uint_a < uint_b)
		return -1;
	else
		return 1;
}

int compare_strings(void *a, void *b)
{
	char *str_a = (char *)a;
	char *str_b = (char *)b;

	return strcmp(str_a, str_b);
}

void free_simple_pair(void *data)
{
	pair_t *pair = (pair_t *)data;

	free(pair->key);
	free(pair->value);
	free(pair);
}

ht_t *ht_create(u_int hmax, u_int (*hash_function)(void *),
					   int (*compare_function)(void*, void*),
					   void (*free_pair_function)(void *))
{
	/* Create the hashtable. */
	ht_t *ht = (ht_t *)malloc(sizeof(ht_t));
	DIE(!ht, "malloc() failed");

	/* Initialize the fields of the hashtable. */
	ht->size = 0;
	ht->hmax = hmax;
	ht->hash_function = hash_function;
	ht->compare_function = compare_function;
	ht->free_pair_function = free_pair_function;

	/* Create every list from hashtable. */
	ht->buckets = (ll_t **)malloc(hmax * sizeof(ll_t *));
	DIE(!ht->buckets, "malloc() failed\n");
	for (u_int i = 0; i < hmax; ++i)
		ht->buckets[i] = ll_create(sizeof(pair_t), free_pair_function);

	/* Return the hashmap. */
	return ht;
}

int ht_has_key(ht_t *ht, void *key)
{
	/* Find the index of the list where to search. */
	u_int index = ht->hash_function(key) % ht->hmax;

	/* Take every node from the list and verify the key. */
	ll_node_t *curr_node = ht->buckets[index]->head;
	while (curr_node) {
		pair_t *pair = (pair_t *)curr_node->data;
		if (!ht->compare_function(pair->key, key))
			return 1;
		curr_node = curr_node->next;
	}
	return 0;
}

void *ht_get(ht_t *ht, void *key)
{
	/* Find the index of the list where to search. */
	u_int index = ht->hash_function(key) % ht->hmax;

	/* Take every node from the list and verify the key. */
	ll_node_t *curr_node = ht->buckets[index]->head;
	while (curr_node) {
		pair_t *pair = (pair_t *)curr_node->data;
		if (!ht->compare_function(pair->key, key))
			return pair->value;
		curr_node = curr_node->next;
	}
	return NULL;
}

void ht_double_hmax(ht_t *ht) {
	/* Verify the argument. */
	if (!ht) {
		fprintf(stderr, "ht_double_size() - ht can't be NULL\n");
		return;
	}

	/* Save old info. */
	ll_t **old_buckets = ht->buckets;
	u_int old_hmax = ht->hmax;

	/* Make the hashtable empty and double his hmax. */
	ht->size = 0;
	ht->hmax *= 2;
	ht->buckets = (ll_t **)malloc(ht->hmax * sizeof(ll_t *));
	DIE(!ht->buckets, "malloc() failed\n");
	for (u_int i = 0; i < ht->hmax; ++i)
		ht->buckets[i] = ll_create(sizeof(pair_t), ht->free_pair_function);

	/* Put every pair back in hashtable. */
	for (u_int i = 0; i < old_hmax; ++i) {
		ll_node_t *curr_node = old_buckets[i]->head;
		while (curr_node) {
			/* Put pair */
			pair_t *pair = (pair_t *)curr_node->data;
			ht_put(ht, pair->key, pair->key_size,
					pair->value, pair->value_size);

			/* Go at the next node. */
			curr_node = curr_node->next;
		}
	}

	/* Free the memory of old buckets array. */
	for (int i = 0; i < old_hmax; ++i) {
		ll_free(&old_buckets[i]);
	}
	free(old_buckets);
}

void ht_put(ht_t *const ht, void *const key, const u_int key_size,
			void *const value, const u_int value_size)
{
	/* If the key is associated already with a value, we must
	 * remove that entry before to create the new one. */
	if (ht_has_key(ht, key))
		ht_remove_entry(ht, key);

	/* Create the new pair key-value which will be put in the hashtable. */
	pair_t *new_pair = (pair_t *)malloc(sizeof(pair_t));
	DIE(!new_pair, "malloc() failed\n");

	new_pair->key = malloc(key_size);
	DIE(!new_pair->key, "malloc() failed\n");
	memcpy(new_pair->key, key, key_size);
	new_pair->key_size = key_size;

	new_pair->value = malloc(value_size);
	DIE(!new_pair->value, "malloc() failed\n");
	memcpy(new_pair->value, value, value_size);
	new_pair->value_size =value_size;

	/* Find the index of the list where will put the pair. */
	u_int index = ht->hash_function(key) % ht->hmax;

	/* Put the pair in that list. */
	ll_t *curr_list = ht->buckets[index];
	ll_add_nth_node(curr_list, curr_list->size, new_pair);

	/* Update the parameters of the hashtable. */
	ht->size++;

	/* Free the unecessary memory. */
	free(new_pair);

	/* Do we need to double the size of the hashmap? */
	if ((double) ht->size / ht->hmax >= LOAD_FACTOR) {
		ht_double_hmax(ht);
	}
}

void ht_remove_entry(ht_t *ht, void *key)
{
	/* Find the index of the list where to search. */
	u_int index = ht->hash_function(key) % ht->hmax;

	/* Take every node from the list and verify the key. */
	ll_t *curr_list = ht->buckets[index];
	ll_node_t *curr_node = curr_list->head;
	u_int pos = 0;
	while (curr_node) {
		pair_t *pair = (pair_t *)curr_node->data;
		if (!ht->compare_function(pair->key, key)) {
			/* Take out the node from the list. */
			curr_node = ll_remove_nth_node(curr_list, pos);

			/* Free the memory/ */
			free_ll_node(curr_node, curr_list->free_data);

			/* Update the parameters of the hashtable. */
			ht->size--;

			/* We finished our work. */
			return;
		}
		curr_node = curr_node->next;
		pos++;
	}
}

void ht_free(ht_t **ht_ptr)
{
	/* Get to the hashmap. */
	ht_t *ht = *ht_ptr;

	/* Verify the parameter. */
	if (!ht) {
		fprintf(stderr, "ht_free() - hashtable isn't valid\n");
		return;
	}

	/* Free the memory of every list from array. */
	for (u_int i = 0; i < ht->hmax; ++i) {
		ll_free(&ht->buckets[i]);
	}

	/* Free the array of lists. */
	free(ht->buckets);

	/* Free the hashtable structure's memory. */
	free(ht);

	/* Lose the address of the hashtable, which doesn't exist anymore. */
	*ht_ptr = NULL;
}

u_int ht_get_size(ht_t *ht)
{
	if (!ht)
		return 0;
	return ht->size;
}
