#include "dLib/aTest/TestHarness.h"

#include "dLib/dString/dPath.h"
#include "dLib/dFile/dBrowse.h"

using namespace FTS;

SUITE(dBrowse_)

TEST_INSUITE(dBrowse_, Wildcard)
{
    auto actual = dBrowse(Path::datadir("Graphics/ui"), "*.png");
    CHECK_EQUAL(3, actual.size());
    CHECK_EQUAL("icp.png", actual[0]);
    CHECK_EQUAL("Loading.English.png", actual[1]);
    CHECK_EQUAL("rb.png", actual[2]);
}

TEST_INSUITE(dBrowse_, all)
{
    auto actual = dBrowse(Path::datadir("Graphics/ui/cursors"));
    CHECK_EQUAL(3, actual.size());
    CHECK_EQUAL("std.anim", actual[0]);
    CHECK_EQUAL("std.png", actual[1]);
    CHECK_EQUAL("std.xml", actual[2]);
}
