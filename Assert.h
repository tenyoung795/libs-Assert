#ifndef MYASSERT_H
#define MYASSERT_H

#include <cstddef>
#include <string>
#include <sstream>
#include <stdexcept>
#include <functional>

/*
 * The base assertion-exception class.
 */
class AssertException: public std::logic_error
{
    public:
    /*
     * Creates an assertion exception whose what() is
     *  file:line: msg
     */
    AssertException(const std::string &file, size_t line, const std::string &msg);
};

/*
 * Throws an exception at the current line in the current file with a given message.
 */
#define fail(msg) throw AssertException(__FILE__, __LINE__, (msg))

void assertTrueExplicit(const std::string &file, size_t line, const std::string &expr, bool, const std::string &msg);
/*
 * Asserts that an expression is true.
 * If not, throws an exception at the current line in the current file
 * whose message contains the expression.
 */
#define assertTrueMsg(expr, msg) assertTrueExplicit(__FILE__, __LINE__, #expr, (expr), (msg))
#define assertTrue(expr) assertTrueMsg(expr, "")

void assertFalseExplicit(const std::string &file, size_t line, const std::string &expr, bool, const std::string &msg);
/*
 * Asserts that an expression is false.
 * If not, throws an exception at the current line in the current file
 * whose message contains the expression.
 */
#define assertFalseMsg(expr, msg) assertFalseExplicit(__FILE__, __LINE__, #expr, (expr), (msg))
#define assertFalse(expr) assertFalseMsg(expr, "")

template <class T>
std::string assertEqualsMessage(const std::string &expectedExpr, const T &expected, const std::string &actualExpr, const T &actual, const std::string &msg)
{
    std::stringstream expectedStream;
    expectedStream << expected;
    std::string expectedString = expectedStream.str();

    std::stringstream actualStream;
    actualStream << actual;
    std::string actualString = actualStream.str();

    return assertEqualsMessage(expectedExpr, expectedString, actualExpr, actualString, msg);
}

template <class T>
class AssertEqualsException: public AssertException
{
    public:
    AssertEqualsException(const std::string &file, size_t line,
        const std::string &expectedExpr, const T &expected, const std::string &actualExpr, const T &actual, const std::string &msg):
        AssertException(file, line, assertEqualsMessage(expectedExpr, expected, actualExpr, actual, msg)) {}
};

template <class T>
void assertEqualsExplicit(const std::string &file, size_t line,
    const std::string &expectedExpr, const T &expected, const std::string &actualExpr, const T &actual, const std::string &msg)
    throw(AssertException)
{
    if (expected != actual)
    {
        throw AssertEqualsException<T>(file, line, expectedExpr, expected, actualExpr, actual, msg);
    }
}

/*
 * Asserts that an expected expression matches an actual expression of the same type.
 * If not, throws an exception at the current line in the current file
 * whose message consists of the two expressions and their values,
 * followed by the output of an invocation to the diff tool if possible.
 */
#define assertEqualsMsg(type, expectedExpr, actualExpr, msg) assertEqualsExplicit<type>(__FILE__, __LINE__, #expectedExpr, (expectedExpr), #actualExpr, (actualExpr), msg)
#define assertEquals(type, expectedExpr, actualExpr) assertEqualsMsg(type, expectedExpr, actualExpr, "")

#endif
