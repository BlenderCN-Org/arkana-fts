/**
 * \file string_file.cpp
 * \author Pompei2
 * \date unknown (very old)
 * \brief This file implements util functions for files.
 **/

#include "utilities/utilities.h"
#include "logging/logger.h"
#include "dLib/dConf/configuration.h"

using namespace FTS;

/// Returns a string in the current language.
/** This function goes read a string from the file LANGS/[current lang]/[in_sFile]
 *  that means it reads a string in the current user language.
 *
 * \param in_sString The ID of the string to get.
 * \param in_sFile   The file to get the string from.
 * \param out_bFound The address of a boolean to store if the string was
 *                   found or not. Ignored if it is NULL.
 *
 * \return If successfull: The translated string.
 * \return If failed:      An empty string.
 *
 * Example: \n
 *   Assuming the file LANGS/English/blub.conf contains the following line:
 *   MyMessage = "Hello, World !"
 *   And the current language is "English":
 *   \code
 *       String str = getTranslatedString( "MyMessage", "blub.conf" );
 *       // str is now "Hello, World !" \endcode
 *
 * \author Pompei2
 */
String FTS::getTranslatedString(const String & in_sString,
                                const String & in_sFile,
                                bool *out_bFound)
{
    String ret;

    if(out_bFound)
        *out_bFound = true;

    if(!in_sString || !in_sFile) {
        FTS18N("InvParam", MsgType::Horror, "getTranslatedString");
        return ret;
    }

    Configuration conf ("conf.xml", ArkanaDefaultSettings());

    Path sFile = Path::datadir("Languages") + Path(conf.get("Language")) + Path(in_sFile + ".xml");
    Options optTranslation = Configuration::buildFromFile(sFile);
    Configuration confTranslation(optTranslation);
    ret = confTranslation.get(in_sString);

    // If there was an error, first try in english. If that doesn't work, load default error string.
    if(ret.empty()) {
        sFile = Path::datadir("Languages/English") + Path(in_sFile + ".xml");;
        Options optTranslation = Configuration::buildFromFile(sFile);
        Configuration conf(optTranslation);
        ret = conf.get(in_sString);
        // Print a warning to the logfile.
        if(ret.empty()) {
            FTSMSG(String("Missing translation for ") + in_sString, MsgType::WarningNoMB);
        }
    }

    return ret;
}
