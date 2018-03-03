#include "../../TestHarness.h"

TEST (MyTest)
{
    float fnum = 2.00001f;
    CHECK_EQUAL("Hello", std::string("World!"));
    CHECK_DOUBLES_EQUAL (fnum, 3.0f);
}
