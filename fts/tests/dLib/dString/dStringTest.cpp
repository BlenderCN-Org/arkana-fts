#include "dLib/aTest/TestHarness.h"

#include <iostream>
#include <limits>
#include <iomanip>
#include <stdexcept>
#include <ciso646>

#include "dLib/dString/dString.h"
#include "utilities/Math.h"

SUITE(dString)


class StringUtf8Setup : public TestSetup
{
public:
    void setup()
    {
        s1 = "ที่รัก";
        s2 = "Beautiful ";
        s3 = "World!";
        s213 = "Beautiful ที่รักWorld!";
        s5 = "กWorld!";
    }
    void teardown() {}
protected:
    FTS::String s1, s2, s3, s213,s5;
};

TEST_INSUITE_WITHSETUP(dString, StringUtf8, Utf8)
{
    CHECK_EQUAL(6, s1.len());
    CHECK_EQUAL(18, s1.byteCount());
    CHECK_EQUAL(s2.len(),s2.byteCount());
    CHECK_EQUAL(10, s2.len());
    CHECK_EQUAL(10, s2.byteCount());
    FTS::String left2 = s1.left(2);
    CHECK_EQUAL(2, left2.len());
    CHECK_EQUAL(6, left2.byteCount());

    CHECK_EQUAL("Hello,ที่รัก World!", FTS::String("Fucking ello,ที่รัก World!").replaceStr(0, 8, "H"));
    CHECK_EQUAL("Hello, World!", FTS::String("Hที่รักello, World!").replaceStr(0, 7, FTS::String("H")));
    FTS::String s = s2 + s1 + s3 ;
    CHECK_EQUAL(s213, s);
    FTS::String sresult = s.mid(s2.len(), s3.len());
    CHECK_EQUAL(s1, sresult);
    sresult = s.right(s3.len() + 1);
    CHECK_EQUAL(s5,sresult);
}


TEST_INSUITE(dString, ConstructionSimple)
{
    /* A whole bunch of constructors. */
    CHECK_EQUAL(std::string(""), FTS::String().str());
    CHECK_EQUAL(std::string(""), FTS::String("").str());
    CHECK_EQUAL(std::string(""), FTS::String((const char *)NULL).str());
    CHECK_EQUAL(std::string("hello, world"), FTS::String("hello, world").str());
    CHECK_EQUAL(std::string("hello"), FTS::String(FTS::String("hello")).str());
    CHECK_EQUAL(std::string("hello"), FTS::String(std::string("hello")).str());
#ifdef D_STRING_CEGUI
    CHECK_EQUAL(std::string("hello"), FTS::String(CEGUI::String("hello")).str());
#endif
}

