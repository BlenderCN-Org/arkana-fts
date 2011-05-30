/**
 * \file dString.cpp
 * \author Pompei2
 * \date 07 January 2007
 * \brief This file implements my string class, why reinvent the wheel ?
 *        I prefer this then extending existing ones ... maybe i'll rewrite OpenGL ... nooo
 **/

#include "dLib/dString/dString.h"

#include <sstream>
#include <limits>
#include <algorithm>

#ifdef D_USE_CEGUI
#  include "CEGUIString.h"
#endif

FTS::String FTS::String::EMPTY;
const FTS::String FTS::String::sWhiteSpace = " \n\r\t";

using namespace FTS;

template <typename T>
String String::fromSpecialNumber(T number)
{
    if(std::numeric_limits<T>::has_infinity) {
        if(number == std::numeric_limits<T>::infinity()) {
            return String("\342\210\236"); // Infinity symbol
        }
    }

    // Check for NaN.
    if(number != number) {
        return String("NaN");
    }

    return String::EMPTY;
}

/** Default constructor, creates an empty string. */
String::String()
{
}

/// Creates a string containing 1 char.
/** Creates a string based on one single charachter.
 *
 * \param in_cChar The charachter wich to copy in the string.
 *
 * \author Pompei2
 */
String String::chr(unsigned char in_cChar)
{
    return String(std::string(1, in_cChar));
}

String::String(const char* in_pszString)
    : m_s(in_pszString == NULL ? "" : in_pszString)
{
}

String::String(const unsigned char* in_pszString)
    : m_s(reinterpret_cast<const char*>(in_pszString == NULL ? reinterpret_cast<const unsigned char*>("") : in_pszString))
{
}

String::String(const String& in_sString)
    : m_s(in_sString.str())
{
}

String::String(const std::string& in_sString)
    : m_s(in_sString)
{
}

String::StringSize String::strlen() const
{
    const char * p = m_s.c_str();
    std::string::size_type len = 0 ;
    int byteSize = 0;
    while( *p != '\0' ) {
        if( !(*p & 0x80) ) {
            // 1 byte char
            byteSize++; p++;
        } else if( (*p & 0xF0) == 0xC0 && (*(p+1) & 0xC0) == 0x80 ) {
            // 2 byte len
            byteSize +=2 ; p += 2;
        } else if( (*p & 0xF0) == 0xE0 && (*(p+1) & 0xC0) == 0x80 ) {
            // 3 byte len
            byteSize +=3 ; p += 3;
        } else  if( (*p & 0xF0) == 0xF0 && (*(p+1) & 0xC0) == 0x80 ) {
            // 4 byte len
            byteSize +=4 ; p += 4;
        } else {
            // handle as a default 8 bit ASCII 
            byteSize ++ ; p ++;
        }
        len++;
    }
    std::tuple<std::string::size_type, int> ret(len, byteSize);
    return ret;  
}

const char* String::advance(const char* in_pszString, std::string::size_type in_utf8Len)
{
    const char * p = in_pszString;
    if(in_utf8Len <= 0 ) return p;
    while( *p != '\0' && in_utf8Len--) {
        if( !(*p & 0x80) ) {
            // 1 byte char
            p++;
        } else if( (*p & 0xF0) == 0xC0 && (*(p+1) & 0xC0) == 0x80 ) {
            // 2 byte len
            p += 2;
        } else if( (*p & 0xF0) == 0xE0 && (*(p+1) & 0xC0) == 0x80 ) {
            // 3 byte len
            p += 3;
        } else  if( (*p & 0xF0) == 0xF0 && (*(p+1) & 0xC0) == 0x80 ) {
            // 4 byte len
            p += 4;
        } else {
            // handle as a default 8 bit ASCII
            p ++;
        }
    }
    return p;
}

int String::getByteCount(const char* in_pszString, std::string::size_type in_utf8Len)
{
    const char * p = in_pszString;
    int byteSize = 0;
    while( *p != '\0' && in_utf8Len--) {
        if( !(*p & 0x80) ) {
            // 1 byte char
            byteSize++; p++;
        } else if( (*p & 0xF0) == 0xC0 && (*(p+1) & 0xC0) == 0x80 ) {
            // 2 byte len
            byteSize +=2 ; p += 2;
        } else if( (*p & 0xF0) == 0xE0 && (*(p+1) & 0xC0) == 0x80 ) {
            // 3 byte len
            byteSize +=3 ; p += 3;
        } else  if( (*p & 0xF0) == 0xF0 && (*(p+1) & 0xC0) == 0x80 ) {
            // 4 byte len
            byteSize +=4 ; p += 4;
        } else {
            // handle as a default 8 bit ASCII
            byteSize ++ ; p ++;
        }
    }
    return byteSize;
    
}

int String::byteCount() const
{
    StringSize len = strlen();
    return std::get<1>(len);
}

void String::fromStringWithLength(const char* in_pszString, std::string::size_type in_iStart, std::string::size_type in_iLength)
{
    if(in_pszString == nullptr) {
        m_s.clear();
        return;
    }
    
    // This is the expression in_iStart + in_iLength, but saturated at
    // (std::string::size_type)-1, instead of creating an overlow.
    std::string::size_type startPlusLen = (in_iLength <= (std::string::size_type)-1 - in_iStart) ?
                                          in_iStart + in_iLength :
                                          (std::string::size_type)-1;

    // We need to find what comes first: a 0 or in_iLength bytes? That means
    // will we use the full in_iLength bytes or less?
    const char* p = in_pszString;
    std::string::size_type iLen = 0;
    for(iLen = 0 ; iLen < startPlusLen && *p != 0 ; ++iLen, ++p) {
    }

    // If the start argument is silly, make an empty string.
    if(iLen <= in_iStart) {
        m_s.clear();
        return;
    }

    // If we stopped to count because we encountered a 0 char, there's no
    // problem, just use the stl string constructor.
    p = advance(in_pszString, in_iStart);
    int iLength = in_iLength ;
    if(iLen < startPlusLen) {
        iLength = iLen - in_iStart;
    }
    iLength = getByteCount(p, iLength);
    m_s = std::string(p, iLength);
}

#ifdef D_USE_CEGUI
String::String(const CEGUI::String& in_sString, std::string::size_type in_iStart, std::string::size_type in_iLength)
{
    this->fromStringWithLength(in_sString.c_str(), in_iStart, in_iLength);
}
#endif

