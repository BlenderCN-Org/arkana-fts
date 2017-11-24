/**
 * \file parse.cpp
 * \author Pompei2
 * \date unknown (very old)
 * \brief This file implements functions to parse files.
 **/

#include <stdarg.h>
#include "parse.h"
#include "logging/logger.h"
#include "DateTime.h"

#include "dLib/dFile/dFile.h"

using namespace FTS;

CParser::CParser(void)
{
    m_sFile = String::EMPTY;
    m_pData = NULL;
    m_p = NULL;
    m_nLine = 1;
    m_nPos = 0;
    m_bEOF = true;
}

CParser::CParser(const String & in_sFileName)
{
    m_sFile = in_sFileName;
    m_pData = NULL;
    m_p = NULL;
    m_nLine = 1;
    m_nPos = 0;
    m_bEOF = true;
}

CParser::~CParser(void)
{
    this->unload();
}

/// Open a file for parsing.
/** This function opens a file and sets up a ParseData structure for parsing.
 *
 * \param in_sFileName The name of the file to open for parsing, if none specified,
 *                     the one chosen in the constructor will be taken.
 *
 * \return If successful: ERR_OK.
 * \return If failed:      Error code < 0
 *
 * \note This completely loads the file into memory.
 *
 * \author Pompei2
 */
int CParser::load(const String & in_sFileName)
{
    if(!in_sFileName.empty())
        m_sFile = in_sFileName;

    try {
        File::Ptr f = File::open(m_sFile, File::Read);
        return this->load(*f);
    } catch(...) {
        return -1;
    }
}

/// Load a string for parsing.
/** This function prepares the parser class to parse a string. It copies the
 *  string into memory, so you can change the string after a call to this
 *  function, this will have no effect.
 *
 * \param in_sString The string to parse.
 *
 * \return If successful: ERR_OK.
 * \return If failed:      Error code < 0
 *
 * \author Pompei2
 */
int CParser::loadStr(const String & in_sString)
{
    m_sFile = "From Memory";

    if(NULL == (m_pData = (char *)malloc(in_sString.len()+1)))
        return -3;

    strncpy(m_pData, in_sString.c_str(), in_sString.len());
    m_pData[in_sString.len()] = '\0';

    m_p = m_pData;
    m_bEOF = false;

    return ERR_OK;
}

int CParser::load(const FTS::File &in_f)
{
    if(NULL == (m_pData = (char *)malloc(sizeof(char) * in_f.getDataContainer().getSize() + sizeof(char))))
        return -3;

    memcpy(m_pData, in_f.getDataContainer().getData(), in_f.getDataContainer().getSize());
    m_pData[in_f.getDataContainer().getSize()] = '\0';

    m_p = m_pData;
    m_bEOF = false;

    return ERR_OK;
}

/// Close a parse class.
/** This function frees the memory allocated by the load function.
 *
 * \return If successful: ERR_OK.
 * \return If failed:      Error code < 0
 *
 * \author Pompei2
 */
int CParser::unload(void)
{
    SAFE_FREE(m_pData);

    m_sFile = String::EMPTY;

    m_p = NULL;
    m_nLine = 1;
    m_nPos = 0;
    m_bEOF = true;

    return ERR_OK;
}

/// Return the current position.
/** This function returns the current position of the cursor, in characters.
 *
 * \return the current position of the cursor
 *
 * \author Pompei2
 */
size_t CParser::getPos(void) const
{
    return m_nPos;
}

/// Return the current line number.
/** This function returns the current line number, that is 1-based.
 *
 * \return the current line number
 *
 * \author Pompei2
 */
size_t CParser::getLine(void) const
{
    return m_nLine;
}

/// Return the name of the file being parsed.
/** This function returns the name of the file being parsed.
 *
 * \return the name of the file being parsed.
 *
 * \author Pompei2
 */
String CParser::getFile(void) const
{
    return m_sFile;
}

/// Check if i am EOF.
/** This function returns true if the end of file was reached or an error occurred.
 *
 * \return true if the end of file was reached or an error occurred, false else.
 *
 * \author Pompei2
 */
bool CParser::isEOF(void) const
{
    return m_bEOF;
}

