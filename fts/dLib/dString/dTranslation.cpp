/**
 * \file dTranslation.cpp
 * \author Klaus Beyer
 * \date 08.04.2012
 * \brief This file implements string translation class.
 **/


#include "dString.h"
#include "dLib/dConf/configuration.h"
#include "dTranslation.h"
#include "logging/logger.h"

using namespace FTS;

Translation::Translation(const String& in_sFile)
    : m_bLogging(true)
{
    Configuration conf("conf.xml", ArkanaDefaultSettings());
    Path sFile = Path::datadir("Languages") + Path(conf.get("Language")) + Path(in_sFile + ".xml");
    Options optTranslation = Configuration::buildFromFile(sFile);
    m_confTranslation = new Configuration(optTranslation);
    sFile = Path::datadir("Languages/English") + Path(in_sFile + ".xml");;
    optTranslation = Configuration::buildFromFile(sFile);
    m_confTranslationDefault = new Configuration(optTranslation);

}
Translation::~Translation() 
{
    delete m_confTranslation;
    delete m_confTranslationDefault;
}

String Translation::get(String in_sString)
{
    String ret = m_confTranslation->get(in_sString);
    // If there was an error, first try in english. If that doesn't work, load default error string.
    if(ret.empty()) {
        ret = m_confTranslationDefault->get(in_sString);
        // Print a warning to the logfile.
        if(ret.empty() && m_bLogging) {
            FTSMSG(String("Missing translation for ") + in_sString, MsgType::WarningNoMB);
        }
    }
    return ret;
}

