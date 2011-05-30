/**
 * \file support.h
 * \author Pompei2
 * \date 19 July 2006
 * \brief This file contains All defines that enable/disable the support of some features
 **/

#ifndef D_SUPPORT_H
#  define D_SUPPORT_H

#  include "main/defines.h"

/// Activate debugging mode
#  if WINDOOF
#    ifdef _DEBUG
#      define DEBUG
#    endif
#  endif

/// This is to avoid visual c++ compiler warnings.
#  if WINDOOF
#    define DO_DEFINE_MAX 0
#  else
#    define DO_DEFINE_MAX 1
#  endif

/// Wether in a wildcard expression a questionmark ('?') can be no match or not.
#  define D_QUEST_CAN_BE_NONE	1

#  define FTS_UI_SKIN_FILE "ArkanaLook.scheme"
#  define FTS_UI_SKIN      "ArkanaLook"

#endif                          /* D_SUPPORT_H */

 /* EOF */
