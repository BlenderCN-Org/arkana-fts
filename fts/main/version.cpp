#include "version.h"
#ifndef _WIN32
#  include "revision.h"
#endif

#define MAJ 0
#define MIN 0
#define REL 4

#ifndef D_FTS_FULL_VERSION_STR
#  define STR_HELPER(x) #x
#  define STR(x) STR_HELPER(x)
#  define D_FTS_FULL_VERSION_STR "v" STR(MAJ) "." STR(MIN) "." STR(REL)
#endif

#include <sstream>

/// \return The major FTS version.
int getMajorVersion()
{
    return MAJ;
}

/// \return The minor FTS version, increment it on big steps done.
int getMinorVersion()
{
    return MIN;
}

/// \return The release FTS version, increment it on every new release.
int getReleaseVersion()
{
    return REL;
}

/// \return A string that describes the exact version to a human
std::string getFTSVersionString()
{
    return D_FTS_FULL_VERSION_STR;
}

uint32_t makeFTSVersionUInt32(int iMaj, int iMin, int iRel)
{
    return (uint32_t)((uint32_t)(iMaj) * (uint32_t)100000000) +
           (uint32_t)((uint32_t)(iMin) * (uint32_t)100000) +
           (uint32_t)((uint32_t)(iRel) * (uint32_t)1);
}

uint64_t makeFTSVersionUInt64(int iMaj, int iMin, int iRel, int iRev)
{
    return (uint64_t)((uint64_t)(iMaj) * (uint64_t)100000000*(uint64_t)1000000) +
           (uint64_t)((uint64_t)(iMin) * (uint64_t)100000000*(uint64_t)1000) +
           (uint64_t)((uint64_t)(iRel) * (uint64_t)100000000) +
           (uint64_t)((uint64_t)(iRev) * (uint64_t)1);
}

uint32_t getFTSVersionUInt32()
{
    return makeFTSVersionUInt32(getMajorVersion(), getMinorVersion(), getReleaseVersion());
}

 /* EOF */