/// Return the data to parse..
/** This function returns the data you are parsing  (usually the file content).
 *
 * \return A pointer to the data you are parsing.
 *
 * \author Pompei2
 */
const char *CParser::getData(void)
{
    return m_pData;
}

/// Return a pointer into the data at the cursor's current position.
/** This function returns a pointer into the data at the cursor's current position.
 *  You get the same as if you did parser->getData( ) + parser->getPos( );
 *
 * \return Pointer into the current position in the data.
 *
 * \author Pompei2
 */
const char *CParser::getCursor(void)
{
    return m_p;
}

/// Skips the current line.
/** This function skips the current line you are parsing, and puts the cursor at the
 *  beginning of the next line (after the linebreak, in front of the first character).
 *
 * \return The number of characters that were skipped.
 *
 * \author Pompei2
 */
size_t CParser::skipLine(void)
{
    size_t nRead = 0;

    if(m_bEOF)
        return (unsigned int)-1;

    while(m_p[0] != '\n' && m_p[0] != '\0') {
        m_p++;
        nRead++;
    }

    if(m_p[0] == '\0') {
        m_bEOF = true;
        return nRead;
    } else {
        m_p++;
        m_nLine++;
        nRead++;
    }

    m_nPos += nRead;
    return nRead;
}

/// Skip all possible spaces at the current position.
/** This function skips all kind of spacing characters at the current position
 *  and puts the cursor in front of the next nonspace character.
 *
 * \return The number of characters that were skipped.
 *
 * \note The possible spacing characters are the following (like in isspace):
 *        - space   ' '
 *        - tab     '\\t'
 *        - newline '\\n'
 *
 * \author Pompei2
 */
size_t CParser::skipSpaces(void)
{
    size_t nRead = 0;

    if(m_bEOF)
        return (unsigned int)-1;

    while(isspace((unsigned char)m_p[0]) && m_p[0] != '\0') {
        if(m_p[0] == '\n')
            m_nLine++;
        m_p++;
        nRead++;
    }

    if(m_p[0] == '\0')
        m_bEOF = true;

    m_nPos += nRead;
    return nRead;
}

/// Skips all possible spaces and comments at the current position.
/** This function skips all comments and spaces at the current position
 *  and puts the cursor at front of the next nonspace/comment character.
 *
 * \return The number of characters that were skipped.
 *
 * \note Comments begin with a # and end at the end of line.\n
 *
 * \author Pompei2
 */
size_t CParser::skipComments(void)
{
    size_t nRead = 0;

    if(m_bEOF)
        return (unsigned int)-1;

    nRead += this->skipSpaces();

    while(m_p[0] == '#' && m_p[0] != '\0') {
        nRead += this->skipLine();
        nRead += this->skipSpaces();
    }

    if(m_p[0] == '\0')
        m_bEOF = true;

    return nRead;
}

/// Jumps the cursor to the first line that begins with a string.
/** This function searches for a line beginning with the given string
 *  and places the cursor at the front of this line, behind the '\n' sign
 *  of the previous line.
 *
 * \param in_sBegin The string you want the line to begin with.
 *
 * \return ERR_OK if such a line was found.
 *         -1 if there is no such line.
 *         An error code < -1 if there was an error.
 *
 * \note If there was a problem or there is no line beginning with that string,
 *       the cursor's position is not changed.
 *
 * \author Pompei2
 */
int CParser::jumpToLineBeginning(const String & in_sBegin)
{
    char *p = m_p;
    size_t nChars = 0;
    size_t nLines = 0;

    if(m_bEOF || !in_sBegin)
        return -2;

    while(p && *p != '\0') {
        // Compare the beginning of the line with the needle.
        if( !in_sBegin.neq(p) )
            goto skipline;

        // If we got a line beginning with it, we're done.
        m_p = p;
        m_nPos += nChars;
        m_nLine += nLines;
        return ERR_OK;

      skipline:
        // If we haven't got it, skip this line.
        while(*p != '\n' && *p != '\0') {
            p++;
            nChars++;
        }

        if(*p != '\0') {
            p++;
            nChars++;
            nLines++;
        }
    }

    return -1;
}

