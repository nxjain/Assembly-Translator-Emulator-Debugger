/**
 * @file darray.c
 * @brief Implementation file for a dynamic array (DArray) data structure.
 *
 * This file provides functions to initialize, manipulate, and free a dynamic array.
 * It supports operations such as adding elements, retrieving elements by index,
 * setting elements at specific indices, removing elements, iterating over elements,
 * printing elements, and clearing/freeing the array.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "../utils.h"
#include "darray.h"

#define INITIAL_CAPACITY 10

#define check_index_in_bounds(da, index) do { \
    assert_msg((index >= 0 && index < da->length), "Index %d out of bounds for array length %d\n", index, da->length); \
} while (0)


// Structure definition for the dynamic array (DArray)
struct DArray {
    void **array;                   // Array to store elements
    int length;                     // Current number of elements in the array
    int capacity;                   // Capacity of the array (max elements it can currently hold)
    void (*free_element)(void *element);  // Function pointer to free elements (optional)
};

/**
 * @brief Initializes a dynamic array (DArray) with a given free element function.
 * @param free_element Function pointer to free each element when the array is cleared or freed.
 * @return Initialized DArray pointer.
 * @note The initial capacity of the array is set to INITIAL_CAPACITY.
 */
DArray *darray_init(void (*free_element)(void *element)) {
    DArray *da = malloc(sizeof(DArray));
    assert_msg(da != NULL, "Memory allocation failed\n");

    da->array = malloc(INITIAL_CAPACITY * sizeof(void *));
    assert_msg(da->array != NULL, "Memory allocation failed\n");

    da->length = 0;
    da->capacity = INITIAL_CAPACITY;
    da->free_element = free_element;

    return da;
}

/**
 * @brief Resizes the dynamic array if the current length equals its capacity.
 * @param da Dynamic array to check and resize if necessary.
 */
static void resize_if_necessary(DArray *da) {
    if (da->length == da->capacity) {
        da->capacity *= 2;
        da->array = realloc(da->array, da->capacity * sizeof(void *));
        assert_msg(da->array != NULL, "Memory allocation failed\n");
    }
}

/**
 * @brief Retrieves the element at the specified index in the dynamic array.
 * @param da Dynamic array from which to retrieve the element.
 * @param index Index of the element to retrieve.
 * @return Pointer to the element at the specified index.
 * @note Asserts if the dynamic array pointer is NULL or if the index is out of bounds.
 */
void *darray_get(const DArray *da, int index) {
    assert_msg(da != NULL, "Dynamic array pointer passed in is null.\n");
    check_index_in_bounds(da, index);

    return da->array[index];
}

/**
 * @brief Adds an element to the end of the dynamic array.
 * @param da Dynamic array to which the element is added.
 * @param element Element to be added to the dynamic array.
 * @note Asserts if the dynamic array pointer is NULL or if the array is full (length equals capacity).
 */
void darray_add(DArray *da, void *element) {
    assert_msg(da != NULL, "Dynamic array pointer passed in is null.\n");

    resize_if_necessary(da);
    assert(da->length < da->capacity);
    da->array[da->length++] = element;
}

/**
 * @brief Sets an element at the specified index in the dynamic array.
 * @param da Dynamic array in which to set the element.
 * @param index Index at which to set the element.
 * @param element Element to set in the dynamic array.
 * @note Asserts if the dynamic array pointer is NULL or if the index is out of bounds.
 *       Frees the existing element at the index if a free element function is provided.
 */
void darray_set(DArray *da, int index, void *element) {
    assert_msg(da != NULL, "Dynamic array pointer passed in is null.\n");
    check_index_in_bounds(da, index);

    if (da->free_element != NULL) {
        da->free_element(da->array[index]);
    }

    da->array[index] = element;
}

/**
 * @brief Returns the current number of elements in the dynamic array.
 * @param da Dynamic array for which to retrieve the length.
 * @return Current number of elements in the dynamic array.
 */
int darray_length(const DArray *da) {
    return da->length;
}

/**
 * @brief Finds the index of a specific element in the dynamic array using a comparison function.
 * @param da Dynamic array in which to search for the element.
 * @param element Element to search for.
 * @param cmpf Comparison function to determine equality between elements.
 * @return Index of the element if found, otherwise -1.
 * @note Asserts if the dynamic array pointer or the comparison function pointer is NULL.
 */
