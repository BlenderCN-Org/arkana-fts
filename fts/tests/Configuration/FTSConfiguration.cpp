// FTSConfiguration.cpp : Defines the entry point for the console application.
//
#include "dLib/aTest/TestHarness.h"

#include "dLib/dConf/configuration.h"
#include "dLib/dConf/DefaultOptions.h"
#include "Settings.h"

using namespace FTS;
using namespace std;

namespace {
class TestOptions : public DefaultOptions
{
public:
    TestOptions() {
        add("float", 1.23f);
        add("bool", true);
        add("int", 100);
        add("char", "CHAR");
        add("String", String("String"));
        add("float_integer_digits", 10.f);
    }
};
}

SUITE(tConfiguration);

TEST_INSUITE(tConfiguration, ReadAndSet)
{
    std::string confFileName = "conf_test.xml";
    Configuration conf(confFileName, Settings("German"), false);

    CHECK_EQUAL(32, conf.get<int>("BPP"));
    CHECK_EQUAL(true, conf.get<bool>("ComplexQuads"));
    CHECK_EQUAL("arkana-fts.servegame.org", conf.get<string>("MasterServerName"));

    conf.set("BPP", 16);
    conf.set("ComplexQuads", false);
    conf.set("MasterServerName", "master.fts.arkana.org");
    CHECK_EQUAL(16, conf.get<int>("BPP"));
    CHECK_EQUAL(false, conf.get<bool>("ComplexQuads"));
    CHECK_EQUAL("master.fts.arkana.org", conf.get<string>("MasterServerName"));

    conf.save();
    Configuration confChanged(confFileName, Settings("German"), false);
    CHECK_EQUAL(16, confChanged.get<int>("BPP"));
    CHECK_EQUAL(false, confChanged.get<bool>("ComplexQuads"));
    CHECK_EQUAL("master.fts.arkana.org", confChanged.get<string>("MasterServerName"));
    
    // Restore orgiginal vvalues.
    confChanged.set("BPP", 32);
    confChanged.set("ComplexQuads", true);
    confChanged.set("MasterServerName", "arkana-fts.servegame.org");
    confChanged.save();
}

TEST_INSUITE(tConfiguration, TypeTestGoodCase)
{
    std::string confFileName = "conf_test2.xml";
    Configuration conf(confFileName, TestOptions(), false);
    CHECK_EQUAL(1.23f, conf.get<float>("float"));
    CHECK_EQUAL(true, conf.get<bool>("bool"));
    CHECK_EQUAL(100, conf.get<int>("int"));
    CHECK_EQUAL("CHAR", conf.get<string>("char"));
    CHECK_EQUAL("String", conf.get<string>("String"));
}

TEST_INSUITE(tConfiguration, TypeTestBool)
{
    std::string confFileName = "conf_test2.xml";
    Configuration conf(confFileName, TestOptions(), false);
    try {
        auto v = conf.get<bool>("int");
        FAIL("Get bool of a int option");
    } catch(CorruptDataException& ) {
        CHECK(true);
    }
    try {
        auto v = conf.get<bool>("float");
        FAIL("Get bool of a float option");
    } catch(CorruptDataException& ) {
        CHECK(true);
    }
    try {
        auto v = conf.get<bool>("char");
        FAIL("Get bool of a char option");
    } catch(CorruptDataException& ) {
        CHECK(true);
    }
    try {
        auto v = conf.get<bool>("String");
        FAIL("Get bool of a String option");
    } catch(CorruptDataException& ) {
        CHECK(true);
    }
}

TEST_INSUITE(tConfiguration, TypeTestInt)
{
    std::string confFileName = "conf_test2.xml";
    Configuration conf(confFileName, TestOptions(), false);
    try {
        auto v = conf.get<int>("bool");
        FAIL("Get int of a bool option");
    } catch(CorruptDataException& ) {
        CHECK(true);
    }
    try {
        auto v = conf.get<int>("float");
        FAIL("Get int of a float option");
    } catch(CorruptDataException& ) {
        CHECK(true);
    }
    try {
        auto v = conf.get<int>("char");
        FAIL("Get int of a char option");
    } catch(CorruptDataException& ) {
        CHECK(true);
    }
    try {
        auto v = conf.get<int>("String");
        FAIL("Get int of a String option");
    } catch(CorruptDataException& ) {
        CHECK(true);
    }
    try {
        auto v = conf.get<int>("float_integer_digits");
        FAIL("Get int of a String option");
    } catch(CorruptDataException& ) {
        CHECK(true);
    }
}

TEST_INSUITE(tConfiguration, TypeTestFloat)
{
    std::string confFileName = "conf_test2.xml";
    Configuration conf(confFileName, TestOptions(), false);
    try {
        auto v = conf.get<float>("bool");
        FAIL("Get float of a bool option");
    } catch(CorruptDataException& ) {
        CHECK(true);
    }
    try {
        // An integer can always converted into a float.
        auto v = conf.get<float>("int");
        CHECK_EQUAL(100, v);
        //FAIL("Get float of an int"); // <-- see comment above
    } catch(CorruptDataException& ex) {
        FAIL(ex.what());
    }
    try {
        auto v = conf.get<float>("char");
        FAIL("Get float of a char option");
    } catch(CorruptDataException& ) {
        CHECK(true);
    }
    try {
        auto v = conf.get<float>("String");
        FAIL("Get float of a String option");
    } catch(CorruptDataException& ) {
        CHECK(true);
    }
}

TEST_INSUITE(tConfiguration, TypeTestString)
{
    std::string confFileName = "conf_test2.xml";
    Configuration conf(confFileName, TestOptions(), false);
    // Get option as a string always works.
    try {
        auto v = conf.get<string>("bool");
        CHECK_EQUAL("True", v);
    } catch(CorruptDataException& ) {
        FAIL("Get String of a bool option");
    }
    try {
        auto v = conf.get<string>("int");
        CHECK_EQUAL("100", v);
    } catch(CorruptDataException& ) {
        FAIL("Get String of a int option");
    }
    try {
        auto v = conf.get<string>("float");
        CHECK(String(v).contains("1.23"));
    } catch(CorruptDataException& ) {
        FAIL("Get String of a float option");
    }
}

TEST_INSUITE(tConfiguration, Unknown_Key)
{
    std::string confFileName = "conf_test2.xml";
    Configuration conf(confFileName, TestOptions(), false);

    try {
        auto v = conf.get<string>("Character");
        CHECK_EQUAL("", v);
        FAIL("Get unknown int option");
    } catch(CorruptDataException&) {
    }
    try {
        auto v = conf.get<int>("Integer");
        CHECK_EQUAL(0, v);
        FAIL("Get unknown int option");
    } catch(CorruptDataException& ) {
    }
    try {
        auto v = conf.get<float>("Floating");
        CHECK_EQUAL(0.f, v);
        FAIL("Get unknown float option");
    } catch(CorruptDataException& ) {
    }
    try {
        auto v = conf.get<bool>("Boolean");
        CHECK_EQUAL(false, v);
        FAIL("Get unknown bool option");
    } catch(CorruptDataException& ) {
    }

    // TODO Should fail..
    try {
        auto v = conf.get<string>("Integer");
        CHECK_EQUAL("", v);
        FAIL("Get unknown int option");
    } catch(CorruptDataException&) {
    }
}