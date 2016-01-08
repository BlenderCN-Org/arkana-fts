// FTSConfiguration.cpp : Defines the entry point for the console application.
//
#include "dLib/aTest/TestHarness.h"

#include "dLib/dConf/configuration.h"
#include "dLib/dConf/DefaultOptions.h"
#include "Settings.h"

using namespace FTS;

SUITE(tConfiguration);

TEST_INSUITE(tConfiguration, ReadAndSet)
{
    std::string confFileName = "conf_test.xml";
    Configuration conf(confFileName, Settings("German"), false);

    CHECK_EQUAL(32, conf.getInt("BPP"));
    CHECK_EQUAL(true, conf.getBool("ComplexQuads"));
    CHECK_EQUAL("arkana-fts.servegame.org", conf.get("MasterServerName"));

    conf.set("BPP", 16);
    conf.set("ComplexQuads", false);
    conf.set("MasterServerName", "master.fts.arkana.org");
    CHECK_EQUAL(16, conf.getInt("BPP"));
    CHECK_EQUAL(false, conf.getBool("ComplexQuads"));
    CHECK_EQUAL("master.fts.arkana.org", conf.get("MasterServerName"));

    conf.save();
    Configuration confChanged(confFileName, Settings("German"), false);
    CHECK_EQUAL(16, confChanged.getInt("BPP"));
    CHECK_EQUAL(false, confChanged.getBool("ComplexQuads"));
    CHECK_EQUAL("master.fts.arkana.org", confChanged.get("MasterServerName"));
    
    // Restore orgiginal vvalues.
    confChanged.set("BPP", 32);
    confChanged.set("ComplexQuads", true);
    confChanged.set("MasterServerName", "arkana-fts.servegame.org");
    confChanged.save();
}