TEST_INSUITE(dString, ConstructionLen)
{
#ifdef D_STRING_CEGUI
    CHECK_EQUAL(std::string(""), FTS::String(CEGUI::String("hello"), 0, 0).str());
    CHECK_EQUAL(std::string(""), FTS::String(CEGUI::String("hello"), 0, -1).str());
    CHECK_EQUAL(std::string("h"), FTS::String(CEGUI::String("hello"), 0, 1).str());
    CHECK_EQUAL(std::string("hello"), FTS::String(CEGUI::String("hello"), 0, 5).str());
    CHECK_EQUAL(std::string("hello"), FTS::String(CEGUI::String("hello"), 0, 9).str());

    CHECK_EQUAL(std::string(""), FTS::String(CEGUI::String("hello"), -1).str());
    CHECK_EQUAL(std::string(""), FTS::String(CEGUI::String("hello"), 5).str());
    CHECK_EQUAL(std::string("o"), FTS::String(CEGUI::String("hello"), 4).str());
    CHECK_EQUAL(std::string("lo"), FTS::String(CEGUI::String("hello"), 3).str());
    CHECK_EQUAL(std::string("ello"), FTS::String(CEGUI::String("hello"), 1).str());

    CHECK_EQUAL(std::string(""), FTS::String(CEGUI::String("hello"), -1, 0).str());
    CHECK_EQUAL(std::string(""), FTS::String(CEGUI::String("hello"), 5, 0).str());
    CHECK_EQUAL(std::string(""), FTS::String(CEGUI::String("hello"), 3, 0).str());
    CHECK_EQUAL(std::string("l"), FTS::String(CEGUI::String("hello"), 3, 1).str());
    CHECK_EQUAL(std::string("lo"), FTS::String(CEGUI::String("hello"), 3, 2).str());
    CHECK_EQUAL(std::string("lo"), FTS::String(CEGUI::String("hello"), 3, 3).str());
    CHECK_EQUAL(std::string("lo"), FTS::String(CEGUI::String("hello"), 3, -1).str());
#endif

    CHECK_EQUAL(std::string(""), FTS::String("hello", 0, 0).str());
    CHECK_EQUAL(std::string("hello"), FTS::String("hello", 0, -1).str());
    CHECK_EQUAL(std::string("h"), FTS::String("hello", 0, 1).str());
    CHECK_EQUAL(std::string("hello"), FTS::String("hello", 0, 5).str());
    CHECK_EQUAL(std::string("hello"), FTS::String("hello", 0, 9).str());

    CHECK_EQUAL(std::string(""), FTS::String("hello", -1).str());
    CHECK_EQUAL(std::string(""), FTS::String("hello", 5).str());
    CHECK_EQUAL(std::string("o"), FTS::String("hello", 4).str());
    CHECK_EQUAL(std::string("lo"), FTS::String("hello", 3).str());
    CHECK_EQUAL(std::string("ello"), FTS::String("hello", 1).str());

    CHECK_EQUAL(std::string(""), FTS::String("hello", -1, 0).str());
    CHECK_EQUAL(std::string(""), FTS::String("hello", 5, 0).str());
    CHECK_EQUAL(std::string(""), FTS::String("hello", 3, 0).str());
    CHECK_EQUAL(std::string("l"), FTS::String("hello", 3, 1).str());
    CHECK_EQUAL(std::string("lo"), FTS::String("hello", 3, 2).str());
    CHECK_EQUAL(std::string("lo"), FTS::String("hello", 3, 3).str());
    CHECK_EQUAL(std::string("lo"), FTS::String("hello", 3, -1).str());

    CHECK_EQUAL(std::string(""), FTS::String(reinterpret_cast<const int8_t*>("hello"), 0, 0).str());
    CHECK_EQUAL(std::string("hello"), FTS::String(reinterpret_cast<const int8_t*>("hello"), 0, -1).str());
    CHECK_EQUAL(std::string("h"), FTS::String(reinterpret_cast<const int8_t*>("hello"), 0, 1).str());
    CHECK_EQUAL(std::string("hello"), FTS::String(reinterpret_cast<const int8_t*>("hello"), 0, 5).str());
    CHECK_EQUAL(std::string("hello"), FTS::String(reinterpret_cast<const int8_t*>("hello"), 0, 9).str());

    CHECK_EQUAL(std::string(""), FTS::String(reinterpret_cast<const int8_t*>("hello"), -1).str());
    CHECK_EQUAL(std::string(""), FTS::String(reinterpret_cast<const int8_t*>("hello"), 5).str());
    CHECK_EQUAL(std::string("o"), FTS::String(reinterpret_cast<const int8_t*>("hello"), 4).str());
    CHECK_EQUAL(std::string("lo"), FTS::String(reinterpret_cast<const int8_t*>("hello"), 3).str());
    CHECK_EQUAL(std::string("ello"), FTS::String(reinterpret_cast<const int8_t*>("hello"), 1).str());

    CHECK_EQUAL(std::string(""), FTS::String(reinterpret_cast<const int8_t*>("hello"), -1, 0).str());
    CHECK_EQUAL(std::string(""), FTS::String(reinterpret_cast<const int8_t*>("hello"), 5, 0).str());
    CHECK_EQUAL(std::string(""), FTS::String(reinterpret_cast<const int8_t*>("hello"), 3, 0).str());
    CHECK_EQUAL(std::string("l"), FTS::String(reinterpret_cast<const int8_t*>("hello"), 3, 1).str());
    CHECK_EQUAL(std::string("lo"), FTS::String(reinterpret_cast<const int8_t*>("hello"), 3, 2).str());
    CHECK_EQUAL(std::string("lo"), FTS::String(reinterpret_cast<const int8_t*>("hello"), 3, 3).str());
    CHECK_EQUAL(std::string("lo"), FTS::String(reinterpret_cast<const int8_t*>("hello"), 3, -1).str());

    CHECK_EQUAL(std::string(""), FTS::String(reinterpret_cast<const uint8_t*>("hello"), 0, 0).str());
    CHECK_EQUAL(std::string("hello"), FTS::String(reinterpret_cast<const uint8_t*>("hello"), 0, -1).str());
    CHECK_EQUAL(std::string("h"), FTS::String(reinterpret_cast<const uint8_t*>("hello"), 0, 1).str());
    CHECK_EQUAL(std::string("hello"), FTS::String(reinterpret_cast<const uint8_t*>("hello"), 0, 5).str());
    CHECK_EQUAL(std::string("hello"), FTS::String(reinterpret_cast<const uint8_t*>("hello"), 0, 9).str());

    CHECK_EQUAL(std::string(""), FTS::String(reinterpret_cast<const uint8_t*>("hello"), -1).str());
    CHECK_EQUAL(std::string(""), FTS::String(reinterpret_cast<const uint8_t*>("hello"), 5).str());
    CHECK_EQUAL(std::string("o"), FTS::String(reinterpret_cast<const uint8_t*>("hello"), 4).str());
    CHECK_EQUAL(std::string("lo"), FTS::String(reinterpret_cast<const uint8_t*>("hello"), 3).str());
    CHECK_EQUAL(std::string("ello"), FTS::String(reinterpret_cast<const uint8_t*>("hello"), 1).str());

    CHECK_EQUAL(std::string(""), FTS::String(reinterpret_cast<const uint8_t*>("hello"), -1, 0).str());
    CHECK_EQUAL(std::string(""), FTS::String(reinterpret_cast<const uint8_t*>("hello"), 5, 0).str());
    CHECK_EQUAL(std::string(""), FTS::String(reinterpret_cast<const uint8_t*>("hello"), 3, 0).str());
    CHECK_EQUAL(std::string("l"), FTS::String(reinterpret_cast<const uint8_t*>("hello"), 3, 1).str());
    CHECK_EQUAL(std::string("lo"), FTS::String(reinterpret_cast<const uint8_t*>("hello"), 3, 2).str());
    CHECK_EQUAL(std::string("lo"), FTS::String(reinterpret_cast<const uint8_t*>("hello"), 3, 3).str());
    CHECK_EQUAL(std::string("lo"), FTS::String(reinterpret_cast<const uint8_t*>("hello"), 3, -1).str());

    CHECK_EQUAL(std::string(""), FTS::String(FTS::String("hello"), 0, 0).str());
    CHECK_EQUAL(std::string("hello"), FTS::String(FTS::String("hello"), 0, -1).str());
    CHECK_EQUAL(std::string("h"), FTS::String(FTS::String("hello"), 0, 1).str());
    CHECK_EQUAL(std::string("hello"), FTS::String(FTS::String("hello"), 0, 5).str());
    CHECK_EQUAL(std::string("hello"), FTS::String(FTS::String("hello"), 0, 9).str());

    CHECK_EQUAL(std::string(""), FTS::String(FTS::String("hello"), -1).str());
    CHECK_EQUAL(std::string(""), FTS::String(FTS::String("hello"), 5).str());
    CHECK_EQUAL(std::string("o"), FTS::String(FTS::String("hello"), 4).str());
    CHECK_EQUAL(std::string("lo"), FTS::String(FTS::String("hello"), 3).str());
    CHECK_EQUAL(std::string("ello"), FTS::String(FTS::String("hello"), 1).str());

    CHECK_EQUAL(std::string(""), FTS::String(FTS::String("hello"), -1, 0).str());
    CHECK_EQUAL(std::string(""), FTS::String(FTS::String("hello"), 5, 0).str());
    CHECK_EQUAL(std::string(""), FTS::String(FTS::String("hello"), 3, 0).str());
    CHECK_EQUAL(std::string("l"), FTS::String(FTS::String("hello"), 3, 1).str());
    CHECK_EQUAL(std::string("lo"), FTS::String(FTS::String("hello"), 3, 2).str());
    CHECK_EQUAL(std::string("lo"), FTS::String(FTS::String("hello"), 3, 3).str());
    CHECK_EQUAL(std::string("lo"), FTS::String(FTS::String("hello"), 3, -1).str());

    CHECK_EQUAL(std::string(""), FTS::String(std::string("hello"), 0, 0).str());
    CHECK_EQUAL(std::string("hello"), FTS::String(std::string("hello"), 0, -1).str());
    CHECK_EQUAL(std::string("h"), FTS::String(std::string("hello"), 0, 1).str());
    CHECK_EQUAL(std::string("hello"), FTS::String(std::string("hello"), 0, 5).str());
    CHECK_EQUAL(std::string("hello"), FTS::String(std::string("hello"), 0, 9).str());

    CHECK_EQUAL(std::string(""), FTS::String(std::string("hello"), -1).str());
    CHECK_EQUAL(std::string(""), FTS::String(std::string("hello"), 5).str());
    CHECK_EQUAL(std::string("o"), FTS::String(std::string("hello"), 4).str());
    CHECK_EQUAL(std::string("lo"), FTS::String(std::string("hello"), 3).str());
    CHECK_EQUAL(std::string("ello"), FTS::String(std::string("hello"), 1).str());

    CHECK_EQUAL(std::string(""), FTS::String(std::string("hello"), -1, 0).str());
    CHECK_EQUAL(std::string(""), FTS::String(std::string("hello"), 5, 0).str());
    CHECK_EQUAL(std::string(""), FTS::String(std::string("hello"), 3, 0).str());
    CHECK_EQUAL(std::string("l"), FTS::String(std::string("hello"), 3, 1).str());
    CHECK_EQUAL(std::string("lo"), FTS::String(std::string("hello"), 3, 2).str());
    CHECK_EQUAL(std::string("lo"), FTS::String(std::string("hello"), 3, 3).str());
    CHECK_EQUAL(std::string("lo"), FTS::String(std::string("hello"), 3, -1).str());
}

