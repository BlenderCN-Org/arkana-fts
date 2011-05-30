#ifndef CONFIG_H
#define CONFIG_H

/* Define to the library version */
#define ALSOFT_VERSION "1.6.372"

/* Define if we have the ALSA backend */
/* #undef HAVE_ALSA */

/* Define if we have the OSS backend */
/* #undef HAVE_OSS */

/* Define if we have the Solaris backend */
/* #undef HAVE_SOLARIS */

/* Define if we have the DSound backend */
#define HAVE_DSOUND

/* Define if we have the Windows Multimedia backend */
#define HAVE_WINMM

/* Define if we have the SDL backend */
/* #undef HAVE_SDL */

/* Define if we have dlfcn.h */
/* #undef HAVE_DLFCN_H */

/* Define if we have the sqrtf function */
/* #undef HAVE_SQRTF */

/* Define if we have the acosf function */
/* #undef HAVE_ACOSF */

/* Define if we have the atanf function */
/* #undef HAVE_ATANF */

/* Define if we have the fabsf function */
/* #undef HAVE_FABSF */

/* Define if we have the strtof function */
/* #undef HAVE_STRTOF */

/* Define if we have stdint.h */
/* #undef HAVE_STDINT_H */

/* Define if we have SDL/SDL.h */
/* #undef HAVE_SDL_SDL_H */

/* Define if we have SDL.h */
/* #undef HAVE_SDL_H */

/* Define if we have the __int64 type */
#define HAVE___INT64

/* Define to the size of a long int type */
#define SIZEOF_LONG 4

/* Define to the size of a long long int type */
#define SIZEOF_LONG_LONG 8

/* Define to the size of an unsigned int type */
#define SIZEOF_UINT 4

/* Define to the size of a void pointer type */
#define SIZEOF_VOIDP 4

/* Define if we have GCC's destructor attribute */
/* #undef HAVE_GCC_DESTRUCTOR */

/* Define if we have pthread_np.h */
/* #undef HAVE_PTHREAD_NP_H */

/* Define if we have float.h */
#define HAVE_FLOAT_H

/* Define if we have fenv.h */
/* #undef HAVE_FENV_H */

/* Define if we have fesetround() */
/* #undef HAVE_FESETROUND */

/* Define if we have _controlfp() */
#define HAVE__CONTROLFP

#endif