/// Jumps the cursor to the first line that begins with a label.
/** This function searches for a line beginning with the given label
 *  and places the cursor at the front of this line, behind the '\n' sign
 *  of the previous line.
 *
 * \param in_sBegin The label you want the line to begin with.
 *
 * \return ERR_OK if such a line was found.
 *         -1 if there is no such line.
 *         An error code < -1 if there was an error.
 *
 * \note If there was a problem or there is no line beginning with that string,
 *       the cursor's position is not changed.\n
 *       A label is a string that can contain alphanumeric characters and a '_'.
 *       I can't explain it that good :) take a look at this example:\n
 *
 *       If you want a line beginning with "FTS", lines beginning with "FTS_Blub"
 *       or "FTSxyz" are ignored, but lines like "FTS-bla" or "FTS bla" or even "FTS?Bla"
 *       are accepted.
 *
 * \author Pompei2
 */
int CParser::jumpToLineBeginningLabel(const String & in_sBegin)
{
    char *p = m_p;
    size_t nChars = 0;
    size_t nLines = 0;

    if(m_bEOF || !in_sBegin)
        return -2;

    while(p && *p != '\0') {
        size_t nNeedleLen = in_sBegin.len();

        // Compare the beginning of the line with the needle.
        size_t i;

        for(i = 0; i < nNeedleLen; i++) {
            if(p[i] == '\0' || p[i] == '\n'
               || p[i] != in_sBegin.getCharAt(i))
                goto skipline;
        }

        // Check if the next character contains a label-included char,
        // it's not the correct line.
        // For example, if you want a line beginning with "FTS",
        // this ignores lines beginning with "FTS_Blub" or "FTSxyz",
        // but accepts lines like "FTS-bla" or "FTS bla" or even "FTS?Bla"
        if(isalnum((unsigned char)p[i]) || p[i] == '_')
            goto skipline;

        // If we got a line beginning with it, we're done.
        m_p = p;
        m_nPos += nChars;
        m_nLine += nLines;
        return ERR_OK;

      skipline:
        // If we haven't got it, skip this line.
        while(*p != '\n' && *p != '\0') {
            p++;
            nChars++;
        }

        if(*p != '\0') {
            p++;
            nChars++;
            nLines++;
        }
    }

    return -1;
}

/// Parse a parsed file.
/** This function parses a textfile, in a similar way as scanf.
 *
 * \param in_sFmt a format string similar as for scanf.
 * \param ...     a list of values, dependent on the content of \a in_sFmt
 *
 * \return the number of CHARACTERS read. In case of error, it *should* return -1.
 *
 * \note If there was an error, the pointer is at the place the error occurred, but this
 *       class is in the same state as when it comes to the end of the file !\n
 *       Currently, the possible special characters are (as you see all preceded by a %):
 *         - %# is any number of newlines and comments, until the next
 *                char that is not a comment neither a newline.
 *         - %n Parses until the newline character '\n' and places the cursor behind it.
 *         - %  (%[space], '% ') is any number of spacing characters as recognized by the isspace
 *                function.
 *         - %% parses a simple % sign.
 *         - %d is a number
 *         - %f is a floating point
 *         - %b is a boolean string ("true" or "false", ignoring the case)
 *         - %s is a "-surrounded string, the space for the string is allocated by the function.
 *         - %S is a string, including everything until the next spacing character or the
 *                end of file. The space for the string is allocated by the function.
 *         - %p is a alphanumeric string, the space for the string is allocated by the function
 *                (p stands for 'printable').
 *         - %L is a label, that is the same as %p, but including the '_' and '/' sign.
 *         - %r is the rest of the line, not including the newline character.
 *         - %t is a date or a time or both, in the format: (yyyy-mm-dd hh:mm:ss:mmmm).
 *         - %Xd where X is a number from 2 to 9 is a ensemble of X integers like (1,12,3) when X is 3
 *         - %Xf where X is a number from 2 to 9 is a ensemble of X floats like (1.2,4,5.678) when X is 3
 *         - %^X where X is any character you want. This parses everything until (including)
 *               the character X or the end of the line.
 *         - %*X where X is any character you want. This parses everything until (including)
 *               the character X.
 *
 * Example: \n
 *       test.txt:\code
 *        # This is a test file.
 *        unit_set_name( "SELECTED_UNIT", "Killor" )
 *        player_set_color( 1, (255,128,0) )
 *       \endcode
 *       \code
 *       char *unit_id, *new_name, *property;
 *       SColor col;
 *       int player;
 *       CParser p( "test.txt" );
 *       if( ERR_OK != p.load( ) )
 *            return -1;
 *       if( p.parse( _S("%#unit_set_name% (% %s% ,% %s% )"), &unit_id, &new_name ) == -1 )
 *            return -2;
 *       p.skip_comments( );
 *       if( p.parse( _S("player_set_%S% (% %d% ,% %3d% )"), &property, &player, &col.r, &col.g, &col.b ) == -1 )
 *            return -3;
 *       p.unload( ); // Don't really need this here, but its nicer :) \endcode
 *
 * \author Pompei2
 */
