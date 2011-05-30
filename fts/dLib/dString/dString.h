#ifndef D_STRING_H
#define D_STRING_H

/* Why do my own ? because I like reinvent the wheel ... maybe i'll write my own OpenGL hehe ... no. */

#include "main.h"
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>

#include "main/defines.h"

#ifdef D_STRING_STD_ONLY
#  ifdef D_USE_CEGUI
#    undef D_USE_CEGUI
#  endif
#  ifdef D_STRING_MYSQL
#    undef D_STRING_MYSQL
#  endif
#endif

#ifdef D_USE_CEGUI
#  include <CEGUIString.h>
#endif

#ifdef D_STRING_MYSQL
#  include <mysql/mysql.h>
#endif

#include "main.h"

namespace FTS {

class String {
private:
    std::string m_s;

    template <typename T>
    static String fromSpecialNumber(T number);
    void fromStringWithLength(const char* in_pszString, std::string::size_type in_iStart, std::string::size_type in_iLength);
    int getByteCount(const char* in_pszString, std::string::size_type in_utf8Len);
    const char* advance(const char* in_pszString, std::string::size_type in_utf8Len);
    typedef std::tuple<std::string::size_type, int> StringSize;
    StringSize strlen() const;
public:
    static const String sWhiteSpace;

    // A whole bunch of constructors.
    String();

    // We need those three for optimization.
    String(const char* in_pszString);
    String(const unsigned char* in_pszString);
    String(const String& in_sString);
    String(const std::string& in_sString);

    String(const char *in_pszString, std::string::size_type in_iStart, std::string::size_type in_iLength = (std::string::size_type)-1);
    String(const int8_t *in_pszString, std::string::size_type in_iStart, std::string::size_type in_iLength = (std::string::size_type)-1);
    String(const uint8_t *in_pszString, std::string::size_type in_iStart, std::string::size_type in_iLength = (std::string::size_type)-1);
    String(const String& in_sString, std::string::size_type in_iStart, std::string::size_type in_iLength = (std::string::size_type)-1);
    String(const std::string &in_sString, std::string::size_type in_iStart, std::string::size_type in_iLength = (std::string::size_type)-1);
#ifdef D_USE_CEGUI
    String(const CEGUI::String &in_sString, std::string::size_type in_iStart = 0, std::string::size_type in_iLength = (std::string::size_type)-1);
#endif

    // Constructors from other types.
    static String chr(unsigned char in_cChar);
    static String nr(const int8_t& o, std::streamsize in_iWidth = 0, char in_cFill = ' ', std::ios_base::fmtflags in_fmtfl = std::ios::dec);
    static String nr(const uint8_t& o, std::streamsize in_iWidth = 0, char in_cFill = ' ', std::ios_base::fmtflags in_fmtfl = std::ios::dec);
    static String nr(const int16_t& o, std::streamsize in_iWidth = 0, char in_cFill = ' ', std::ios_base::fmtflags in_fmtfl = std::ios::dec);
    static String nr(const uint16_t& o, std::streamsize in_iWidth = 0, char in_cFill = ' ', std::ios_base::fmtflags in_fmtfl = std::ios::dec);
    static String nr(const int32_t& o, std::streamsize in_iWidth = 0, char in_cFill = ' ', std::ios_base::fmtflags in_fmtfl = std::ios::dec);
    static String nr(const uint32_t& o, std::streamsize in_iWidth = 0, char in_cFill = ' ', std::ios_base::fmtflags in_fmtfl = std::ios::dec);
    static String nr(const int64_t& o, std::streamsize in_iWidth = 0, char in_cFill = ' ', std::ios_base::fmtflags in_fmtfl = std::ios::dec);
    static String nr(const uint64_t& o, std::streamsize in_iWidth = 0, char in_cFill = ' ', std::ios_base::fmtflags in_fmtfl = std::ios::dec);
    static String nr(const float& o, std::streamsize in_iPrecision = -1, std::streamsize in_iWidth = 0, char in_cFill = ' ', std::ios_base::fmtflags in_fmtfl = std::ios::fixed | std::ios::right);
    static String nr(const double& o, std::streamsize in_iPrecision = -1, std::streamsize in_iWidth = 0, char in_cFill = ' ', std::ios_base::fmtflags in_fmtfl = std::ios::fixed | std::ios::right);
    static String b(const bool& o);

    template <class Generator>
    static String random(const String& in_sPattern, Generator gen);

    virtual ~String();

    String left(std::string::size_type in_iLength) const;
    String right(std::string::size_type in_iLength) const;
    String mid(std::string::size_type in_iLengthLeft, std::string::size_type in_iLengthRight) const;

    // Hex string <-> data conversion.
    static String sfromHex(uint8_t in_cNumber, bool in_bCaps = true);
    static uint8_t byteFromHex(const String &in_sString, bool &out_bSuccess);
    static String hexFromData(const void *in_pData, uint32_t in_uiLen = 0, bool in_bCaps = true);
    uint8_t *dataFromHex() const;

