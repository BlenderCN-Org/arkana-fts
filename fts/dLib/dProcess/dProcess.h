#ifndef D_PROCESS_H
#define D_PROCESS_H

#include "main.h"

int spawnv_sync( const char *in_pszExecutable, const char *in_ppszArgs[] );
int spawnp_sync( const char *in_pszExecutable, const char *in_ppszArgs );

int spawnv_async( const char *in_pszExecutable, const char *in_ppszArgs[] );
int spawnp_async( const char *in_pszExecutable, const char *in_ppszArgs );

#endif /* D_PROCESS_H */

 /* EOF */
