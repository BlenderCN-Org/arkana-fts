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
    Path sFile = Path::datadir("Languages") + Path(conf.get<std::string>("Language")) + Path(in_sFile + ".xml");
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
    try {
        return m_confTranslation->get<std::string>(in_sString);
    } catch(CorruptDataException& ) {
        try {
            // If there was an error, first try in English. If that doesn't work, load default error string.
            return m_confTranslationDefault->get<std::string>(in_sString);
        } catch(CorruptDataException&) {
            // Print a warning to the logfile.
            if(m_bLogging) {
                FTSMSG(String("Missing translation for ") + in_sString, MsgType::WarningNoMB);
            }
        }
    }
    return String::EMPTY;
}

