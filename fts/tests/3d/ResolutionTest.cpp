#include "dLib/aTest/TestHarness.h"

#include "3d/Resolution.h"
#include "dLib/dString/dString.h"

using namespace FTS;

SUITE(tResolution);

TEST_INSUITE(tResolution, Construction)
{
    // Note: we do not check the default constructor that reads a resolution
    //       from config file. Too tedious :-p
    CHECK_EQUAL(100, Resolution(100, 0, false).w);
    CHECK_EQUAL(100, Resolution(0, 100, false).h);
    CHECK_EQUAL(false, Resolution(0, 100, false).fs);
    CHECK_EQUAL(100, Resolution(100, 0, true).w);
    CHECK_EQUAL(100, Resolution(0, 100, true).h);
    CHECK_EQUAL(true, Resolution(0, 100, true).fs);

    CHECK_EQUAL(1280, Resolution("1280x1024").w);
    CHECK_EQUAL(1024, Resolution("1280x1024").h);
    CHECK_EQUAL(true, Resolution("1280x1024").fs);

    CHECK_EQUAL(1280, Resolution("1280x1024, fullscreen").w);
    CHECK_EQUAL(1024, Resolution("1280x1024, fullscreen").h);
    CHECK_EQUAL(true, Resolution("1280x1024, fullscreen").fs);

    CHECK_EQUAL(1280, Resolution("1280x1024, windowed").w);
    CHECK_EQUAL(1024, Resolution("1280x1024, windowed").h);
    CHECK_EQUAL(false, Resolution("1280x1024, windowed").fs);

    CHECK_EQUAL(0, Resolution("helloxaxaxa").w);
    CHECK_EQUAL(0, Resolution("helloxaxaxa").h);
    CHECK_EQUAL(true, Resolution("helloxaxaxa").fs);
}

TEST_INSUITE(tResolution, ToString)
{
    CHECK_EQUAL("100x0, windowed", Resolution(100, 0, false).toString(true));
    CHECK_EQUAL("0x100, windowed", Resolution(0, 100, false).toString(true));
    CHECK_EQUAL("100x0, fullscreen", Resolution(100, 0, true).toString(true));
    CHECK_EQUAL("0x100, fullscreen", Resolution(0, 100, true).toString(true));

    CHECK_EQUAL("100x0", Resolution(100, 0, false).toString(false));
    CHECK_EQUAL("0x100", Resolution(0, 100, false).toString(false));
    CHECK_EQUAL("100x0", Resolution(100, 0, true).toString(false));
    CHECK_EQUAL("0x100", Resolution(0, 100, true).toString(false));
}
