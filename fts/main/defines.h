#ifndef FTS_DEFINES_H
#  define FTS_DEFINES_H

#  if defined(_WIN32) || defined(__CYGWIN__)
#    define WINDOOF 1
#  else
#    define WINDOOF 0
#  endif

/// This is the delta used to round.
#define D_FTS_DELTA (float)(1e-5)

/* When a function returns this, no error occurred.
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
#  define FTS_UI_SKIN_FILE "ArkanaLook.scheme"
#  define FTS_UI_SKIN      "ArkanaLook"

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
