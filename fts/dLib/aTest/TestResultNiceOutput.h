#ifndef TESTRESULTNICEOUTPUT_H
#define TESTRESULTNICEOUTPUT_H

#include "TestResult.h"
#include <string>
#include <ostream>
#include <list>

class TestResultNiceOutput : public TestResult
{
    bool m_bDelay;
    std::ostream& m_out;
    std::string m_sIdent;
    std::list<std::string> m_prefix;
    size_t m_nFailures;
    std::list<std::pair<const Test*, Failure> >m_failures;

public:
    TestResultNiceOutput(std::ostream& out, bool bDelayFailureReport = true, const std::string& ident = "  ");
    virtual ~TestResultNiceOutput();

    virtual void startTests ();
    virtual void endTests ();
    virtual void startTest (const Test& test);
    virtual void addFailure (const Test& test, const Failure & failure);
    virtual void testWasRun (const Test& test);
    virtual void testWasSkipped(const Test& test);
    virtual void startSuite(const TestSuite& suite);
    virtual void suiteWasSkipped (const TestSuite& suite);
    virtual void endSuite(const TestSuite& suite);
};


#endif

