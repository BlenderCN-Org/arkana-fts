#ifndef D_MAIN_H
#  define D_MAIN_H

#  define _CRT_SECURE_NO_DEPRECATE

#  include "main/defines.h"
#  include "main/support.h"
#  include "dLib/dProcess/dProcess.h"
#  include "log.h"

#  include <malloc.h>
#  include <stdio.h>
#  include <string.h>
#  include <stdarg.h>
#  include <stdlib.h>

#  if WINDOOF
#    include <process.h>
#  endif

#  include <sys/types.h>
#  include <sys/stat.h>

#  include <SDL/SDL.h>
#  include <SDL/SDL_opengl.h>
#  include <GL/gl.h>
#  include <RendererModules/OpenGLGUIRenderer/openglrenderer.h>
#  include <CEGUI.h>
#  include <CEGUIDefaultResourceProvider.h>
using namespace CEGUI;

#  define _FTPLIB_NO_COMPAT
#  include <ftplib.h>

#  define D_USER  "fts"
#  define D_PASS  "1fts5admin0"
#  define BUFSIZE 102400

#  if WINDOOF
#    define D_FILE_LIST "v.i2"
#  else
#    define D_FILE_LIST "v.i2.lin"
#  endif

#  define FTS_MK_VERSION_INT(a,b,c)	((int)((a) * 10000) + \
                                         (int)((b) * 100) +   \
                                         (int)((c) * 1)       \
                                        )
#  define FTS_MK_VERSION_LONG(a,b,c,d)	((long)((a) * 1000000000) + \
                                         (long)((b) * 10000000) +   \
                                         (long)((c) * 100000) +     \
                                         (long)((d) * 1)            \
                                        )
class CUIActionList;

extern CUIActionList *g_pActions;
extern CEGUI::Window * g_rootWin;
extern netbuf *g_connection;
extern bool g_updateable;
extern bool g_bVerbose;
extern int g_version[4];

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

#  define SAFE_FREE(p) { if( (p) ) { free( (p) ); (p) = NULL; } }

/* crc32.cpp */
int crc_list_make(FILE * pF, char *pszRootDir);
unsigned long crc_list_get_total_size(char *pszList);
unsigned long CRC32_Fichier(char *NomFichier,
                            unsigned long *TailleFichier);

/* conf.cpp */
bool confExistsInt(char *in_pszFile, char *in_pszName);
bool confExistsStr(char *in_pszFile, char *in_pszName);
bool confExistsBool(char *in_pszFile, char *in_pszName);

#endif                          /* D_MAIN_H */

 /* EOF */