    // Functions that format the string itself.
    String fmt(const String &in_sArg1 = String::EMPTY,
                const String &in_sArg2 = String::EMPTY,
                const String &in_sArg3 = String::EMPTY,
                const String &in_sArg4 = String::EMPTY,
                const String &in_sArg5 = String::EMPTY,
                const String &in_sArg6 = String::EMPTY,
                const String &in_sArg7 = String::EMPTY,
                const String &in_sArg8 = String::EMPTY,
                const String &in_sArg9 = String::EMPTY
               ) const;
    static String sfmt(const String &in_sFmt,
                        const String &in_sArg1 = String::EMPTY,
                        const String &in_sArg2 = String::EMPTY,
                        const String &in_sArg3 = String::EMPTY,
                        const String &in_sArg4 = String::EMPTY,
                        const String &in_sArg5 = String::EMPTY,
                        const String &in_sArg6 = String::EMPTY,
                        const String &in_sArg7 = String::EMPTY,
                        const String &in_sArg8 = String::EMPTY,
                        const String &in_sArg9 = String::EMPTY
                       );
    String fmtRemoveEmpty(const String &in_sArg1 = String::EMPTY,
                           const String &in_sArg2 = String::EMPTY,
                           const String &in_sArg3 = String::EMPTY,
                           const String &in_sArg4 = String::EMPTY,
                           const String &in_sArg5 = String::EMPTY,
                           const String &in_sArg6 = String::EMPTY,
                           const String &in_sArg7 = String::EMPTY,
                           const String &in_sArg8 = String::EMPTY,
                           const String &in_sArg9 = String::EMPTY
                          ) const;
    static String sfmtRemoveEmpty(const String &in_sFmt,
                                   const String &in_sArg1 = String::EMPTY,
                                   const String &in_sArg2 = String::EMPTY,
                                   const String &in_sArg3 = String::EMPTY,
                                   const String &in_sArg4 = String::EMPTY,
                                   const String &in_sArg5 = String::EMPTY,
                                   const String &in_sArg6 = String::EMPTY,
                                   const String &in_sArg7 = String::EMPTY,
                                   const String &in_sArg8 = String::EMPTY,
                                   const String &in_sArg9 = String::EMPTY
                                  );

    // Some functions to get informations about the string.
    bool isEmpty() const;

    std::string::size_type len() const;
    int lenInt() const;
    int byteCount() const;
    
    // Some conversion functions.
    const char *c_str() const;
    std::string str() const;

#ifdef D_USE_CEGUI
private:
    CEGUI::String cegui() const;
public:
    inline operator CEGUI::String() const {return this->cegui();};
#endif

    // Functions to manipulate the string itself.
    String& removeChar(std::string::size_type in_ulIndex);
    String& addChar(std::string::size_type in_ulIndex, unsigned char in_cChar);
    String& replaceStr(std::string::size_type in_ulIndex, std::string::size_type in_ulLenght,
                       const char *in_pszNew);
    String& replaceStr(std::string::size_type in_ulIndex, std::string::size_type in_ulLenght,
                        const String & in_sNew);
    String& replaceStr(const String & in_sNeedle,
                        const String & in_sNew);
    String& trimThisLeft(const String &in_sWhat = sWhiteSpace);
    String& trimThisRight(const String &in_sWhat = sWhiteSpace);
    String& trimThis(const String &in_sWhat = sWhiteSpace);
    String& lowerThis();
    String& upperThis();

    // Functions that return a manipulated string.
    String lower() const;
    String upper() const;
    String trimLeft(const String &in_sWhat = sWhiteSpace) const;
    String trimRight(const String &in_sWhat = sWhiteSpace) const;
    String trim(const String &in_sWhat = sWhiteSpace) const;

    // Searching functions.
    std::string::size_type find(const char *in_pszNeedle, std::string::size_type in_nIdx = 0) const;
    std::string::size_type find(const String & in_sNeedle, std::string::size_type in_nIdx = 0) const;
    bool ieq(const char *in_pszOther) const;
    bool ieq(const String & in_sOther) const;
    bool ncmp(const char *in_pszNeedle, std::string::size_type in_nChars = 0) const;
    bool ncmp(const String & in_sNeedle, std::string::size_type in_nChars = 0) const;
    bool nicmp(const char *in_pszNeedle, std::string::size_type in_nChars = 0) const;
    bool nicmp(const String & in_sNeedle, std::string::size_type in_nChars = 0) const;
    bool contains(const char *in_pszNeedle) const;
    bool contains(const String & in_sNeedle) const;
    bool matchesPattern(const char *in_pszPat) const;
    bool matchesPattern(const String& in_sPat) const;

