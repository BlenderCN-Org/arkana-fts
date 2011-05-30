
#include "TestHarness.h"

TEST (MyTest)
{
    float fnum = 2.00001f;
    CHECK_DOUBLES_EQUAL (fnum, 2.0f);
}

