#include "TestResultCompiler.h"
#include "Failure.h"
#include "Test.h"
#include "TestSuite.h"

TestResultCompiler::TestResultCompiler(std::ostream& out)
    : m_out(out)
{
}

TestResultCompiler::~TestResultCompiler()
{
}

void TestResultCompiler::startTests ()
{
    TestResult::startTests();
    m_out << "Starting the tests ..." << std::endl;
}

void TestResultCompiler::endTests ()
{
    TestResult::endTests();
    if (failureCount > 0)
        m_out << testCount-skipCount << " tests run, " << failureCount << " failures" << std::endl;
    else
        m_out << "No test failed." << std::endl;
}

void TestResultCompiler::addFailure (const Test& test, const Failure & failure)
{
    TestResult::addFailure(test, failure);
    m_out << failure.fileName << ":" << failure.lineNumber << ": Test " << failure.testName << " failed: " << failure.condition << std::endl;
}
