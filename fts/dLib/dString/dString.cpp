/// \file dString.cpp
/// \author Pompei2
/// \date 07 January 2007
/// \brief This file implements my string class, why reinvent the wheel ?
///       I prefer this then extending existing ones ... maybe i'll rewrite OpenGL ... nooo

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

String::String()
{ }

String String::chr(unsigned char in_cChar)
{
    return String(std::string(1, in_cChar));
}

String::String(const char* in_pszString)
    : m_s(in_pszString == NULL ? "" : in_pszString)
{ }

String::String(const unsigned char* in_pszString)
    : m_s(reinterpret_cast<const char*>(in_pszString == NULL ? reinterpret_cast<const unsigned char*>("") : in_pszString))
{ }

String::String(const String& in_sString)
    : m_s(in_sString.str())
{ }

String::String(const std::string& in_sString)
    : m_s(in_sString)
{ }

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

const char* String::advance(const char* in_pszString, std::allocator< char >::size_type in_utf8Len) const
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

std::string::size_type String::byteCount() const
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

String::String(const char* in_pszString, std::string::size_type in_iStart, std::string::size_type in_iLength)
{
    this->fromStringWithLength(in_pszString, in_iStart, in_iLength);
}

String::String(const int8_t *in_pszString, std::string::size_type in_iStart, std::string::size_type in_iLength)
{
    this->fromStringWithLength(reinterpret_cast<const char*>(in_pszString), in_iStart, in_iLength);
}

