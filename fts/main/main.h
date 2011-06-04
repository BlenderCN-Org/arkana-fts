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

// Help KDevelop's parser understand C++0x
#ifdef IN_KDEVELOP_PARSER
#  define __GXX_EXPERIMENTAL_CXX0X__
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

#  if WINDOOF
#    include <windows.h>
#    if !defined(_MSC_VER)
#      include <unistd.h>
#    else
#      include <direct.h>
#    endif
#  endif

#  include "dLib/dMem/dMem.h"

/* ------------------------------- */
/* Workarounds for older compilers */
/* ------------------------------- */
// GCC 4.5 nullptr workaround
#if defined(__GNUC__) && (__GNUC__ == 4 && __GNUC_MINOR__ == 5)

class nullptr_t {
public:
    template<class T>        // convertible to any type
    operator T*() const      // of null non-member
    { return 0; }            // pointer...

    template<class C, class T> // or any type of null
    operator T C::*() const    // member pointer...
    { return 0; }

private:
    void operator&() const;    // whose address can't be taken
};

const nullptr_t nullptr = {};

template<class T>
bool operator == (T t, nullptr_t) { return !t; }
template<class T>
bool operator == (nullptr_t, T t) { return !t; }
template<class T>
bool operator != (T t, nullptr_t) { return !!t; }
template<class T>
bool operator != (nullptr_t, T t) { return !!t; }

#endif // GCC 4.5 nullptr workaround

// GCC < 4.5 nullptr workaround
#if defined(__GNUC__) && (__GNUC__ == 4 && __GNUC_MINOR__ < 5)
const                        // this is a const object...
class {
public:
    template<class T>          // convertible to any type
    operator T*() const      // of null non-member
    { return 0; }            // pointer...

    template<class C, class T> // or any type of null
    operator T C::*() const  // member pointer...
    { return 0; }

private:
    void operator&() const;    // whose address can't be taken
} nullptr = {};              // and whose name is nullptr

#endif // GCC < 4.5 nullptr workaround

#endif                          /* D_MAIN_H */

 /* EOF */