/// Construct the string with a maximum lenght, only starting from the
/// \a in_iStart th character.
/** Creates a string based on a C-string, but with a maximum length and
 *  starting only from a specified character index. If the index is higher
 *  then then string length, the created string is empty.
 *
 * \param in_pszString The C-String to use.
 * \param in_iStart    The (0-based) index of the first character to copy.
 * \param in_iLength   The maximum lenght of the string.
 *
 * \author Pompei2
 */
String::String(const char* in_pszString, std::string::size_type in_iStart, std::string::size_type in_iLength)
{
    this->fromStringWithLength(in_pszString, in_iStart, in_iLength);
}

/// Construct the string with a maximum lenght
/** Creates a string based on a C-string, but with a maximum length.
 *
 * \param in_pszString The C-String to use.
 * \param in_iLength   The maximum lenght of the string.
 *
 * \author Pompei2
 */
String::String(const int8_t *in_pszString, std::string::size_type in_iStart, std::string::size_type in_iLength)
{
    this->fromStringWithLength(reinterpret_cast<const char*>(in_pszString), in_iStart, in_iLength);
}

/// Construct the string with a maximum lenght
/** Creates a string based on a C-string, but with a maximum length.
 *
 * \param in_pszString The C-String to use.
 * \param in_iLength   The maximum lenght of the string.
 *
 * \author Pompei2
 */
String::String(const uint8_t *in_pszString, std::string::size_type in_iStart, std::string::size_type in_iLength)
{
    this->fromStringWithLength(reinterpret_cast<const char*>(in_pszString), in_iStart, in_iLength);
}

/// Construct the string with a maximum lenght
/** Creates a string based on another string, but with a maximum length.
 *
 * \param in_pszString The String to use.
 * \param in_iLength   The maximum lenght of the string.
 *
 * \author Pompei2
 */
String::String(const String& in_sString, std::string::size_type in_iStart, std::string::size_type in_iLength)
{
    this->fromStringWithLength(in_sString.c_str(), in_iStart, in_iLength);
}

String::String(const std::string& in_sString, std::string::size_type in_iStart, std::string::size_type in_iLength)
{
    this->fromStringWithLength(in_sString.c_str(), in_iStart, in_iLength);
}

String String::nr(const int8_t& o, std::streamsize in_iWidth, char in_cFill, std::ios_base::fmtflags in_fmtfl)
{
    String s = String::fromSpecialNumber<int8_t>(o);
    if(!s.isEmpty())
        return s;

    std::stringstream out;
    out.width(in_iWidth);
    out.fill(in_cFill);
    out.flags(in_fmtfl);
    // Cast to int prevents that it is taken as a char, without the cast,
    // if o is for example 16, the ascii char 16 would be created instead of
    // a string containing "16" (one six).
    out << (int)o;

    return String(out.str());
}

String String::nr(const uint8_t& o, std::streamsize in_iWidth, char in_cFill, std::ios_base::fmtflags in_fmtfl)
{
    String s = String::fromSpecialNumber<uint8_t>(o);
    if(!s.isEmpty())
        return s;

    std::stringstream out;
    out.width(in_iWidth);
    out.fill(in_cFill);
    out.flags(in_fmtfl);
    // Cast to int prevents that it is taken as a char, without the cast,
    // if o is for example 16, the ascii char 16 would be created instead of
    // a string containing "16" (one six).
    out << (int)o;

    return String(out.str());
}

String String::nr(const int16_t& o, std::streamsize in_iWidth, char in_cFill, std::ios_base::fmtflags in_fmtfl)
{
    String s = String::fromSpecialNumber<int16_t>(o);
    if(!s.isEmpty())
        return s;

    std::stringstream out;
    out.width(in_iWidth);
    out.fill(in_cFill);
    out.flags(in_fmtfl);
    out << (short)o;

    return String(out.str());
}

String String::nr(const uint16_t& o, std::streamsize in_iWidth, char in_cFill, std::ios_base::fmtflags in_fmtfl)
{
    String s = String::fromSpecialNumber<uint16_t>(o);
    if(!s.isEmpty())
        return s;

    std::stringstream out;
    out.width(in_iWidth);
    out.fill(in_cFill);
    out.flags(in_fmtfl);
    out << (unsigned short)o;

    return String(out.str());
}

String String::nr(const int32_t& o, std::streamsize in_iWidth, char in_cFill, std::ios_base::fmtflags in_fmtfl)
{
    String s = String::fromSpecialNumber<int32_t>(o);
    if(!s.isEmpty())
        return s;

    std::stringstream out;
    out.width(in_iWidth);
    out.fill(in_cFill);
    out.flags(in_fmtfl);
    out << (int)o;

    return String(out.str());
}

String String::nr(const uint32_t& o, std::streamsize in_iWidth, char in_cFill, std::ios_base::fmtflags in_fmtfl)
{
    String s = String::fromSpecialNumber<uint32_t>(o);
    if(!s.isEmpty())
        return s;

    std::stringstream out;
    out.width(in_iWidth);
    out.fill(in_cFill);
    out.flags(in_fmtfl);
    out << (unsigned int)o;

    return String(out.str());
}

String String::nr(const int64_t& o, std::streamsize in_iWidth, char in_cFill, std::ios_base::fmtflags in_fmtfl)
{
    String s = String::fromSpecialNumber<int64_t>(o);
    if(!s.isEmpty())
        return s;

    std::stringstream out;
    out.width(in_iWidth);
    out.fill(in_cFill);
    out.flags(in_fmtfl);
    out << (long long)o;

    return String(out.str());
}

String String::nr(const uint64_t& o, std::streamsize in_iWidth, char in_cFill, std::ios_base::fmtflags in_fmtfl)
{
    String s = String::fromSpecialNumber<uint64_t>(o);
    if(!s.isEmpty())
        return s;

    std::stringstream out;
    out.width(in_iWidth);
    out.fill(in_cFill);
    out.flags(in_fmtfl);
    out << (unsigned long long)o;

    return String(out.str());
}

