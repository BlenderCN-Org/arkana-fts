/**
 * \file utilities.cpp
 * \author Pompei2
 * \date unknown (very old)
 * \brief This file implements general functions.
 **/

#include <thread>
#include <chrono>

#include "utilities.h"

/// \TODO: This is a really bad fix, but for now it does. Have no other idea
///        right now. Maybe in the cmake file we need to differ preinstalled
///        SDL and the SDL that we ship.
#if WINDOOF
#include <SDL_timer.h>
#else
#include <SDL/SDL_timer.h>
#endif

#include "logging/logger.h"
#include "dLib/dString/dTranslation.h"
#include "tools/server2/checksum/md5.h"
#include "tools/server2/checksum/sha2.h"

using namespace FTS;

/// Sleeps the thread for a certain amount of time.
/** This function only returns after a certain amount of milliseconds.
 *
 * \param in_ulMilliseconds The amount of msec to sleep.
 *
 * \author Pompei2
 */
void FTS::dSleep(unsigned long in_ulMilliseconds)
{
    std::this_thread::sleep_for( std::chrono::milliseconds( in_ulMilliseconds ) );
}

uint32_t FTS::dGetTicks()
{
    return SDL_GetTicks();
}

DateTime FTS::DateTime::EMPTY;

FTS::DateTime::DateTime()
{
    m_iYear = 1;
    m_ucMonth = 1;
    m_ucDay = 1;
    m_ucHour = 0;
    m_ucMinute = 0;
    m_ucSecond = 0;
    m_usMs = 0;
    m_bIsSet = false;
}

FTS::DateTime::DateTime(int in_iYear, unsigned char in_ucMonth, unsigned char in_ucDay)
{
    this->setYear(in_iYear);
    this->setMonth(in_ucMonth);
    this->setDay(in_ucDay);
    m_ucHour = 0;
    m_ucMinute = 0;
    m_ucSecond = 0;
    m_usMs = 0;
    m_bIsSet = true;
}

FTS::DateTime::DateTime(unsigned char in_ucHour, unsigned char in_ucMinute,
                          unsigned char in_ucSecond, unsigned short in_usMs)
{
    m_iYear = 1;
    m_ucMonth = 1;
    m_ucDay = 1;
    this->setHour(in_ucHour);
    this->setMinute(in_ucMinute);
    this->setSecond(in_ucSecond);
    this->setMs(in_usMs);
    m_bIsSet = true;
}

FTS::DateTime::DateTime(int in_iYear, unsigned char in_ucMonth,
                          unsigned char in_ucDay, unsigned char in_ucHour,
                          unsigned char in_ucMinute, unsigned char in_ucSecond,
                          unsigned short in_usMs)
{
    this->setYear(in_iYear);
    this->setMonth(in_ucMonth);
    this->setDay(in_ucDay);
    this->setHour(in_ucHour);
    this->setMinute(in_ucMinute);
    this->setSecond(in_ucSecond);
    this->setMs(in_usMs);
    m_bIsSet = true;
}

String FTS::DateTime::toInternStr() const
{
    return String::nr(this->getYear())+"-"+String::nr(this->getMonth())+"-"+String::nr(this->getDay())
      +" "+String::nr(this->getHour())+":"+String::nr(this->getMinute())+":"+String::nr(this->getSecond())+":"+String::nr(this->getMs());
}

int FTS::DateTime::fromInternStr(const String &in_str)
{
    int i = 0, j = 0, k = 0, l = 0, m = 0, n = 0, o = 0;
    int count = sscanf(in_str.c_str(), "%d%d%d %d:%d:%d:%d", &i, &j, &k, &l, &m, &n, &o );
    if( count == 7) {
        this->setYear(i);
        this->setMonth(-j);
        this->setDay(-k);
        this->setHour(l);
        this->setMinute(m);
        this->setSecond(n);
        this->setMs(o);
    } else if( count == 6 ) {
        this->setYear(i);
        this->setMonth(-j);
        this->setDay(-k);
        this->setHour(l);
        this->setMinute(m);
        this->setSecond(n);
        this->setMs(0);
    } else {
        assert(false);
    }
    return ERR_OK;
}

String FTS::DateTime::toStr() const
{
    struct tm time;
    const char *psz = NULL;

    time.tm_sec = this->getSecond();
    time.tm_min = this->getMinute();
    time.tm_hour = this->getHour();
    time.tm_mday = this->getDay();
    time.tm_mon = this->getMonth();
    time.tm_year = /*this->getYear()*/100;
    time.tm_wday = 0;
    time.tm_yday = 0;
    time.tm_isdst = -1;

    // Calculate the wday and yday.
    mktime(&time);
    psz = asctime(&time);

    // Ignore the year.
    String s(psz, 0, 20);

    // Add the year again, but don't use a negative number, prefer using a suffix.
    s += String::nr(std::abs(this->getYear()));
    if(this->getYear()<0) {
        Translation trans("ui");
        s += trans.get("General_ac");
    }

    return s;
}