TEST_INSUITE(dString, ConstructionFancy)
{
    // Constructors from other types.
    CHECK_EQUAL("0", FTS::String::chr('0').str());

    CHECK_EQUAL("127", FTS::String::nr(static_cast<int8_t>(127)).str());
    CHECK_EQUAL("127", FTS::String::nr(static_cast<int8_t>(127), 1).str());
    CHECK_EQUAL("aa127", FTS::String::nr(static_cast<int8_t>(127), 5, 'a').str());
    CHECK_EQUAL("aaa7f", FTS::String::nr(static_cast<int8_t>(127), 5, 'a', std::ios::hex).str());

    CHECK_EQUAL("255", FTS::String::nr(static_cast<uint8_t>(-1)).str());
    CHECK_EQUAL("255", FTS::String::nr(static_cast<uint8_t>(-1), 1).str());
    CHECK_EQUAL("aa255", FTS::String::nr(static_cast<uint8_t>(-1), 5, 'a').str());
    CHECK_EQUAL("aaaff", FTS::String::nr(static_cast<uint8_t>(-1), 5, 'a', std::ios::hex).str());

    CHECK_EQUAL("32767", FTS::String::nr(static_cast<int16_t>(32767)).str());
    CHECK_EQUAL("32767", FTS::String::nr(static_cast<int16_t>(32767), 1).str());
    CHECK_EQUAL("aa32767", FTS::String::nr(static_cast<int16_t>(32767), 7, 'a').str());
    CHECK_EQUAL("aaa7fff", FTS::String::nr(static_cast<int16_t>(32767), 7, 'a', std::ios::hex).str());

    CHECK_EQUAL("65535", FTS::String::nr(static_cast<uint16_t>(-1)).str());
    CHECK_EQUAL("65535", FTS::String::nr(static_cast<uint16_t>(-1), 1).str());
    CHECK_EQUAL("aa65535", FTS::String::nr(static_cast<uint16_t>(-1), 7, 'a').str());
    CHECK_EQUAL("aaaffff", FTS::String::nr(static_cast<uint16_t>(-1), 7, 'a', std::ios::hex).str());

    CHECK_EQUAL("2147483647", FTS::String::nr(static_cast<int32_t>(2147483647)).str());
    CHECK_EQUAL("2147483647", FTS::String::nr(static_cast<int32_t>(2147483647), 1).str());
    CHECK_EQUAL("aa2147483647", FTS::String::nr(static_cast<int32_t>(2147483647), 12, 'a').str());
    CHECK_EQUAL("aaaa7fffffff", FTS::String::nr(static_cast<int32_t>(2147483647), 12, 'a', std::ios::hex).str());

    CHECK_EQUAL("4294967295", FTS::String::nr(static_cast<uint32_t>(-1)).str());
    CHECK_EQUAL("4294967295", FTS::String::nr(static_cast<uint32_t>(-1), 1).str());
    CHECK_EQUAL("aa4294967295", FTS::String::nr(static_cast<uint32_t>(-1), 12, 'a').str());
    CHECK_EQUAL("aaaaffffffff", FTS::String::nr(static_cast<uint32_t>(-1), 12, 'a', std::ios::hex).str());

    CHECK_EQUAL("9223372036854775807", FTS::String::nr(static_cast<int64_t>(9223372036854775807UL)).str());
    CHECK_EQUAL("9223372036854775807", FTS::String::nr(static_cast<int64_t>(9223372036854775807UL), 1).str());
    CHECK_EQUAL("aaa9223372036854775807", FTS::String::nr(static_cast<int64_t>(9223372036854775807UL), 22, 'a').str());
    CHECK_EQUAL("aaaaaa7fffffffffffffff", FTS::String::nr(static_cast<int64_t>(9223372036854775807UL), 22, 'a', std::ios::hex).str());

    CHECK_EQUAL("18446744073709551615", FTS::String::nr(static_cast<uint64_t>(-1)).str());
    CHECK_EQUAL("18446744073709551615", FTS::String::nr(static_cast<uint64_t>(-1), 1).str());
    CHECK_EQUAL("aa18446744073709551615", FTS::String::nr(static_cast<uint64_t>(-1), 22, 'a').str());
    CHECK_EQUAL("aaaaaaffffffffffffffff", FTS::String::nr(static_cast<uint64_t>(-1), 22, 'a', std::ios::hex).str());

    CHECK_EQUAL("2.000000", FTS::String::nr(2.0f).str());
    CHECK_EQUAL("2", FTS::String::nr(2.0f, 0).str());
    CHECK_EQUAL("2.5", FTS::String::nr(2.5f, 1).str());
    CHECK_EQUAL("2.1", FTS::String::nr(2.051f, 1).str());
    CHECK_EQUAL("2.0", FTS::String::nr(2.05f, 1).str());
    CHECK_EQUAL("2.0", FTS::String::nr(2.f, 1).str());

    CHECK_EQUAL("\342\210\236", FTS::String::nr(std::numeric_limits<float>::infinity()).str());
    CHECK_EQUAL("NaN", FTS::String::nr(std::numeric_limits<float>::signaling_NaN()).str());
    CHECK_EQUAL("NaN", FTS::String::nr(std::numeric_limits<float>::quiet_NaN()).str());

    CHECK_EQUAL("2.000000", FTS::String::nr(2.0).str());
    CHECK_EQUAL("2", FTS::String::nr(2.0, 0).str());
    CHECK_EQUAL("2.5", FTS::String::nr(2.5, 1).str());
    CHECK_EQUAL("2.1", FTS::String::nr(2.051, 1).str());
    CHECK_EQUAL("2.0", FTS::String::nr(2.05, 1).str());
    CHECK_EQUAL("2.0", FTS::String::nr(2., 1).str());
    CHECK_EQUAL("2.0", FTS::String::nr(2., 1).str());

    CHECK_EQUAL("\342\210\236", FTS::String::nr(std::numeric_limits<double>::infinity()).str());
    CHECK_EQUAL("NaN", FTS::String::nr(std::numeric_limits<double>::signaling_NaN()).str());
    CHECK_EQUAL("NaN", FTS::String::nr(std::numeric_limits<double>::quiet_NaN()).str());

    CHECK_EQUAL("True", FTS::String::b(true).str());
    CHECK_EQUAL("False", FTS::String::b(false).str());
    CHECK_EQUAL("True", FTS::String::b(!0).str());
    CHECK_EQUAL("False", FTS::String::b(!1).str());

    CHECK_EQUAL(6, FTS::String::random("######", FTS::random<int>).len());
    CHECK_EQUAL(3, FTS::String::random("#\\##", FTS::random<int>).len());
    CHECK_EQUAL('#', FTS::String::random("#\\##", FTS::random<int>).getCharAt(1));
    CHECK_EQUAL('\\', FTS::String::random("#\\\\##", FTS::random<int>).getCharAt(1));
    CHECK_EQUAL(4, FTS::String::random("#\\\\##", FTS::random<int>).len());
}

TEST_INSUITE(dString, LeftRightMid)
{
    CHECK_EQUAL("Hello", FTS::String("Hello, nice world!").left(5).str());
    CHECK_EQUAL("world!", FTS::String("Hello, nice world!").right(6).str());
    CHECK_EQUAL("nice", FTS::String("Hello, nice world!").mid(7,7).str());
    CHECK_EQUAL("world", FTS::String("Hello, nice world!").mid(12,1).str());

    CHECK_EQUAL("", FTS::String("Hello, nice world!").left(0).str());
    CHECK_EQUAL("H", FTS::String("Hello, nice world!").left(1).str());
    CHECK_EQUAL("Hello, nice world!", FTS::String("Hello, nice world!").left(18).str());
    CHECK_EQUAL("Hello, nice world!", FTS::String("Hello, nice world!").left(19).str());
    CHECK_EQUAL("Hello, nice world!", FTS::String("Hello, nice world!").left(100).str());

    CHECK_EQUAL("", FTS::String("Hello, nice world!").right(0).str());
    CHECK_EQUAL("!", FTS::String("Hello, nice world!").right(1).str());
    CHECK_EQUAL("Hello, nice world!", FTS::String("Hello, nice world!").right(18).str());
    CHECK_EQUAL("Hello, nice world!", FTS::String("Hello, nice world!").right(19).str());
    CHECK_EQUAL("Hello, nice world!", FTS::String("Hello, nice world!").right(100).str());

    CHECK_EQUAL("Hello, nice world!", FTS::String("Hello, nice world!").mid(0,0).str());
    CHECK_EQUAL("Hello, nice world", FTS::String("Hello, nice world!").mid(0,1).str());
    CHECK_EQUAL("ello, nice world!", FTS::String("Hello, nice world!").mid(1,0).str());
    CHECK_EQUAL("ello, nice world", FTS::String("Hello, nice world!").mid(1,1).str());
    CHECK_EQUAL("", FTS::String("Hello, nice world!").mid(18,0).str());
    CHECK_EQUAL("", FTS::String("Hello, nice world!").mid(18,1).str());
    CHECK_EQUAL("", FTS::String("Hello, nice world!").mid(19,0).str());
    CHECK_EQUAL("", FTS::String("Hello, nice world!").mid(19,1).str());
    CHECK_EQUAL("", FTS::String("Hello, nice world!").mid(0,18).str());
    CHECK_EQUAL("", FTS::String("Hello, nice world!").mid(1,18).str());
    CHECK_EQUAL("", FTS::String("Hello, nice world!").mid(0,19).str());
    CHECK_EQUAL("", FTS::String("Hello, nice world!").mid(1,19).str());
}