String String::nr(const float& o, std::streamsize in_iPrecision, std::streamsize in_iWidth, char in_cFill, std::ios_base::fmtflags in_fmtfl)
{
    String s = String::fromSpecialNumber<float>(o);
    if(!s.isEmpty()) {
        std::stringstream out;
        out.width(in_iWidth);
        out.fill(in_cFill);
        out << s;
        return String(out.str());
    }

    std::stringstream out;
    if(in_iPrecision != (std::streamsize)-1)
        out.precision(in_iPrecision);
    out.width(in_iWidth);
    out.fill(in_cFill);
    out.flags(in_fmtfl);
    out << o;

    return String(out.str());
}

String String::nr(const double& o, std::streamsize in_iPrecision, std::streamsize in_iWidth, char in_cFill, std::ios_base::fmtflags in_fmtfl)
{
    String s = String::fromSpecialNumber<double>(o);
    if(!s.isEmpty()) {
        std::stringstream out;
        out.width(in_iWidth);
        out.fill(in_cFill);
        out << s;
        return String(out.str());
    }

    std::stringstream out;
    if(in_iPrecision != (std::streamsize)-1)
        out.precision(in_iPrecision);
    out.width(in_iWidth);
    out.fill(in_cFill);
    out.flags(in_fmtfl);
    out << o;

    return String(out.str());
}

String String::b(const bool& o)
{
    return o ? String("True") : String("False");
}

/** Default destructor. */
String::~String()
{
}

/// Returns a string consisting of only the first \a in_iLength characters of
/// this one.
///
/// \param in_iLength The number of characters to take from the left of this string.
///
/// \return A string consisting of the first \a in_iLenght characters of this one.
String String::left(std::string::size_type in_iLength) const
{
    if(this->isEmpty() || in_iLength < 1)
        return String::EMPTY;

    return String(m_s, 0, in_iLength);
}

/// Returns a string consisting of only the last \a in_iLength characters of
/// this one.
///
/// \param in_iLength The number of characters to take from the right of this string.
///
/// \return A string consisting of the last \a in_iLenght characters of this one.
String String::right(std::string::size_type in_iLength) const
{
    if(this->isEmpty() || in_iLength < 1)
        return String::EMPTY;

    if(in_iLength >= this->len())
        return String(this->str());

    return String(this->str(), this->len()-in_iLength, in_iLength);
}

/// Returns a string consisting of only the middle part of this one. So you can
/// cut-off something on the left and something on the right.
///
/// \param in_iLengthLeft The number of characters to cut-off on the left.
/// \param in_iLengthRight The number of characters to cut-off on the right.
///
/// \note If \a in_iLengthRight is 0, then it will take the string from
///       \a in_iLengthLeft until the end of the string.
///
/// \return A string consisting of the characters in the middle of this string.
String String::mid(std::string::size_type in_iLengthLeft, std::string::size_type in_iLengthRight) const
{
    // In some special cases, just copy the whole string.
    if((in_iLengthLeft == 0 && in_iLengthRight == 0) || this->isEmpty())
        return String(this->c_str());

    // Or return an empty string if nothing would fit.
    if(in_iLengthLeft + in_iLengthRight >= this->len())
        return String::EMPTY;

    return String(this->str(), in_iLengthLeft, this->len()-in_iLengthLeft-in_iLengthRight);
}

/// Converts one byte into a two-letters-hex-string.
/** Creates a string that contains two letters that represent the hex value
 *  of the given number.
 *
 * \param in_cNumber The number to create the string with.
 * \param in_bCaps Shall the alphabeticals in the string be capitals or not?
 *
 * \return The string containing the number.
 *
 * \example Let in_cNumber be 0x6A, in_bCaps be true, then the string returned will be "6A"
 *
 * \author Pompei2
 */
String String::sfromHex(uint8_t in_cNumber, bool in_bCaps)
{
    uint8_t c[3] = {0, 0, 0};
    char c2[3] = {0, 0, 0};
    c[1] = '0' + (in_cNumber & 0x0F);
    if(c[1] > '9' || c[1] < '0')
        c[1] = ((in_bCaps ? 'A' : 'a') + ((in_cNumber & 0x0F) - 0xA));
    c[0] = '0' + (in_cNumber >> 4);
    if(c[0] > '9' || c[0] < '0')
        c[0] = ((in_bCaps ? 'A' : 'a') + ((in_cNumber >> 4) - 0xA));

    c2[0] = (char)c[0];
    c2[1] = (char)c[1];
    c2[2] = (char)c[2];

    return String(c2);
}

/// Converts a one-or-two-letters-hex-string into a byte.
/** Creates one byte using a string that contains two letters that represent
 *  the hex value of the given byte.
 *
 * \param in_sString The string to get the byte from.
 * \param out_bSuccess Was the conversion successfull or not?
 *
 * \return The data byte.
 *
 * \example Let in_sString be "6A", then the returned byte will be 106
 *
 * \note If the string contains more then 2 chars, only the two first are taken.\n
 *       A leading "0x" or similar in the string will make the conversion fail.\n
 *       \n
 *       Module-tested.
 *
 * \author Pompei2
 */
uint8_t String::byteFromHex(const String& in_sString, bool& out_bSuccess)
{
    out_bSuccess = false;
    unsigned char txt[3] = {'0', '0', '\0'};

    if(in_sString == String::EMPTY)
        return 0;

    if(in_sString.len() == 1) {
        txt[1] = in_sString.getCharAt(0);
    } else if(in_sString.len() > 1) {
        txt[0] = in_sString.getCharAt(0);
        txt[1] = in_sString.getCharAt(1);
    }

    uint8_t valHigh = 0;
    if(txt[0] >= '0' && txt[0] <= '9') {
        valHigh = txt[0] - '0';
    } else if(txt[0] >= 'A' && txt[0] <= 'F') {
        valHigh = 10 + txt[0] - 'A';
    } else if(txt[0] >= 'a' && txt[0] <= 'f') {
        valHigh = 10 + txt[0] - 'a';
    } else {
        return 0;
    }

    uint8_t valLow = 0;
    if(txt[1] >= '0' && txt[1] <= '9') {
        valLow = txt[1] - '0';
    } else if(txt[1] >= 'A' && txt[1] <= 'F') {
        valLow = 10 + txt[1] - 'A';
    } else if(txt[1] >= 'a' && txt[1] <= 'f') {
        valLow = 10 + txt[1] - 'a';
    } else {
        return 0;
    }

    out_bSuccess = true;
    return valHigh*16 + valLow;
}

