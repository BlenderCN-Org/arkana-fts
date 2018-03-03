/*
 * $Id: $
 * \file fts_snd.h
 *
 * \author kabey
 * \brief Main header file for the FTS sound system. Setup the right header files
 * according to the used sound subsystem.
 * \remark Currently only the OpenAL sound system is used.
 */

#pragma once
#include "main.h"
#define D_FTS_NoSound 0
#define D_FTS_OpenAL 1
#  ifndef D_SND_SYS
#    define D_SND_SYS D_FTS_NoSound
#  endif
#  if D_SND_SYS == D_FTS_OpenAL
#    include <AL/al.h>
#    include <AL/alc.h>
#  else
#    if (WINDOOF)
#      pragma message( "No sub sound system is defined" )
#    else
#      warning "No sub sound system is defined"
#    endif
#  endif
#  include "3d/3d.h"
#  include "SndFile.h"
#  include "SndTypes.h"
#  include "SndGrp.h"
#  include "SndSys.h"
#  include "SndSysOpenAL.h"
#  include "SndObj.h"
#  include "SndPlayList.h"
