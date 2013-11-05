#ifndef ASSERT_H
#define ASSERT_H

#include <cstddef>
#include <string>
#include <sstream>
#include <stdexcept>
#include <functional>

namespace test
{
    /*
        The base assertion-exception class.
    */
    class assert_exception: public std::logic_error
    {
        public:
        /*
            Creates an assertion exception with the file, function, and line number.
        */
        assert_exception(const std::string &msg,
            const std::string &file, const std::string &func, std::size_t lineno);

        /*
            Creates an assertion exception without a prefix.
        */
        assert_exception(const std::string &msg);
    };

    /*
        Throws an exception at the current line in the current file with a given message.
     */
    #define TEST_FAIL(msg) throw ::test::assert_exception((msg), __FILE__, __func__, __LINE__)

    void assert_true(const std::string &expr, bool, const std::string &msg,
        const std::string &file, const std::string &func, std::size_t lineno);
    void assert_true(const std::string &expr, bool, const std::string &msg);

    /*
        Asserts that an expression is true.
        If not, throws an exception at the current line in the current file
        whose message contains the expression.
     */
    #define TEST_ASSERT_TRUE_MSG(expr, msg) ::test::assert_true(#expr, (expr), (msg), __FILE__, __func__, __LINE__)
    #define TEST_ASSERT_TRUE(expr) TEST_ASSERT_TRUE_MSG(expr, "")

    void assert_false(const std::string &expr, bool, const std::string &msg,
        const std::string &file, const std::string &func, std::size_t lineno);
    void assert_false(const std::string &expr, bool, const std::string &msg);
    /*
     * Asserts that an expression is false.
     * If not, throws an exception at the current line in the current file
     * whose message contains the expression.
     */
    #define TEST_ASSERT_FALSE_MSG(expr, msg) ::test::assert_false(#expr, (expr), (msg), __FILE__, __func__, __LINE__)
    #define TEST_ASSERT_FALSE(expr) TEST_ASSERT_FALSE_MSG(expr, "")

    template <class T>
    std::string assert_equals_msg
    (
        const std::string &expected_expr, const T &expected, const std::string &actual_expr, const T &actual,
        const std::string &msg
    )
    {
        std::stringstream expected_stream;
        expected_stream << expected;
        std::string expected_string = expected_stream.str();

        std::stringstream actual_stream;
        actual_stream << actual;
        std::string actual_string = actual_stream.str();

        return assert_equals_msg(expected_expr, expected_string, actual_expr, actual_string, msg);
    }

    template <class T>
    void assert_equals
    (
        const std::string &expected_expr, const T &expected, const std::string &actual_expr, const T &actual,
        const std::string &msg, const std::string &file, const std::string &func, std::size_t lineno
    )
    {
        if (expected != actual)
            throw assert_exception(assert_equals_msg(expected_expr, expected, actual_expr, actual, msg),
                file, func, lineno);
    }

    template <class T>
    void assert_equals
    (
        const std::string &expected_expr, const T &expected, const std::string &actual_expr, const T &actual,
        const std::string &msg
    )
    {
        if (expected != actual)
            throw assert_exception(assert_equals_msg(expected_expr, expected, actual_expr, actual, msg));
    }

    /*
     * Asserts that an expected expression matches an actual expression of the same type.
     * If not, throws an exception at the current line in the current file
     * whose message consists of the two expressions and their values,
     * followed by the output of an invocation to the diff tool if possible.
     */
    #define TEST_ASSERT_EQUALS_MSG(type, expectedExpr, actualExpr, msg) ::test::assert_equals<type>(#expectedExpr, (expectedExpr), #actualExpr, (actualExpr), msg, __FILE__, __func__, __LINE__)
    #define TEST_ASSERT_EQUALS(type, expectedExpr, actualExpr) TEST_ASSERT_EQUALS_MSG(type, expectedExpr, actualExpr, "")
};

#endif
