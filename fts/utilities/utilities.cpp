/**
 * \file utilities.cpp
 * \author Pompei2
 * \date unknown (very old)
 * \brief This file implements general functions.
 **/

#include "utilities.h"

#include "logging/logger.h"
#include "dLib/dString/dTranslation.h"
#include "tools/server2/checksum/md5.h"
#include "tools/server2/checksum/sha2.h"

using namespace FTS;


/// Calculates the sha256 checksum of a string.
/** This calculates the sha256 checksum of a string and stores it
 *  into another string, that is exactly 64 chars long and contains
 *  only the chars 0-9a-f.
 *
 * \param in_pszData The string (or data) you want to encode.
 * \param in_iLen    The length of in_pszData.
 * \param out_pszSHA A buffer that can hold at least 64 chars.
 *
 * \return If successful: ERR_OK.
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
