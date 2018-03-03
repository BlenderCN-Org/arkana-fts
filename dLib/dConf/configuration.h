/*
 * \file Configuration.h
 * \author Klaus Beyer
 * \brief Interface file for the configuration
 */
#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H
#include <sstream>
#include "dLib/dString/dPath.h"
#include "dLib/dConf/DefaultOptions.h"
#include "dLib/dFile/dFile.h"
#include <ceguitinyxml/tinyxml.h>
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

            template<class T, typename std::enable_if < (std::is_arithmetic<T>{} && !std::is_same<T, bool>{}), int > ::type = 0 >
            T get(String in_optName)
            {
                T ret;
                std::istringstream i(get< std::string > (in_optName));
                char c = 0;
                if(!(i >> ret) || i.get(c))
                    throw CorruptDataException(in_optName, "Bad cast of option in Configuration::get<>()", MsgType::Horror);
                return ret;
            }
            template<class T, typename std::enable_if < std::is_same<T, bool>{}, int > ::type = 0 >
            T get(String in_optName)
            {
                auto v = get<std::string>(in_optName);
                if(v == "True") {
                    return true;
                }
                if(v == "False") {
                    return false;
                }
                throw CorruptDataException(in_optName, "Bad cast of option in Configuration::get<>()", MsgType::Horror);
            }
            template<class T, typename std::enable_if < std::is_same<T, std::string>{}, int > ::type = 0 >
            T get(String in_optName)
            {
                if(m_opts.count(in_optName) != 0) {
                    return m_opts[in_optName].str();
                }
                throw CorruptDataException(in_optName, "Unknown option name in Configuration::get<>()", MsgType::Horror);
            }
            
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
}

#endif
