

#include "TestHarness.h"
#include "MyTestClass.h"


class MyFixtureSetup : public TestSetup
{
public:
    void setup()
    {
        someValue = 2.0;
        str = "Hello";
    }

    void teardown()
    {
    }

protected:
    float someValue;
    std::string str;
    MyTestClass myObject;
};



TEST_WITHSETUP (MyFixture,Test1)
{
    CHECK_DOUBLES_EQUAL (someValue, 2.0f);
    someValue = 0;

    // CppUnitLite doesn't handle system exceptions very well either
    //myObject.UseBadPointer();

    // A regular exception works nicely though
    myObject.ThrowException();
}

TEST_WITHSETUP (MyFixture,Test2)
{
    CHECK_DOUBLES_EQUAL (someValue, 2.0f);
    CHECK_EQUAL (str, std::string("Hello"));
}


TEST_WITHSETUP (MyFixture,Test3)
{
    // Unfortunately, it looks like the framework creates 3 instances of MyTestClass
    // right at the beginning instead of doing it on demand for each test. We would
    // have to do it dynamically in the setup/teardown steps ourselves.
    CHECK_EQUAL (1, myObject.s_currentInstances);
    CHECK_EQUAL (3, myObject.s_instancesCreated);
    CHECK_EQUAL (1, myObject.s_maxSimultaneousInstances);
}

TEST_WITHSETUP (MyFixture,Test4)
{
    // Check for pompei2's std::exception catching:
    myObject.ThrowUserException();
}

TEST_WITHSETUP (MyFixture,Test5)
{
    // Check for pompei2's nice object checking:
    MyTestClass mySecondObject;
    CHECK_EQUAL(myObject, mySecondObject);
}

TEST_WITHSETUP (MyFixture,Test6)
{
    // Check for pompei2's nice object checking even works with primitives:
    CHECK_EQUAL(3, 7);
}