TEST_INSUITE(dString, HexData)
{
    for(uint8_t byte=0 ; byte!=255 ; ++byte) {
        std::stringstream ss;
        ss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)byte;
        std::string s = ss.str();
        CHECK_EQUAL(s, FTS::String::sfromHex(byte).str());
    }

    for(uint8_t byte=0 ; byte!=255 ; ++byte) {
        std::stringstream ss;
        ss << std::hex << std::nouppercase << std::setw(2) << std::setfill('0') << (int)byte;
        std::string s = ss.str();
        CHECK_EQUAL(s, FTS::String::sfromHex(byte, false).str());
    }

    for(uint8_t byte=0 ; byte!=255 ; ++byte) {
        std::stringstream ss;
        ss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)byte;
        std::string s = ss.str();
        bool bSuccess = false;
        CHECK_EQUAL((int)byte, (int)FTS::String::byteFromHex(s, bSuccess));
        CHECK(bSuccess);
    }
    for(uint8_t byte=0 ; byte!=16 ; ++byte) {
        std::stringstream ss;
        ss << std::hex << std::uppercase << (int)byte;
        std::string s = ss.str();
        bool bSuccess = false;
        CHECK_EQUAL((int)byte, (int)FTS::String::byteFromHex(s, bSuccess));
        CHECK(bSuccess);
    }

    for(uint8_t byte=0 ; byte!=255 ; ++byte) {
        std::stringstream ss;
        ss << std::hex << std::nouppercase << std::setw(2) << std::setfill('0') << (int)byte;
        std::string s = ss.str();
        bool bSuccess = false;
        CHECK_EQUAL((int)byte, (int)FTS::String::byteFromHex(s, bSuccess));
        CHECK(bSuccess);
    }
    for(uint8_t byte=0 ; byte!=16 ; ++byte) {
        std::stringstream ss;
        ss << std::hex << std::nouppercase << (int)byte;
        std::string s = ss.str();
        bool bSuccess = false;
        CHECK_EQUAL((int)byte, (int)FTS::String::byteFromHex(s, bSuccess));
        CHECK(bSuccess);
    }

    bool bSuccess = false;
    CHECK_EQUAL(0, FTS::String::byteFromHex("", bSuccess));
    CHECK(!bSuccess);
    CHECK_EQUAL(0, FTS::String::byteFromHex("xyz", bSuccess));
    CHECK(!bSuccess);
    CHECK_EQUAL(0, FTS::String::byteFromHex("xy", bSuccess));
    CHECK(!bSuccess);
    CHECK_EQUAL(0, FTS::String::byteFromHex("0g", bSuccess));
    CHECK(!bSuccess);
    CHECK_EQUAL(0, FTS::String::byteFromHex("g", bSuccess));
    CHECK(!bSuccess);
    CHECK_EQUAL(0, FTS::String::byteFromHex("g0", bSuccess));
    CHECK(!bSuccess);
    CHECK_EQUAL(0, FTS::String::byteFromHex("XYZ", bSuccess));
    CHECK(!bSuccess);
    CHECK_EQUAL(0, FTS::String::byteFromHex("XY", bSuccess));
    CHECK(!bSuccess);
    CHECK_EQUAL(0, FTS::String::byteFromHex("0G", bSuccess));
    CHECK(!bSuccess);
    CHECK_EQUAL(0, FTS::String::byteFromHex("G", bSuccess));
    CHECK(!bSuccess);
    CHECK_EQUAL(0, FTS::String::byteFromHex("G0", bSuccess));
    CHECK(!bSuccess);
    CHECK_EQUAL(0, FTS::String::byteFromHex(FTS::String::chr('0'-1), bSuccess));
    CHECK(!bSuccess);

    // For those, let's just make some little tests.

    CHECK_EQUAL("000102030405060708090A0B0C0D0E0F10FF", FTS::String::hexFromData("\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F\x10\xFF",18));
    CHECK_EQUAL("000102030405060708090a0b0c0d0e0f10ff", FTS::String::hexFromData("\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F\x10\xFF",18, false));

    // Hmmm...
    uint8_t* testData1 = FTS::String("000102030405060708090A0B0C0D0E0F10FF").dataFromHex();
    uint8_t* testData2 = FTS::String("000102030405060708090a0b0c0d0e0f10ff").dataFromHex();
    uint8_t* testData3 = FTS::String("000102030g05060708090a0b0c0d0e0f10ff").dataFromHex();
    if(testData1) {
        CHECK_EQUAL(0, memcmp(testData1, "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F\x10\xFF", 18));
        delete [] testData1;
    } else {
        FAIL("testData1 = NULL");
    }
    if(testData2) {
        CHECK_EQUAL(0, memcmp(testData2, "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F\x10\xFF", 18));
        delete [] testData2;
    } else {
        FAIL("testData2 = NULL");
    }
    CHECK_EQUAL(NULL, testData3);
}

TEST_INSUITE(dString, Fmt)
{
    CHECK_EQUAL("", FTS::String::sfmt(""));
    CHECK_EQUAL("abcdefghi{10}", FTS::String::sfmt("{1}{2}{3}{4}{5}{6}{7}{8}{9}{10}", "a", "b", "c", "d", "e", "f", "g", "h", "i"));
    CHECK_EQUAL("abcdefghijklmnopqrs{10}u", FTS::String::sfmt("a{1}c{2}e{3}g{4}i{5}k{6}m{7}o{8}q{9}s{10}u", "b", "d", "f", "h", "j", "l", "n", "p", "r"));
    CHECK_EQUAL("", FTS::String::sfmtRemoveEmpty(""));
    CHECK_EQUAL("abcdefg{10}", FTS::String::sfmtRemoveEmpty("{1}{2}{3}{4}{5}{6}{7}{8}{9}{10}", "a", "b", "c", "d", "e", "f", "g"));
    CHECK_EQUAL("abcdefghijklmnoqs{10}u", FTS::String::sfmtRemoveEmpty("a{1}c{2}e{3}g{4}i{5}k{6}m{7}o{8}q{9}s{10}u", "b", "d", "f", "h", "j", "l", "n"));

    CHECK_EQUAL("", FTS::String("").fmt(""));
    CHECK_EQUAL("abcdefghi{10}", FTS::String("{1}{2}{3}{4}{5}{6}{7}{8}{9}{10}").fmt("a", "b", "c", "d", "e", "f", "g", "h", "i"));
    CHECK_EQUAL("abcdefghijklmnopqrs{10}u", FTS::String("a{1}c{2}e{3}g{4}i{5}k{6}m{7}o{8}q{9}s{10}u").fmt("b", "d", "f", "h", "j", "l", "n", "p", "r"));
    CHECK_EQUAL("", FTS::String("").fmtRemoveEmpty(""));
    CHECK_EQUAL("abcdefg{10}", FTS::String("{1}{2}{3}{4}{5}{6}{7}{8}{9}{10}").fmtRemoveEmpty("a", "b", "c", "d", "e", "f", "g"));
    CHECK_EQUAL("abcdefghijklmnoqs{10}u", FTS::String("a{1}c{2}e{3}g{4}i{5}k{6}m{7}o{8}q{9}s{10}u").fmtRemoveEmpty("b", "d", "f", "h", "j", "l", "n"));
}

TEST_INSUITE(dString, Trivial)
{
    CHECK(FTS::String("").isEmpty());
    CHECK(FTS::String::EMPTY.isEmpty());

    CHECK_EQUAL(0, FTS::String("").len());
    CHECK_EQUAL(0, FTS::String("").lenInt());
    CHECK_EQUAL(1, FTS::String("a").len());
    CHECK_EQUAL(1, FTS::String("a").lenInt());
    CHECK_EQUAL(5, FTS::String("abcde").len());
    CHECK_EQUAL(5, FTS::String("abcde").lenInt());

    /* Some conversion functions. */
    CHECK_EQUAL(std::string(""), std::string(FTS::String("").c_str()));
    CHECK_EQUAL(std::string("abcde"), std::string(FTS::String("abcde").c_str()));
    CHECK_EQUAL(std::string(""), FTS::String("").str());
    CHECK_EQUAL(std::string("abcde"), FTS::String("abcde").str());
}

