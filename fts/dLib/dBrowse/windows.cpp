#define _CRT_SECURE_NO_DEPRECATE

#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include "dBrowse.h"

PDBrowseInfo dBrowse_Open(const FTS::String &in_sDir, bool in_bOpenRootOnFailure)
{
    PDBrowseInfo dbi = NULL;

    dbi = new SDBrowseInfo;
    dbi->bFirst = true;
//    dbi->fd;
    dbi->h  = NULL;

    dbi->currDir = in_sDir;
    if(!dbi->currDir) {
        SAFE_DELETE(dbi);
        return NULL;
    }

    return dbi;
}

int dBrowse_Close(PDBrowseInfo dbi)
{
    if(NULL == dbi)
        return -1;

    FindClose(dbi->h);
    SAFE_DELETE(dbi);

    return 0;
}

FTS::String dBrowse_DoGetNextInitial( PDBrowseInfo dbi )
{
    FTS::String sTmp;

    /* Add the "*" at the end. */
    int i = dbi->currDir.len( );
    if( FTS_IS_DIR_SEPARATOR(dbi->currDir[i-1]) )
        sTmp = dbi->currDir + "*";
    else
        sTmp = dbi->currDir + FTS_DIR_SEPARATOR "*";

    dbi->h = FindFirstFile( sTmp.c_str( ), &dbi->fd );
    if( dbi->h == INVALID_HANDLE_VALUE )
        return FTS::String::EMPTY;

    dbi->bFirst = false;
    return FTS::String(dbi->fd.cFileName);
}

FTS::String dBrowse_GetNext( PDBrowseInfo dbi )
{
    if( NULL == dbi )
        return FTS::String::EMPTY;

    if( dbi->bFirst ) {
        return dBrowse_DoGetNextInitial( dbi );
    } else {
        if( FindNextFile( dbi->h, &dbi->fd ) == 0 )
            return FTS::String::EMPTY;
        if( dbi->h == INVALID_HANDLE_VALUE )
            return FTS::String::EMPTY;
        return FTS::String(dbi->fd.cFileName);
    }
}

FTS::String dBrowse_DoGetNextWWInitial( PDBrowseInfo dbi, const FTS::String &wc )
{
    FTS::String sTmp;

    /* Add the "*" at the end. */
    int i = dbi->currDir.len( );
    if( FTS_IS_DIR_SEPARATOR(dbi->currDir[i-1]) )
        sTmp = dbi->currDir + "*";
    else
        sTmp = dbi->currDir + FTS_DIR_SEPARATOR "*";

    dbi->h = FindFirstFile( sTmp.c_str( ), &dbi->fd );
    if( dbi->h == INVALID_HANDLE_VALUE )
        return FTS::String::EMPTY;

    dbi->bFirst = false;
    if(FTS::String(dbi->fd.cFileName).matchesPattern(wc))
        return FTS::String(dbi->fd.cFileName);

    return FTS::String::EMPTY;
}

FTS::String dBrowse_GetNextWithWildcard( PDBrowseInfo dbi, const FTS::String &wc )
{
    FTS::String ret;

    if( NULL == dbi )
        return ret;

    if( dbi->bFirst ) {
        ret = dBrowse_DoGetNextWWInitial( dbi, wc );

        if( !ret.empty( ) )
            return ret;
    }

    while( FindNextFile( dbi->h, &dbi->fd ) != 0 ) {
        if( dbi->h == INVALID_HANDLE_VALUE )
            return FTS::String::EMPTY;
        if(FTS::String(dbi->fd.cFileName).matchesPattern(wc))
            return FTS::String(dbi->fd.cFileName);
    }

    return FTS::String::EMPTY;
}

int dBrowse_GetType( PDBrowseInfo dbi )
{
    if( NULL == dbi )
        return DB_ERR;

    if( dbi->currDir == NULL )
        return DB_ERR;

    if( dbi->fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
        return DB_DIR;
    else if( dbi->fd.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE )
        return DB_TODO;
    else
        return DB_FILE;
}
