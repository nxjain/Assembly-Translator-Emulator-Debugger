/**
 * @file darray.h
 * @brief Dynamic Array (DArray) header file.
 *
 * This header file defines the interface for a Dynamic Array (DArray), which is a resizable array
 * that allows operations such as initialization, access, addition, modification, removal, iteration,
 * printing, clearing, and freeing of elements.
 */

#ifndef DARRAY_H
#define DARRAY_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct DArray DArray;

// Initializes a new dynamic array with the given element freeing function
extern DArray *darray_init(void (*free_element)(void *element));

// Retrieves the element at the specified index
extern void *darray_get(const DArray *da, int index);

// Adds a new element to the end of the dynamic array
extern void darray_add(DArray *da, void *element);

// Sets the element at the specified index to a new value
extern void darray_set(DArray *da, int index, void *element);

// Returns the current length (number of elements) of the dynamic array
extern int darray_length(const DArray *da);

// Finds the index of the element in the dynamic array using a comparison function
extern int darray_index_of(const DArray *da, const void *element, __compar_fn_t cmpf);

// Removes the element at the specified index from the dynamic array
extern void *darray_remove(DArray *da, int index);

// Applies a callback function to each element in the dynamic array
extern void darray_for_each(DArray *da, void (*call_back)(int index, void *element, void *state), void *state);

// Iterates through the dynamic array using an index and retrieves each element
extern bool darray_iterator(const DArray *da, int *i, void **pelement);

// Prints the elements of the dynamic array to a specified file using a print function
extern void darray_print(const DArray *da, FILE *file, void (*print_element)(FILE *file, void *element));

// Clears all elements from the dynamic array, resetting its state
extern void darray_clear(DArray *da);

// Frees the memory occupied by the dynamic array and all its elements
extern void darray_free(void *da);

#endif /* DARRAY_H */
