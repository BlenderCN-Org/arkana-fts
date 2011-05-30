#include "TestHarness.h"
#include "TestResultNiceOutput.h"


int main()
{
    TestResultNiceOutput result(std::cout);
    TestRegistry::runAllTests(result);
    return (result.getFailureCount());
}
