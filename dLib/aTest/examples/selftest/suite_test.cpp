
#include "TestHarness.h"

SUITE(Royal);

TEST_INSUITE(Royal,Construction)
{
    float fnum = 2.1f;
    CHECK_DOUBLES_EQUAL (fnum, 2.0f);
}

TEST_INSUITE(Royal,Destruction)
{
    float fnum = 2.2f;
    CHECK_DOUBLES_EQUAL (fnum, 2.0f);
}

SUITE_INSUITE(Royal, Canin);

TEST_INSUITE(Royal,Usage)
{
    CHECK_EQUAL ("\\Heavy'Loaded\"String\n\\", std::string("'Heavy\\Loaded\"String\n'"));
}

TEST_INSUITE(Canin,MoreUsage)
{
    CHECK_EQUAL ("That", std::string("Rocks"));
}

TEST_INSUITE(Canin,EvenMoreUsage)
{
    CHECK_EQUAL ("the royal", std::string("cat"));
}
