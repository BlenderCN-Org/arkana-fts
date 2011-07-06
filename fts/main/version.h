/**
 * \file version.h
 * \author Pompei2
 * \date 19 July 2006
 * \brief This file defines all the constants relative to the versionning. take care when including this file ! Your code that includes it will be recompiled EVERY time ! (to update FTS_VERSION_BUILD)
 **/

#ifndef FTS_VERSION_H
#define FTS_VERSION_H

#include "main/defines.h"
#include <string>

int getMajorVersion();
int getMinorVersion();
int getReleaseVersion();

std::string getFTSVersionString();

uint32_t getFTSVersionUInt32();
uint32_t makeFTSVersionUInt32(int iMaj, int iMin, int iRel);
uint64_t makeFTSVersionUInt64(int iMaj, int iMin, int iRel, int iRev);

#endif

 /* EOF */
