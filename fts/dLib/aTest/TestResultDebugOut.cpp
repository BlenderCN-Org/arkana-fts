#include "TestResultDebugOut.h"
#include "Failure.h"
#include <iostream>
#include <sstream>
#if defined(_MSC_VER)
#include <windows.h>
#endif


void TestResultDebugOut::startTests ()
{
#if defined(_MSC_VER)
    OutputDebugStringA("\n\nRunning unit tests...\n\n");
#endif
}


void TestResultDebugOut::addFailure (const Test& test, const Failure & failure)
{
    TestResult::addFailure(test, failure);

    std::ostringstream oss;
    oss << failure;
#if defined(_MSC_VER)
    OutputDebugStringA(oss.str().c_str());
#endif
}

void TestResultDebugOut::endTests ()
{
    TestResult::endTests();

    std::ostringstream oss;
    oss << testCount << " tests run" << std::endl;
    if (failureCount > 0)
        oss << "****** There were " << failureCount << " failures.";
    else
        oss << "There were no test failures.";
    oss << "(time: " << secondsElapsed << " s)" << std::endl;

#if defined(_MSC_VER)
    OutputDebugStringA(oss.str().c_str());
    OutputDebugStringA("\n");
#endif
}