int darray_index_of(const DArray *da, const void *element, __compar_fn_t cmpf) {
    assert_msg(da != NULL, "Dynamic array pointer passed in is null.\n");
    assert_msg(cmpf != NULL, "Comparison function passed in is null.\n" );

    for (int i = 0; i < da->length; i++) {
        if (cmpf(da->array[i], element) == 0) {
            return i;
        }
    }

    return -1;
}

/**
 * @brief Removes and returns the element at the specified index in the dynamic array.
 * @param da Dynamic array from which to remove the element.
 * @param index Index of the element to remove.
 * @return Removed element.
 * @note Asserts if the dynamic array pointer is NULL or if the index is out of bounds.
 */
void *darray_remove(DArray *da, int index) {
    assert_msg(da != NULL, "Dynamic array pointer passed in is null.\n");
    check_index_in_bounds(da, index);

    void *element = da->array[index];
    da->length--;
    memmove(da->array + index, da->array + index + 1, (da->length - index) * sizeof(void *));
    return element;
}

/**
 * @brief Applies a callback function to each element in the dynamic array.
 * @param da Dynamic array to iterate over.
 * @param call_back Callback function to apply to each element.
 * @param state State parameter passed to the callback function.
 * @note Asserts if the dynamic array pointer or the callback function pointer is NULL.
 */
void darray_for_each(DArray *da, void (*call_back)(int index, void *element, void *state), void *state) {
    assert_msg(da != NULL, "Dynamic array pointer passed in is null.\n");
    assert_msg(call_back != NULL, "Callback function pointer passed in is null.\n");

    for (int i = 0; i < da->length; i++) {
        call_back(i, da->array[i], state);
    }
}

/**
 * @brief Iterates over the dynamic array, retrieving elements sequentially.
 * @param da Dynamic array to iterate over.
 * @param i Pointer to the current index (updated by reference).
 * @param pelement Pointer to store the retrieved element (updated by reference).
 * @return true if an element is retrieved successfully, false otherwise.
 * @note Asserts if the dynamic array pointer is NULL.
 *       This function is typically used for sequential iteration over elements.
 */
bool darray_iterator(const DArray *da, int *i, void **pelement) {
    assert_msg(da != NULL, "Dynamic array pointer passed in is null.\n");
    
    *(pelement) = da->array[(*i)++];
    return da->length != *i - 1;
}

/**
 * @brief Helper function to print an element's memory address to a file.
 * @param file File pointer to which the element's address is printed.
 * @param element Element whose address is printed.
 */
static void print_address_helper(FILE *file, void *element) {
    fprintf(file, "%p", element);
}

/**
 * @brief Prints the elements of the dynamic array to a specified file.
 * @param da Dynamic array to print.
 * @param file File pointer to which the elements are printed.
 * @param print_element Function pointer to custom print function for each element (optional).
 * @note Asserts if the dynamic array pointer or the file pointer is NULL.
 *       Uses a default printing function if a custom print function is not provided.
 */
void darray_print(const DArray *da, FILE *file, void (*print_element)(FILE *file, void *element)) {
    assert_msg(da != NULL, "Dynamic array pointer passed in is null.\n");
    assert_msg(file != NULL, "File pointer passed in is null.\n");

    if (print_element == NULL) print_element = print_address_helper;

    fprintf(file, "[");

    for (int i = 0; i < da->length; i++) {
        print_element(file, da->array[i]);

        if (i != da->length - 1) {
            fprintf(file, ", ");
        }
    }

    fprintf(file, "]\n");
}

/**
 * @brief Frees all elements in the dynamic array using its free element function (if provided).
 * @param da Dynamic array to clear.
 * @note Asserts if the dynamic array pointer is NULL.
 */
static void free_elements(DArray *da) {
    assert_msg(da != NULL, "Dynamic array pointer passed in is null.\n");

    if (da->free_element != NULL) {
        for (int i = 0; i < da->length; i++) {
            da->free_element(da->array[i]);
        }
    }
}

/**
 * @brief Clears the dynamic array by removing all elements and resetting its length to zero.
 * @param da Dynamic array to clear.
 * @note Asserts if the dynamic array pointer is NULL.
 */
void darray_clear(DArray *da) {
    assert_msg(da != NULL, "Dynamic array pointer passed in is null.\n");

    free_elements(da);
    da->length = 0;
}

/**
 * @brief Frees the entire dynamic array, including its array, elements, and the array structure itself.
 * @param da Dynamic array to free.
 */
void darray_free(void *da) {
    assert_msg(da != NULL, "Dynamic array pointer passed in is null.\n");

    DArray *arr = da;

    free_elements(arr);
    free(arr->array);
    free(arr);
}
