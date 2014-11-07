/**
 * \file utilities.h
 * \author Pompei2
 * \date 01 May 2006
 * \brief This file contains all the util functions that go nowhere else.
 **/

#ifndef FTS_UTILITIES_H
#define FTS_UTILITIES_H

#include "main.h"

namespace FTS {


/* ============= */
/* MISCELLIANOUS */
/* ============= */

int shaEncode(const char *in_pszData, int in_iLen, char *out_pszSHA);
int md5Encode(const char *in_pszData, int in_iLen, char *out_pszMD5);
float skillPercent(int in_nWins, int in_nDraws, int in_nLooses);
void skillPercentColor(float skillPercent,
                       float & out_fR, float & out_fG,
                       float & out_fB, float & out_fA);


} // namespace FTS

#endif                          /* FTS_UTILITIES_H */

/* EOF */