String::String(const uint8_t *in_pszString, std::string::size_type in_iStart, std::string::size_type in_iLength)
{
    this->fromStringWithLength(reinterpret_cast<const char*>(in_pszString), in_iStart, in_iLength);
}

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
    if(!s.empty())
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
    if(!s.empty())
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
    if(!s.empty())
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
    if(!s.empty())
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
    if(!s.empty())
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
    if(!s.empty())
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
    if(!s.empty())
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
    if(!s.empty())
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
    if(!s.empty()) {
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
    if(!s.empty()) {
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

String::~String()
{ }

String String::left(std::string::size_type in_iLength) const
{
    if(this->empty() || in_iLength < 1)
        return String::EMPTY;

    return String(m_s, 0, in_iLength);
}

String String::right(std::string::size_type in_iLength) const
{
    if(this->empty() || in_iLength < 1)
        return String::EMPTY;

    if(in_iLength >= this->len())
        return String(this->str());

    return String(this->str(), this->len()-in_iLength, in_iLength);
}

String String::mid(std::string::size_type in_iLengthLeft, std::string::size_type in_iLengthRight) const
{
    // In some special cases, just copy the whole string.
    if((in_iLengthLeft == 0 && in_iLengthRight == 0) || this->empty())
        return String(this->c_str());

    // Or return an empty string if nothing would fit.
    if(in_iLengthLeft + in_iLengthRight >= this->len())
        return String::EMPTY;

    return String(this->str(), in_iLengthLeft, this->len()-in_iLengthLeft-in_iLengthRight);
}

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

const char *String::c_str() const
{
    return m_s.c_str();
}

std::string String::str() const
{
    return m_s;
}

#ifdef D_USE_CEGUI
CEGUI::String String::cegui() const
{
    if(this->empty())
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
    if(!in_sArg1.empty())
        s2.replaceStr("{1}", in_sArg1);
    if(!in_sArg2.empty())
        s2.replaceStr("{2}", in_sArg2);
    if(!in_sArg3.empty())
        s2.replaceStr("{3}", in_sArg3);
    if(!in_sArg4.empty())
        s2.replaceStr("{4}", in_sArg4);
    if(!in_sArg5.empty())
        s2.replaceStr("{5}", in_sArg5);
    if(!in_sArg6.empty())
        s2.replaceStr("{6}", in_sArg6);
    if(!in_sArg7.empty())
        s2.replaceStr("{7}", in_sArg7);
    if(!in_sArg8.empty())
        s2.replaceStr("{8}", in_sArg8);
    if(!in_sArg9.empty())
        s2.replaceStr("{9}", in_sArg9);
    return s2;
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

bool String::empty() const
{
    return m_s.empty();
}

std::string::size_type String::len() const
{
    StringSize len = strlen();
    return std::get<0>(len);
}

int String::lenInt() const
{
    StringSize len = strlen();
    return (int)std::get<0>(len);
}

String& String::removeChar(std::string::size_type in_ulIndex)
{
    if(this->empty())
        return *this;

    if(in_ulIndex >= this->len()) {
        m_s = this->left(this->len()-1).str();
        return *this;
    }

    String tmpA(*this, 0, in_ulIndex);
    String tmpB(*this, in_ulIndex+1);

    m_s = (tmpA + tmpB).str();
    return *this;
}

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

    String strA(*this, 0, in_ulIndex);
    String strB(*this, in_ulIndex);

    m_s = (strA + String::chr(in_cChar) + strB).str();
    return *this;
}

String& String::replaceStr(std::string::size_type in_ulIndex,
                           std::string::size_type in_ulLenght,
                           const char *in_pszNew)
{
    if(this->empty()) {
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

String& String::replaceStr(std::string::size_type in_ulIndex,
                           std::string::size_type in_ulLenght,
                           const String& in_sNew)
{
    return this->replaceStr(in_ulIndex, in_ulLenght, in_sNew.c_str());
}

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

String String::trimLeft(const String& in_sWhat) const
{
    // Move start to the first position that has a charachter which is not in "in_sWhat"
    std::size_t start = 0;
    while(start < this->len() && in_sWhat.contains(String::chr(this->getCharAt(start)))) {
        start++;
    }

    return String(*this, start);
}

String String::trimRight(const String& in_sWhat) const
{
    // Move start to the first position that has a charachter which is not in "in_sWhat"
    std::size_t len = this->len();
    while(len > 0 && in_sWhat.contains(String::chr(this->getCharAt(len-1)))) {
        len--;
    }

    return String(*this, 0, len);
}

String String::trim(const String& in_sWhat) const
{
    // Could be optimized if it turns out to be a bottleneck, which I guess it won't
    return this->trimLeft(in_sWhat).trimRight(in_sWhat);
}

String String::lower() const
{
    std::string copy = this->str();
    std::transform(copy.begin(), copy.end(), copy.begin(), ::tolower);
    return String(copy);
}

String String::upper() const
{
    std::string copy = this->str();
    std::transform(copy.begin(), copy.end(), copy.begin(), ::toupper);
    return String(copy);
}

std::string::size_type String::find(const char *in_pszNeedle, std::string::size_type in_nIdx) const
{
    if(in_pszNeedle == NULL || in_pszNeedle[0] == '\0')
        return std::string::npos;

    if(in_nIdx > this->len() || this->empty())
        return std::string::npos;

    // Can't be a oneliner because p points into the temporary.
    String substr(*this, in_nIdx);
    const char *p = strstr(substr.c_str(), in_pszNeedle);

    if(p == nullptr)
        return std::string::npos;

    return this->len() - String(p).len();
}

std::string::size_type String::find(const String& in_sNeedle, std::string::size_type in_nIdx) const
{
    return this->find(in_sNeedle.c_str(), in_nIdx);
}

bool String::ieq(const char *in_pszOther) const
{
    return this->ieq(String(in_pszOther));
}

bool String::ieq(const String& in_sOther) const
{
    return in_sOther.lower() == this->lower();
}

bool String::neq(const char *in_pszNeedle, std::string::size_type in_nChars) const
{
    String needle(in_pszNeedle, 0, in_nChars);
    return String(*this, 0, needle.len()) == needle;
}

bool String::neq(const String& in_sNeedle, std::string::size_type in_nChars) const
{
    return this->neq(in_sNeedle.c_str(), in_nChars);
}

bool String::nieq(const char *in_pszNeedle, std::string::size_type in_nChars) const
{
    return this->lower().neq(String(in_pszNeedle).lower(), in_nChars);
}

bool String::nieq(const String& in_sNeedle, std::string::size_type in_nChars) const
{
    return this->lower().neq(in_sNeedle.lower(), in_nChars);
}

bool String::contains(const char *in_pszNeedle) const
{
    return std::string::npos != this->find(in_pszNeedle);
}

bool String::contains(const String& in_sNeedle) const
{
    return this->contains(in_sNeedle.c_str());
}

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
            if(*str == 0 && *pat == 0) return true; // At the end of both.
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

bool String::matchesPattern(const String& in_sPat) const
{
    return this->matchesPattern(in_sPat.c_str());
}

#ifdef D_STRING_MYSQL
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
    if(in_sString.empty())
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
    if(in_sString.empty())
        return;

    this->operator +=(in_sString.c_str());
}

bool String::operator == (const char *in_pszString) const
{
    if(in_pszString == NULL || in_pszString[0] == '\0') {
        if(this->empty())
            return true;
        return false;
    }

    return m_s == in_pszString;
}

bool String::operator == (const String& in_sString) const
{
    if(in_sString == NULL || in_sString.getCharAt(0) == '\0') {
        if(this->empty())
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
    return this->empty();
}

bool String::operator <(const char *in_pszString) const
{
    if(in_pszString == NULL || in_pszString[0] == '\0') {
        if(this->empty())
            return false; // False, because they are equal.
        return false; // Else, the empty string is smaller as anything else.
    }

    return strcmp(this->c_str(), in_pszString) < 0;
}

bool String::operator <(const String& in_sString) const
{
    return this->operator <(in_sString.c_str());
}

unsigned char& String::operator[] (std::string::size_type in_iIndex)
{
    return reinterpret_cast<unsigned char&>(m_s.at(in_iIndex));
}

unsigned char String::getCharAt(std::string::size_type in_iIndex) const
{
    if(this->empty() || in_iIndex >= this->len())
        return 0;

    return *(advance(this->c_str(), in_iIndex));
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

namespace FTS {
    template<>
    bool String::to<bool>() const
    {
        return this->ieq("True");
    }

    template<>
    bool String::toExactly<bool>() const
    {
        if(this->ieq("True")) {
            return true;
        } else if(this->ieq("False")) {
            return false;
        } else {
            throw std::bad_cast();
        }
    }
}

std::ostream& operator<< (std::ostream& os, const FTS::String& me)
{
    return os << me.c_str();
}
