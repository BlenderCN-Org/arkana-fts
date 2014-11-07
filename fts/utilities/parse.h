/**
* \file parse.h
* \author Pompei2
* \date 01 May 2006
* \brief This file contains TEXT FILE PARSING.
* \remark Moved in from the utilities header file.
**/

#ifndef FTS_PARSE_H
#define FTS_PARSE_H

#include "main.h"
#include "dLib/dString/dString.h"

namespace FTS {

    class File;

    class CParser
    {
    private:
        FTS::String m_sFile;       ///< The name of the file we are parsing.
        char *m_pData;              ///< The whole file is stored here.
        char *m_p;                  ///< Points at the point in pData we're at.
        size_t m_nLine;             ///< The number of the current line.
        size_t m_nPos;              ///< The current position of the 'cursor'.
        bool m_bEOF;                ///< We came to the end of the file.

    public:
        CParser( void );
        CParser( const FTS::String & in_sFileName );
        virtual ~CParser( void );

        int load( const FTS::String & in_sFileName = FTS::String::EMPTY );
        int load( const FTS::File &in_f );
        int loadStr( const FTS::String & in_sString );
        int unload( void );

        size_t getPos( void ) const;
        size_t getLine( void ) const;
        FTS::String getFile( void ) const;
        bool isEOF( void ) const;

        const char *getData( void );
        const char *getCursor( void );

        size_t skipLine( void );
        size_t skipSpaces( void );
        size_t skipComments( void );

        int jumpToLineBeginning( const FTS::String & in_sBegin );
        int jumpToLineBeginningLabel( const FTS::String & in_sBegin );
        int parse( FTS::String in_sFmt, ... );
    };

}
#endif
