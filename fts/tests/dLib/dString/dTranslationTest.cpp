#include "dLib/aTest/TestHarness.h"

#include <ciso646>

#include "dLib/dString/dString.h"
#include "dLib/dConf/configuration.h"
#include "dLib/dString/dTranslation.h"
#include "logging/ftslogger.h"

using namespace FTS;
using namespace std;

SUITE(dTranslation)

TEST_INSUITE(dTranslation, UI_de)
{
    Configuration conf("conf.xml", ArkanaDefaultSettings());
    String org_lang = conf.get<string>("Language");
    conf.set("Language", "German");
    conf.save();
    Translation trans("ui");
    CHECK_EQUAL("Fehler !", trans.get("Msg_Err"));
    CHECK_EQUAL("Warnung", trans.get("Msg_Warn"));
    conf.set("Language", org_lang);
    conf.save();
}

TEST_INSUITE(dTranslation, UI_en)
{
    Configuration conf("conf.xml", ArkanaDefaultSettings());
    String org_lang = conf.get<string>("Language");
    conf.set("Language", "English");
    conf.save();
    Translation trans("ui");
    CHECK_EQUAL("Error !", trans.get("Msg_Err"));
    CHECK_EQUAL("Warning", trans.get("Msg_Warn"));
    conf.set("Language", org_lang);
    conf.save();
}

TEST_INSUITE(dTranslation, message_de)
{
    Configuration conf("conf.xml", ArkanaDefaultSettings());
    String org_lang = conf.get<string>("Language");
    conf.set("Language", "German");
    conf.save();
    Translation trans("messages");

    CHECK_EQUAL("Betreten des Runlevels:", trans.get("EnterRlv"));
    CHECK_EQUAL("Verlassen des Runlevels:", trans.get("LeaveRlv"));
    conf.set("Language", org_lang);
    conf.save();
}

TEST_INSUITE(dTranslation, message_en)
{
    Configuration conf("conf.xml", ArkanaDefaultSettings());
    String org_lang = conf.get<string>("Language");
    conf.set("Language", "English");
    conf.save();
    Translation trans("messages");

    CHECK_EQUAL("Entering Runlevel:", trans.get("EnterRlv"));
    CHECK_EQUAL("Leaving  Runlevel:", trans.get("LeaveRlv"));
    conf.set("Language", org_lang);
    conf.save();
}

TEST_INSUITE(dTranslation, message_de_missing)
{
    Configuration conf("conf.xml", ArkanaDefaultSettings());
    String org_lang = conf.get<string>("Language");
    conf.set("Language", "German");
    conf.save();
    Translation trans("messages");

    CHECK_EQUAL("This is for testing.", trans.get("UnitTest"));
    conf.set("Language", org_lang);
    conf.save();
}

TEST_INSUITE(dTranslation, message_unknown)
{
    Configuration conf("conf.xml", ArkanaDefaultSettings());
    Translation trans("messages");
    trans.setNoLogging();
    
    CHECK_EQUAL("", trans.get("UnitTestUnknown"));
}

