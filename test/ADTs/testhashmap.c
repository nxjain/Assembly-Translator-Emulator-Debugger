#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "../Unity/src/unity.h"
#include "../../src/ADTs/hashmap.h"

HashMap *hmap;
int *n1;
int *n2;
char test_key[] = "TestKey";
char not_test_key[] = "NotTestKey";

void setUp(void) {
    hmap = hashmap_init(free);
}

void tearDown(void) {
    hashmap_free(hmap);
}

void test_clear() {
    n1 = malloc(sizeof(int));
    *n1 = 7;
    hashmap_set(hmap, test_key, n1);
    TEST_ASSERT_EQUAL(n1, hashmap_get(hmap, test_key));
    hashmap_clear(hmap);
    TEST_ASSERT_EQUAL(0, hashmap_size(hmap));
}

void test_contains_when_empty() {
    TEST_ASSERT_FALSE(hashmap_contains(hmap, test_key));
}

void test_contains_after_add() {
    n1 = malloc(sizeof(int));
    *n1 = 3;
    hashmap_set(hmap, test_key, n1);
    TEST_ASSERT_TRUE(hashmap_contains(hmap, test_key));
}

void test_get_returns_null() {
    n1 = malloc(sizeof(int));
    n2 = malloc(sizeof(int));
    *n1 = 3;
    *n2 = 7;
    TEST_ASSERT_EQUAL(NULL, hashmap_set(hmap, not_test_key, n1));
    TEST_ASSERT_EQUAL(n1, hashmap_set(hmap, not_test_key, n2));
    free(n1);
    TEST_ASSERT_NULL(hashmap_get(hmap, test_key));
}

void test_get_returns_latest_value() {
    n1 = malloc(sizeof(int));
    n2 = malloc(sizeof(int));
    *n1 = 3;
    *n2 = 7;
    TEST_ASSERT_EQUAL(NULL, hashmap_set(hmap, test_key, n1));
    TEST_ASSERT_EQUAL(n1, hashmap_set(hmap, test_key, n2));
    free(n1);
    TEST_ASSERT_EQUAL(n2, hashmap_get(hmap, test_key));
}

void test_remove_returns_correct_value() {
    n1 = malloc(sizeof(int));
    n2 = malloc(sizeof(int));
    *n1 = 3;
    *n2 = 7;
    TEST_ASSERT_EQUAL(NULL, hashmap_set(hmap, test_key, n1));
    TEST_ASSERT_EQUAL(n1, hashmap_set(hmap, test_key, n2));
    free(n1);

    TEST_ASSERT_EQUAL(n2, hashmap_remove(hmap, test_key));
    free(n2);
    TEST_ASSERT_NULL(hashmap_remove(hmap, test_key));
}

static char *int_to_str(int num, char *str)
{
    if(str == NULL) {
        return NULL;
    }
    sprintf(str, "%d", num);
    return str;
}

// to test for resize()
void test_large_input_for_each() {
    int test_size = 128;

    char *c = malloc(sizeof(char) * 4);
    for (int i = 0; i < test_size; i++) {
        int *num = malloc(sizeof(int));
        assert(num != NULL);

        *num = i;
        c = int_to_str(i, c);
        hashmap_set(hmap, c, num);
    }

    TEST_ASSERT_EQUAL(test_size, hashmap_size(hmap));

    free(c);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_clear);
    RUN_TEST(test_contains_when_empty);
    RUN_TEST(test_contains_after_add);
    RUN_TEST(test_get_returns_null);
    RUN_TEST(test_get_returns_latest_value);
    RUN_TEST(test_remove_returns_correct_value);
    RUN_TEST(test_large_input_for_each);
    return UNITY_END();
}
