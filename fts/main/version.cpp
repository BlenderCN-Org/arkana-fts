#include "version.h"
#include "revision.h"

#define MAJ 0
#define MIN 0
#define REL 4

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

/// \return The revision version.
int getRevisionVersion()
{
    int iRev = 0;
    std::istringstream ss(D_FTS_REVISION_STR);
    ss >> iRev;
    return iRev;
}

/// \return A string that looks like this: "0.1.5.2567"
std::string getFTSVersionString()
{
    std::ostringstream ss;
    ss << getMajorVersion() << "." << getMinorVersion() << "." << getReleaseVersion() << "." << getRevisionVersion();
    return ss.str();
}

#  define FTS_MK_VERSION_UINT32(a,b,c) ((uint32_t)((uint32_t)(a) * (uint32_t)100000000) + \
                                        (uint32_t)((uint32_t)(b) * (uint32_t)100000) +   \
                                        (uint32_t)((uint32_t)(c) * (uint32_t)1)       \
                                       )
#  define FTS_MK_VERSION_UINT64(a,b,c,d) ((uint64_t)((uint64_t)(a) * (uint64_t)1000000000*(uint64_t)1000000) + \
                                          (uint64_t)((uint64_t)(b) * (uint64_t)1000000000*(uint64_t)1000) +   \
                                          (uint64_t)((uint64_t)(c) * (uint64_t)1000000000) +     \
                                          (uint64_t)((uint64_t)(d) * (uint64_t)1)            \
                                         )

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

uint64_t getFTSVersionUInt64()
{
    return makeFTSVersionUInt64(getMajorVersion(), getMinorVersion(), getReleaseVersion(), getRevisionVersion());
}

 /* EOF */
