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
#include <cstdint>

#include "dLib/dMem/dMem.h"

#if WINDOOF
#  if defined(_MSC_VER)
#    include <direct.h>
#  endif
#  include <Winsock2.h>
#  include <windows.h>
#else
#  include <sys/select.h>
#  include <sys/socket.h>
#  include <sys/types.h>
#  include <arpa/inet.h>
#  include <resolv.h>
#  include <netdb.h>
#  include <fcntl.h>
#endif

#endif /* FTS_TOOLS_MAIN_H */
