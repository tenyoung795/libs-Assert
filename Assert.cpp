#include "Assert.h"

#include <cstdlib>
#include <string>
#include <cstdio>
#include <sstream>
#include <ostream>
#include <system_error>
#include <cerrno>

using namespace std;

AssertException::AssertException(const string &file, size_t line, const string &msg):
    logic_error(file + ':' + to_string(line) + ": " + msg) {}


class AssertTrueException: public AssertException
{
    static string message(const string &expr, const string &msg)
    {
        string s("Expected " + expr + " to be true");
        if (!msg.empty()) s += ": " + msg;
        return s;
    }

    public:
    AssertTrueException(const string &file, size_t line, const string &expr, const string &msg):
        AssertException(file, line, message(expr, msg)) {}
};

void assertTrueExplicit(const string &file, size_t line, const string &expr, bool b, const string &msg) throw(AssertException)
{
    if (!b) throw AssertTrueException(file, line, expr, msg);
}

class AssertFalseException: public AssertException
{
    static string message(const string &expr, const string &msg)
    {
        string s("Expected " + expr + " to be false");
        if (!msg.empty()) s += ": " + msg;
        return s;
    }

    public:
    AssertFalseException(const string &file, size_t line, const string &expr, const string &msg):
        AssertException(file, line, message(expr, msg)) {}
};

void assertFalseExplicit(const string &file, size_t line, const string &expr, bool b, const string &msg) throw(AssertException)
{
    if (b) throw AssertFalseException(file, line, expr, msg);
}

template <>
string assertEqualsMessage(const string &expectedExpr, const string &expected, const string &actualExpr, const string &actual,
                           const string &msg)
{
    stringstream s;
    if (!msg.empty()) s << msg;
    s   << "\nExpected:\n" << expectedExpr << "\n==\n" << expected
        << "\nActual:\n" << actualExpr << "\n==\n" << actual;

    static const auto errnoAsSystemError = []()
    {
        return system_error(errno, system_category());
    };

    // POSIX only (requires diff):
    bool canDiff;
    try
    {
        int status = system("command -v diff > /dev/null");
        if (status == -1) throw errnoAsSystemError();
        canDiff = status == 0;
    }
    catch (system_error &e)
    {
        canDiff = false;
        s << "\nCannot determine whether diff exists: " << e.what(); 
    }

    if (canDiff)   
    {
        try
        {
            static const auto createTempFileWithContent = [&](const string &content)
            {
                char name[] = "/tmp/Assert.XXXXXX";
                int fd = mkostemp(name, 0600);
                if (fd == -1) throw errnoAsSystemError();

                FILE *file = fdopen(fd, "a");
                if (file == nullptr) 
                {
                    // attempt to use low-level API instead
                    while (write(fd, content.data(), content.size()) == -1)
                    {
                        if (errno != EINTR) throw errnoAsSystemError();
                    }
                    if (close(fd) == -1) throw errnoAsSystemError();
                }
                else if (fputs(content.data(), file) == EOF || fclose(file) == EOF)
                {
                    throw errnoAsSystemError();
                }

                return string(name);
            };

            string expectedName(createTempFileWithContent(expected));
            string actualName(createTempFileWithContent(actual));
            string command("diff -u " + expectedName + ' ' + actualName);
            class PipedOutput
            {
                ostream &log;
                
                public:
                FILE *const p;
                PipedOutput(string &command, stringstream &stream): log(stream), p(popen(command.data(), "r"))
                {
                    if (p == nullptr) throw errnoAsSystemError();
                }

                ~PipedOutput()
                {
                    if (pclose(p) == -1)
                    {
                        log << "\nWarning - error on closing pipe to diff: " << errnoAsSystemError().what();
                    }
                }
            } p(command, s);

            s << "\nDiff:\n";

            // discard two lines
            // do not attempt to replace the top two lines if this failed
            if (fscanf(p.p, "%*[^\n]\n%*[^\n]\n") != EOF)
            {
                s << "--- expected\n+++ actual\n";
            }
            
            char buffer[64];
            while (fgets(buffer, sizeof(buffer) - 1, p.p) != nullptr)
            {
                s << buffer;    
            }
            if (ferror(p.p))
            {
                s << "\nCould not complete diff due to a pipe error";
            }

            static const auto attemptRemove = [&](const string &name)
            {
                if (remove(name.data()) == -1)
                {
                    s << "\nWarning: could not remove temporary file: " << errnoAsSystemError().what();
                }
            };
            attemptRemove(expectedName);
            attemptRemove(actualName);
        }
        catch (system_error &e)
        {
            s << "\nCould not execute diff: " << e.what();
        }
    }
        
    return s.str();
}


