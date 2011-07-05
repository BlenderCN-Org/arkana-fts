

#include "Test.h"
#include "TestResult.h"
#include "TestRegistry.h"
#include "TestSuite.h"


void TestRegistry::addTest (Test *test)
{
    instance ().add (test);
}

void TestRegistry::addDo(const std::string& sRestriction)
{
    instance ().addDo_(sRestriction);
}

void TestRegistry::addDont(const std::string& sRestriction)
{
    instance ().addDont_(sRestriction);
}

void TestRegistry::runAllTests (TestResult& result, bool bCatchExceptions)
{
    instance ().run (result, bCatchExceptions);
}


TestRegistry& TestRegistry::instance () {
    static TestRegistry registry;
    return registry;
}


void TestRegistry::add (Test *test) {
    tests.push_back (test);
}

void TestRegistry::addDo_(const std::string& sRestriction) {
    m_dos.push_back (sRestriction);
}

void TestRegistry::addDont_(const std::string& sRestriction) {
    m_donts.push_back (sRestriction);
}


void TestRegistry::run (TestResult& result, bool bCatchExceptions) {
    m_bCatchExceptions = bCatchExceptions;
    result.startTests ();
    for (std::vector<Test *>::iterator it = tests.begin (); it != tests.end (); ++it) {
        (*it)->run (result);
    }
    result.endTests ();
}

bool TestRegistry::catchExceptions() {
    return instance().m_bCatchExceptions;
}

bool TestRegistry::mayIRun(const Test& me)
{
    return instance().checkRun(me);
}

bool TestRegistry::checkRun(const std::string& name)
{
    // Check the dont's first
    for(std::vector<std::string>::iterator i = m_donts.begin() ; i != m_donts.end() ; ++i) {
        // Very bad design :) but works
        if(name.find(*i) != std::string::npos)
            throw "prohibited!";
    }

    // Followed by the do's
    for(std::vector<std::string>::iterator i = m_dos.begin() ; i != m_dos.end() ; ++i) {
        if(name.find(*i) != std::string::npos)
            return true;
    }

    // An empty do's list allows all (ret true), a non-empty not.
    return m_dos.empty();
}

bool TestRegistry::checkRun(const Test& me)
{
    try {
        if(checkRun(me.name()))
            return true;

        // If I may not run becaue of my name, it is still possible that one of
        // my parents may have passed the test => I run anyway.
        for(const TestSuite* par = me.registrar() ; par != NULL ; par = par->registrar()) {
            if(checkRun(par->name()))
                return true;
        }
    } catch(const char *) {
        // Someone in my hierarchy has been hit by the prohibited list (dont's)
        // so I can't run anyway!
        return false;
    }

    return false;
}
