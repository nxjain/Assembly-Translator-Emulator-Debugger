/**
 * @file hashmap.c
 * @brief Implementation of a hashmap data structure with string keys and void pointer values.
 *
 * This file provides functions to initialize, manipulate, and manage a hashmap, which associates
 * string keys with void pointer values. It supports operations such as insertion, retrieval,
 * removal, resizing, and freeing of memory associated with the hashmap and its elements.
 *
 * The hashmap uses separate chaining for collision resolution and dynamically resizes its array
 * of buckets to maintain a load factor of 0.75 for efficient performance.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "../utils.h"
#include "hashmap.h"

#define INITIAL_NUM_BUCKETS 32
#define LOAD_FACTOR 0.75

typedef struct Item Item;

struct Item {
    char *key;
    void *value;
    Item *next;
};

struct HashMap {
    Item **buckets;
    int size;
    int num_buckets;
    void (*free_value)(void *val);
};

typedef uint64_t Hash;

//------------------------------------UTILITY FUNCTIONS------------------------------------

/**
 * @brief Initializes a new Item structure with the provided key, value, and next pointer.
 *
 * @param key The key associated with the item.
 * @param value The value associated with the item.
 * @param next Pointer to the next item in case of collisions.
 * @return Item* Pointer to the newly initialized item.
 */
static Item *item_init(const char *key, void *value, Item *next) {
    Item *item = malloc(sizeof(Item));
    assert_msg(item != NULL, "Memory allocation failed\n");

    item->key = strdup(key); // Duplicate the key string
    assert_msg(item->key != NULL, "Memory allocation failed\n");

    item->value = value;
    item->next  = next;

    return item;
}

/**
 * @brief Traverses the linked list of items to find and return an item with the matching key.
 *
 * @param head Pointer to the head of the linked list of items.
 * @param key The key to search for.
 * @return Item* Pointer to the item if found, NULL otherwise.
 */
static Item *item_list_get_item(Item *head, const char *key) {
    while (head != NULL) {
        if (strcmp(head->key, key) == 0) return head; // Match found
        head = head->next;
    }
    
    return NULL; // Key not found
}

/**
 * @brief Creates and initializes a new array of item buckets with the specified size.
 *
 * @param size Number of buckets to create.
 * @return Item** Pointer to the array of item buckets.
 */
static Item **create_new_buckets(int size) {
    assert_msg(size > 0, "Size must be greater than 0\n");

    Item **buckets = calloc(size, sizeof(Item *));
    assert_msg(buckets != NULL, "Memory allocation failed\n");

    return buckets;
}

//------------------------------------HASHMAP FUNCTIONS------------------------------------

/**
 * @brief Initializes a new HashMap structure.
 *
 * @param free_value Function pointer to free the value associated with each item (can be NULL).
 * @return HashMap* Pointer to the newly initialized hashmap.
 */
HashMap *hashmap_init(void (*free_value)(void *value)) {
    HashMap *hmap = malloc(sizeof(HashMap));
    assert_msg(hmap != NULL, "Memory allocation failed\n");

    hmap->size        = 0; 
    hmap->num_buckets = INITIAL_NUM_BUCKETS;
    hmap->buckets     = create_new_buckets(hmap->num_buckets);
    hmap->free_value  = free_value;
        
    return hmap;
}

/**
 * @brief Computes the hash value of the given key.
 *
 * @param key The key to compute the hash value for.
 * @return Hash The computed hash value.
 */
