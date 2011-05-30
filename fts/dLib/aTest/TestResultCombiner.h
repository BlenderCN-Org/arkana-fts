

#ifndef TESTRESULTCOMBINER_H
#define TESTRESULTCOMBINER_H

#include "TestResult.h"

class TestResultCombiner : public TestResult
{
    TestResult& m_first;
    TestResult& m_second;
public:
    TestResultCombiner(TestResult& first, TestResult& second);
    virtual ~TestResultCombiner();

    virtual void startTests ();
    virtual void endTests ();
    virtual void startTest (const Test& test);
    virtual void addFailure (const Test& test, const Failure & failure);
    virtual void testWasRun (const Test& test);
    virtual void testWasSkipped (const Test& test);
    virtual void startSuite(const TestSuite& suite);
    virtual void endSuite(const TestSuite& suite);

    int getFailureCount() const { return failureCount; }
    int getSkipCount() const { return skipCount; }
};

#endif
