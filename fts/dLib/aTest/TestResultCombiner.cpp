
#include "TestResultCombiner.h"


TestResultCombiner::TestResultCombiner(TestResult& first, TestResult& second) : m_first(first), m_second(second)
{
}

TestResultCombiner::~TestResultCombiner()
{
}


void TestResultCombiner::startTests ()
{
    TestResult::startTests();
    m_first.startTests();
    m_second.startTests();
}

void TestResultCombiner::endTests ()
{
    TestResult::endTests();
    m_first.endTests();
    m_second.endTests();
}

void TestResultCombiner::startTest (const Test& test)
{
    TestResult::startTest(test);
    m_first.startTest(test);
    m_second.startTest(test);
}

void TestResultCombiner::addFailure (const Test& test, const Failure& failure)
{
    TestResult::addFailure(test, failure);
    m_first.addFailure(test, failure);
    m_second.addFailure(test, failure);
}

void TestResultCombiner::testWasRun(const Test& test)
{
    TestResult::testWasRun(test);
    m_first.testWasRun(test);
    m_second.testWasRun(test);
}

void TestResultCombiner::testWasSkipped (const Test& test)
{
    TestResult::testWasSkipped(test);
    m_first.testWasSkipped(test);
    m_second.testWasSkipped(test);
}

void TestResultCombiner::startSuite(const TestSuite& suite)
{
    TestResult::startSuite(suite);
    m_first.startSuite(suite);
    m_second.startSuite(suite);
}

void TestResultCombiner::suiteWasSkipped(const TestSuite& suite)
{
    TestResult::suiteWasSkipped(suite);
    m_first.suiteWasSkipped(suite);
    m_second.suiteWasSkipped(suite);
}

void TestResultCombiner::endSuite(const TestSuite& suite)
{
    TestResult::endSuite(suite);
    m_first.endSuite(suite);
    m_second.endSuite(suite);
}
