/**
 * \file main.h
 * \author Pompei2
 * \date 18 July 2006
 * \brief This file contains some default includes, the runlevel functions and THE ONE GLOBAL variable.
 **/

#ifndef D_MAIN_H
#  define D_MAIN_H

/* -------------------------- */
/* Includes                   */
/* -------------------------- */
#  include "main/support.h"
#  include "main/defines.h"
#  include "main/workarounds.h"

#ifdef D_ARKANA_TESTING
#  ifndef GAME_MAIN
#    define GAME_MAIN mainGame
#  endif // GAME_MAIN
#  ifndef TEST_MAIN
#    define TEST_MAIN main
#  endif // TEST_MAIN
#else
#  ifndef GAME_MAIN
#    define GAME_MAIN main
#  endif // GAME_MAIN
#  ifndef TEST_MAIN
#    define TEST_MAIN mainTest
#  endif // TEST_MAIN
#endif

#  define _USE_MATH_DEFINES // needed for math constants like M_PI (under windows)
#  include <sys/stat.h>
#  include <string.h>
#  include <stdlib.h>
#  include <stdarg.h>
#  include <stdio.h>
#  include <ctype.h>
#  include <errno.h>
#  include <time.h>
#  include <math.h>
#  include <cstdint>

#  if WINDOOF
#    include <windows.h>
#    if !defined(_MSC_VER)
#      include <unistd.h>
#    else
#      include <direct.h>
#    endif
#  endif

#  include "dLib/dMem/dMem.h"

#endif                          /* D_MAIN_H */

 /* EOF */