/// Construct a hexadecimal string from some arbitrary data.
/** Creates a string that contains the data, stored as a hexadecimal string.
 *  That means that every byte from the data is stored into this string, using
 *  hexadecimal numbers.
 *
 * \param in_pData The data to convert into a hex string.
 * \param in_uiLen The length of the data in bytes. This may also be 0, meaning
 *                 that the data is zero-terminated (stops at the first byte that is 0).
 * \param in_bCaps Shall the alphabeticals in the string be capitals or not?
 *
 * \return The string containing the data, stored using hexadecimal numbers.
 *
 * \note Module-tested
 * \author Pompei2
 */
String String::hexFromData(const void *in_pData, uint32_t in_uiLen, bool in_bCaps)
{
    if(in_pData == NULL) {
        return String::EMPTY;
    }

    String sRet = String::EMPTY;
    uint8_t *p = (uint8_t *)in_pData;

    if(in_uiLen == 0) {
        for( ; *p != 0 ; p++) {
            sRet += String::sfromHex(*p, in_bCaps);
        }
    } else {
        for(uint32_t i = 0 ; i < in_uiLen ; i++, p++) {
            sRet += String::sfromHex(*p, in_bCaps);
        }
    }

    return sRet;
}

/// Constructs data from an hexadecimal string.
/** Creates data from the hexadecimal string.
 *  That means that every two-signs in the string will be converted into a byte
 *  that corresponds to the hexadecimal represented by the two signs.
 *
 * \return The data (allocated automatically). This has to be freed (delete [])
 *         by the user !\n
 *         NULL on failure.
 *
 * \note If there is any non-hex sign in the string, NULL will be returned.
 *
 * \author Pompei2
 */
uint8_t *String::dataFromHex() const
{
    std::string::size_type len = this->len();

    // The data will be half-size of the string-length, as two signs in the string
    // represent one byte. (like 6A)
    std::string::size_type iDataLen = len / 2;
    uint8_t *pData = new uint8_t[iDataLen];

    // Convert every pair of chars into a byte and write it to the data.
    unsigned char cBuf[3] = {'\0', '\0', '\0'};
    bool bSuccess = false;
    String tmp;
    for(std::string::size_type i = 0 ; i < iDataLen ; i++) {
        cBuf[0] = this->getCharAt(i*2);
        cBuf[1] = this->getCharAt(i*2+1);
        tmp = cBuf;
        pData[i] = String::byteFromHex(tmp, bSuccess);
        if(!bSuccess) {
            delete [] pData;
            return NULL;
        }
    }

    return pData;
}

/// Get the C-String (read-only).
/** Get the C-string that builds this string, in read-only mode.
 *
 * \return the C-string.
 *
 * \author Pompei2
 */
const char *String::c_str() const
{
    return m_s.c_str();
}

/** Get a copy of this string as a C++ std::string.
 *
 * \return A copy of this string as a C++ std::string.
 *
 * \author Pompei2
 */
std::string String::str() const
{
    return m_s;
}

#ifdef D_USE_CEGUI
/// Get the CEGUI-String.
/** Get the CEGUI-string pendant to this string. A copy is returned.
 *
 * \return A CEGUI-style copy of this string.
 *
 * \author Pompei2
 */
CEGUI::String String::cegui() const
{
    if(this->isEmpty())
        return CEGUI::String("");
    else
        return CEGUI::String((const CEGUI::utf8 *)this->c_str());
}
#endif

String String::fmt(const String& in_sArg1,
                   const String& in_sArg2, const String& in_sArg3,
                   const String& in_sArg4, const String& in_sArg5,
                   const String& in_sArg6, const String& in_sArg7,
                   const String& in_sArg8, const String& in_sArg9
                  ) const
{
    String s2(this->c_str());
    if(!in_sArg1.isEmpty())
        s2.replaceStr("{1}", in_sArg1);
    if(!in_sArg2.isEmpty())
        s2.replaceStr("{2}", in_sArg2);
    if(!in_sArg3.isEmpty())
        s2.replaceStr("{3}", in_sArg3);
    if(!in_sArg4.isEmpty())
        s2.replaceStr("{4}", in_sArg4);
    if(!in_sArg5.isEmpty())
        s2.replaceStr("{5}", in_sArg5);
    if(!in_sArg6.isEmpty())
        s2.replaceStr("{6}", in_sArg6);
    if(!in_sArg7.isEmpty())
        s2.replaceStr("{7}", in_sArg7);
    if(!in_sArg8.isEmpty())
        s2.replaceStr("{8}", in_sArg8);
    if(!in_sArg9.isEmpty())
        s2.replaceStr("{9}", in_sArg9);
    return s2;
}

String String::sfmt(const String& in_sFmt, const String& in_sArg1,
                    const String& in_sArg2,const String& in_sArg3,
                    const String& in_sArg4,const String& in_sArg5,
                    const String& in_sArg6,const String& in_sArg7,
                    const String& in_sArg8,const String& in_sArg9)
{
    return in_sFmt.fmt(in_sArg1, in_sArg2, in_sArg3, in_sArg4, in_sArg5,
                       in_sArg6, in_sArg7, in_sArg8, in_sArg9);
}

String String::fmtRemoveEmpty(const String& in_sArg1,
                              const String& in_sArg2, const String& in_sArg3,
                              const String& in_sArg4, const String& in_sArg5,
                              const String& in_sArg6, const String& in_sArg7,
                              const String& in_sArg8, const String& in_sArg9
                             ) const
{
    String s2(this->c_str());
    s2.replaceStr("{1}", in_sArg1);
    s2.replaceStr("{2}", in_sArg2);
    s2.replaceStr("{3}", in_sArg3);
    s2.replaceStr("{4}", in_sArg4);
    s2.replaceStr("{5}", in_sArg5);
    s2.replaceStr("{6}", in_sArg6);
    s2.replaceStr("{7}", in_sArg7);
    s2.replaceStr("{8}", in_sArg8);
    s2.replaceStr("{9}", in_sArg9);
    return s2;
}

String String::sfmtRemoveEmpty(const String& in_sFmt, const String& in_sArg1,
                               const String& in_sArg2,const String& in_sArg3,
                               const String& in_sArg4,const String& in_sArg5,
                               const String& in_sArg6,const String& in_sArg7,
                               const String& in_sArg8,const String& in_sArg9)
{
    return in_sFmt.fmtRemoveEmpty(in_sArg1, in_sArg2, in_sArg3, in_sArg4, in_sArg5,
                                  in_sArg6, in_sArg7, in_sArg8, in_sArg9);
}

