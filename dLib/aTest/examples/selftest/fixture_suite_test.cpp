
#include "TestHarness.h"
#include "MyTestClass.h"

class MyFixturedSuiteSetup : public TestSetup
{
public:
    void setup()
    {
        suitewide_str = "Hello";
    }

    void teardown()
    {
    }

public:
    std::string suitewide_str;
};

class KittyFixtureSetup : public TestSetup
{
public:
    void setup()
    {
        kittyname = "MonorailCat";
    }

    void teardown()
    {
    }

public:
    std::string kittyname;
};

class MyFixtureRocksSetup : public TestSetup
{
public:
    void setup()
    {
        someValue = 4.0;
        str = "World";
    }

    void teardown()
    {
    }

protected:
    float someValue;
    std::string str;
    MyTestClass myObject;
};

SUITE_WITHSETUP(Imperial, MyFixturedSuite);

TEST_INSUITE_WITHSETUP(Imperial,MyFixtureRocks,Construction)
{
    CHECK_DOUBLES_EQUAL (someValue, 4.0f);
    someValue = 0;
    CHECK_EQUAL ("Hello", Imperial.suitewide_str);
    Imperial.suitewide_str = "World";
    str = "Wine?";

    // CppUnitLite doesn't handle system exceptions very well either
    //myObject.UseBadPointer();

    // A regular exception works nicely though
    myObject.ThrowException();
}

TEST_INSUITE_WITHSETUP(Imperial,MyFixtureRocks,Destruction)
{
    CHECK_EQUAL (str, Imperial.suitewide_str);
    CHECK_DOUBLES_EQUAL (2.0000001, 2.0f);
    CHECK_EQUAL ("Bla", std::string("HelloMoto"));
}

TEST_INSUITE(Imperial,IDontHaveSetupMyselfButMySuiteHas)
{
    CHECK_DOUBLES_EQUAL (2.0000001, 2.0f);
    CHECK_EQUAL ("baaaa", "baaaa");
}

SUITE_INSUITE_WITHSETUP(Imperial, Kitty, KittyFixture);

TEST_INSUITE(Imperial,Usage)
{
    CHECK_EQUAL ("\\Heavy'Loaded\"String\n\\", std::string("'Heavy\\Loaded\"String\n'"));
}

TEST_INSUITE(Kitty,MoreUsage)
{
    CHECK_EQUAL (Kitty.kittyname, "Hovercat");
    Kitty.kittyname = "Garfield";
}

TEST_INSUITE(Kitty,EvenMoreUsage)
{
    CHECK_EQUAL ("Garfield", Kitty.kittyname);
    throw "bla";
}
