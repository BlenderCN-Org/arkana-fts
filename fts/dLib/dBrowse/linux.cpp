#include <sys/stat.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include "dBrowse.h"

#include "main/defines.h"

#ifdef USE_STD_ALLOC
PDBrowseInfo dBrowse_Open( const char *in_sDir, bool in_bOpenRootOnFailure )
#else
PDBrowseInfo dBrowse_Open( const FTS::String &in_sDir, bool in_bOpenRootOnFailure )
#endif
{
    PDBrowseInfo dbi = NULL;

#ifdef USE_STD_ALLOC
    dbi = (PDBrowseInfo)malloc( sizeof(SDBrowseInfo) );
    if( !dbi ) {
        fprintf( stderr, "error: not enough memory !\n" );
    }
#else
    dbi = new SDBrowseInfo;
#endif
    if( NULL == dbi )
        return NULL;

    dbi->dir = NULL;
    dbi->dent = NULL;

#ifdef USE_STD_ALLOC
    dbi->dir = opendir( in_sDir );
#else
    dbi->dir = opendir( in_sDir.c_str( ) );
#endif
    if( dbi->dir == NULL ) {
        if(!in_bOpenRootOnFailure) {
#ifdef USE_STD_ALLOC
            if( dbi ) free( dbi );
            dbi = NULL;
#else
            SAFE_DELETE(dbi);
#endif
            return NULL;
        }
        dbi->dir = opendir( "/" );
        if( dbi->dir == NULL ) {
#ifdef USE_STD_ALLOC
            if( dbi ) free( dbi );
            dbi = NULL;
#else
            SAFE_DELETE(dbi);
#endif
            return NULL;
        }

#ifdef USE_STD_ALLOC
        dbi->currDir = (char *)malloc( strlen("/") + 1 );
        strcpy( dbi->currDir, "/" );
#else
        dbi->currDir = "/";
#endif
    } else {
#ifdef USE_STD_ALLOC
        dbi->currDir = (char *)malloc( strlen(in_sDir) + 1 );
        strcpy( dbi->currDir, in_sDir );
#else
        dbi->currDir = in_sDir;
#endif
    }
    if( NULL == dbi->currDir ) {
#ifdef USE_STD_ALLOC
        if( dbi ) free( dbi );
        dbi = NULL;
#else
        SAFE_DELETE(dbi);
#endif
        return NULL;
    }

    return dbi;
}

int dBrowse_Close( PDBrowseInfo dbi )
{
    if( NULL == dbi )
        return -1;

    closedir( dbi->dir );
#ifdef USE_STD_ALLOC
    if( dbi->currDir ) {
        free( dbi->currDir );
        dbi->currDir = NULL;
    }
#else
    dbi->currDir = FTS::String::EMPTY;
#endif

#ifdef USE_STD_ALLOC
    if( dbi ) free( dbi );
    dbi = NULL;
#else
    SAFE_DELETE(dbi);
#endif

    return 0;
}

#ifdef USE_STD_ALLOC
char *dBrowse_GetNext( PDBrowseInfo dbi )
#else
FTS::String dBrowse_GetNext( PDBrowseInfo dbi )
#endif
{
#ifdef USE_STD_ALLOC
    char *sErrorRet = NULL;
#else
    FTS::String sErrorRet = FTS::String::EMPTY;
#endif

    if( NULL == dbi )
        return sErrorRet;

    dbi->dent = readdir( dbi->dir );
    if( dbi->dent == NULL )
        return sErrorRet;

    /* We ignore the . directory. */
/*    if( !strcmp( dbi->dent->d_name, "." ) )
        return dBrowse_GetNext( dbi );
    else*/
#ifdef USE_STD_ALLOC
        return dbi->dent->d_name;
#else
        return FTS::String(dbi->dent->d_name);
#endif
}

#ifdef USE_STD_ALLOC
char *dBrowse_GetNextWithWildcard( PDBrowseInfo dbi, const char *wc )
#else
FTS::String dBrowse_GetNextWithWildcard( PDBrowseInfo dbi, const FTS::String &wc )
#endif
{
#ifdef USE_STD_ALLOC
    char *sErrorRet = NULL;
#else
    FTS::String sErrorRet = FTS::String::EMPTY;
#endif

    if( NULL == dbi )
        return sErrorRet;

    while( (dbi->dent = readdir( dbi->dir )) != NULL ) {
        /* We ignore the . directory. */
/*        if( !strcmp( dbi->dent->d_name, "." ) ) {
            return dBrowse_GetNextWithWildcard( dbi, wc );
        } else {*/
#ifdef USE_STD_ALLOC
            if( MatchPattern( dbi->dent->d_name, wc ) )
                return dbi->dent->d_name;
#else
            if( FTS::String(dbi->dent->d_name).matchesPattern(wc) )
                return FTS::String(dbi->dent->d_name);
#endif
//         }
    }

    return sErrorRet;
}

int dBrowse_GetType( PDBrowseInfo dbi )
{
    struct stat s;

    if( NULL == dbi )
        return DB_ERR;

    if( dbi->currDir == NULL )
        return DB_ERR;

    /* Concatenate em both (the dire + the file name). */
#ifdef USE_STD_ALLOC
    int i = strlen(dbi->currDir);
    char *pszTemp = NULL;
    if( FTS_IS_DIR_SEPARATOR(dbi->currDir[i-1]) ) {
        pszTemp = (char *)malloc( i + strlen(dbi->dent->d_name) + 1 );
        if( NULL == pszTemp ) {
            fprintf( stderr, "error: not enough memory !\n" );
            return DB_ERR;
        }
        if( 0 > sprintf( pszTemp, "%s%s", dbi->currDir, dbi->dent->d_name ) )
            return DB_ERR;
    } else {
        pszTemp = (char *)malloc( i + strlen(dbi->dent->d_name) + 2 );
        if( NULL == pszTemp ) {
            fprintf( stderr, "error: not enough memory !\n" );
            return DB_ERR;
        }
        if( 0 > sprintf( pszTemp, "%s"FTS_DIR_SEPARATOR"%s", dbi->currDir, dbi->dent->d_name ) )
            return DB_ERR;
    }

    if( 0 != lstat( pszTemp, &s ) ) {
        free( pszTemp );
        pszTemp = NULL;
        return DB_ERR;
    }

    free( pszTemp );
    pszTemp = NULL;
#else
    FTS::String sTemp;
    if( FTS_IS_DIR_SEPARATOR( dbi->currDir[dbi->currDir.len( )-1] ) ) {
        sTemp = dbi->currDir + dbi->dent->d_name;
    } else {
        sTemp = dbi->currDir + FTS_DIR_SEPARATOR + FTS::String(dbi->dent->d_name);
    }

    if( 0 != lstat( sTemp.c_str( ), &s ) ) {
#  if DEBUG
#  endif
        return DB_ERR;
    }
#endif

    if( S_ISREG(s.st_mode) )
        return DB_FILE;
    else if( S_ISDIR(s.st_mode) )
        return DB_DIR;
    else
        return DB_TODO;
}