int CParser::parse(String in_sFmt, ...)
{
    const char *f = in_sFmt.c_str();
    va_list ap;
    int nRead = 0;
    int i = 0, j = 0, k = 0, l = 0, n = 0;

    int *iVal = NULL;
    float *fVal = NULL;
    bool *bVal = NULL;
    char **sVal = NULL;
    char *pszTmp = NULL;
    DateTime *pDateVal = NULL;
    char cVal = 0;

    if(!in_sFmt) {
        FTS18N("InvParam", MsgType::Horror, "CParser::parse");
        return -1;
    }

    if(m_bEOF)
        return -1;

    va_start(ap, in_sFmt);

    while(f[0] && m_p[0] != '\0' && !m_bEOF) {
        if(f[0] == '%') {
            switch (f[1]) {
            case ' ':
                // Skip all spaces.
                f += 2;
                i = this->skipSpaces();
                nRead += i;
                break;
            case '#':
                // Skip newlines and comments.
                f += 2;
                i = this->skipComments();
                nRead += i;
                break;
            case 'n':
                // Skip newlines and comments.
                f += 2;
                i = this->skipLine();
                nRead += i;
                break;
            case 'd':
                f += 2;
                iVal = va_arg(ap, int *);

                if(EOF == sscanf(m_p, "%d%n", iVal, &i)) {
                    FTS18N("Parse_expectI", MsgType::Error, m_sFile, String::nr(m_nLine), String::chr(m_p[0]));
                    m_bEOF = true;
                    goto error_end;
                }
                m_p += i;
                nRead += i;
                m_nPos += i;
                break;
            case 'f':
                f += 2;
                fVal = va_arg(ap, float *);

                if(EOF == sscanf(m_p, "%f%n", fVal, &i)) {
                    FTS18N("Parse_expectF", MsgType::Error, m_sFile, String::nr(m_nLine), String::chr(m_p[0]));
                    m_bEOF = true;
                    goto error_end;
                }
                m_p += i;
                nRead += i;
                m_nPos += i;
                break;
            case 'b':
                f += 2;
                bVal = va_arg(ap, bool *);
                if(this->parse("%S", &pszTmp) == -1) {
                    FTS18N("Parse_expectB", MsgType::Error, m_sFile, String::nr(m_nLine), String::chr(m_p[0]));
                    m_bEOF = true;
                    goto error_end;
                }

                if(String("true").nieq(pszTmp)) {
                    *bVal = true;
                } else if(String("false").nieq(pszTmp)) {
                    *bVal = false;
                } else {
                    FTS18N("Parse_expectB", MsgType::Error, m_sFile, String::nr(m_nLine), pszTmp);
                    SAFE_FREE(pszTmp);
                    m_bEOF = true;
                    goto error_end;
                }

                SAFE_FREE(pszTmp);
                break;
            case 's':
                f += 2;
                sVal = va_arg(ap, char **);

                if(m_p[0] != '"') {
                    FTS18N("Parse_expect", MsgType::Error, m_sFile, String::nr(m_nLine), "\"", String::chr(m_p[0]), in_sFmt);
                    m_bEOF = true;
                    goto error_end;
                }
                m_p++;
                nRead++;
                m_nPos++;

                for(i = 0; m_p[i] != '\0'; i++) {
                    /* All this shit to allow escaping like \" */
                    if(m_p[i] == '"') {
                        if(i > 0) {
                            if(m_p[i - 1] != '\\')
                                break;
                        } else
                            break;
                    }
                }

                if(m_p[i] == '\0') {
                    FTS18N("Parse_eof", MsgType::Error, m_sFile, String::nr(m_nLine));
                    m_bEOF = true;
                    goto error_end;
                }

                *sVal = (char *)malloc(i + 1);
                (*sVal)[i] = '\0';
                strncpy(*sVal, m_p, i);
                m_p += i + 1;
                nRead += i + 1;
                m_nPos += i + 1;
                break;
            case 'S':
                f += 2;
                sVal = va_arg(ap, char **);

                for(i = 0;
                    !isspace((unsigned char)m_p[i]) && m_p[i] != '\0';
                    i++) ;

                if(m_p[i] == '\0') {
                    m_bEOF = true;
                }

                *sVal = (char *)malloc(i + 1);
                strncpy(*sVal, m_p, i);
                (*sVal)[i] = '\0';
                m_p += i;
                nRead += i;
                m_nPos += i;
                break;
            case 'L':
                f += 2;
                sVal = va_arg(ap, char **);

                for(i = 0;
                    (isalnum((unsigned char)m_p[i]) || m_p[i] == '_' || m_p[i] == '/')
                    && m_p[i] != '\0'; i++) ;

                if(m_p[i] == '\0') {
                    FTS18N("Parse_eof", MsgType::Error, m_sFile, String::nr(m_nLine));
                    m_bEOF = true;
                    goto error_end;
                }

                *sVal = (char *)malloc(i + 1);
                strncpy(*sVal, m_p, i);
                (*sVal)[i] = '\0';
                m_p += i;
                nRead += i;
                m_nPos += i;
                break;
            case 'r':
                f += 2;
                sVal = va_arg(ap, char **);

                for(i = 0; m_p[i] != '\n' && m_p[i] != '\0'; i++)
                    ;

                if(m_p[i] == '\0')
                    m_bEOF = true;

                *sVal = (char *)malloc(i + 1);
                strncpy(*sVal, m_p, i);
                (*sVal)[i] = '\0';
                m_p += i;
                nRead += i;
                m_nPos += i;
                break;
            case 'p':
                f += 2;
                sVal = va_arg(ap, char **);

                for(i = 0; isalnum(m_p[i]) && m_p[i] != '\0'; i++) ;

                if(m_p[i] == '\0') {
                    FTS18N("Parse_eof", MsgType::Error, m_sFile, String::nr(m_nLine));
                    m_bEOF = true;
                    goto error_end;
                }

                *sVal = (char *)malloc(i + 1);
                strncpy(*sVal, m_p, i);
                (*sVal)[i] = '\0';
                m_p += i;
                nRead += i;
                m_nPos += i;
                break;
            case 't':
                f += 2;
                pDateVal = va_arg(ap, DateTime *);

                // Get the first number (year or hours)
                if((j = this->parse("%d", &i)) == -1) {
                    FTS18N("Parse_expectDT", MsgType::Error, m_sFile, String::nr(m_nLine), String(m_p, 0, 10));
                    goto error_end;
                }

                // Check if it's a date or a time.
                if(m_p[0] == '-') {
                    pDateVal->setYear(i);

                    // Get the month.
                    if((j = this->parse("-%d", &i)) == -1) {
                        FTS18N("Parse_expectDT", MsgType::Error, m_sFile, String::nr(m_nLine), String(m_p, 0, 10));
                        goto error_end;
                    }
                    pDateVal->setMonth(i);

                    // Get the day.
                    if((j = this->parse("-%d", &i)) == -1) {
                        FTS18N("Parse_expectDT", MsgType::Error, m_sFile, String::nr(m_nLine), String(m_p, 0, 10));
                        goto error_end;
                    }
                    pDateVal->setDay(i);

                    // Maybe get the time.
                    if(m_p[0] == ' ' && m_p[1] && isdigit(m_p[1])) {
                        // Get the hours.
                        if((j = this->parse(" %d", &i)) == -1) {
                            FTS18N("Parse_expectDT", MsgType::Error, m_sFile, String::nr(m_nLine), String(m_p, 0, 10));
                            goto error_end;
                        }
                        pDateVal->setHour(i);

                        // Get the minutes.
                        if((j = this->parse(":%d", &i)) == -1) {
                            FTS18N("Parse_expectDT", MsgType::Error, m_sFile, String::nr(m_nLine), String(m_p, 0, 10));
                            goto error_end;
                        }
                        pDateVal->setMinute(i);

                        // Get the seconds.
                        if((j = this->parse(":%d", &i)) == -1) {
                            FTS18N("Parse_expectDT", MsgType::Error, m_sFile, String::nr(m_nLine), String(m_p, 0, 10));
                            goto error_end;
                        }
                        pDateVal->setSecond(i);

                        // Maybe get the milliseconds.
                        if(m_p[0] == ':') {
                            if((j = this->parse(":%d", &i)) == -1) {
                                FTS18N("Parse_expectDT", MsgType::Error, m_sFile, String::nr(m_nLine), String(m_p, 0, 10));
                                goto error_end;
                            }
                            pDateVal->setMs(i);
                        }
                    }

                } else if(m_p[0] == ':') {
                    pDateVal->setHour(i);

                    // Get the minutes.
                    if((j = this->parse(":%d", &i)) == -1) {
                        FTS18N("Parse_expectDT", MsgType::Error, m_sFile, String::nr(m_nLine), String(m_p, 0, 10));
                        goto error_end;
                    }
                    pDateVal->setMinute(i);

                    // Get the seconds.
                    if((j = this->parse(":%d", &i)) == -1) {
                        FTS18N("Parse_expectDT", MsgType::Error, m_sFile, String::nr(m_nLine), String(m_p, 0, 10));
                        goto error_end;
                    }
                    pDateVal->setSecond(i);

                    // Maybe get the milliseconds.
                    if(m_p[0] == ':') {
                        if((j = this->parse(":%d", &i)) == -1) {
                            FTS18N("Parse_expectDT", MsgType::Error, m_sFile, String::nr(m_nLine), String(m_p, 0, 10));
                            goto error_end;
                        }
                        pDateVal->setMs(i);
                    }

                } else {
                    FTS18N("Parse_expectDT", MsgType::Error, m_sFile, String::nr(m_nLine), String(m_p, 0, 10));
                    m_bEOF = true;
                    goto error_end;
                }
                break;
            case '2':
                n = 2;
                goto n_set;
            case '3':
                n = 3;
                goto n_set;
            case '4':
                n = 4;
                goto n_set;
            case '5':
                n = 5;
                goto n_set;
            case '6':
                n = 6;
                goto n_set;
            case '7':
                n = 7;
                goto n_set;
            case '8':
                n = 8;
                goto n_set;
            case '9':
                n = 9;
n_set:
                switch (f[2]) {
                case 'd':
                    f += 3;
                    if((j = this->parse("(% ")) == -1) {
                        FTS18N("Parse_expectXI", MsgType::Error, m_sFile, String::nr(m_nLine), String::nr(n), String(m_p, 0, 5));
                        goto error_end;
                    }
                    for(i = 0; i < n - 1; i++) {
                        iVal = va_arg(ap, int *);

                        if((j = this->parse("%d% ,% ", iVal)) < 2) {
                            FTS18N("Parse_expectXI", MsgType::Error, m_sFile, String::nr(m_nLine), String::nr(n), String(m_p, 0, 5));
                            goto error_end;
                        }
                    }
                    iVal = va_arg(ap, int *);

                    if((j = this->parse("%d% )", iVal)) < 1) {
                        FTS18N("Parse_expectXI", MsgType::Error, m_sFile, String::nr(m_nLine), String::nr(n), String(m_p, 0, 5));
                        goto error_end;
                    }
                    break;
                case 'f':
                    f += 3;
                    if((j = this->parse("(% ")) < 1) {
                        FTS18N("Parse_expectXF", MsgType::Error, m_sFile, String::nr(m_nLine), String::nr(n), String(m_p, 0, 5));
                        goto error_end;
                    }
                    for(i = 0; i < n - 1; i++) {
                        fVal = va_arg(ap, float *);

                        if((j = this->parse("%f% ,% ", fVal)) < 2) {
                            FTS18N("Parse_expectXF", MsgType::Error, m_sFile, String::nr(m_nLine), String::nr(n), String(m_p, 0, 5));
                            goto error_end;
                        }
                    }
                    fVal = va_arg(ap, float *);

                    if((j = this->parse("%f% )", fVal)) < 1) {
                        FTS18N("Parse_expectXF", MsgType::Error, m_sFile, String::nr(m_nLine), String::nr(n), String(m_p, 0, 5));
                        goto error_end;
                    }
                    break;
                }
                break;
            case '^':
                cVal = f[2];
                f += 3;

                sVal = va_arg(ap, char **);

                j = 0;
                for(i = 0; m_p[i] != '\0' && m_p[i] != '\n' ; i++) {
                    /* All this shit to allow escaping like \cVal */
                    if(m_p[i] == cVal) {
                        if(i > 0) {
                            if(m_p[i - 1] == '\\')
                                j++;
                            else
                                break;
                        } else
                            break;
                    }
                }

                if(m_p[i] == '\0') {
                    FTS18N("Parse_eof", MsgType::Error, m_sFile, String::nr(m_nLine));
                    m_bEOF = true;
                    goto error_end;
                }

                *sVal = (char *)malloc(i + 1 - j);
                (*sVal)[i-j] = '\0';

                // Now replace escaped signs (\cVal) by the sign itself.
                for(k = 0, l = 0 ; k < i ; k++, l++) {
                    if(m_p[k] == '\\' && m_p[k+1] == cVal) {
                        (*sVal)[l] = cVal;
                        k++;
                    } else {
                        (*sVal)[l] = m_p[k];
                    }
                }

                if(m_p[i] == '\n')
                    m_nLine++;
                m_p += i + 1;
                nRead += i + 1;
                m_nPos += i + 1;
                break;
            case '*':
                cVal = f[2];
                f += 3;

                sVal = va_arg(ap, char **);

                j = 0;
                for(i = 0; m_p[i] != '\0' ; i++) {
                    /* All this shit to allow escaping like \cVal */
                    if(m_p[i] == cVal) {
                        if(i > 0) {
                            if(m_p[i - 1] == '\\')
                                j++;
                            else
                                break;
                        } else
                            break;
                    } else if(m_p[i] == '\n') {
                        m_nLine++;
                    }
                }

                if(m_p[i] == '\0') {
                    FTS18N("Parse_eof", MsgType::Error, m_sFile, String::nr(m_nLine));
                    m_bEOF = true;
                    goto error_end;
                }

                *sVal = (char *)malloc(i + 1 - j);
                (*sVal)[i-j] = '\0';

                // Now replace escaped signs (\cVal) by the sign itself.
                for(k = 0, l = 0 ; k < i ; k++, l++) {
                    if(m_p[k] == '\\' && m_p[k+1] == cVal) {
                        (*sVal)[l] = cVal;
                        k++;
                    } else {
                        (*sVal)[l] = m_p[k];
                    }
                }

                m_p += i + 1;
                nRead += i + 1;
                m_nPos += i + 1;
                break;
            case '%':
                if(m_p[0] != '%') {
                    FTS18N("Parse_expect", MsgType::Error, m_sFile, String::nr(m_nLine), "%", String::chr(m_p[0]), in_sFmt);
                    m_bEOF = true;
                    goto error_end;
                }

                m_p++;
                f += 2;
                nRead++;
                m_nPos++;
                break;
            default:
                f += 2;
                FTSMSG("Programmer's parse error, did you forget to use %% instead of % ?\n parse string: '{1}'", MsgType::Horror, in_sFmt);
                break;
            }
        } else { /* m_p[0] == '%' */
            if(m_p[0] != f[0]) {
                FTS18N("Parse_expect", MsgType::Error, m_sFile, String::nr(m_nLine), String::chr(f[0]), String::chr(m_p[0]), in_sFmt);
                m_bEOF = true;
                goto error_end;
            }

            if(m_p[0] == '\0' && f[0] != '\0') {
                FTS18N("Parse_eof", MsgType::Error, m_sFile, String::nr(m_nLine));
                m_bEOF = true;
                goto error_end;
            }

            if(m_p[0] == '\n')
                m_nLine++;
            m_p++;
            f++;
            nRead++;
            m_nPos++;
        }
    }

    va_end(ap);
    return nRead;

  error_end:
    va_end(ap);
    return -1;
}
