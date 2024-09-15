#include <assert.h>

#include "../Unity/src/unity.h"
#include "../../src/ADTs/darray.h"

DArray *da;
int length;

void setUp(void) {
    //initializes darray with elements 0..9
    da = darray_init(free);
    length = 10;

    for (int i = 0; i < length; i++) {
        int *num = malloc(sizeof(int));
        assert(num != NULL);

        *num = i;
        darray_add(da, num);
    }
}

void tearDown(void) {
    darray_free(da);
}

// static void print_int(FILE *file, void *element) {
//     int *num = element;
//     fprintf(file, "%d", *num);
// }

void test_add(void) {
    TEST_ASSERT_EQUAL(length, darray_length(da));
    darray_add(da, NULL);
    TEST_ASSERT_EQUAL(length + 1, darray_length(da));
}

void test_get(void) {
    for (int i = 0; i < length; i++) {
        int *num = darray_get(da, i);
        TEST_ASSERT_EQUAL(i, *num);
    }
}

void test_set(void) {
    for (int i = 0; i < length; i++) {
        int *num = malloc(sizeof(int));
        assert(num != NULL);
        *num = 10 - i;
        darray_set(da, i, num);
    }

    for (int i = 0; i < length; i++) {
        int *num = darray_get(da, i);
        TEST_ASSERT_EQUAL(length - i, *num);
    }
}

static int compar_int(const void *element1, const void *element2) {
    const int *num1 = element1;
    const int *num2 = element2;
    return *num1 - *num2;
}

void test_index_of(void) {
    int target = 3;
    TEST_ASSERT_EQUAL(target, darray_index_of(da, &target, compar_int));

    target = 10; //should not be in the array
    TEST_ASSERT_EQUAL(-1, darray_index_of(da, &target, compar_int));
}

void test_remove(void) {
    int index = length - 1;
    length--;
    int *element = darray_remove(da, index);
    TEST_ASSERT_EQUAL(index, *element);
    TEST_ASSERT_EQUAL(length, darray_length(da));
    free(element);

    index = 0;
    element = darray_remove(da, index);
    TEST_ASSERT_EQUAL(index, *element);
    TEST_ASSERT_EQUAL(length - 1, darray_length(da));

    for (int i = index; i < darray_length(da); i++) {
        int *num = darray_get(da, i);
        TEST_ASSERT_EQUAL(i + 1, *num);
    }

    free(element);
}

static void sum_helper(int index, void *element, void *state) {
    int *sum = state;
    int *num = element;

    *sum += *num;
}

void test_for_each(void) {
    int sum = 0;
    darray_for_each(da, sum_helper, &sum);
    TEST_ASSERT_EQUAL(length * (length - 1) / 2, sum);
}

void test_iterator(void) {
    int index = 0;
    int **pelement = malloc(sizeof(int *));
    int i = 0;

    while (darray_iterator(da, &index, (void **) pelement)) {
        TEST_ASSERT_EQUAL(i, index - 1);
        TEST_ASSERT_EQUAL(i, **pelement);
        i++;
    }
    
    free(pelement);
}

void test_clear(void) {
    darray_clear(da);
    TEST_ASSERT_EQUAL(0, darray_length(da));
}

void test_resize(void) {
    for (int i = 0; i < 100; i++) {
        int *num = malloc(sizeof(int));
        *num = i;
        darray_add(da, num);
        length++;
    }

    TEST_ASSERT_EQUAL(length, darray_length(da));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_get);
    RUN_TEST(test_set);
    RUN_TEST(test_index_of);
    RUN_TEST(test_remove);
    RUN_TEST(test_for_each);
    RUN_TEST(test_iterator);
    RUN_TEST(test_clear);
    RUN_TEST(test_resize);
    return UNITY_END();
}
