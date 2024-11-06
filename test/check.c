#include "../lib/acutest.h"

void test_first(void)
{
    TEST_CHECK(1 == 2);
}

void test_second(void)
{
    TEST_CHECK(1 == 1);
}

TEST_LIST = {
    { "first", test_first },
    { "second", test_second },
    { NULL, NULL }
};
