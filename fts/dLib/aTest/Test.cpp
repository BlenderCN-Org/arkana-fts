#include "Test.h"
#include "TestSuite.h"
#include "TestRegistry.h"
#include "TestResult.h"
#include "Failure.h"

#include <exception>


Test::Test (const std::string& testName, TestSuite* whereToRegister)
    : my_name (testName)
    , my_registrar (whereToRegister)
{
    if(whereToRegister)
        whereToRegister->addTest(this);
    else
        TestRegistry::addTest(this);
}


void Test::run (TestResult& result)
{
    if(!TestRegistry::mayIRun(*this)) {
        result.startTest(*this);
        result.testWasSkipped(*this);
        return;
    }

    if(TestRegistry::catchExceptions()) {
        try
        {
            result.startTest(*this);
            setup();
            runTest (result);
        } catch (const std::exception& e) {
            result.addFailure (*this, Failure (std::string("Unhandled exception: ") + std::string(e.what()), my_name, "", 0));
        } catch (...) {
            result.addFailure (*this, Failure ("Unhandled exception", my_name, "", 0));
        }
    } else {
        result.startTest(*this);
        setup();
        runTest (result);
    }

    teardown();
	result.testWasRun(*this);
}