/// Wether the string is empty or not.
/** Returns a boolean specifying wether the string is empty or not.
 *  It is empty if it hasn't been initialised AND if it's "".
 *
 * \return true if the string is empty, else false.
 *
 * \author Pompei2
 */
bool String::isEmpty() const
{
    return m_s.empty();
}

/// get the length of the string.
/** This function returns the length of the string, in a quick way (see notes).
 *
 * \return the length of the string, 0 if it is empty.
 *
 * \note This function is always very quick, because it doesn't need to calculate
 *       the strings lenght, the lenght is calculated only once, when the string changes.
 *
 * \author Pompei2
 */
std::string::size_type String::len() const
{
    StringSize len = strlen();
    return std::get<0>(len);
}

/// get the length of the string.
/** This function returns the length of the string, in a quick way (see notes).
 *
 * \return the length of the string, 0 if it is empty.
 *
 * \note This function is always very quick, because it doesn't need to calculate
 *       the strings length, the length is calculated only once, when the string changes.
 *
 * \author Pompei2
 */
int String::lenInt() const
{
    StringSize len = strlen();
    return (int)std::get<0>(len);
}

/// Remove Nth character from a string.
/** Removes the \a in_ulIndex the character from the string.
 *
 * \param in_ulIndex The index of the character you want to delete.
 *
 * \return *this.
 *
 * \note If in_ulIndex is 0, the first character is deleted (it is 0-based).
 * \note If in_ulIndex is bigger then the length of the string, the last
 *       character of the string is deleted.
 *
 * Example: \n \code
 *       String str( "HelloO, World !" );
 *       str.removeChar( 5 )
 *       // str is now "Hello, World !"\endcode
 *
 * \author Pompei2
 */
String& String::removeChar(std::string::size_type in_ulIndex)
{
    if(this->isEmpty())
        return *this;

    if(in_ulIndex >= this->len()) {
        m_s = this->left(this->len()-1).str();
        return *this;
    }

    String tmpA(this->c_str(), 0, in_ulIndex);
    String tmpB(&(this->c_str()[in_ulIndex+1]));

    m_s = (tmpA + tmpB).str();
    return *this;
}

/// Adds Nth character to a sring.
/** Adds the character \a in_cChar at the \a in_ulIndex th place of the string.
 *
 * \param in_ulIndex The index where to add the character. (If higher then the
 *                   length of the string, the character is added at the end.)
 * \param in_cChar   The character you want to add.
 *
 * \return this.
 *
 * \Note If \a in_ulIndex is 0, the charachter is added at the beginning. (0-based)
 *
 * Example: \n \code
 *       String str( "ello, World" );
 *       str.addChar( 0, 'H' )
 *       str.addChar( 99, '!' )
 *       // str is now "Hello, World!"\endcode
 *
 * \author Pompei2
 */
String& String::addChar(std::string::size_type in_ulIndex, unsigned char in_cChar)
{
    if(in_ulIndex == 0) {
        m_s = (String::chr(in_cChar) + *this).str();
        return *this;
    }

    if(in_ulIndex >= this->len()) {
        m_s = (*this + String::chr(in_cChar)).str();
        return *this;
    }

    String strA(this->c_str(), 0, in_ulIndex);
    String strB(&(this->c_str()[in_ulIndex]));

    m_s = (strA + String::chr(in_cChar) + strB).str();
    return *this;
}

/// Replace a part of a string with another one.
/** Replaces the part of the string that goes from \a in_ulIndex to
 *  \a in_ulIndex + \a in_ulLenght with \a in_pszNew.
 *
 * \param in_ulIndex  The index of the first charachter to replace (begins at 0).
 * \param in_ulLenght The lenght of the string to replace.
 * \param in_pszNew   The string with wich to replace the part index -> lenght.
 *
 * \return this.
 *
 * Example: \n \code
 *       String str( "Hello, Awful World !" );
 *       str.replaceStr( 7, 5, "Beautifull" );
 *       // str is now "Hello, Beautifull World !"
 *
 * \author Pompei2
 */
String& String::replaceStr(std::string::size_type in_ulIndex,
                           std::string::size_type in_ulLenght,
                           const char *in_pszNew)
{
    if(this->isEmpty()) {
        *this = in_pszNew;
        return *this;
    }

    if(in_ulIndex > this->len())
        return *this;

    if(in_ulLenght > this->len() || in_ulLenght + in_ulIndex > this->len())
        in_ulLenght = this->len() - in_ulIndex;

    String strA(*this, 0, in_ulIndex);
    String strB(*this, in_ulIndex + in_ulLenght);

    m_s = (strA + in_pszNew + strB).str();
    return *this;
}

/// Replace a part of a string with another one.
/** Replaces the part of the string that goes from \a in_ulIndex to
 *  \a in_ulIndex + \a in_ulLenght with \a in_sNew.
 *
 * \param in_ulIndex  The index of the first charachter to replace (begins at 0).
 * \param in_ulLenght The lenght of the string to replace.
 * \param in_sNew     The string with wich to replace the part index -> lenght.
 *
 * \return this.
 *
 * Example: \n \code
 *       String str( "Hello, Awful World !" );
 *       str.replaceStr( 7, 5, "Beautifull" );
 *       // str is now "Hello, Beautifull World !"
 *
 * \author Pompei2
 */
String& String::replaceStr(std::string::size_type in_ulIndex,
                           std::string::size_type in_ulLenght,
                           const String& in_sNew)
{
    return this->replaceStr(in_ulIndex, in_ulLenght, in_sNew.c_str());
}

/// Replace all occurrences of \a in_sNeedle with \a in_sNew.
/** Replaces all occurrences of \a in_sNeedle found in this string by
 *  \a in_sNew.
 *
 * \param in_sNeedle  The string to replace with \a in_ulLenght.
 * \param in_ulLenght The string that replaces \a in_sNeedle.
 *
 * \return this.
 *
 * \author Pompei2
 */
String& String::replaceStr(const String& in_sNeedle,
                           const String& in_sNew)
{
    std::string::size_type idx = 0;

    while((idx = this->find(in_sNeedle, idx)) != std::string::npos) {
        this->replaceStr(idx, in_sNeedle.len(), in_sNew);
        idx += in_sNew.len();
    }

    return *this;
}

