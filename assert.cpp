#include "assert.h"

#include <cstddef>
#include <cstdlib>
#include <string>
#include <cstdio>
#include <sstream>
#include <ostream>
#include <system_error>
#include <cerrno>

test::assert_exception::assert_exception
(
    const std::string &msg,
    const std::string &file, const std::string &func, std::size_t line):
    std::logic_error(file + ':' + func + ':' + std::to_string(line) + ": " + msg
)
{} 

test::assert_exception::assert_exception(const std::string &msg):
    std::logic_error(msg)
{} 

namespace test
{
    template <bool b>
    std::string assert_bool_msg(const std::string &expr, const std::string &msg)
    {
        return "Expected " + expr + " to be " + (b? "true" : "false") + (msg.empty()? "" : (": " + msg));
    }

    std::string assert_true_msg(const std::string &expr, const std::string &msg)
    {
        return assert_bool_msg<true>(expr, msg);
    }

    std::string assert_false_msg(const std::string &expr, const std::string &msg)
    {
        return assert_bool_msg<false>(expr, msg);
    }
};

void test::assert_true(const std::string &expr, bool b, const std::string &msg,
    const std::string &file, const std::string &func, std::size_t lineno)
{
    if (!b) throw assert_exception(assert_true_msg(expr, msg), file, func, lineno);
}

void test::assert_true(const std::string &expr, bool b, const std::string &msg)
{
    if (!b) throw assert_exception(assert_true_msg(expr, msg));
}

void test::assert_false(const std::string &expr, bool b, const std::string &msg,
    const std::string &file, const std::string &func, std::size_t lineno)
{
    if (b) throw assert_exception(assert_false_msg(expr, msg), file, func, lineno);
}

void test::assert_false(const std::string &expr, bool b, const std::string &msg)
{
    if (b) throw assert_exception(assert_false_msg(expr, msg));
}

namespace std
{
    system_error errno_as_system_error()
    {
        return system_error(errno, system_category());
    }   
};

namespace test
{
    template <>
    std::string assert_equals_msg
    (
        const std::string &expected_expr, const std::string &expected,
        const std::string &actual_expr, const std::string &actual,
        const std::string &msg
    )
    {
        std::stringstream s;
        if (!msg.empty()) s << msg;
        s   << "\nExpected:\n" << expected_expr << "\n==\n" << expected
            << "\nActual:\n" << actual_expr << "\n==\n" << actual;

        // POSIX only (requires diff):
        bool can_diff;
        int status = std::system("command -v diff > /dev/null");
        if (status == -1)
        {
            can_diff = false;
            s << "\nCannot determine whether diff exists: " << std::errno_as_system_error().what(); 
        }
        else
            can_diff = status == 0;

        if (can_diff)   
            try
            {
                static const auto temp_file_with = [&](const std::string &content)
                {
                    char name[] = "/tmp/Assert.XXXXXX";
                    int fd = mkostemp(name, 0600);
                    if (fd == -1) throw std::errno_as_system_error();

                    std::FILE *file = fdopen(fd, "a");
                    if (file == nullptr) 
                    {
                        // attempt to use low-level API instead
                        while (write(fd, content.data(), content.size()) == -1)
                            if (errno != EINTR) throw std::errno_as_system_error();
                        if (close(fd) == -1) throw std::errno_as_system_error();
                    }
                    else if (std::fputs(content.data(), file) == EOF || std::fclose(file) == EOF)
                        throw std::errno_as_system_error();

                    return std::string(name);
                };

                std::string expected_name(temp_file_with(expected));
                std::string actual_name(temp_file_with(actual));
                std::string command("diff -u " + expected_name + ' ' + actual_name);
                class piped_output
                {
                    std::ostream &log;
                    
                    public:
                    std::FILE *const p;
                    piped_output(std::string &command, std::stringstream &stream): log(stream), p(popen(command.data(), "r"))
                    {
                        if (p == nullptr) throw std::errno_as_system_error();
                    }

                    ~piped_output()
                    {
                        if (pclose(p) == -1)
                            log << "\nWarning - error on closing pipe to diff: " << std::errno_as_system_error().what();
                    }
                } p(command, s);

                s << "\nDiff:\n";

                // discard two lines
                // do not attempt to replace the top two lines if this failed
                if (std::fscanf(p.p, "%*[^\n]\n%*[^\n]\n") != EOF)
                    s << "--- expected\n+++ actual\n";
                
                char buffer[64];
                while (std::fgets(buffer, sizeof(buffer) - 1, p.p) != nullptr)
                    s << buffer;    
                if (std::ferror(p.p))
                    s << "\nCould not complete diff due to a pipe error";

                static const auto attempt_remove = [&](const std::string &name)
                {
                    if (std::remove(name.data()) == -1)
                        s << "\nWarning: could not remove temporary file: " << std::errno_as_system_error().what();
                };
                attempt_remove(expected_name);
                attempt_remove(actual_name);
            }
            catch (std::system_error &e)
            {
                s << "\nCould not execute diff: " << e.what();
            }
            
        return s.str();
    }
};

