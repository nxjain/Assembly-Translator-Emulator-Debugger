/**
 * @file hashmap.h
 * @brief Hash Map header file.
 *
 * This header file defines the interface for a Hash Map (HashMap), which is a data structure
 * that associates keys with values, allowing operations such as initialization, key existence check,
 * retrieval, insertion, removal, size query, clearing, and freeing of elements.
 */

#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct HashMap HashMap;

// Initializes a new hash map with the given value freeing function
extern HashMap *hashmap_init(void (*free_value)(void *value));

// Checks if the hash map contains the specified key
extern bool hashmap_contains(const HashMap *hmap, const char *key);

// Retrieves the value associated with the specified key from the hash map
extern void *hashmap_get(const HashMap *hmap, const char *key);

// Inserts or updates the value associated with the specified key in the hash map
extern void *hashmap_set(HashMap *hmap, const char *key, void *val);

// Removes the item with the specified key from the hash map
extern void *hashmap_remove(HashMap *hmap, const char *key);

// Returns the number of key-value pairs in the hash map
extern int hashmap_size(HashMap *hmap);

// Clears all key-value pairs from the hash map, resetting its state
extern void hashmap_clear(HashMap *hmap);

// Frees the memory occupied by the hash map and all its elements
extern void hashmap_free(HashMap *hmap);

#endif /* HASHMAP_H */
