
#include "TestResult.h"
#include "Failure.h"
#include "TestSuite.h"


TestResult::TestResult() : failureCount (0), testCount(0), skipCount(0), secondsElapsed(0)
{
    ::time(&startTime);
}


void TestResult::startTests ()
{
    ::time(&startTime);
}

void TestResult::endTests ()
{
    time_t endTime;
    ::time(&endTime);
    secondsElapsed = endTime - startTime;
}

void TestResult::startTest (const Test& /*test*/)
{
}

void TestResult::addFailure (const Test& /*test*/, const Failure& /*failure*/)
{
    failureCount++;
}

void TestResult::testWasRun(const Test& /*test*/)
{
    testCount++;
}

void TestResult::testWasSkipped (const Test& /*test*/)
{
    testCount++;
    skipCount++;
}

void TestResult::startSuite(const TestSuite& /*suite*/)
{
}

void TestResult::endSuite(const TestSuite& /*suite*/)
{
}