static Hash get_hash(const unsigned char *key) {
    Hash hash = 0x1505;
    int c;
    while ((c = *key++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    assert_msg(hash >= 0, "lol");
    
    return hash;
}

/**
 * @brief Computes the index of the bucket where the item with the given key should reside.
 *
 * @param hmap Pointer to the hashmap.
 * @param key The key used to compute the bucket index.
 * @return int The computed bucket index.
 */
static int bucket_index(const HashMap *hmap, const char *key) {
    return get_hash((const unsigned char *)key) & (hmap->num_buckets - 1);
}

/**
 * @brief Resizes the hashmap if necessary based on the load factor.
 *
 * @param hmap Pointer to the hashmap.
 */
static void resize_if_necessary(HashMap *hmap) {
    if (hmap->size < hmap->num_buckets * LOAD_FACTOR) {
        return; // No need to resize
    }

    // Store current buckets information
    Item **previous_buckets  = hmap->buckets;
    int previous_num_buckets = hmap->num_buckets;

    // Double the number of buckets
    hmap->num_buckets *= 2;
    hmap->buckets = create_new_buckets(hmap->num_buckets);

    // Reinsert all items into the new buckets
    Item *head;
    Item *temp;
    for (int i = 0; i < previous_num_buckets; i++) {
        head = previous_buckets[i];

        while (head != NULL) {
            temp = head;
            head = head->next;
            temp->next = NULL;

            int index = bucket_index(hmap, temp->key);

            // Insert at the beginning of the bucket list
            if (hmap->buckets[index] == NULL) {
                hmap->buckets[index] = temp;
            } else {
                temp->next = hmap->buckets[index];
                hmap->buckets[index] = temp;
            }
        }
    }

    // Free the memory allocated for previous buckets
    free(previous_buckets);
}

/**
 * @brief Checks if the hashmap contains an item with the specified key.
 *
 * @param hmap Pointer to the hashmap.
 * @param key The key to search for.
 * @return true If the hashmap contains an item with the specified key.
 * @return false Otherwise.
 */
bool hashmap_contains(const HashMap *hmap, const char *key) {
    return item_list_get_item(hmap->buckets[bucket_index(hmap, key)], key) != NULL;
}

/**
 * @brief Retrieves the value associated with the specified key from the hashmap.
 *
 * @param hmap Pointer to the hashmap.
 * @param key The key to retrieve the value for.
 * @return void* Pointer to the value if found, NULL otherwise.
 */
void *hashmap_get(const HashMap *hmap, const char *key) {
    Item *item = item_list_get_item(hmap->buckets[bucket_index(hmap, key)], key);
    
    if (item != NULL) {
        return item->value;
    }
    return NULL;
}

/**
 * @brief Sets the value associated with the specified key in the hashmap.
 *
 * @param hmap Pointer to the hashmap.
 * @param key The key to set the value for.
 * @param value The value to set.
 * @return void* Previous value associated with the key if exists, NULL otherwise.
 */
void *hashmap_set(HashMap *hmap, const char *key, void *value) {
    int index = bucket_index(hmap, key);
    Item **buckets = hmap->buckets;

    // If bucket is empty, create a new item and add to the bucket
    if (buckets[index] == NULL) {
        buckets[index] = item_init(key, value, NULL);
        hmap->size++;
        resize_if_necessary(hmap);
        return NULL;
    }

    // Traverse the linked list in the bucket
    Item *head = buckets[index];
    while (head != NULL) {
        if (strcmp(head->key, key) == 0) {
            // Update the value and return the previous value
            void *old_value = head->value;
            head->value = value;
            return old_value;
        }
        head = head->next;
    }

    // If key not found, create a new item and add to the beginning of the list
    Item *item = item_init(key, value, buckets[index]);
    buckets[index] = item;
    hmap->size++;
    resize_if_necessary(hmap);
    return NULL;
}

/**
 * @brief Returns the current size (number of items) in the hashmap.
 *
 * @param hmap Pointer to the hashmap.
 * @return int Current size of the hashmap.
 */
int hashmap_size(HashMap *hmap) {
    return hmap->size;
}

/**
 * @brief Removes the item with the specified key from the hashmap.
 *
 * This function removes the item associated with the given key from the hashmap.
 * If the key is found, the associated value is returned and the memory for the item
 * is freed. If the key is not found, NULL is returned.
 *
 * @param hmap Pointer to the hashmap.
 * @param key The key of the item to remove.
 * @return void* Value associated with the removed key if found, NULL otherwise.
 */
void *hashmap_remove(HashMap *hmap, const char *key) {
    int index = bucket_index(hmap, key);

    if (hmap->buckets[index] == NULL) {
        return NULL; // Bucket is empty
    }

    // If the item to remove is the first in the list
    if (strcmp(hmap->buckets[index]->key, key) == 0) {
        Item *item = hmap->buckets[index];
        hmap->buckets[index] = item->next;
        hmap->size--;
        void *value = item->value;
        free(item->key);
        free(item);
        return value;
    }

    // Traverse the linked list to find and remove the item
    Item *head = hmap->buckets[index];
    while (head->next != NULL) {
        if (strcmp(head->next->key, key) == 0) {
            Item *item = head->next;
            head->next = head->next->next;
            hmap->size--;
            void *value = item->value;
            free(item->key);
            free(item);
            return value;
        }
        head = head->next;
    }
    
    return NULL;
}

/**
 * @brief Frees the memory occupied by all items in a given bucket.
 *
 * This function frees the memory occupied by all items in a given bucket of the hashmap.
 * It iterates through the linked list of items, freeing the memory for each item's key,
 * value, and the item structure itself.
 *
 * @param head Pointer to the first item in the bucket's linked list.
 * @param free_value Function pointer to a function that frees the value associated with each item.
 */
static void free_bucket(Item *head, void (*free_value)(void *value)) {
    if (head == NULL) {
        return; // Bucket is already empty
    }
    
    Item *temp;
    while (head != NULL) {
        temp = head;
        head = head->next;

        free(temp->key);
        if (free_value != NULL) {
            free_value(temp->value);
        }
        free(temp);
    }
}

/**
 * @brief Clears the hashmap, freeing all memory allocated to it.
 *
 * This function clears the hashmap by freeing all memory occupied by its buckets,
 * and then re-initializing the hashmap with the initial number of buckets and size.
 *
 * @param hmap Pointer to the hashmap.
 */
void hashmap_clear(HashMap *hmap) {
    for (int i = 0; i < hmap->num_buckets; i++) {
        free_bucket(hmap->buckets[i], hmap->free_value);
    }
    free(hmap->buckets);

    hmap->buckets = create_new_buckets(INITIAL_NUM_BUCKETS);
    hmap->num_buckets = INITIAL_NUM_BUCKETS;
    hmap->size = 0;
}

/**
 * @brief Frees the memory occupied by the hashmap and all its items.
 *
 * This function frees the memory occupied by the hashmap itself, including all its buckets,
 * items within each bucket, and associated key-value pairs. It also calls the provided
 * free_value function for each item's value if it is specified.
 *
 * @param hmap Pointer to the hashmap.
 */
void hashmap_free(HashMap *hmap) {
    for (int i = 0; i < hmap->num_buckets; i++) {
        free_bucket(hmap->buckets[i], hmap->free_value);
    }

    free(hmap->buckets);
    free(hmap);
}
