#ifndef FTS_DEFINES_H
#  define FTS_DEFINES_H

#  if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(_WIN32) || defined(_WIN32_) || defined(__CYGWIN__)
#    if 0
#      define MS_VISUAL 1
#    endif
#    define WINDOOF 1
#    define NOMINMAX
#  else
#    define WINDOOF 0
#  endif

/// This is the delta used to round.
#define D_FTS_DELTA (float)(1e-5)

/* Defines to get it compiling on windows ......... */
/* ================================================ */
#  if defined(_MSC_VER)
#    define VC_EXTRALEAN 1
#    define WIN32_LEAN_AND_MEAN 1
#    define _CRT_SECURE_NO_DEPRECATE
#  if _MSC_VER < 1600
#    define EISCONN WSAEISCONN
#    define EINPROGRESS WSAEINPROGRESS
#    define EALREADY WSAEALREADY
#  endif
#  if _MSC_VER < 1500
#    define vsnprintf _vsnprintf
#  endif
#    define snprintf _snprintf
#    define strdup _strdup
#    define strcasecmp _stricmp
#    define strncasecmp _strnicmp

#    define unlink _unlink

#    define beginthread _beginthread
#    define S_ISREG(m) ((m)& S_IFREG)
#  endif

/* For Visual Studio, we have to define these needed things manually ... */
#  ifdef _MSC_VER
        /* inttypes.h - int64_t */
typedef signed __int8		int8_t;
typedef unsigned __int8		uint8_t;
typedef signed __int16		int16_t;
typedef unsigned __int16	uint16_t;
typedef signed __int32		int32_t;
typedef unsigned __int32	uint32_t;
typedef signed __int64		int64_t;
typedef unsigned __int64	uint64_t;
#  else
#    include <inttypes.h>
#  endif

////////////////////////////////////////////////////////////
// Identify if C++0x can be used.
////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#  if _MSC_VER >= 1600  // Visual Studio 2010 or newer
#    define D_CPP0X 1
#  else
#    define D_CPP0X 0
#  endif
#elif defined(__GXX_EXPERIMENTAL_CXX0X__) // Thank you, Gcc
#  define D_CPP0X 1
#else
#  define D_CPP0X 0
#endif

#if !D_CPP0X
#  error A C++0x compiler is required!
#endif

/* When a function returns this, no error occured.
 * If your function want to return an error, return NEGATIVE values.
 */
#  define ERR_OK (int)0

/* The folders */
/* =========== */
        /* The sign that separates directorys. */
#  define FTS_DIR_SEPARATOR   "/"
#  define FTS_IS_DIR_SEPARATOR(ch) ((ch) == FTS_DIR_SEPARATOR[0])

    // The Dir containing all the data. TODO: Make configurable at compile-time.
#  define DATA "Data" FTS_DIR_SEPARATOR

//#define FTS_WINDOW_TITLE "FTS Version "FTS_VERSION_STRING" by Pompei2"
#  define FTS_WINDOW_TITLE "FTS by Pompei2"

/* The Macros */
/* ========== */
        /* CALCULATION MACROS. */

        /* Calls a pointer-to-member-function.
         * To use like that: ret = CALL_MEMBER_FN(obj,fct)(arg1,Arg2,...);
         */
#  define CALL_MEMBER_FN( object, ptrToMember ) ((object).*(ptrToMember))

        /// Correctly closes a file pointer.
#  define SAFE_FCLOSE( f ) { if( (f) ) { fflush((f)); fclose((f)); (f) = NULL; } } (void)0

#  define D_DEFAULT_SERVER_NAME "arkana-fts.org"
#  define D_DEFAULT_SERVER_DESC "The official Arkana-FTS server"
#  define D_DEFAULT_SERVER_PORT 0xAF75

#endif                          /* FTS_DEFINES_H */

 /* EOF */
