#ifndef TEST_H
#define TEST_H

#ifndef D_TEST_DOUBLE_DELTA
#  define D_TEST_DOUBLE_DELTA 0.001
#endif // D_TEST_DOUBLE_DELTA

// Test is a base class for all tests.  It provides a command interface for
// running tests (run) as well as a data member for recording the name of
// the test.
//
// Tests are constructed using the TEST macro.  TEST creates a subclass of
// Test and static instance of that subclass.  If you look at the constructor
// for the Test class, you'll notice that it registers the created object
// with a global TestRegistry.  These features combine to make test creation
// particularly easy.

#include <vector>
#include <string>
#include <sstream>
#include <math.h>
#include <exception>


class TestSetup
{
public:
    virtual void setup() = 0;
    virtual void teardown() = 0;

    virtual ~TestSetup() {};
};



class TestResult;
class TestSuite;


class Test : public TestSetup
{
public:
    Test (const std::string& testName, TestSuite* whereToRegister = NULL);
    virtual ~Test() {};

    virtual void run (TestResult& result);
    virtual void runTest (TestResult& result) = 0;

    std::string name() const {return my_name;};
    const TestSuite* registrar() const {return my_registrar;}

protected:
    std::string my_name;
    const TestSuite* my_registrar;

};

#define TEST(name)\
	class name##Test : public Test\
	{ \
		public: \
			name##Test () : Test (#name) {} \
            void setup() {}; \
            void teardown() {}; \
			void runTest (TestResult& result_); \
	} name##Instance; \
	void name##Test::runTest (TestResult& result_) \


#define TEST_WITHSETUP(fixturename,testcasename)\
	class testcasename##fixturename##Test : public Test, fixturename##Setup\
	{ \
		public: \
			testcasename##fixturename##Test () : Test (#testcasename) {} \
            void setup() {fixturename##Setup::setup();} \
            void teardown() {fixturename##Setup::teardown();} \
			void runTest (TestResult& result_); \
	} testcasename##fixturename##Instance; \
	void testcasename##fixturename##Test::runTest (TestResult& result_) \


// Here is a collection of testing macros that can be used in the
// bodies of tests.  CHECK tests a boolean expression and records
// a failure if the expression evaluates to false. CHECK_DOUBLES_EQUAL
// compares doubles with a certain delta.
//
// To make this an industrial strength test harness, all objects you use in test
// cases for comparison should implement both an operator== and a global-scoped
// operator <<(const std::ostringstream&, const YourClass&). If both are
// implemented, CHECK_EQUAL will use your operator== to check for equality and
// will use your operator << to create a meaningful error message.


#define FAIL(info) \
    result_.addFailure(*this, Failure (info, my_name, __FILE__, __LINE__));


#define CHECK(condition) \
    if(TestRegistry::catchExceptions()) { \
        try { \
        if (!(condition)) \
        result_.addFailure (*this, Failure (#condition, my_name, __FILE__, __LINE__)); \
        } catch(const std::exception& e) { \
            std::string msg = std::string("Unhandled exception: ") + e.what(); \
            result_.addFailure (*this, Failure (msg, my_name, __FILE__, __LINE__)); \
        } catch(...) { \
            result_.addFailure (*this, Failure ("Unhandled exception", my_name, __FILE__, __LINE__)); \
        } \
    } else { \
        if (!(condition)) \
            result_.addFailure (*this, Failure (#condition, my_name, __FILE__, __LINE__)); \
    }

#define CHECK_EQUAL(expected,actual) \
    if(TestRegistry::catchExceptions()) { \
        try { \
            if (!(expected==actual)) {\
                std::ostringstream ss;\
                ss << "expected '" << expected;\
                ss << "' but was: '" << actual;\
                ss << "'";\
                result_.addFailure (*this, Failure (ss.str(), my_name, __FILE__, __LINE__));\
            }\
        } catch(const std::exception& e) { \
            std::string msg = std::string("Unhandled exception: ") + e.what(); \
            result_.addFailure (*this, Failure (msg, my_name, __FILE__, __LINE__)); \
        } catch(...) { \
            result_.addFailure (*this, Failure ("Unhandled exception", my_name, __FILE__, __LINE__)); \
        } \
    } else { \
        if (!(expected==actual)) {\
            std::ostringstream ss;\
            ss << "expected '" << expected << "' but was: '" << actual << "'";\
            result_.addFailure (*this, Failure (ss.str(), my_name, __FILE__, __LINE__));\
        } \
    }

#define CHECK_DOUBLES_EQUAL(expected,actual)\
    if(TestRegistry::catchExceptions()) { \
        try { \
            double _expected = (expected);\
            double _actual = (actual);\
            if (fabs ((_expected)-(_actual)) > D_TEST_DOUBLE_DELTA) {\
                std::ostringstream ss;\
                ss << "expected '" << _expected << "' but was: '" << _actual << "'";\
                result_.addFailure (*this, Failure (ss.str(), my_name, __FILE__, __LINE__));\
            }\
        } catch(const std::exception& e) { \
            std::string msg = std::string("Unhandled exception: ") + e.what(); \
            result_.addFailure (*this, Failure (msg, my_name, __FILE__, __LINE__)); \
        } catch(...) { \
            result_.addFailure (*this, Failure ("Unhandled exception", my_name, __FILE__, __LINE__)); \
        }\
    } else { \
        double _expected = (expected);\
        double _actual = (actual);\
        if (fabs ((_expected)-(_actual)) > D_TEST_DOUBLE_DELTA) {\
            std::ostringstream ss;\
            ss << "expected '" << _expected << "' but was: '" << _actual << "'";\
            result_.addFailure (*this, Failure (ss.str(), my_name, __FILE__, __LINE__));\
        } \
    }

#endif