TEST_INSUITE(dString, RemRepl)
{
    CHECK_EQUAL("", FTS::String("!").removeChar(0));
    CHECK_EQUAL("", FTS::String("").removeChar(0));
    CHECK_EQUAL("", FTS::String("").removeChar(1));
    CHECK_EQUAL("", FTS::String("!").removeChar(1));
    CHECK_EQUAL("Hello, World!", FTS::String("!Hello, World!").removeChar(0));
    CHECK_EQUAL("Hello, World!", FTS::String("H!ello, World!").removeChar(1));
    CHECK_EQUAL("Hello, World!", FTS::String("Hello, World!?").removeChar(13));
    CHECK_EQUAL("Hello, World", FTS::String("Hello, World!").removeChar(13));
    CHECK_EQUAL("Hello, World", FTS::String("Hello, World!").removeChar(14));
    CHECK_EQUAL("Hello, World", FTS::String("Hello, World!").removeChar(-1));

    CHECK_EQUAL("!", FTS::String("").addChar(0, '!'));
    CHECK_EQUAL("!", FTS::String("").addChar(1, '!'));
    CHECK_EQUAL("!", FTS::String("").addChar(10, '!'));
    CHECK_EQUAL("Hello, World!", FTS::String("ello, World!").addChar(0, 'H'));
    CHECK_EQUAL("Hello, World!", FTS::String("Hllo, World!").addChar(1, 'e'));
    CHECK_EQUAL("Hello, World!", FTS::String("Hello, World").addChar(12, '!'));
    CHECK_EQUAL("Hello, World!", FTS::String("Hello, World").addChar(13, '!'));
    CHECK_EQUAL("Hello, World!", FTS::String("Hello, World").addChar(130, '!'));

    CHECK_EQUAL("d", FTS::String("Hello, World").replaceStr(0, 11, ""));
    CHECK_EQUAL("d", FTS::String("Hello, World").replaceStr(0, 11, FTS::String("")));
    CHECK_EQUAL("H", FTS::String("Hello, World").replaceStr(1, 11, ""));
    CHECK_EQUAL("H", FTS::String("Hello, World").replaceStr(1, 11, FTS::String("")));
    CHECK_EQUAL("Hd", FTS::String("Hello, World").replaceStr(1, 10, ""));
    CHECK_EQUAL("Hd", FTS::String("Hello, World").replaceStr(1, 10, FTS::String("")));
    CHECK_EQUAL("", FTS::String("Hello, World").replaceStr(0, 12, ""));
    CHECK_EQUAL("", FTS::String("Hello, World").replaceStr(0, 12, FTS::String("")));
    CHECK_EQUAL("", FTS::String("Hello, World").replaceStr(0, 120, ""));
    CHECK_EQUAL("", FTS::String("Hello, World").replaceStr(0, 120, FTS::String("")));
    CHECK_EQUAL("Hello, World!", FTS::String("you suck").replaceStr(0, 8, "Hello, World!"));
    CHECK_EQUAL("Hello, World!", FTS::String("you suck").replaceStr(0, 8, FTS::String("Hello, World!")));
    CHECK_EQUAL("Hello, World!", FTS::String("you suck").replaceStr(0, 120, "Hello, World!"));
    CHECK_EQUAL("Hello, World!", FTS::String("you suck").replaceStr(0, 120, FTS::String("Hello, World!")));
    CHECK_EQUAL("Hello, World!", FTS::String("Mello, World!").replaceStr(0, 1, "H"));
    CHECK_EQUAL("Hello, World!", FTS::String("Mello, World!").replaceStr(0, 1, FTS::String("H")));
    CHECK_EQUAL("Hello, World!", FTS::String("llo, World!").replaceStr(0, 0, "He"));
    CHECK_EQUAL("Hello, World!", FTS::String("llo, World!").replaceStr(0, 0, FTS::String("He")));
    CHECK_EQUAL("Hello, World!", FTS::String("Fucking ello, World!").replaceStr(0, 8, "H"));
    CHECK_EQUAL("Hello, World!", FTS::String("Fucking ello, World!").replaceStr(0, 8, FTS::String("H")));
    CHECK_EQUAL("Hello, World!", FTS::String("Fucking llo, World!").replaceStr(0, 8, "He"));
    CHECK_EQUAL("Hello, World!", FTS::String("Fucking llo, World!").replaceStr(0, 8, FTS::String("He")));
    CHECK_EQUAL("Hello, World!", FTS::String("Hello, Worlorz?").replaceStr(11, 4, "d!"));
    CHECK_EQUAL("Hello, World!", FTS::String("Hello, Worlorz?").replaceStr(11, 4, FTS::String("d!")));
    CHECK_EQUAL("Hello, World!", FTS::String("Hello, Worlorz?").replaceStr(11, 10, "d!"));
    CHECK_EQUAL("Hello, World!", FTS::String("Hello, Worlorz?").replaceStr(11, 10, FTS::String("d!")));
    CHECK_EQUAL("Hello, World!", FTS::String("you suck").replaceStr(0, 120, "Hello, World!"));
    CHECK_EQUAL("Hello, World!", FTS::String("you suck").replaceStr(0, 120, FTS::String("Hello, World!")));
    CHECK_EQUAL("you sucHello, World!", FTS::String("you suck").replaceStr(7, -1, "Hello, World!"));
    CHECK_EQUAL("you sucHello, World!", FTS::String("you suck").replaceStr(7, -1, FTS::String("Hello, World!")));
    CHECK_EQUAL("Hello, World!", FTS::String("Hello, World!").replaceStr(100, 3, "abc"));
    CHECK_EQUAL("Hello, World!", FTS::String("Hello, World!").replaceStr(100, 3, FTS::String("abc")));
    CHECK_EQUAL("Hello, World!", FTS::String("Hello, World!").replaceStr(100, 120, "abc"));
    CHECK_EQUAL("Hello, World!", FTS::String("Hello, World!").replaceStr(100, 120, FTS::String("abc")));

    CHECK_EQUAL("", FTS::String("").replaceStr("", ""));
    CHECK_EQUAL("c", FTS::String("c").replaceStr("c", "c"));
    CHECK_EQUAL("c", FTS::String("c").replaceStr("a", "b"));
    CHECK_EQUAL("c", FTS::String("b").replaceStr("b", "c"));
    CHECK_EQUAL("asd", FTS::String("ard").replaceStr("ar", "as"));
    CHECK_EQUAL("dos", FTS::String("dos").replaceStr("dosa", "aaa"));
    CHECK_EQUAL("dos", FTS::String("dos").replaceStr("ados", "aaa"));
    CHECK_EQUAL("Hello, World!", FTS::String("Hello,World!").replaceStr(",", ", "));
    CHECK_EQUAL("Hello, World!", FTS::String("Bye, Dude").replaceStr("Bye, Dude", "Hello, World!"));
    CHECK_EQUAL("Hello, World!", FTS::String("Bye, World!").replaceStr("Bye", "Hello"));

}

TEST_INSUITE(dString, Trimming)
{
    CHECK_EQUAL("", FTS::String("").trimThisLeft());
    CHECK_EQUAL("", FTS::String("").trimLeft());
    CHECK_EQUAL("", FTS::String(" \n \r \t ").trimThisLeft());
    CHECK_EQUAL("", FTS::String(" \n \r \t ").trimLeft());
    CHECK_EQUAL("Hello, World!", FTS::String("Hello, World!").trimThisLeft());
    CHECK_EQUAL("Hello, World!", FTS::String("Hello, World!").trimLeft());
    CHECK_EQUAL("Hello, World!", FTS::String("  Hello, World!").trimThisLeft());
    CHECK_EQUAL("Hello, World!", FTS::String("  Hello, World!").trimLeft());
    CHECK_EQUAL("Hello, World!", FTS::String("\n\r\t Hello, World!").trimThisLeft());
    CHECK_EQUAL("Hello, World!", FTS::String("\n\r\t Hello, World!").trimLeft());
    CHECK_EQUAL("Hello, World!", FTS::String(" \n \r \t Hello, World!").trimThisLeft());
    CHECK_EQUAL("Hello, World!", FTS::String(" \n \r \t Hello, World!").trimLeft());
    CHECK_EQUAL("Hello, World!  ", FTS::String("Hello, World!  ").trimThisLeft());
    CHECK_EQUAL("Hello, World!  ", FTS::String("Hello, World!  ").trimLeft());
    CHECK_EQUAL("Hello, World!", FTS::String("!!!?!?!?!11Hello, World!").trimThisLeft("!?1"));
    CHECK_EQUAL("Hello, World!", FTS::String("!!!?!?!?!11Hello, World!").trimLeft("!?1"));
    CHECK_EQUAL("Hello, World!", FTS::String("!!!Hello, World!").trimThisLeft("!!!!"));
    CHECK_EQUAL("Hello, World!", FTS::String("!!!Hello, World!").trimLeft("!!!!"));

    CHECK_EQUAL("", FTS::String("").trimThisRight());
    CHECK_EQUAL("", FTS::String("").trimRight());
    CHECK_EQUAL("", FTS::String(" \n \r \t ").trimThisRight());
    CHECK_EQUAL("", FTS::String(" \n \r \t ").trimRight());
    CHECK_EQUAL("Hello, World!", FTS::String("Hello, World!").trimThisRight());
    CHECK_EQUAL("Hello, World!", FTS::String("Hello, World!").trimRight());
    CHECK_EQUAL("Hello, World!", FTS::String("Hello, World!  ").trimThisRight());
    CHECK_EQUAL("Hello, World!", FTS::String("Hello, World!  ").trimRight());
    CHECK_EQUAL("Hello, World!", FTS::String("Hello, World!\n\r\t ").trimThisRight());
    CHECK_EQUAL("Hello, World!", FTS::String("Hello, World!\n\r\t ").trimRight());
    CHECK_EQUAL("Hello, World!", FTS::String("Hello, World! \n \r \t ").trimThisRight());
    CHECK_EQUAL("Hello, World!", FTS::String("Hello, World! \n \r \t ").trimRight());
    CHECK_EQUAL("  Hello, World!", FTS::String("  Hello, World!").trimThisRight());
    CHECK_EQUAL("  Hello, World!", FTS::String("  Hello, World!").trimRight());
    CHECK_EQUAL("Hello, World", FTS::String("Hello, World?1!!!!?!?!?!11").trimThisRight("!?1"));
    CHECK_EQUAL("Hello, World", FTS::String("Hello, World?1!!!!?!?!?!11").trimRight("!?1"));
    CHECK_EQUAL("Hello, World", FTS::String("Hello, World!!!").trimThisRight("!!!!"));
    CHECK_EQUAL("Hello, World", FTS::String("Hello, World!!!").trimRight("!!!!"));

    CHECK_EQUAL("", FTS::String("").trimThis());
    CHECK_EQUAL("", FTS::String("").trim());
    CHECK_EQUAL("", FTS::String(" \n \r \t ").trimThis());
    CHECK_EQUAL("", FTS::String(" \n \r \t ").trim());
    CHECK_EQUAL("Hello, World!", FTS::String("Hello, World!").trimThis());
    CHECK_EQUAL("Hello, World!", FTS::String("Hello, World!").trim());
    CHECK_EQUAL("Hello, World!", FTS::String("  Hello, World!  ").trimThis());
    CHECK_EQUAL("Hello, World!", FTS::String("  Hello, World!  ").trim());
    CHECK_EQUAL("Hello, World!", FTS::String("\n\r\t Hello, World!\n\r\t ").trimThis());
    CHECK_EQUAL("Hello, World!", FTS::String("\n\r\t Hello, World!\n\r\t ").trim());
    CHECK_EQUAL("Hello, World!", FTS::String(" \n \r \t Hello, World! \n \r \t ").trimThis());
    CHECK_EQUAL("Hello, World!", FTS::String(" \n \r \t Hello, World! \n \r \t ").trim());
    CHECK_EQUAL("Hello, World", FTS::String("Hello, World").trimThis(" ,"));
    CHECK_EQUAL("Hello, World", FTS::String("Hello, World").trim(" ,"));
    CHECK_EQUAL("Hello, World", FTS::String("!?!?!?!11Hello, World?1!!!!?!?!?!11").trimThis("!?1"));
    CHECK_EQUAL("Hello, World", FTS::String("!?!?!?!11Hello, World?1!!!!?!?!?!11").trim("!?1"));
    CHECK_EQUAL("Hello, World", FTS::String("!!!Hello, World!!!").trimThis("!!!!"));
    CHECK_EQUAL("Hello, World", FTS::String("!!!Hello, World!!!").trim("!!!!"));

    FTS::String hello("  Hello  ");
    CHECK_EQUAL("Hello  ", hello.trimLeft());
    CHECK_EQUAL("  Hello  ", hello);
    CHECK_EQUAL("  Hello", hello.trimRight());
    CHECK_EQUAL("  Hello  ", hello);
    CHECK_EQUAL("Hello", hello.trim());
    CHECK_EQUAL("  Hello  ", hello);
}

