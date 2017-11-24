#include "dLib/aTest/TestHarness.h"

#include <ciso646>

#include "dLib/dString/dPath.h"

using namespace FTS;

SUITE(dPath)

TEST_INSUITE(dPath, Construction)
{
    CHECK(Path().empty());

    CHECK_EQUAL("/", Path("/"));
    CHECK_EQUAL("/", Path("////"));
    CHECK_EQUAL("/", Path("\\"));
    CHECK_EQUAL("/", Path("/\\/"));
    CHECK_EQUAL("/:______", Path("/:*?\"<>|\\"));

    CHECK_EQUAL("", Path(""));
    CHECK_EQUAL(".", Path("."));
    CHECK_EQUAL(".", Path("./"));
    CHECK_EQUAL(".", Path("./././"));

    /// \todo: Fix these tests to pass!
//     CHECK_EQUAL("/", Path("/."));
//     CHECK_EQUAL("/", Path("/.\\./."));
//     CHECK_EQUAL("file:///", Path("file:///"));

    CHECK_EQUAL("..", Path(".."));
    CHECK_EQUAL("..", Path("../"));
    CHECK_EQUAL("../..", Path("../.."));
    CHECK_EQUAL("../..", Path("../../"));
    CHECK_EQUAL("/bla/..", Path("/bla/.."));
    CHECK_EQUAL("/bla/..", Path("/bla/../"));

#if WINDOOF
    CHECK_EQUAL("C:/", Path("C:/"));
    CHECK_EQUAL("C:/", Path("C:\\"));
#endif
    // French (just by hitting all keyboard keys, may have forgotten some)
    CHECK_EQUAL("&é'(-è_çà)='1°+¹²~#{[`^@]}$^ù,;!.§%µ£¨ëäẗÿüïöḧẅẍâẑêŷûîôŝĝĥĵŵĉÂẐÊŶÛÎÔŜĜĤĴŴĈÄËŸÜÏÖḦẄẌ",
           Path("&é'(-è_çà)='1°+¹²~#{[`^@]}$^ù,;!.§%µ£¨ëäẗÿüïöḧẅẍâẑêŷûîôŝĝĥĵŵĉÂẐÊŶÛÎÔŜĜĤĴŴĈÄËŸÜÏÖḦẄẌ"));

    // German (just by hitting all keyboard keys, may have forgotten some)
    CHECK_EQUAL("^°§$%&{[()]}=ßẃéŕźúíóṕǘáśǵḱĺýćńḿśẁèùìòǜàỳǹẂÉŔŹÚÍÓṔǗÁŚǴḰĹÝĆŃḾẀÈÙÌÒǛÀỲǸ",
           Path("^°§$%&{[()]}=ßẃéŕźúíóṕǘáśǵḱĺýćńḿśẁèùìòǜàỳǹẂÉŔŹÚÍÓṔǗÁŚǴḰĹÝĆŃḾẀÈÙÌÒǛÀỲǸ"));

    // Thai (just by hitting all keyboard keys, may have forgotten some)
    CHECK_EQUAL("ๅ๑๒ภ๓ถ๔ุูึ฿ค๕ต๖จ๗ข๘ช๙ๆ๐ไำฎพฑะธัํี๊รณนฯยญบฐล,ฟฤหฆกฏดโเฌ้็่๋าษสศวซง.ฃฅผปแฉอฮิฺื์ทมฒใฬฝฦ",
           Path("ๅ๑๒ภ๓ถ๔ุูึ฿ค๕ต๖จ๗ข๘ช๙ๆ๐ไำฎพฑะธัํี๊รณนฯยญบฐล,ฟฤหฆกฏดโเฌ้็่๋าษสศวซง.ฃฅผปแฉอฮิฺื์ทมฒใฬฝฦ"));

    CHECK_EQUAL("/usr/local/FTS", Path("/usr/local/FTS"));
}

TEST_INSUITE(dPath, EscapeCheck)
{
    CHECK(not Path::wouldEscapeFile("Hello.txt"));
    CHECK(not Path::wouldEscapeFile("Hello_Moto.txt"));
    CHECK(not Path::wouldEscapeFile("&é'(-è_çà)='1°+¹²~#{[`^@]}$^ù,;!.§%µ£¨ëäẗÿüïöḧẅẍâẑêŷûîôŝĝĥĵŵĉÂẐÊŶÛÎÔŜĜĤĴŴĈÄËŸÜÏÖḦẄẌ"));
    CHECK(not Path::wouldEscapeFile("^°§$%&{[()]}=ßẃéŕźúíóṕǘáśǵḱĺýćńḿśẁèùìòǜàỳǹẂÉŔŹÚÍÓṔǗÁŚǴḰĹÝĆŃḾẀÈÙÌÒǛÀỲǸ"));
    CHECK(not Path::wouldEscapeFile("ๅ๑๒ภ๓ถ๔ุูึ฿ค๕ต๖จ๗ข๘ช๙ๆ๐ไำฎพฑะธัํี๊รณนฯยญบฐล,ฟฤหฆกฏดโเฌ้็่๋าษสศวซง.ฃฅผปแฉอฮิฺื์ทมฒใฬฝฦ"));
#if WINDOOF
    CHECK(Path::wouldEscapeFile("C:/Hello.txt"));
#endif
    CHECK(Path::wouldEscapeFile("/Hello_Moto.txt"));

#if WINDOOF
    CHECK(not Path::wouldEscapePath("C:/Hello.txt"));
    CHECK(not Path::wouldEscapePath("C:/Hello\\world .txt"));
    CHECK(not Path::wouldEscapePath("C:/Hello\\world .txt\\ "));
    CHECK(not Path::wouldEscapePath("C:/Hello\\world .txt\\ ."));
    CHECK(Path::wouldEscapePath("C:/Hello:world .txt\\ "));
#endif
    CHECK(not Path::wouldEscapePath("/Hello_Moto.txt"));
    CHECK(not Path::wouldEscapePath("/Hello/Moto .txt"));
    CHECK(not Path::wouldEscapePath("/Hello/Moto .txt/ "));
    CHECK(not Path::wouldEscapePath("/Hello/\\Moto .txt/ ."));
    CHECK(Path::wouldEscapePath("/Hello*Moto .txt/ "));
}

