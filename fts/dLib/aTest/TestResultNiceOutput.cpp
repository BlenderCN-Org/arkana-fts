#include "TestResultNiceOutput.h"
#include "Failure.h"
#include "Test.h"
#include "TestSuite.h"
#include <iostream>
#include <iomanip>
#include <numeric>
#include <algorithm>

TestResultNiceOutput::TestResultNiceOutput(std::ostream& out,
                                           bool bDelayFailureReport,
                                           const std::string& ident)
    : m_bDelay(bDelayFailureReport)
    , m_out(out)
    , m_sIdent(ident)
    , m_nFailures(0)
{
}

TestResultNiceOutput::~TestResultNiceOutput()
{
}

void TestResultNiceOutput::startTests ()
{
    TestResult::startTests();

    m_out << "Starting the tests." << std::endl;
}

void TestResultNiceOutput::endTests ()
{
    TestResult::endTests();

    if(m_bDelay) {
        std::list<std::pair<const Test*, Failure> >::iterator i = m_failures.begin();
        for( ; i != m_failures.end() ; ++i) {
            m_out << "===============================================================================" << std::endl
                  << "[FAILURE]: " << i->first->name() << std::endl
                  << i->second << std::endl;
        }
    }

    m_out << "-------------------------------------------------------------------------------" << std::endl
          << "ran " << testCount << " tests in " << secondsElapsed << "s" << std::endl
          << std::endl;
    if(failureCount > 0)
        m_out << "FAILURE";
    else
        m_out << "SUCCESS";

    m_out << " (failures=" << failureCount << ", skipped=" << skipCount << ", successes=" << testCount - failureCount - skipCount << ")" << std::endl;
}

void TestResultNiceOutput::startSuite(const TestSuite& suite)
{
    TestResult::startSuite(suite);

    std::string prefix = std::accumulate(m_prefix.begin(), m_prefix.end(), std::string(""));
    m_out << prefix << "suite " << suite.name() << ":" << std::endl;

    m_prefix.push_back(m_sIdent);
}

void TestResultNiceOutput::suiteWasSkipped (const TestSuite& suite)
{
    TestResult::suiteWasSkipped(suite);

    m_out <<  "                                                                      [SKIPPED]" << std::endl;

    m_prefix.pop_back();
}

void TestResultNiceOutput::endSuite(const TestSuite& suite)
{
    TestResult::endSuite(suite);

    m_prefix.pop_back();
}

void TestResultNiceOutput::startTest (const Test& test)
{
    TestResult::startTest(test);

    std::string prefix = std::accumulate(m_prefix.begin(), m_prefix.end(), std::string(""));
    m_out << prefix << test.name();
    m_out.fill(' ');
    m_out.width(std::max(0, 80-15-(int)prefix.length()-(int)test.name().length()));
    m_out << std::left << "...";
}

void TestResultNiceOutput::testWasRun (const Test& test)
{
    TestResult::testWasRun(test);

    m_out.fill(' ');
    m_out.width(14);
    if(m_nFailures == 0) {
        m_out << std::right << "[OK]" << std::endl;
    } else if(m_nFailures == 1) {
        m_out << std::right << "[1 FAILURE]" << std::endl;
    } else {
        std::stringstream ss;
        ss << "[" << m_nFailures << " FAILURES]";
        m_out << std::right << ss.str() << std::endl;
    }

    m_nFailures = 0;
}

void TestResultNiceOutput::testWasSkipped(const Test& test)
{
    TestResult::testWasSkipped(test);

    m_out << "     [SKIPPED]" << std::endl;
}

void TestResultNiceOutput::addFailure(const Test& test, const Failure& failure)
{
    TestResult::addFailure(test, failure);

    m_nFailures++;

    if(m_bDelay) {
        m_failures.push_back(std::make_pair(&test, failure));
    }
}