/// Calculates the sha256 checksum of a string.
/** This calculates the sha256 checksum of a string and stores it
 *  into another string, that is exactly 64 chars long and contains
 *  only the chars 0-9a-f.
 *
 * \param in_pszData The string (or data) you want to encode.
 * \param in_iLen    The length of in_pszData.
 * \param out_pszSHA A buffer that can hold at least 64 chars.
 *
 * \return If successfull: ERR_OK.
 * \return If failed:      Error code < 0
 *
 * \example The best way to call this function is like this:
 *      \code
 *         char data[21] = "blabla data to encode";
 *         char pszSHA[64];
 *         shaEncode( data, 21, pszSHA );
 *         pszSHA == "f89d26241014e3283fd6894e36d4983de9fdee32151336953bfdb5a155ff3a4a"
 *      \endcode
 *
 * \author Pompei2
 */
int FTS::shaEncode(const char *in_pszData, int in_iLen, char *out_pszSHA)
{
    unsigned char buf[33];
    char conversionBuf[3];

    sha256((unsigned char *)in_pszData, in_iLen, buf);

    for(int i = 0; i < 32; i++) {
        // I first format this into a buffer ...
        if(buf[i] < 0x10)
            snprintf(conversionBuf, 3, "0%x", buf[i]);
        else
            snprintf(conversionBuf, 3, "%x", buf[i]);

        // And then copy the two chars of the buffer into the string.
        // This is because the printf family ALWAYS adds a trailing 0, what we don't want !
        out_pszSHA[i * 2] = conversionBuf[0];
        out_pszSHA[i * 2 + 1] = conversionBuf[1];
    }

    return 0;
}

/// Calculates the MD5 checksum of a string.
/** This calculates the MD5 checksum of a string and stores it
 *  into another string, that is exactly 32 chars long and contains
 *  only the chars 0-9a-f.
 *
 * \param in_pszData The string (or data) you want to encode.
 * \param in_iLen    The length of in_pszData.
 * \param out_pszSHA A buffer that can hold at least 32 chars.
 *
 * \return If successfull: ERR_OK.
 * \return If failed:      Error code < 0
 *
 * \example The best way to call this function is like this:
 *      \code
 *         char data[21] = "blabla data to encode";
 *         char pszMD5[32];
 *         shaEncode( data, 21, pszMD5 );
 *         pszMD5 == "683f7780be1e4db988150a71689feefc"
 *      \endcode
 *
 * \author Pompei2
 */
int FTS::md5Encode(const char *in_pszData, int in_iLen, char *out_pszMD5)
{
    unsigned char buf[16];
    char conversionBuf[3];

    md5_csum((unsigned char *)in_pszData, in_iLen, buf);

    for(int i = 0; i < 16; i++) {
        // I first format this into a buffer ...
        if(buf[i] < 0x10)
            snprintf(conversionBuf, 3, "0%x", buf[i]);
        else
            snprintf(conversionBuf, 3, "%x", buf[i]);

        // And then copy the two chars of the buffer into the string.
        // This is because the printf family ALWAYS adds a trailing 0, what we don't want !
        out_pszMD5[i * 2] = conversionBuf[0];
        out_pszMD5[i * 2 + 1] = conversionBuf[1];
    }

    return 0;
}

/// Calculates the skill percent of a player.
/** This returns the skill percent of a player, wich is calculated
 *  dynamically based on his wins, looses, draws and number of games.
 *
 * \param in_nWins   The number of games the player won.
 * \param in_nDraws  The number of games the player did a draw.
 * \param in_nLooses The number of games the player lost.
 *
 * \return The skill percent of the player.
 *
 * \author Pompei2
 */
float FTS::skillPercent(int in_nWins, int in_nDraws, int in_nLooses)
{
    int nGames = in_nWins + in_nDraws + in_nLooses;

    if(nGames <= 0)
        return 0;

    return (float)in_nWins / (float)nGames;
}

/// Calculates the color that corresponds to a skill percent.
/** This calculates a color that represents a certain skill percent value.
 *
 * \param in_fSkillPercent The skillPercent to get the color.
 * \param out_fR The amount of red in the color (from 0.0 -> 1.0)
 * \param out_fG The amount of green in the color (from 0.0 -> 1.0)
 * \param out_fB The amount of blue in the color (from 0.0 -> 1.0)
 * \param out_fA The amount of alpha in the color (from 0.0 -> 1.0)
 *
 * \author Pompei2
 */
void FTS::skillPercentColor(float in_fSkillPercent,
                            float & out_fR, float & out_fG,
                            float & out_fB, float & out_fA)
{
    float r1 = 0.00f, g1 = 0.13f, b1 = 0.3f, a1 = 1.0f;
    float r0 = 0.75f, g0 = 0.92f, b0 = 1.0f, a0 = 1.0f;

    out_fR = r0 + (r1 - r0) * in_fSkillPercent;
    out_fG = g0 + (g1 - g0) * in_fSkillPercent;
    out_fB = b0 + (b1 - b0) * in_fSkillPercent;
    out_fA = a0 + (a1 - a0) * in_fSkillPercent;

    return ;
}

/* EOF */
