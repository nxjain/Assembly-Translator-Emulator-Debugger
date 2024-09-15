#include "../Unity/src/unity.h"
#include "../../src/emulator/register.h"

void setUp(void) {
    init_register();
}

void tearDown(void) {
    // clean stuff up here
}

void test_general_register() {
    uint64_t value = 0x1234567812345678;
    set_reg_value(0, value);

    TEST_ASSERT_EQUAL_UINT64(value, get_reg_value_64(0));
    TEST_ASSERT_EQUAL_UINT32((uint32_t) value, get_reg_value_32(0));
}

void test_special_register() {
    TEST_ASSERT_EQUAL(0, get_spec_register(ZERO_REGISTER));
    TEST_ASSERT_EQUAL(0, get_spec_register(PROGRAM_COUNTER));
    set_spec_register(PROGRAM_COUNTER, 100);
    TEST_ASSERT_EQUAL(100, get_spec_register(PROGRAM_COUNTER));
    increment_pc();
    TEST_ASSERT_EQUAL(104, get_spec_register(PROGRAM_COUNTER));
    increase_pc(-100);
    TEST_ASSERT_EQUAL(4, get_spec_register(PROGRAM_COUNTER));
    increase_pc(100);
    TEST_ASSERT_EQUAL(104, get_spec_register(PROGRAM_COUNTER));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_general_register);
    RUN_TEST(test_special_register);
    return UNITY_END();
}