/// Removes all leading characters.
/** This removes all characters included in \a in_sWhat that are at the front
 *  of the string.
 *
 * \param in_sWhat A list of all characters that shall be trimmed (removed).
 *
 * \return this.
 *
 * \author Pompei2
 */
String& String::trimThisLeft(const String& in_sWhat)
{
    while(this->len() > 0 && in_sWhat.contains(String::chr(this->getCharAt(0)))) {
        this->removeChar(0);
    }

    return *this;
}

/// Removes all trailing characters.
/** This removes all characters included in \a in_sWhat that are at the back
 *  of the string.
 *
 * \param in_sWhat A list of all characters that shall be trimmed (removed).
 *
 * \return this.
 *
 * \author Pompei2
 */
String& String::trimThisRight(const String& in_sWhat)
{
    while(this->len() > 0 && in_sWhat.contains(String::chr(this->getCharAt(this->len()-1)))) {
        this->removeChar(this->len()-1);
    }

    return *this;
}

/// Removes all leading and trailing characters.
/** This removes all characters included in \a in_sWhat that are at the front and
 *  at the back of the string.
 *
 * \param in_sWhat A list of all characters that shall be trimmed.
 *
 * \return this.
 *
 * \author Pompei2
 */
String& String::trimThis(const String& in_sWhat)
{
    return this->trimThisLeft(in_sWhat).trimThisRight(in_sWhat);
}

/// Creates a copy of the string that has removes all leading characters.
/** This removes all characters included in \a in_sWhat that are at the front
 *  of the string (in a copy of that string).\n
 *  The original string does not get touched at all.
 *
 * \param in_sWhat A list of all characters that shall be trimmed (removed).
 *
 * \return the new, trimmed string
 *
 * \author Pompei2
 */
String String::trimLeft(const String& in_sWhat) const
{
    String sCopy(this->c_str());
    sCopy.trimThisLeft(in_sWhat);
    return sCopy;
}

/// Creates a copy of the string that has removes all trailing characters.
/** This removes all characters included in \a in_sWhat that are at the back
 *  of the string (in a copy of that string).\n
 *  The original string does not get touched at all.
 *
 * \param in_sWhat A list of all characters that shall be trimmed (removed).
 *
 * \return the new, trimmed string
 *
 * \author Pompei2
 */
String String::trimRight(const String& in_sWhat) const
{
    String sCopy(this->c_str());
    sCopy.trimThisRight(in_sWhat);
    return sCopy;
}

/// Creates a copy of the string that has removes all leading and trailing characters.
/** This removes all characters included in \a in_sWhat that are at the front and
 *  at the back of the string (in a copy of that string).
 *  The original string does not get touched at all.
 *
 * \param in_sWhat A list of all characters that shall be trimmed (removed).
 *
 * \return the new, trimmed string
 *
 * \author Pompei2
 */
String String::trim(const String& in_sWhat) const
{
    String sCopy(this->c_str());
    sCopy.trimThis(in_sWhat);
    return sCopy;
}

/// Converts this string to lowercase.
/** This converts the string itself to lowercase contents.
 *
 * \return This
 *
 * \author Pompei2
 */
String& String::lowerThis()
{
    std::transform(m_s.begin(), m_s.end(), m_s.begin(), ::tolower);
    return *this;
}

/// Converts this string to uppercase.
/** This converts the string itself to uppercase contents.
 *
 * \return This
 *
 * \author Pompei2
 */
String& String::upperThis()
{
    std::transform(m_s.begin(), m_s.end(), m_s.begin(), ::toupper);
    return *this;
}

/// Returns (a copy of) the lowercase version of this string.
/** this creates a string that is a copy of this one and then makes it all lowercase.
 *
 * \return The lowercase copy of this string.
 *
 * \author Pompei2
 */
String String::lower() const
{
    String ret(*this);
    ret.lowerThis();
    return ret;
}

/// Returns (a copy of) the uppercase version of this string.
/** this creates a string that is a copy of this one and then makes it all uppercase.
 *
 * \return The uppercase copy of this string.
 *
 * \author Pompei2
 */
String String::upper() const
{
    String ret(*this);
    ret.upperThis();
    return ret;
}

/// Searches for a sub-string in this string.
/** Searches for a sub-string in this string and returns the (0-based) index
 *  of the first occurence found.
 *
 * \param in_pszNeedle The string you want to search for.
 * \param in_nIdx      The index where to begin the search (0 = search the whole string).
 *
 * \return If the string was found, the index (0-based) of the first occurence of it.
 * \return If the string wasn't found, std::string::npos.
 *
 * \note searching for the empty string will always return std::string::npos.
 *
 * \author Pompei2
 */
std::string::size_type String::find(const char *in_pszNeedle, std::string::size_type in_nIdx) const
{
    if(in_pszNeedle == NULL || in_pszNeedle[0] == '\0')
        return std::string::npos;

    if(in_nIdx > this->len() || this->isEmpty())
        return std::string::npos;

    const char *p = strstr(&this->c_str()[in_nIdx], in_pszNeedle);

    if(p == nullptr)
        return std::string::npos;

    return this->len() - ::strlen(p);
}

/// Searches for a sub-string in this string.
/** Searches for a sub-string in this string and returns the (0-based) index
 *  of the first occurence found.
 *
 * \param in_sNeedle The string you want to search for.
 * \param in_nIdx    The index where to begin the search (0 = search the whole string).
 *
 * \return If the string was found, the index (0-based) of the first occurence of it.
 * \return If the string wasn't found, -1.
 *
 * \author Pompei2
 */
std::string::size_type String::find(const String& in_sNeedle, std::string::size_type in_nIdx) const
{
    return this->find(in_sNeedle.c_str(), in_nIdx);
}

/// Compares this string to another, ignoring the case.
/** This compares the current string with the string \a in_pszOther, ignoring
 *  case. If they are the same (ignoring case), true is returned, else false.
 *
 * \param in_pszOther The string you want to compare to.
 *
 * \return True if they are the same (ignoring case), false else.
 *
 * \author Pompei2
 */
