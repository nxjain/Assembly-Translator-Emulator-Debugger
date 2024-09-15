#include "../Unity/src/unity.h"
#include "../../src/emulator/memory.h"

void setUp(void) {
    init_memory();
}

void tearDown(void) {
    // clean stuff up here
}

void test_word() {
    set_word(0, 0x12341234);
    set_word(4, 0x56785678);

    TEST_ASSERT_EQUAL_UINT32(0x12341234, get_word(0));
    TEST_ASSERT_EQUAL_UINT32(0x56785678, get_word(4));
    TEST_ASSERT_EQUAL_UINT32(0, get_word(8));

    set_word(0, 0x5678);
    TEST_ASSERT_EQUAL_UINT32(0x5678, get_word(0));
    TEST_ASSERT_EQUAL_UINT32(0x78000056, get_word(1));

    set_word(1, 0x1234);
    TEST_ASSERT_EQUAL_UINT32(0x00123478, get_word(0));
}

void test_double_word() {   
    set_word(0, 0x12341234);
    set_word(4, 0x56785678);

    TEST_ASSERT_EQUAL_UINT64(0x5678567812341234, get_double_word(0));
    TEST_ASSERT_EQUAL_UINT64(0x0000000056785678, get_double_word(4));

    set_double_word(0, 0x8765432112345678);
    TEST_ASSERT_EQUAL_UINT64(0x8765432112345678, get_double_word(0));
    TEST_ASSERT_EQUAL_UINT32(0x12345678, get_word(0));
    TEST_ASSERT_EQUAL_UINT32(0x87654321, get_word(4));
    set_double_word(1, 0x8765432112345678);
    TEST_ASSERT_EQUAL_UINT64(0x8765432112345678, get_double_word(1));
    TEST_ASSERT_EQUAL_UINT64(0x6543211234567878, get_double_word(0));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_word);
    RUN_TEST(test_double_word);
    return UNITY_END();
}