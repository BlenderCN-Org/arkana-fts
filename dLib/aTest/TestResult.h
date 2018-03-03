

#ifndef TESTRESULT_H
#define TESTRESULT_H

#include <time.h>


class Failure;
class Test;
class TestSuite;

class TestResult
{
public:
    TestResult ();
    virtual ~TestResult() {};

    virtual void startTests ();
    virtual void endTests ();
    virtual void startTest (const Test& test);
    virtual void addFailure (const Test& test, const Failure & failure);
    virtual void testWasRun (const Test& test);
    virtual void testWasSkipped (const Test& test);
    virtual void startSuite(const TestSuite& suite);
    virtual void suiteWasSkipped (const TestSuite& suite);
    virtual void endSuite(const TestSuite& suite);

    size_t getFailureCount() const { return failureCount; }
    size_t getSkipCount() const { return skipCount; }

protected:
    size_t failureCount;
    size_t testCount;
    size_t skipCount;
    time_t startTime;
    time_t secondsElapsed;
};

#endif
