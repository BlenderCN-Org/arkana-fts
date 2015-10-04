#ifndef D_BROWSE_H
#define D_BROWSE_H

#include "main.h"

#if WINDOOF
#  include <windows.h>
#else
#  include <dirent.h>
#endif

#include "dLib/dString/dString.h"

typedef struct _SDBrowseInfo {
    FTS::String currDir;
#if WINDOOF
    WIN32_FIND_DATA fd;
    HANDLE          h;
    bool            bFirst;
#else
    DIR *dir;
    dirent *dent;
#endif
} SDBrowseInfo, *PDBrowseInfo;

#define DB_ERR  -1
#define DB_TODO 0
#define DB_DIR  1
#define DB_FILE 2

PDBrowseInfo dBrowse_Open(const FTS::String &in_sDir, bool in_bOpenRootOnFailure = false);
FTS::String dBrowse_GetNext(PDBrowseInfo dbi);
FTS::String dBrowse_GetNextWithWildcard(PDBrowseInfo dbi, const FTS::String &wc);
int dBrowse_Close(PDBrowseInfo dbi);
int dBrowse_GetType(PDBrowseInfo dbi);

#endif

 /* EOF */
