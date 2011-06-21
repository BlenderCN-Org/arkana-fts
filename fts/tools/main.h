#ifndef FTS_TOOLS_MAIN_H
#define FTS_TOOLS_MAIN_H

#include "main/defines.h"
#include "main/workarounds.h"

#include <time.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <stdarg.h>

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(_WIN32) || defined(_WIN32_) || defined(__CYGWIN__)
#  if 0
#    define MS_VISUAL 1
#  endif
#  define WINDOOF 1
#  define NOMINMAX
#  include <sys/stat.h>
#else
#  define WINDOOF 0
#endif

#include "dLib/dMem/dMem.h"

#if WINDOOF
#  if !defined(_MSC_VER)
#    include <unistd.h>
#  else
#    include <direct.h>
#  endif
#  include <Winsock2.h>
#  include <windows.h>
#  define _USE_MATH_DEFINES // needed for math constants like M_PI

// Gotta work this around with a function cuz a define would be too risked.
inline void close(SOCKET s)
{
    closesocket(s);
    return;
}
#else
#  include <unistd.h>
#  include <pthread.h>
#  include <sys/select.h>
#  include <sys/socket.h>
#  include <sys/types.h>
#  include <arpa/inet.h>
#  include <resolv.h>
#  include <netdb.h>
#  include <fcntl.h>
#endif

#endif /* FTS_TOOLS_MAIN_H */
