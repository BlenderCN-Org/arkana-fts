#include "aTest/TestHarness.h"
#include "aTest/TestResultCompiler.h"
#include "aTest/TestResultCombiner.h"
#include "aTest/TestResultSQL.h"
#include <fstream>

int main(int argc, char *argv[])
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
    TestResultCompiler hello(std::cerr);
    TestResultSQL world(sql, sProjName);
    TestResultCombiner result(hello, world);
    TestRegistry::runAllTests(result);
    return (result.getFailureCount());
}
