#include <iostream>
#include <functional>
#include <cstdlib>
#include <cassert> // just a little ironic
#include "assert.h"

void print_assert_exception(const std::function<void()> &thunk)
{
    try
    {
        thunk();
        assert("Expected an assert exception" && false);
    }
    catch (const test::assert_exception &e)
    {
        std::cout << e.what() << '\n';
    }
}

int main()
{
    print_assert_exception([]{ TEST_FAIL("Good"); });

    TEST_ASSERT_TRUE(true);
    TEST_ASSERT_TRUE_MSG(true, "Bad");
    
    std::string a("Hello world");
    std::string b("Hello, world");

    print_assert_exception([&]{ TEST_ASSERT_TRUE(a == b); });
    print_assert_exception([&]{ TEST_ASSERT_TRUE_MSG(a == b, "Good"); });

    TEST_ASSERT_FALSE(false);
    TEST_ASSERT_FALSE_MSG(false, "Bad");
    
    print_assert_exception([&]{ TEST_ASSERT_FALSE(a != b); });
    print_assert_exception([&]{ TEST_ASSERT_FALSE_MSG(a != b, "Good"); });

    TEST_ASSERT_EQUALS(std::string, a, "Hello world");
    TEST_ASSERT_EQUALS_MSG(std::string, a, "Hello world", "Bad");

    print_assert_exception([&]{ TEST_ASSERT_EQUALS(std::string, a, b); });
    print_assert_exception([&]{ TEST_ASSERT_EQUALS_MSG(std::string, a, b, "Good"); });

    return EXIT_SUCCESS;
}

