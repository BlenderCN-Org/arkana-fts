/**
 * \file utilities.h
 * \author Pompei2
 * \date 01 May 2006
 * \brief This file contains all the util functions that go nowhere else.
 **/

#ifndef FTS_UTILITIES_H
#define FTS_UTILITIES_H

#include "main.h"
#include <cstdlib> // for std::abs

#include "dLib/dString/dString.h"

namespace FTS {
    class File;

/* ==================== */
/* CONSOLE MANIPULATION */
/* ==================== */
typedef enum {
    D_BLACK = 0,                /* Dark colors. */
    D_DARKRED,
    D_DARKGREEN,
    D_DARKYELLOW,
    D_DARKBLUE,
    D_DARKMAGENTA,
    D_DARKCYAN,
    D_LIGHTGRAY,                /* Bright colors. */
    D_GRAY,
    D_RED,
    D_GREEN,
    D_YELLOW,
    D_BLUE,
    D_MAGENTA,
    D_CYAN,
    D_WHITE
} D_COLOR;

typedef enum {
    D_NORMAL = 0,
    D_BOLD,
    D_ULINE,
    D_BLINK,
    D_REVERSE,
    D_INVIS,
    D_CHANGEBG,
    D_CHANGEFG
} D_CONSOLEATTRIBUTE;

int ConsAttr(D_CONSOLEATTRIBUTE in_Action, ...);
int ForegroundConsole(bool bFore);
int EnableUTF8Console();

/* ================= */
/* TEXT FILE PARSING */
/* ================= */

class CParser {
private:
    FTS::String m_sFile;       ///< The name of the file we are parsing.
    char *m_pData;              ///< The whole file is stored here.
    char *m_p;                  ///< Points at the point in pData we're at.
    size_t m_nLine;             ///< The number of the current line.
    size_t m_nPos;              ///< The current position of the 'cursor'.
    bool m_bEOF;                ///< We came to the end of the file.

public:
    CParser(void);
    CParser(const FTS::String & in_sFileName);
    virtual ~ CParser(void);

    int load(const FTS::String & in_sFileName = FTS::String::EMPTY);
    int load(const FTS::File &in_f);
    int loadStr(const FTS::String & in_sString);
    int unload(void);

    size_t getPos(void) const;
    size_t getLine(void) const;
    FTS::String getFile(void) const;
    bool isEOF(void) const;

    const char *getData(void);
    const char *getCursor(void);

    size_t skipLine(void);
    size_t skipSpaces(void);
    size_t skipComments(void);

    int jumpToLineBeginning(const FTS::String & in_sBegin);
    int jumpToLineBeginningLabel(const FTS::String & in_sBegin);
    int parse(FTS::String in_sFmt, ...);
};

/* ============= */
/* Date and Time */
/* ============= */
class DateTime {
private:
    int m_iYear;
    unsigned char m_ucMonth;
    unsigned char m_ucDay;
    unsigned char m_ucHour;
    unsigned char m_ucMinute;
    unsigned char m_ucSecond;
    unsigned short m_usMs;

    bool m_bIsSet;
public:
    DateTime();
    DateTime(int in_iYear, unsigned char in_ucMonth,
             unsigned char in_ucDay);
    DateTime(unsigned char in_ucHour, unsigned char in_ucMinute,
             unsigned char in_ucSecond, unsigned short in_usMs = 0);
    DateTime(int in_iYear, unsigned char in_ucMonth,
             unsigned char in_ucDay, unsigned char in_ucHour,
             unsigned char in_ucMinute, unsigned char in_ucSecond,
             unsigned short in_usMs = 0);

    inline ~DateTime() {};

    inline int            getYear  () const { return this->isSet() ? m_iYear : 0; };
    inline unsigned char  getMonth () const { return this->isSet() ? m_ucMonth : 0; };
    inline unsigned char  getDay   () const { return this->isSet() ? m_ucDay : 0; };
    inline unsigned char  getHour  () const { return this->isSet() ? m_ucHour : 0; };
    inline unsigned char  getMinute() const { return this->isSet() ? m_ucMinute : 0; };
    inline unsigned char  getSecond() const { return this->isSet() ? m_ucSecond : 0; };
    inline unsigned short getMs    () const { return this->isSet() ? m_usMs : 0; };
    inline bool isSet() const {return m_bIsSet;};
    inline void empty() {m_iYear = 0;m_ucMonth = m_ucDay = m_ucHour = m_ucMinute = m_ucSecond = 0; m_usMs = 0; m_bIsSet = false;};

    inline void setYear(int value) { m_iYear = std::abs(value) < 2147483641 ? value : 1; m_bIsSet = std::abs(value) < 2147483641; };
    inline void setMonth(unsigned char value) { if(value > 0 && value < 13) {m_ucMonth = value; m_bIsSet = true;} };
    inline void setDay(unsigned char value) { if(value > 0 && value < 32) {m_ucDay = value; m_bIsSet = true;} };
    inline void setHour(unsigned char value) { if(value > 0 && value < 25) {m_ucHour = value; m_bIsSet = true;} };
    inline void setMinute(unsigned char value) { if(value > 0 && value < 60) {m_ucMinute = value; m_bIsSet = true;} };
    inline void setSecond(unsigned char value) { if(value > 0 && value < 60) {m_ucSecond = value; m_bIsSet = true;} };
    inline void setMs(unsigned short value) { if(value > 0 && value < 1000) {m_usMs = value; m_bIsSet = true;} };

    int fromInternStr(const FTS::String &in_str);
    FTS::String toInternStr() const;
    FTS::String toStr() const;

    static DateTime EMPTY;
};

/* ============= */
/* MISCELLIANOUS */
/* ============= */

int shaEncode(const char *in_pszData, int in_iLen, char *out_pszSHA);
int md5Encode(const char *in_pszData, int in_iLen, char *out_pszMD5);
float skillPercent(int in_nWins, int in_nDraws, int in_nLooses);
void skillPercentColor(float skillPercent,
                       float & out_fR, float & out_fG,
                       float & out_fB, float & out_fA);
void dSleep(unsigned long in_ulMS);
uint32_t dGetTicks();

void RadixSort11(float *in_pfArray, float *out_ppSorted, unsigned long in_nElements);

} // namespace FTS

#endif                          /* FTS_UTILITIES_H */

/* EOF */
