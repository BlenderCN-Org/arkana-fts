
#ifndef TESTREGISTRY_H
#define TESTREGISTRY_H

// TestRegistry is a primitive singleton which collects all of the
// tests in a system and allows them to be executed.  To see how
// tests are added to the TestRegistry look at the Test.h file


#include <vector>

class Test;
class TestResult;

class TestRegistry
{
public:
    static void addTest (Test *test);
    static void addDo(const std::string& only);
    static void addDont(const std::string& disallow);
    static void runAllTests (TestResult& result, bool bCatchExceptions = true);
    static bool catchExceptions();
    static bool mayIRun(const Test& me);

private:

    static TestRegistry&    instance ();
    void                    add (Test *test);
    void                    addDo_(const std::string& s);
    void                    addDont_(const std::string& s);
    void                    run (TestResult& result, bool bCatchExceptions);
    bool                    checkRun (const std::string& me);
    bool                    checkRun (const Test& me);

    std::vector<Test *> tests;
    bool m_bCatchExceptions;
    std::vector<std::string> m_dos;
    std::vector<std::string> m_donts;
};



#endif

