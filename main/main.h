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
#  include <cstdint>
#  include "main/defines.h"

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

#  if WINDOOF
#    include <windows.h>
#  endif

#  include "dLib/dMem/dMem.h"

#endif                          /* D_MAIN_H */

 /* EOF */