bool String::ieq(const char *in_pszOther) const
{
    // First, compare lengths.
    if(this->len() != ::strlen(in_pszOther))
        return false;

    // If they have the same length, compare the strings, ignoring case.
    for(std::string::size_type i = 0 ; i < this->len() ; ++i) {
        if(::tolower(this->getCharAt(i)) != ::tolower(in_pszOther[i]))
            return false;
    }

    return true;
}

/// Compares this string to another, ignoring the case.
/** This compares the current string with the string \a in_pszOther, ignoring
 *  case. If they are the same (ignoring case), true is returned, else false.
 *
 * \param in_sOther The string you want to compare to.
 *
 * \return True if they are the same (ignoring case), false else.
 *
 * \author Pompei2
 */
bool String::ieq(const String& in_sOther) const
{
    // First, compare lengths.
    if(this->len() != in_sOther.len())
        return false;

    // If they have the same length, compare the strings, ignoring case.
    for(std::string::size_type i = 0 ; i < this->len() ; ++i) {
        if(::tolower(this->getCharAt(i)) != ::tolower(in_sOther.getCharAt(i)))
            return false;
    }

    return true;
}

/// Acts like strncmp
/** Compare the first in_nChars charachters of in_pszNeedle with the first
 *  in_nChars charachters of this, if they all equal, returns true, else it
 *  returns false. If in_nChars is 0, the amount of chars to compare will be
 *  equal to the amount of char in_pszNeedle or this (if lower) has.
 *
 * \param in_pszNeedle The string you want to compare to.
 * \param in_nChars    The number of charachters to compare, or 0 to compare the
 *                     whole \a in_pszNeedle string.
 *
 * \return If the string beginning matches, true.
 * \return If the string beginning doesn't match, false.
 *
 * \author Pompei2
 */
bool String::ncmp(const char *in_pszNeedle, std::string::size_type in_nChars) const
{
    std::string::size_type nChars = in_nChars;

    if((in_pszNeedle == NULL || in_pszNeedle[0] == '\0') && this->isEmpty())
        return true;

    if(in_pszNeedle == NULL || in_pszNeedle[0] == '\0' || this->isEmpty())
        return false;

    if(nChars == 0 || nChars > ::strlen(in_pszNeedle))
        nChars = ::strlen(in_pszNeedle);

    // We look for something that is longer as the haystack - impossible.
    if(nChars > this->len())
        return false;

    for(std::string::size_type i = 0; i < nChars; i++) {
        if(this->getCharAt(i) != in_pszNeedle[i])
            return false;
    }

    return true;
}

/// Acts like strncmp
/** Compare the first in_nChars charachters of in_pszNeedle with the first
 *  in_nChars charachters of this, if they all equal, returns true, else it
 *  returns false. If in_nChars is 0, the amount of chars to compare will be
 *  equal to the amount of char in_pszNeedle or this (if lower) has.
 *
 * \param in_sNeedle  The string you want to compare to.
 * \param in_nChars   The number of charachters to compare, or 0 to compare the whole string.
 *
 * \return If the string beginning matches, true.
 * \return If the string beginning doesn't match, false.
 *
 * \author Pompei2
 */
bool String::ncmp(const String& in_sNeedle, std::string::size_type in_nChars) const
{
    return this->ncmp(in_sNeedle.c_str(), in_nChars);
}

/// Acts like strncmp, but ignoring the case.
/** Compare the first in_nChars charachters of in_pszNeedle with the first
 *  in_nChars charachters of this, if they all equal (ignoring case), returns true, else it
 *  returns false. If in_nChars is 0, the amount of chars to compare will be
 *  equal to the amount of char in_pszNeedle or this (if lower) has.
 *
 * \param in_pszNeedle The string you want to compare to.
 * \param in_nChars    The number of charachters to compare, or 0 to compare the whole string.
 *
 * \return If the string beginning matches, true.
 * \return If the string beginning doesn't match, false.
 *
 * \author Pompei2
 */
bool String::nicmp(const char *in_pszNeedle, std::string::size_type in_nChars) const
{
    return this->lower().ncmp(String(in_pszNeedle).lower(), in_nChars);
}

/// Acts like strncmp, but ignoring the case.
/** Compare the first in_nChars charachters of in_pszNeedle with the first
 *  in_nChars charachters of this, if they all equal (ignoring case), returns true, else it
 *  returns false. If in_nChars is 0, the amount of chars to compare will be
 *  equal to the amount of char in_pszNeedle or this (if lower) has.
 *
 * \param in_sNeedle The string you want to compare to.
 * \param in_nChars  The number of charachters to compare, or 0 to compare the whole string.
 *
 * \return If the string beginning matches, true.
 * \return If the string beginning doesn't match, false.
 *
 * \author Pompei2
 */
bool String::nicmp(const String& in_sNeedle, std::string::size_type in_nChars) const
{
    return this->lower().ncmp(in_sNeedle.lower(), in_nChars);
}

/// Looks wether a sub-string is present in this string.
/** Looks wether a sub-string is present in this string.
 *
 * \param in_pszNeedle The string you want to search for.
 *
 * \return If the string is present true, else false.
 *
 * \note the empty string is contained nowhere.
 *
 * \author Pompei2
 */
bool String::contains(const char *in_pszNeedle) const
{
    return std::string::npos != this->find(in_pszNeedle);
}

/// Looks wether a sub-string is present in this string.
/** Looks wether a sub-string is present in this string.
 *
 * \param in_sNeedle The string you want to search for.
 *
 * \return If the string is present true, else false.
 *
 * \author Pompei2
 */
bool String::contains(const String& in_sNeedle) const
{
    return this->contains(in_sNeedle.c_str());
}