TEST_INSUITE(dString, UpperLower)
{
    CHECK_EQUAL("", FTS::String("").lowerThis());
    CHECK_EQUAL("", FTS::String("").lower());
    CHECK_EQUAL("0123456789", FTS::String("0123456789").lowerThis());
    CHECK_EQUAL("0123456789", FTS::String("0123456789").lower());
    CHECK_EQUAL("abcdefghijklmnopqrstuvwxyz", FTS::String("abcdefghijklmnopqrstuvwxyz").lowerThis());
    CHECK_EQUAL("abcdefghijklmnopqrstuvwxyz", FTS::String("abcdefghijklmnopqrstuvwxyz").lower());
    CHECK_EQUAL("abcdefghijklmnopqrstuvwxyz", FTS::String("ABCDEFGHIJKLMNOPQRSTUVWXYZ").lowerThis());
    CHECK_EQUAL("abcdefghijklmnopqrstuvwxyz", FTS::String("ABCDEFGHIJKLMNOPQRSTUVWXYZ").lower());
    CHECK_EQUAL("abcdefghijklmnopqrstuvwxyz", FTS::String("AbCdEfGhIjKlMnOpQrStUvWxYz").lowerThis());
    CHECK_EQUAL("abcdefghijklmnopqrstuvwxyz", FTS::String("AbCdEfGhIjKlMnOpQrStUvWxYz").lower());
    CHECK_EQUAL("abcdefghijklmnopqrstuvwxyz", FTS::String("aBcDeFgHiJkLmNoPqRsTuVwXyZ").lowerThis());
    CHECK_EQUAL("abcdefghijklmnopqrstuvwxyz", FTS::String("aBcDeFgHiJkLmNoPqRsTuVwXyZ").lower());

    CHECK_EQUAL("", FTS::String("").upperThis());
    CHECK_EQUAL("", FTS::String("").upper());
    CHECK_EQUAL("0123456789", FTS::String("0123456789").upperThis());
    CHECK_EQUAL("0123456789", FTS::String("0123456789").upper());
    CHECK_EQUAL("ABCDEFGHIJKLMNOPQRSTUVWXYZ", FTS::String("ABCDEFGHIJKLMNOPQRSTUVWXYZ").upperThis());
    CHECK_EQUAL("ABCDEFGHIJKLMNOPQRSTUVWXYZ", FTS::String("ABCDEFGHIJKLMNOPQRSTUVWXYZ").upper());
    CHECK_EQUAL("ABCDEFGHIJKLMNOPQRSTUVWXYZ", FTS::String("abcdefghijklmnopqrstuvwxyz").upperThis());
    CHECK_EQUAL("ABCDEFGHIJKLMNOPQRSTUVWXYZ", FTS::String("abcdefghijklmnopqrstuvwxyz").upper());
    CHECK_EQUAL("ABCDEFGHIJKLMNOPQRSTUVWXYZ", FTS::String("AbCdEfGhIjKlMnOpQrStUvWxYz").upperThis());
    CHECK_EQUAL("ABCDEFGHIJKLMNOPQRSTUVWXYZ", FTS::String("AbCdEfGhIjKlMnOpQrStUvWxYz").upper());
    CHECK_EQUAL("ABCDEFGHIJKLMNOPQRSTUVWXYZ", FTS::String("aBcDeFgHiJkLmNoPqRsTuVwXyZ").upperThis());
    CHECK_EQUAL("ABCDEFGHIJKLMNOPQRSTUVWXYZ", FTS::String("aBcDeFgHiJkLmNoPqRsTuVwXyZ").upper());

    FTS::String a("AbCd");
    CHECK_EQUAL("abcd", a.lower());
    CHECK_EQUAL("AbCd", a);
    CHECK_EQUAL("ABCD", a.upper());
    CHECK_EQUAL("AbCd", a);
}

