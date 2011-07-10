#include "main.h"
#include "dLib/aTest/TestHarness.h"
#include "dLib/aTest/TestResultNiceOutput.h"
#include "dLib/aTest/TestResultCombiner.h"
#include "dLib/aTest/TestResultSQL.h"
#include "dLib/aTest/TestResultDebugOut.h"

#include <fstream>

int run_tests(int argc, const char *argv[])
{
    std::string sProjName = "unnamed project";

    if(argc > 1) {
        sProjName = argv[1];

        for(int i = 2 ; i < argc ; ++i) {
            if(argv[i][0] == '-')
                TestRegistry::addDont(&argv[i][1]);
            else
                TestRegistry::addDo(argv[i]);
        }
    }

    std::ofstream sql((sProjName + ".sql").c_str());
    TestResultNiceOutput hello(std::cerr);
    TestResultSQL world(sql, sProjName);
    TestResultDebugOut debugOut;
    TestResultCombiner result(hello, world);
    TestResultCombiner result2(result, debugOut);
    TestRegistry::runAllTests(result2);

    return (result.getFailureCount());
}
