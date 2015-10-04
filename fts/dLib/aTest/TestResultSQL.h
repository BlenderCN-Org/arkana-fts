#ifndef TESTRESULTSQL_H
#define TESTRESULTSQL_H

#include "TestResult.h"
#include <string>
#include <ostream>

class TestResultSQL : public TestResult
{
    std::string m_sProjectsTbl, m_sSuitesTbl, m_sCasesTbl, m_sFailuresTbl;
    bool m_bKeepOld;
    std::ostream& m_out;
    std::string m_sProjectName;

    std::string now() const;
    std::string wsToUnderscore(const std::string& s) const;

public:
    TestResultSQL(std::ostream& out, const std::string& in_sProjectName,
                  bool in_bKeepOld = false,
                  const std::string& in_sProjectsTbl = "cppunit_projects",
                  const std::string& in_sSuitesTbl = "cppunit_suites",
                  const std::string& in_sCasesTbl = "cppunit_cases",
                  const std::string& in_sFailuresTbl = "cppunit_failures");
    virtual ~TestResultSQL();

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

