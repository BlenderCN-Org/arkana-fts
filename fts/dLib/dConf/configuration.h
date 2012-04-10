/*
 * \file Configuration.h
 * \author Klaus Beyer
 * \brief Interface file for the configuration
 */
#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H
#include "dLib/dString/dPath.h"
#include "dLib/dConf/DefaultOptions.h"
#include "dLib/dFile/dFile.h"
#include <XMLParserModules/TinyXMLParser/ceguitinyxml/tinyxml.h>
#include "dLib/dConf/ArkanaDefaultSettings.h"

namespace FTS {
    class Configuration 
    {
        public:
            static Options buildFromFile(const Path& in_fileName);
            Configuration(File& in_file, const DefaultOptions& in_defaults );
            Configuration(const String& in_fileName, const DefaultOptions& in_defaults, bool in_useUserPath = true );
            Configuration(const Options& in_options); 
            Configuration ( const FTS::DefaultOptions& in_defaults );
            void save();
            String get(String in_optName);
            bool getBool(String in_optName);
            int getInt(String in_optName);
            float getFloat(String in_optName);
            void set(String in_optName, const char * value);
            void set(String in_optName, String value);
            void set(String in_optName, int value);
            void set(String in_optName, float value);
            void set(String in_optName, bool value);
        private:
            void parse(CEGUITinyXML::TiXmlDocument& doc);
            Path m_confFileName;
            Options m_opts;
            Options m_defaults;
    };
    
    class EnhancedXMLDocument : public CEGUITinyXML::TiXmlDocument 
    {
    public:
        EnhancedXMLDocument( File& filename );
    };

};

#endif