TEST_INSUITE(dString, Searching)
{
    CHECK_EQUAL(std::string::npos, FTS::String("").find(""));
    CHECK_EQUAL(std::string::npos, FTS::String("").find(FTS::String("")));
    CHECK_EQUAL(std::string::npos, FTS::String("abc").find(""));
    CHECK_EQUAL(std::string::npos, FTS::String("abc").find(FTS::String("")));
    CHECK_EQUAL(std::string::npos, FTS::String("").find("abc"));
    CHECK_EQUAL(std::string::npos, FTS::String("").find(FTS::String("abc")));
    CHECK_EQUAL(std::string::npos, FTS::String("a").find("a", 1));
    CHECK_EQUAL(std::string::npos, FTS::String("a").find(FTS::String("a"), 1));
    CHECK_EQUAL(std::string::npos, FTS::String("a").find("a", 100));
    CHECK_EQUAL(std::string::npos, FTS::String("a").find(FTS::String("a"), 100));
    CHECK_EQUAL(std::string::npos, FTS::String("a").find("b"));
    CHECK_EQUAL(std::string::npos, FTS::String("a").find(FTS::String("b")));
    CHECK_EQUAL(std::string::npos, FTS::String("bab").find("b", 3));
    CHECK_EQUAL(std::string::npos, FTS::String("bab").find(FTS::String("b"), 3));
    CHECK_EQUAL(4, FTS::String("bababbabbba").find("bbabb"));
    CHECK_EQUAL(4, FTS::String("bababbabbba").find(FTS::String("bbabb")));
    CHECK_EQUAL(4, FTS::String("bababbabbba").find("bbabb", 1));
    CHECK_EQUAL(4, FTS::String("bababbabbba").find(FTS::String("bbabb"), 1));

    CHECK(FTS::String("").ieq(""));
    CHECK(FTS::String("").ieq(FTS::String("")));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").ieq("!abcdefghijklmnopqrstuvwxyz?"));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").ieq(FTS::String("!abcdefghijklmnopqrstuvwxyz?")));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").ieq("!ABCDEFGHIJKLMNOPQRSTUVWXYZ?"));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").ieq(FTS::String("!ABCDEFGHIJKLMNOPQRSTUVWXYZ?")));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").ieq("!AbCdEfGhIjKlMnOpQrStUvWxYz?"));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").ieq(FTS::String("!AbCdEfGhIjKlMnOpQrStUvWxYz?")));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").ieq("!aBcDeFgHiJkLmNoPqRsTuVwXyZ?"));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").ieq(FTS::String("!aBcDeFgHiJkLmNoPqRsTuVwXyZ?")));
    CHECK(FTS::String("0123456789").ieq("0123456789"));
    CHECK(FTS::String("0123456789").ieq(FTS::String("0123456789")));
    CHECK(not FTS::String("0123456789").ieq(FTS::String("0123456788")));
    CHECK(not FTS::String("0123456789").ieq(FTS::String("1123456789")));
    CHECK(not FTS::String("!abcdefghijklmnopqrstuvwxyz?").ieq("!ABCDEFGHIJKLMNOPQRSTUVWXYZ"));

    CHECK(FTS::String("").ncmp(""));
    CHECK(FTS::String("").ncmp(FTS::String("")));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").ncmp("!abc"));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").ncmp(FTS::String("!abc")));
    CHECK(not FTS::String("!abc").ncmp("!abcdefghijklmnopqrstuvwxyz?"));
    CHECK(not FTS::String("!abc").ncmp(FTS::String("!abcdefghijklmnopqrstuvwxyz?")));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").ncmp("!abcdefghijklmnopqrstuvwxyz?"));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").ncmp(FTS::String("!abcdefghijklmnopqrstuvwxyz?")));
    CHECK(not FTS::String("!abcdefghijklmnopqrstuvwxyz?").ncmp("!abcxyz", 0));
    CHECK(not FTS::String("!abcdefghijklmnopqrstuvwxyz?").ncmp(FTS::String("!abcxyz"), 0));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").ncmp("!abcxyz", 1));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").ncmp(FTS::String("!abcxyz"), 1));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").ncmp("!abcxyz", 2));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").ncmp(FTS::String("!abcxyz"), 2));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").ncmp("!abcxyz", 3));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").ncmp(FTS::String("!abcxyz"), 3));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").ncmp("!abcxyz", 4));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").ncmp(FTS::String("!abcxyz"), 4));
    CHECK(not FTS::String("!abcdefghijklmnopqrstuvwxyz?").ncmp("!abcxyz", 5));
    CHECK(not FTS::String("!abcdefghijklmnopqrstuvwxyz?").ncmp(FTS::String("!abcxyz"), 5));

    CHECK(FTS::String("").nicmp(""));
    CHECK(FTS::String("").nicmp(FTS::String("")));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").nicmp("!abc"));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").nicmp("!ABC"));
    CHECK(FTS::String("!aBcdefghijklmnopqrstuvwxyz?").nicmp("!abC"));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").nicmp(FTS::String("!abc")));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").nicmp(FTS::String("!ABC")));
    CHECK(FTS::String("!aBcdefghijklmnopqrstuvwxyz?").nicmp(FTS::String("!abC")));
    CHECK(not FTS::String("!abc").nicmp("!abcdefghijklmnopqrstuvwxyz?"));
    CHECK(not FTS::String("!aBc").nicmp("!abCdefghijklmnopqrstuvwxyz?"));
    CHECK(not FTS::String("!abc").nicmp(FTS::String("!abcdefghijklmnopqrstuvwxyz?")));
    CHECK(not FTS::String("!aBc").nicmp(FTS::String("!abCdefghijklmnopqrstuvwxyz?")));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").nicmp("!abcdefghijklmnopqrstuvwxyz?"));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").nicmp("!ABCDEFGHIJKLMNOPQRSTUVWXYZ?"));
    CHECK(FTS::String("!aBcDeFgHiJkLmNoPqRsTuVwXyZ?").nicmp("!AbCdEfGhIjKlMnOpQrStUvWxYz?"));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").nicmp(FTS::String("!abcdefghijklmnopqrstuvwxyz?")));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").nicmp(FTS::String("!ABCDEFGHIJKLMNOPQRSTUVWXYZ?")));
    CHECK(FTS::String("!aBcDeFgHiJkLmNoPqRsTuVwXyZ?").nicmp(FTS::String("!AbCdEfGhIjKlMnOpQrStUvWxYz?")));
    CHECK(not FTS::String("!abcdefghijklmnopqrstuvwxyz?").nicmp("!abcxyz", 0));
    CHECK(not FTS::String("!abcdefghijklmnopqrstuvwxyz?").nicmp("!ABCXYZ", 0));
    CHECK(not FTS::String("!AbCdefghijklmnopqrstuvwxyz?").nicmp("!aBcXyZ", 0));
    CHECK(not FTS::String("!abcdefghijklmnopqrstuvwxyz?").nicmp(FTS::String("!abcxyz"), 0));
    CHECK(not FTS::String("!abcdefghijklmnopqrstuvwxyz?").nicmp(FTS::String("!ABCXYZ"), 0));
    CHECK(not FTS::String("!AbCdefghijklmnopqrstuvwxyz?").nicmp(FTS::String("!aBcXyZ"), 0));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").nicmp("!abcxyz", 1));
    CHECK(FTS::String("!abCdefghijklmnopqrstuvwxyz?").nicmp("!aBcxyz", 1));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").nicmp(FTS::String("!abcxyz"), 1));
    CHECK(FTS::String("!abCdefghijklmnopqrstuvwxyz?").nicmp(FTS::String("!aBcxyz"), 1));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").nicmp("!abcxyz", 2));
    CHECK(FTS::String("!abCdefghijklmnopqrstuvwxyz?").nicmp("!aBcxyz", 2));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").nicmp(FTS::String("!abcxyz"), 2));
    CHECK(FTS::String("!abCdefghijklmnopqrstuvwxyz?").nicmp(FTS::String("!aBcxyz"), 2));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").nicmp("!abcxyz", 3));
    CHECK(FTS::String("!abCdefghijklmnopqrstuvwxyz?").nicmp("!aBcxyz", 3));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").nicmp(FTS::String("!abcxyz"), 3));
    CHECK(FTS::String("!abCdefghijklmnopqrstuvwxyz?").nicmp(FTS::String("!aBcxyz"), 3));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").nicmp("!abcxyz", 4));
    CHECK(FTS::String("!abCdefghijklmnopqrstuvwxyz?").nicmp("!aBcxyz", 4));
    CHECK(FTS::String("!abcdefghijklmnopqrstuvwxyz?").nicmp(FTS::String("!abcxyz"), 4));
    CHECK(FTS::String("!abCdefghijklmnopqrstuvwxyz?").nicmp(FTS::String("!aBcxyz"), 4));
    CHECK(not FTS::String("!abcdefghijklmnopqrstuvwxyz?").nicmp("!abcxyz", 5));
    CHECK(not FTS::String("!abCdefghijklmnopqrstuvwxyz?").nicmp("!aBcxyz", 5));
    CHECK(not FTS::String("!abcdefghijklmnopqrstuvwxyz?").nicmp(FTS::String("!abcxyz"), 5));
    CHECK(not FTS::String("!abCdefghijklmnopqrstuvwxyz?").nicmp(FTS::String("!aBcxyz"), 5));

    CHECK(FTS::String("Hello, World!").contains("Hello"));
    CHECK(FTS::String("Hello, World!").contains("World"));
    CHECK(FTS::String("Hello, World!").contains("!"));
    CHECK(not FTS::String("").contains(""));
    CHECK(not FTS::String("Hello, World!").contains(""));
    CHECK(not FTS::String("").contains("Hello"));
    CHECK(not FTS::String("").contains("H"));
    CHECK(not FTS::String("Hello, World!").contains("n"));
    CHECK(not FTS::String("Hello, World!").contains("noob"));
}