TEST_INSUITE(dPath, Basename)
{
#if WINDOOF
    CHECK_EQUAL("Hello.txt", Path("C:\\Hello.txt").basename());
    CHECK_EQUAL("Hello.txt", Path("C:/Hello.txt").basename());
    CHECK_EQUAL("Hello.txt", Path("C:\\World/Hello.txt").basename());
    CHECK_EQUAL("Hello.txt", Path("C:\\World/Hello.txt\\").basename());
#endif
    CHECK_EQUAL("Hello.txt", Path("Hello.txt").basename());
    CHECK_EQUAL("Hello.txt", Path("/Hello.txt").basename());
    CHECK_EQUAL("Hello.txt", Path("/World/Hello.txt").basename());
    CHECK_EQUAL("Hello.txt", Path("/World////Hello.txt").basename());
    CHECK_EQUAL("t", Path("/World////Hello.tx/t").basename());
    CHECK_EQUAL("Hello.txt", Path("/World/Hello.txt/").basename());
}

TEST_INSUITE(dPath, ExtStuff)
{
    CHECK_EQUAL("txt", Path("Hello.txt").ext());
    CHECK_EQUAL("txt", Path("C:/Hello.txt").ext());
    CHECK_EQUAL("txt", Path("C:\\Hello.txt").ext());
    CHECK_EQUAL("txt", Path("/Hello.txt").ext());
    CHECK_EQUAL("", Path("/Hello.txt.").ext());
    CHECK_EQUAL("txt", Path("/.Hello.txt").ext());
    CHECK_EQUAL("Hello", Path("/.Hello").ext());
    CHECK_EQUAL("", Path("/.../../.ab...").ext());

    CHECK_EQUAL("Hello", Path("Hello.txt").withoutExt());
    CHECK_EQUAL("C:/Hello", Path("C:/Hello.txt").withoutExt());
    CHECK_EQUAL("C:/Hello", Path("C:\\Hello.txt").withoutExt());
    CHECK_EQUAL("/Hello", Path("/Hello.txt").withoutExt());
    CHECK_EQUAL("/.Hello", Path("/.Hello.txt").withoutExt());
    CHECK_EQUAL("/", Path("/.Hello").withoutExt());
    CHECK_EQUAL("/.../../.ab..", Path("/.../../.ab...").withoutExt());
}

TEST_INSUITE(dPath, DirectoryStuff)
{
    CHECK_EQUAL(".", Path("Hello.txt").directory());
    CHECK_EQUAL("C:", Path("C:\\Hello.txt").directory());
    CHECK_EQUAL("/", Path("/Hello.txt").directory());
    CHECK_EQUAL(".", Path("./hi").directory());
    CHECK_EQUAL("..", Path("../hi").directory());
    CHECK_EQUAL("/..", Path("/../hi").directory());
    CHECK_EQUAL("C:/Hello/World", Path("C:\\Hello\\World/Bla.txt").directory());
    CHECK_EQUAL("/Hello/moto", Path("/Hello/moto/blabla").directory());

    CHECK_EQUAL("./World.txt", Path("Hello.txt").sameDir("World.txt"));
    CHECK_EQUAL("C:/World.txt", Path("C:\\Hello.txt").sameDir("World.txt"));
    CHECK_EQUAL("/World.txt", Path("/Hello.txt").sameDir("World.txt"));
    CHECK_EQUAL("C:/Hello/World/Bli.txt", Path("C:\\Hello\\World/Bla.txt").sameDir("Bli.txt"));
    CHECK_EQUAL("/Hello/moto/bliblo", Path("/Hello/moto/blabla").sameDir("bliblo"));
//     Path sameDir(const Path& in_sNewFile) const;
}

TEST_INSUITE(dPath, ProtocolStuff)
{
//     FAIL("TODO: Write tests");
//     String protocol() const;
}

TEST_INSUITE(dPath, appendWithSeparator)
{
//     FAIL("TODO: Write tests. Also make tests with protocol in path!!");
//     Path& appendWithSeparator();
}

TEST_INSUITE(dPath, rootLength)
{
#if WINDOOF
    CHECK_EQUAL(3, Path::rootLength("c:\\"));
    CHECK_EQUAL(3, Path::rootLength("c:\\path\\to\\file.txt"));
#endif
    CHECK_EQUAL(1, Path::rootLength("\\path\\to\\file.txt"));
    CHECK_EQUAL(1, Path::rootLength("\\"));
    CHECK_EQUAL(1, Path::rootLength("/"));
    CHECK_EQUAL(1, Path::rootLength("/path/to/file.txt"));
    CHECK_EQUAL(0, Path::rootLength("blalbla.txt"));
    CHECK_EQUAL(0, Path::rootLength(""));
    CHECK_EQUAL(0, Path::rootLength("blub/blalbla.txt"));
    CHECK_EQUAL(0, Path::rootLength("aha/blalbla"));
}
