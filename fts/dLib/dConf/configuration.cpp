/*
 * \file Configuration.cpp
 * \author Klaus Beyer
 * \brief Implementation file for the configuration
 */
//#include "main.h"
#include "dLib/dString/dString.h"
#include "dLib/dString/dPath.h"
#include "logging/logger.h"
#include "stdio.h"
#include "stdlib.h"
#include "configuration.h"

namespace FTS
{

Configuration::Configuration ( const Options& in_options )
{
    m_opts = in_options;
    m_defaults = in_options;
}

Configuration::Configuration ( const FTS::DefaultOptions& in_defaults )
{
    m_opts = in_defaults.getDefaults();
    m_defaults = m_opts;
}

Configuration::Configuration ( File& in_file, const FTS::DefaultOptions& in_defaults )
{
    m_opts = in_defaults.getDefaults() ;
    m_defaults = m_opts;
    CEGUITinyXML::TiXmlDocument docConf;
    docConf.Parse((char*)in_file.getDataContainer().getData());
    parse ( docConf );
}

Configuration::Configuration ( const String& in_fileName, const DefaultOptions& in_defaults , bool in_useUserPath /*= true*/ )
{
    if ( in_useUserPath )
    {
        m_confFileName = Path::getUserConfPath() + Path ( in_fileName );
    }
    else
    {
        m_confFileName = Path::userdir(in_fileName);
    }
    m_opts = in_defaults.getDefaults() ;
    m_defaults = m_opts;
    CEGUITinyXML::TiXmlDocument docConf ( m_confFileName.c_str() );
    if ( docConf.LoadFile() )
    {
        parse ( docConf );
    }
}

void Configuration::parse ( CEGUITinyXML::TiXmlDocument& doc )
{
    CEGUITinyXML::TiXmlHandle h ( &doc );

    CEGUITinyXML::TiXmlHandle  rootHandle = h.FirstChild() ;
    CEGUITinyXML::TiXmlNode * node = rootHandle.Node();
    while ( node && node->Type() != CEGUITinyXML::TiXmlNode::ELEMENT )
    {
        node = node->NextSibling();
    }
    assert ( node );
    CEGUITinyXML::TiXmlElement * root = node->ToElement() ;
    assert ( root );

    for ( auto i = m_opts.begin() ; i != m_opts.end() ; ++i )
    {
        CEGUITinyXML::TiXmlElement * elem= root->FirstChildElement ( i->first.c_str() );
        if ( elem == NULL )
            continue; // the option doesn't exist in the file therefor keep the default.
        const char * value = elem->Attribute ( "value" );
        if ( value != NULL )
        {
            i->second = value;
        }
    }
}

String Configuration::get ( String in_optName )
{
    if(m_opts.find(in_optName) != m_opts.end()){
        return m_opts[in_optName];
    }
    return "";
}

bool Configuration::getBool ( String in_optName )
{
    try {
        bool ret = m_defaults[in_optName].toExactly<bool>();
        return m_opts[in_optName].to<bool>(ret);
    } catch (std::bad_cast ex) {
        throw CorruptDataException(in_optName,"Bad cast of bool option in Configuration::getBool()", MsgType::Horror);
    }
}

int Configuration::getInt ( String in_optName )
{
    try {
        int ret = m_defaults[in_optName].toExactly<int>();
        return m_opts[in_optName].to<int>(ret);
    } catch (std::bad_cast ex) {
        throw CorruptDataException(in_optName,"Bad cast of bool option in Configuration::getInt()", MsgType::Horror);
    }
}

float Configuration::getFloat ( String in_optName )
{
    try {
        float ret = m_defaults[in_optName].toExactly<float>();
        return m_opts[in_optName].to<float>(ret);
    } catch (std::bad_cast ex) {
        throw CorruptDataException(in_optName,"Bad cast of bool option in Configuration::getFloat()", MsgType::Horror);
    }
}

void Configuration::set ( String in_optName, String value )
{
    m_opts[in_optName] = value;
}

void Configuration::set ( String in_optName, const char * value )
{
    set ( in_optName, String ( value ) );
}

void Configuration::set ( String in_optName, int value )
{
    m_opts[in_optName] = String::nr ( value );
}

void Configuration::set ( String in_optName, bool value )
{
    m_opts[in_optName] = String::b ( value );
}

void Configuration::set ( String in_optName, float value )
{
    m_opts[in_optName] = String::nr ( value );
}

void Configuration::save()
{
    if ( m_confFileName.empty() )
    {
        return;
    }
    CEGUITinyXML::TiXmlDocument doc ( m_confFileName.c_str() );
    CEGUITinyXML::TiXmlHandle h ( &doc );
    CEGUITinyXML::TiXmlDeclaration * declaration = new CEGUITinyXML::TiXmlDeclaration ( "1.0", "UTF-8","" );
    doc.LinkEndChild ( declaration );
    CEGUITinyXML::TiXmlElement * root = new CEGUITinyXML::TiXmlElement ( m_confFileName.basename().withoutExt().c_str() );

    for ( auto i = m_opts.begin() ; i != m_opts.end() ; ++i )
    {
        CEGUITinyXML::TiXmlElement * element = new CEGUITinyXML::TiXmlElement ( i->first.c_str() );
        element->SetAttribute ( "value", i->second.c_str() );
        CEGUITinyXML::TiXmlNode* node = root->LinkEndChild ( element );
    }

    CEGUITinyXML::TiXmlNode* node = doc.LinkEndChild ( root );

    FTS::FileUtils::mkdirIfNeeded ( m_confFileName, true );
    if ( !doc.SaveFile() )
    {
        FTSMSGDBG ( doc.ErrorDesc(),1 );
    }
}

Options Configuration::buildFromFile ( const Path& in_fileName )
{
    Options opt;
    CEGUITinyXML::TiXmlDocument doc ( in_fileName.c_str() );
    if ( doc.LoadFile() )
    {
        CEGUITinyXML::TiXmlHandle h ( &doc );
        CEGUITinyXML::TiXmlHandle  rootHandle = h.FirstChild() ;
        CEGUITinyXML::TiXmlNode * node = rootHandle.Node();
        while ( node && node->Type() != CEGUITinyXML::TiXmlNode::ELEMENT )
        {
            node = node->NextSibling();
        }
        assert ( node );
        CEGUITinyXML::TiXmlElement * root = node->ToElement() ;
        assert ( root );
        CEGUITinyXML::TiXmlNode * child = 0;
        while ( ( child = root->IterateChildren ( child ) ) != NULL )
        {
            String key = child->Value();
            String value = child->ToElement()->Attribute ( "value" );
            opt[key] = value;
        }
    }
    return opt;
}
} // namespace FTS