/// Matches a pattern onto this string. The pattern is not a regular expression!
/// It can have * (a star), that means to match any string of any length,
/// including nothing ; it can have a question mark, that means to match any
/// single character (or nothing if D_QUEST_CAN_BE_NONE is defined as 1) and
/// it can have a range of characters using the syntax [a-z] or [xah].
///
/// \param in_pszPat The pattern to try to match.
///
/// \return true if this matches the pattern, false if not or if there is a
///         syntax error in the pattern.
bool String::matchesPattern(const char *in_pszPat) const
{
    char c, p, l;
    const char *str  = this->c_str();
    const char *pat = in_pszPat;

    if(!pat)
        return true;

    while(true) {
        switch(p = *pat++) {
        case 0:                          // end of pattern
            return *str ? false : true;  // if end of string true
        case '*':
            while(*str) {                // match zero or more char
                if(String(str++).matchesPattern(pat))
                    return true;
            }
            return this->matchesPattern(pat);
        case '?':
#if D_QUEST_CAN_BE_NONE
            if(String(str++).matchesPattern(pat))
                return true;
            return String(str).matchesPattern(pat);
#else
            if(*str++ == 0)              // match any one char
                return false;            // not end of string
            break;
#endif
        case '[':
            if((c = *str++) == 0)  // match char set
                return false;      // syntax

            l = 0;
            while((p = *pat++)) {
                if(p == ']')            // if end of char set, then
                    return false;       // no match found
                if(p == '-') {          // check a range of chars?
                    p = *pat;           // get high limit of range
                    if(p == 0 || p == ']')
                        return false;   // syntax
                    if(c >= l && c <= p)
                        break;          // if in range, move on
                }
                l = p;
                if(c == p)              // if char matches this element
                    break;              // move on
            }

            while(p && p != ']')        // got a match in char set
                p = *pat++;             // skip to end of set

            break;
        default:
            if(*str++ != p)          // check for exact char
                return false;        // not a match
            break;
        }
    }
}

/// Matches a pattern onto this string. The pattern is not a regular expression!
/// It can have * (a star), that means to match any string of any length,
/// including nothing ; it can have a question mark, that means to match any
/// single character (or nothing if D_QUEST_CAN_BE_NONE is defined as 1) and
/// it can have a range of characters using the syntax [a-z] or [xah].
///
/// \param in_sPat The pattern to try to match.
///
/// \return true if this matches the pattern, false if not or if there is a
///         syntax error in the pattern.
bool String::matchesPattern(const String& in_sPat) const
{
    return this->matchesPattern(in_sPat.c_str());
}

#ifdef D_STRING_MYSQL
/// Returns the MySQL escaped version of this string.
/** this creates a string that is a copy of this one, but escaped for mysql commands.
 *
 * \param in_pSQL The SQL context used to escape the string.
 *
 * \return The MySQL escaped copy of this string.
 *
 * \author Pompei2
 */
String String::mysqlEscaped( MYSQL *in_pSQL ) const
{
    char *buf = new char[this->len()*2+1];
    int iLen = mysql_real_escape_string(in_pSQL, buf, this->c_str(), this->len());

    String sRet(buf, iLen);
    delete [] buf;

    return sRet;
}
#endif

String& String::operator =(const char *in_pszString)
{
    // Protect against self-assignment.
    if(this->c_str() == in_pszString)
        return *this;

    if(in_pszString == NULL)
        m_s.clear();
    else
        m_s = in_pszString;

    return *this;
}

String& String::operator =(const String& in_sString)
{
    // Protect against self-assignment.
    if(this == &in_sString)
        return *this;

    // Set me to the empty string.
    if(in_sString.isEmpty())
        this->m_s = String::EMPTY.str();

    m_s = in_sString.str();
    return *this;
}

String String::operator + (const char *in_pszString) const
{
    String tmp(this->c_str());

    tmp += in_pszString;
    return tmp;
}

String String::operator + (const String& in_sString) const
{
    String tmp(this->c_str());

    tmp += in_sString;
    return tmp;
}

String operator + (const char *in_pszString, const String& in_sString)
{
    String tmp(in_pszString);

    tmp += in_sString;
    return tmp;
}

void String::operator +=(const char *in_pszString)
{
    if(in_pszString == NULL)
        return;

    m_s += in_pszString;
}

void String::operator +=(const String& in_sString)
{
    if(in_sString.isEmpty())
        return;

    this->operator +=(in_sString.c_str());
}

bool String::operator == (const char *in_pszString) const
{
    if(in_pszString == NULL || in_pszString[0] == '\0') {
        if(this->isEmpty())
            return true;
        return false;
    }

    return m_s == in_pszString;
}

bool String::operator == (const String& in_sString) const
{
    if(in_sString == NULL || in_sString.getCharAt(0) == '\0') {
        if(this->isEmpty())
            return true;
        return false;
    }

    return m_s == in_sString.str();
}

bool String::operator != (const char *in_pszString) const
{
    return !this->operator ==(in_pszString);
}

bool String::operator != (const String& in_sString) const
{
    return !this->operator ==(in_sString);
}

bool String::operator ! () const
{
    return this->isEmpty();
}

bool String::operator <(const char *in_pszString) const
{
    if(in_pszString == NULL || in_pszString[0] == '\0') {
        if(this->isEmpty())
            return false; // False, because they are equal.
        return false; // Else, the empty string is smaller as anything else.
    }

    return strcmp(this->c_str(), in_pszString) < 0;
}

bool String::operator <(const String& in_sString) const
{
    return this->operator <(in_sString.c_str());
}

/// \throw std::out_of_range if \a in_iIndex is bigger than the string.
unsigned char& String::operator[] (std::string::size_type in_iIndex)
{
    return reinterpret_cast<unsigned char&>(m_s.at(in_iIndex));
}

unsigned char String::getCharAt(std::string::size_type in_iIndex) const
{
    if(this->isEmpty() || in_iIndex >= this->len())
        return 0;

    return m_s[in_iIndex];
}

int String::to_Integer()
{
    if(this->isEmpty())
        return 0;
    int retVal ;
    if( convert<int>(retVal) ) {
        return retVal;
    }
    return 0;
}

float String::to_Float()
{
    if(this->isEmpty())
        return 0.0f;
    float retVal ;
    if( convert<float>(retVal) ) {
        return retVal;
    }
    return 0.0f;
}

bool String::to_Boolean()
{
    if(this->isEmpty())
        return 0;
    return this->ieq("True") ? true : false;
}

bool operator ==(const char *in_pszString, const String& in_sString)
{
    return in_sString == in_pszString;
}

bool operator !=(const char *in_pszString, const String& in_sString)
{
    return in_sString != in_pszString;
}

bool operator ==(const std::string& in_s, const FTS::String& in_sString)
{
    return in_sString == in_s;
}

bool operator !=(const std::string& in_s, const FTS::String& in_sString)
{
    return in_sString != in_s;
}

bool operator <(const char *in_pszString, const FTS::String& in_sString)
{
    return FTS::String(in_pszString) < in_sString;
}

std::ostream& operator<< (std::ostream& os, const FTS::String& me)
{
    return os << me.c_str();
}
