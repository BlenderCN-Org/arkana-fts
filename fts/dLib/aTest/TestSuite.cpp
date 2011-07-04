#include "TestSuite.h"
#include "TestResult.h"
#include "TestRegistry.h"
#include "Failure.h"

#include <exception>

TestSuite::TestSuite(const std::string& testSuiteName, TestSuite* whereToRegister)
    : Test(testSuiteName, whereToRegister)
{
}

void TestSuite::addTest(Test *t)
{
    tests.push_back(t);
}

void TestSuite::run(TestResult& result)
{
    result.startSuite(*this);

    if(!TestRegistry::mayIRun(*this)) {
        result.suiteWasSkipped(*this);
        return;
    }

    std::vector<Test *>::iterator it = tests.begin ();
    if(TestRegistry::catchExceptions()) {
        try
        {
            setup();
            for (it = tests.begin (); it != tests.end (); ++it)
                (*it)->run (result);
        } catch (const std::exception& e) {
            result.addFailure (*this, Failure (std::string("Unhandled exception within ") + (*it)->name() + ": " + std::string(e.what()), my_name, "", 0));
        } catch (...) {
            result.addFailure (*this, Failure ("Unhandled exception within " + (*it)->name(), my_name, "", 0));
        }
    } else {
        setup();
        for (it = tests.begin (); it != tests.end (); ++it)
            (*it)->run (result);
    }
    teardown();
    result.endSuite(*this);
}

std::size_t TestSuite::testCount() const
{
    return tests.size();
}
