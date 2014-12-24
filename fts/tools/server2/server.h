#ifndef D_SERVER_H
#define D_SERVER_H

#include <stdarg.h>

#ifdef D_COMPILES_SERVER
#if defined(_MSC_VER)
//#  include <winsock.h>
//#  include <mysql.h>

#  include <fcntl.h>
#  include <io.h>
//#  define open _open
//#  define close _close
#else
#  include <mysql/mysql.h>
#  include <pthread.h>

#  include <sys/select.h>
#  include <sys/socket.h>
#  include <arpa/inet.h>
#  include <sys/types.h>
#  include <stdint.h>
#  include <stdlib.h>
#  include <string.h>
#  include <unistd.h>
#  include <resolv.h>
#  include <netdb.h>
#  include <fcntl.h>
#  include <errno.h>
#  include <malloc.h>
#endif
#  include "dLib/dString/dString.h"

#  include "../toolcompat.h"

// Some global variables.
namespace FTS {
    class Mutex;
}

extern bool g_bVerbose;
extern FTS::Mutex g_bVerboseMutex;
extern bool g_bExit;
#endif                        /* D_COMPILES_SERVER */

#include "constants.h"
#endif                          /* D_SERVER_H */
