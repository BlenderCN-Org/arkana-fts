#ifndef TESTSUITE_H
#define TESTSUITE_H

#include "Test.h"

class TestSuite : public Test
{
public:
    TestSuite(const std::string& testSuiteName, TestSuite* whereToRegister = NULL);
    virtual ~TestSuite() {};

    virtual void addTest(Test *t);
    virtual void run(TestResult& result);
    virtual void runTest(TestResult& result) {};

    virtual void setup() {};
    virtual void teardown() {};

    virtual std::size_t testCount() const;

protected:
    std::vector<Test *>     tests;
};


#define TEST_INSUITE(suitename,casename)\
    class suitename##casename##Test : public Test\
    { \
        public: \
            suitename##casename##Test () : Test (#casename, &suitename) {} \
            void setup() {}; \
            void teardown() {}; \
            void runTest (TestResult& result_); \
    } suitename##casename##Instance; \
    void suitename##casename##Test::runTest (TestResult& result_) \

#define TEST_INSUITE_WITHSETUP(suitename,fixturename,testcasename)\
    class suitename##testcasename##fixturename##Test : public Test, fixturename##Setup\
    { \
        public: \
            suitename##testcasename##fixturename##Test () : Test (#testcasename, &suitename) {} \
            void setup() {fixturename##Setup::setup();} \
            void teardown() {fixturename##Setup::teardown();} \
            void runTest (TestResult& result_); \
    } suitename##testcasename##fixturename##Instance; \
    void suitename##testcasename##fixturename##Test::runTest (TestResult& result_) \

#define SUITE(suitename) \
    TestSuite suitename(#suitename); \

#define SUITE_WITHSETUP(suitename, fixturename) \
    class suitename##fixturename##Suite : public TestSuite, public fixturename##Setup\
    { \
        public: \
            suitename##fixturename##Suite () : TestSuite (#suitename) {} \
            void setup() {fixturename##Setup::setup();} \
            void teardown(); \
    } suitename; \
    void suitename##fixturename##Suite::teardown() {fixturename##Setup::teardown();}

#define SUITE_INSUITE(parentName,suitename) \
    TestSuite suitename(#suitename, &parentName); \

#define SUITE_INSUITE_WITHSETUP(parentName,suitename,fixturename) \
    class suitename##fixturename##Suite : public TestSuite, public fixturename##Setup\
    { \
        public: \
            suitename##fixturename##Suite () : TestSuite (#suitename, &parentName) {} \
            void setup() {fixturename##Setup::setup();} \
            void teardown(); \
    } suitename; \
    void suitename##fixturename##Suite::teardown() {fixturename##Setup::teardown();}

#endif