    /// Splits the string at every occurrence of \a in_sSplit and stores the
    /// substrings into the output iterator \a out. The container either needs
    /// to be big enough, or you have to use something like \a std::inserter
    /// to construct an output iterator that inserts new elements into the
    /// container.
    ///
    /// \param out The output iterator to write the substrings to.
    /// \param in_sSplit the delimiter string to use for splitting.
    template <class OutputIterator>
    void split(OutputIterator out, const String& in_sSplit = " ") const
    {
        std::string::size_type lastpos = 0;
        for(std::string::size_type pos = this->find(in_sSplit) ; pos != std::string::npos ; pos = this->find(in_sSplit, pos+1)) {
            if(pos <= lastpos) {
                *out++ = String::EMPTY;
            } else {
                *out++ = String(*this, lastpos, pos-lastpos);
            }
            lastpos = pos + in_sSplit.len();
        }

        // Now we may still need to get the last part.
        if(lastpos < this->len()) {
            *out = String(*this, lastpos, this->len() - lastpos);
        }
    }

#ifdef D_STRING_MYSQL
    String mysqlEscaped(MYSQL * in_pSQL) const;
#endif

    // A whole bunch of overloaded operators.
    String& operator =(const char *in_pszString);
    String& operator =(const String & in_sString);

    void operator +=(const char *in_pszString);
    void operator +=(const String & in_sString);

    String operator +(const char *in_pszString) const;
    String operator +(const String & in_sString) const;

    bool operator ==(const char *in_pszString) const;
    bool operator ==(const String & in_sString) const;

    bool operator !=(const char *in_pszString) const;
    bool operator !=(const String & in_sString) const;

    bool operator !() const;

    bool operator <(const char *in_pszString) const;
    bool operator <(const String & in_sString) const;

    unsigned char& operator [] (std::string::size_type in_iIndex);
    unsigned char getCharAt(std::string::size_type in_iIndex) const;

    static String EMPTY;

    int to_Integer();
    bool to_Boolean();
    float to_Float();
    template<typename T>
    bool convert(T& x, bool failIfLeftoverChars = true)
    {
        std::istringstream i(m_s);
        char c;
        if (!(i >> x) || (failIfLeftoverChars && i.get(c)))
            return false;
        return true;
    }

};

}; // namespace FTS;

bool operator ==(const char *in_pszString, const FTS::String& in_sString);
bool operator !=(const char *in_pszString, const FTS::String& in_sString);
bool operator ==(const std::string& in_s, const FTS::String& in_sString);
bool operator !=(const std::string& in_s, const FTS::String& in_sString);
bool operator <(const char *in_pszString, const FTS::String& in_sString);
FTS::String operator +(const char *in_pszString, const FTS::String & in_sString);
std::ostream& operator<< (std::ostream& os, const FTS::String& me);

/// Use this as a comparator for the strings (check if str1 is less then str2).
struct cstring_ltcomp {
  bool operator() (const FTS::String& lstr, const FTS::String& rstr) const
  {return lstr<rstr;}
};

/// Use this as a comparator for the strings (check if str1 is equal to str2).
struct cstring_eqcomp {
  bool operator() (const FTS::String& lhs, const FTS::String& rhs) const
  {return lhs==rhs;}
};

/** Very inefficient but useful method to generate a random string following a
 *  pattern.
 *
 * \param in_sPattern The pattern used to generate the random string.
 *                    If this pattern is empty, a random hex string of length 6
 *                    is generated.
 * \param gen A functor to generate random numbers that takes two integers and
 *            returns a random integer between them.
 *
 * \note Everytime a '#' is encountered in the pattern, a random uppercase hex
 *       digit (0-9,A-f) will be inserted in the result.\n
 *       Every backslash '\' (except the last character of the string) will be
 *       ignored and the next character will be used without interpretation.
 *       Use this for escaping special characters: \\# becomes #, \\\\ becomes \.
 *
 * \return a random string (following the given pattern).
 */
template <class Generator>
FTS::String FTS::String::random(const String& in_sPattern, Generator gen)
{
    // Empty pattern means 6-digit random hex.
    String sResult(in_sPattern.isEmpty() ? "######" : in_sPattern);

    for(std::string::size_type i = 0 ; i < sResult.len() ; ++i) {
        switch(sResult.getCharAt(i)) {
        case '#':
        {
            // This has to be replaced by a random uppercase hex number.
            uint8_t num = static_cast<uint8_t>(gen(0, 0xF));
            sResult[i] = (num < 0xA ? ('0'+num) : ('A'+(num-0xA)));
            break;
        }
        case '\\':
            // If there is a character after the backslash, we just remove the backslash
            // and skip the next character w/o interpreting it.
            if(i+1 < sResult.len()) {
                sResult.removeChar(i);
            }
        default: break;
        }
    }

    return sResult;
}


#endif                          /* D_STRING_H */

 /* EOF */
