#include "../../TestHarness.h"
#include "../../TestResultCombiner.h"
#include "../../TestResultNiceOutput.h"
#include "../../TestResultSQL.h"

#include <fstream>

int main(int argc, char *argv[])
{
    for(int i = 1 ; i < argc ; ++i) {
        if(argv[i][0] == '-')
            TestRegistry::addDont(&argv[i][1]);
        else
            TestRegistry::addDo(argv[i]);
    }

    std::ofstream sql("result.sql");
    TestResultNiceOutput hello(std::cerr);
    TestResultSQL world(sql, "peetsa");
    TestResultCombiner result(hello, world);
    TestRegistry::runAllTests(result);
    return (result.getFailureCount());
}
