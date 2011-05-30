#include "dProcess.h"

#if WINDOOF
#  include <process.h>
#else
#  include <spawn.h>
#endif

int spawnv_sync( const char *in_pszExecutable, const char *in_ppszArgs[] )
{
    int pos = 0;
    char pszBuf[1024];
    const char *pCurrent = in_ppszArgs[0];
    while( pCurrent && pos < 1024 ) {
        if( pos + strlen(pCurrent) + 1 > 1024 )
            break;
        strcpy( &pszBuf[pos], pCurrent );
        pos += (int)strlen(pCurrent);
        pszBuf[pos++] = ' ';
        pCurrent++;
    }

    spawnp_sync( in_pszExecutable, pszBuf );
    return ERR_OK;
}

int spawnp_sync( const char *in_pszExecutable, const char *in_ppszArgs )
{
    system( in_pszExecutable );
    return ERR_OK;
}

int spawnv_async( const char *in_pszExecutable, const char *in_ppszArgs[] )
{
#if WINDOOF
    _spawnv( _P_NOWAIT, in_pszExecutable, in_ppszArgs );
#else
    int pid = 0;
    int err = 0;
    char *spawnedEnv[] = { NULL };

    if( 0 != (err = posix_spawn( &pid, in_pszExecutable, NULL, NULL, (char * const *)in_ppszArgs, spawnedEnv)) ) {
        fprintf( stderr, "could not open program: '%s', error code %d", in_pszExecutable, err );
        return -1;
    }
#endif
    return ERR_OK;
}

int spawnp_async( const char *in_pszExecutable, const char *in_ppszArgs )
{
    system( in_pszExecutable );
#if WINDOOF
    _spawnl( _P_NOWAIT, in_pszExecutable, in_ppszArgs );
#else
    int pid = 0;
    int err = 0;
    int argc = 2;
    char *spawnedEnv[] = { NULL };
    char **pArgs = NULL;
    char *pszBuf = NULL;
    char *pCurrentArg = NULL;

    pszBuf = (char *)calloc( 1, strlen(in_ppszArgs)+1 );
    strcpy( pszBuf, in_ppszArgs );

    pCurrentArg = strtok( pszBuf, " \t" );

    /* Count the number of arguments. */
    do {
        if( !pCurrentArg )
            break;
        argc++;
    } while( (pCurrentArg = strtok( NULL, " \t" )) );

    /* Alloc enough memory for the arguments. */
    pArgs = (char **)calloc( sizeof(char *), argc );

    /* The filename as first argument. */
    pArgs[0] = (char *)calloc( 1, strlen(in_pszExecutable)+1 );
    strcpy( pArgs[0], in_pszExecutable );

    /* Now put them into the array. */
    strcpy( pszBuf, in_ppszArgs );
    pCurrentArg = strtok( pszBuf, " \t" );
    for( int i = 1 ; i < argc && pCurrentArg ; i++ ) {
        pArgs[i] = (char *)calloc( 1, strlen(pCurrentArg)+1 );
        strcpy( pArgs[i], pCurrentArg );
        pCurrentArg = strtok( pszBuf, " \t" );
    }

    if( 0 != (err = posix_spawn( &pid, in_pszExecutable, NULL, NULL, pArgs, spawnedEnv)) ) {
        fprintf( stderr, "could not open program: '%s', error code %d", in_pszExecutable, err );
        return -1;
    }

    /* Here we can free the memory used for the args. */
    for( int i = 0 ; i < argc ; i++ )
        free( pArgs[i] );
    free( pArgs );
#endif
    return ERR_OK;
}
