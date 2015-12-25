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
#include <XMLParserModules/TinyXMLParser/ceguitinyxml/tinyxml.h>

namespace FTS
{
EnhancedXMLDocument::EnhancedXMLDocument ( File& filename )
{
    std::size_t length = filename.getSize();
    // If we have a file, assume it is all one big XML file, and read it in.
    // The document parser may decide the document ends sooner than the entire file, however.
    CEGUITinyXML::TIXML_STRING data;
    data.reserve ( length );

    // Subtle bug here. TinyXml did use fgets. But from the XML spec:
    // 2.11 End-of-Line Handling
    // <snip>
    // <quote>
    // ...the XML processor MUST behave as if it normalized all line breaks in external
    // parsed entities (including the document entity) on input, before parsing, by translating
    // both the two-character sequence #xD #xA and any #xD that is not followed by #xA to
    // a single #xA character.
    // </quote>
    //
    // It is not clear fgets does that, and certainly isn't clear it works cross platform.
    // Generally, you expect fgets to translate from the convention of the OS to the c/unix
    // convention, and not work generally.

    char* buf = new char[ length+1 ];
    filename.read ( buf, length, sizeof ( char ) );

    const char* lastPos = buf;
    const char* p = buf;

    buf[length] = 0;
    while ( *p )
    {
        assert ( p < ( buf+length ) );
        if ( *p == 0xa )
        {
            // Newline character. No special rules for this. Append all the characters
            // since the last string, and include the newline.
            data.append ( lastPos, ( p-lastPos+1 ) );   // append, include the newline
            ++p;                                    // move past the newline
            lastPos = p;                            // and point to the new buffer (may be 0)
            assert ( p <= ( buf+length ) );
        }
        else if ( *p == 0xd )
        {
            // Carriage return. Append what we have so far, then
            // handle moving forward in the buffer.
            if ( ( p-lastPos ) > 0 )
            {
                data.append ( lastPos, p-lastPos ); // do not add the CR
            }
            data += ( char ) 0xa;                       // a proper newline

            if ( * ( p+1 ) == 0xa )
            {
                // Carriage return - new line sequence
                p += 2;
                lastPos = p;
                assert ( p <= ( buf+length ) );
            }
            else
            {
                // it was followed by something else...that is presumably characters again.
                ++p;
                lastPos = p;
                assert ( p <= ( buf+length ) );
            }
        }
        else
        {
            ++p;
        }
    }
    // Handle any left over characters.
    if ( p-lastPos )
    {
        data.append ( lastPos, p-lastPos );
    }
    delete [] buf;
    buf = 0;

    Parse ( data.c_str(), 0, CEGUITinyXML::TIXML_ENCODING_UNKNOWN );
}

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
    EnhancedXMLDocument docConf ( in_file );
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
