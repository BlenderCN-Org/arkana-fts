#ifndef D_UMAIN_H
#  define D_UMAIN_H

#  include "../../main/defines.h"
#  include "../../main/support.h"
#  include "../../dLib/dProcess/dProcess.h"
#  include "log.h"

#  include <malloc.h>
#  include <stdio.h>
#  include <string.h>
#  include <stdarg.h>
#  include <stdlib.h>

#  include <sys/types.h>
#  include <sys/stat.h>

#  ifdef WINDOOF
#    include <ctype.h>
#  endif

#  define _FTPLIB_NO_COMPAT
#  include <ftplib.h>

#  define D_HOST  "cesar4.be"
#  define D_USER  "fts"
#  define D_PASS  "1fts5admin0"
#  define BUFSIZE 102400

#  if WINDOOF
#    define D_FILE_LIST "vm.i2"
#  else
#    define D_FILE_LIST "vm.i2.lin"
#  endif

extern netbuf *g_connection;

/* Utils.cpp */
char *MyAllocSPrintf(const char *fmt, ...);
long FLength(char *in_pszFileName, char mode);
int replaceStr(char **out_ppszBase, unsigned long in_ulIndex,
               unsigned long in_ulLenght, char *in_pszNew);
bool mkdir_if_needed(char *path, bool bWithFile);
char *FileToBuf(char *pszFile, char mode);
char *FtpFileToBuf(char *pszFile, netbuf * pConnection, char mode);
int ReadInfo(char *pList, char *pszFile, unsigned long *ulSize,
             unsigned long *ulCRC32);

#ifdef SAFE_FREE
#  undef SAFE_FREE
#  define SAFE_FREE(x) { if( (x) ) { free( (x) ); (x) = NULL; } }
#endif

/* crc32.cpp */
int crc_list_make(FILE * pF, char *pszRootDir);
unsigned long crc_list_get_total_size(char *pszList);
unsigned long CRC32_Fichier(char *NomFichier,
                            unsigned long *TailleFichier);

/* conf.cpp */
bool confExistsInt(char *in_pszFile, char *in_pszName);
bool confExistsStr(char *in_pszFile, char *in_pszName);
bool confExistsBool(char *in_pszFile, char *in_pszName);

#endif                          /* D_UMAIN_H */

 /* EOF */