TEST_INSUITE(dString, Splitting)
{
    FTS::String out[5] = {"", "", "", "", ""};

    FTS::String("Hello World").split(out, " ");
    CHECK_EQUAL("Hello", out[0]);
    CHECK_EQUAL("World", out[1]);
    CHECK_EQUAL("", out[2]);
    CHECK_EQUAL("", out[3]);
    CHECK_EQUAL("", out[4]);

    out[0] = ""; out[1] = ""; out[2] = ""; out[3] = ""; out[4] = "";

    FTS::String("Hello World !").split(out, " ");
    CHECK_EQUAL("Hello", out[0]);
    CHECK_EQUAL("World", out[1]);
    CHECK_EQUAL("!", out[2]);
    CHECK_EQUAL("", out[3]);
    CHECK_EQUAL("", out[4]);

    out[0] = ""; out[1] = ""; out[2] = ""; out[3] = ""; out[4] = "";

    FTS::String(".a.c.").split(out, ".");
    CHECK_EQUAL("", out[0]);
    CHECK_EQUAL("a", out[1]);
    CHECK_EQUAL("c", out[2]);
    CHECK_EQUAL("", out[3]);
    CHECK_EQUAL("", out[4]);

    out[0] = ""; out[1] = ""; out[2] = ""; out[3] = ""; out[4] = "";

    FTS::String("Hello...World").split(out, "...");
    CHECK_EQUAL("Hello", out[0]);
    CHECK_EQUAL("World", out[1]);
    CHECK_EQUAL("", out[2]);
    CHECK_EQUAL("", out[3]);
    CHECK_EQUAL("", out[4]);

    out[0] = ""; out[1] = ""; out[2] = ""; out[3] = ""; out[4] = "";

    FTS::String("Hello...World...!").split(out, "...");
    CHECK_EQUAL("Hello", out[0]);
    CHECK_EQUAL("World", out[1]);
    CHECK_EQUAL("!", out[2]);
    CHECK_EQUAL("", out[3]);
    CHECK_EQUAL("", out[4]);

    out[0] = ""; out[1] = ""; out[2] = ""; out[3] = ""; out[4] = "";

    FTS::String("...a...c...").split(out, "...");
    CHECK_EQUAL("", out[0]);
    CHECK_EQUAL("a", out[1]);
    CHECK_EQUAL("c", out[2]);
    CHECK_EQUAL("", out[3]);
    CHECK_EQUAL("", out[4]);

    out[0] = ""; out[1] = ""; out[2] = ""; out[3] = ""; out[4] = "";

    FTS::String("Hello, World!").split(out, "x");
    CHECK_EQUAL("Hello, World!", out[0]);
    CHECK_EQUAL("", out[1]);
    CHECK_EQUAL("", out[2]);
    CHECK_EQUAL("", out[3]);
    CHECK_EQUAL("", out[4]);
}

class StringOperatorsSetup : public TestSetup
{
public:
    void setup()
    {
        s1 = "Hello, ";
        s2 = "Beautiful ";
        s3 = "World!";
    }
    void teardown() {}
protected:
    FTS::String s1, s2, s3;
};

TEST_INSUITE_WITHSETUP(dString, StringOperators, OperatorSet)
{
    s1 = s2 = s3 = "Nanana";
    CHECK_EQUAL(s1.str(), s2.str());
    CHECK_EQUAL(s2.str(), s3.str());
    CHECK_EQUAL(s1.str(), s3.str());
    CHECK_EQUAL("Nanana", s1.str());
    CHECK_EQUAL("Nanana", s2.str());
    CHECK_EQUAL("Nanana", s3.str());

    s1 = s2 = s3 = "";
    CHECK(s1.isEmpty());
    CHECK(s2.isEmpty());
    CHECK(s3.isEmpty());

    s1 = s2 = s3 = NULL;
    CHECK(s1.isEmpty());
    CHECK(s2.isEmpty());
    CHECK(s3.isEmpty());

    s1 = "Hello";
    s1 = s1 = s1;
    CHECK_EQUAL("Hello", s1.str());
}

TEST_INSUITE_WITHSETUP(dString, StringOperators, OperatorAddSet)
{
    s1 += "";
    CHECK_EQUAL("Hello, ", s1.str());

    s1 += NULL;
    CHECK_EQUAL("Hello, ", s1.str());

    s1 += FTS::String::EMPTY;
    CHECK_EQUAL("Hello, ", s1.str());

    s3 = FTS::String::EMPTY;
    s3 += s1;
    CHECK_EQUAL(s1.str(), s3.str());

    s3 = FTS::String::EMPTY;
    s3 += FTS::String::EMPTY;
    CHECK(s3.isEmpty());

    s3 = FTS::String::EMPTY;
    s3 += "";
    CHECK(s3.isEmpty());

    s3 = FTS::String::EMPTY;
    s3 += NULL;
    CHECK(s3.isEmpty());

    s1 += s2;
    CHECK_EQUAL("Hello, Beautiful ", s1.str());
}

TEST_INSUITE_WITHSETUP(dString, StringOperators, OperatorAdd)
{
    CHECK_EQUAL("Hello, Beautiful World!", s1 + s2 + s3);
    CHECK_EQUAL("Hello, ", s1 + "");
    CHECK_EQUAL("Hello, ", s1 + NULL);
    CHECK_EQUAL("Hello, ", s1 + FTS::String::EMPTY);

    CHECK((FTS::String::EMPTY + FTS::String::EMPTY).isEmpty());
    CHECK(("" + FTS::String::EMPTY).isEmpty());
    CHECK((FTS::String::EMPTY + "").isEmpty());
    CHECK((NULL + FTS::String::EMPTY).isEmpty());
    CHECK((FTS::String::EMPTY + NULL).isEmpty());
}

TEST_INSUITE_WITHSETUP(dString, StringOperators, OperatorEq)
{
    CHECK(not (s1 == s2));
    CHECK(not (s2 == s1));
    CHECK(not (s1 == FTS::String::EMPTY));
    CHECK(not (FTS::String::EMPTY == s1));
    CHECK(not (s1 == ""));
    CHECK(not ("" == s1));
    CHECK(not (s1 == NULL));
    CHECK(not (NULL == s1));
    CHECK(s1 == s1);
    CHECK(s1 == "Hello, ");
    CHECK(s1 == FTS::String("Hello, "));
    CHECK("Hello, " == s1);
    CHECK(FTS::String("Hello, ") == s1);
}

TEST_INSUITE_WITHSETUP(dString, StringOperators, OperatorNotEq)
{
    CHECK(s1 != s2);
    CHECK(s2 != s1);
    CHECK(s1 != FTS::String::EMPTY);
    CHECK(FTS::String::EMPTY != s1);
    CHECK(s1 != "");
    CHECK("" != s1);
    CHECK(s1 != NULL);
    CHECK(NULL != s1);
    CHECK(not (s1 != s1));
    CHECK(not (s1 != "Hello, "));
    CHECK(not (s1 != FTS::String("Hello, ")));
    CHECK(not ("Hello, " != s1));
    CHECK(not (FTS::String("Hello, ") != s1));
}

TEST_INSUITE_WITHSETUP(dString, StringOperators, OperatorNot)
{
    CHECK(not !s1);
    CHECK(!FTS::String::EMPTY);
}

TEST_INSUITE_WITHSETUP(dString, StringOperators, OperatorLess)
{
    CHECK(not ("" < FTS::String::EMPTY));
    CHECK(not (FTS::String::EMPTY < ""));
    CHECK("" < FTS::String("Hello"));
    CHECK(FTS::String("") < "Hello");
    CHECK(not ("Hello" < FTS::String("")));
    CHECK(not (FTS::String("Hello") < ""));
    CHECK("abcd" < FTS::String("abce"));
    CHECK(FTS::String("abcd") < "abce");
    CHECK("bbcd" < FTS::String("dbce"));
    CHECK(FTS::String("bbcd") < "dbce");
    CHECK(not ("abcd" < FTS::String("abcc")));
    CHECK(not (FTS::String("abcd") < "abcc"));
}

TEST_INSUITE_WITHSETUP(dString, StringOperators, OperatorAccessors)
{
    CHECK_EQUAL('H', s1[0]);
    CHECK_EQUAL('e', s1[1]);
    CHECK_EQUAL(' ', s1[6]);
    try {
        s1[7];
        s1[80];
        FAIL("No exception thrown by the string when accessed out-of-bounds");
    } catch(const std::out_of_range&) {
    }
    CHECK_EQUAL('H', s1.getCharAt(0));
    CHECK_EQUAL('e', s1.getCharAt(1));
    CHECK_EQUAL(' ', s1.getCharAt(6));
    CHECK_EQUAL('\0', s1.getCharAt(7));
    CHECK_EQUAL('\0', s1.getCharAt(80));
}

TEST_INSUITE(dString, Empty)
{
    CHECK(FTS::String::EMPTY.isEmpty());
}


