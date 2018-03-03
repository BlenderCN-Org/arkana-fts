#ifndef TESTRESULTCOMPILER_H
#define TESTRESULTCOMPILER_H

#include "TestResult.h"
#include <ostream>

class Test;
class Failure;

class TestResultCompiler : public TestResult
{
    std::ostream& m_out;

public:
    TestResultCompiler(std::ostream& out);
    virtual ~TestResultCompiler();

    virtual void startTests ();
    virtual void endTests ();
    virtual void addFailure (const Test& test, const Failure & failure);
};


#endif

