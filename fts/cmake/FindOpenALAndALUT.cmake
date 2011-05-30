# - Find OpenAL and ALUT (OpenAL Utility Toolkit) libraries
# Find the OpenAL and ALUT includes and libraries
# This module defines
#  OPENAL_ALUT_INCLUDE_DIR, where to find OpenAL and ALUT
#  OPENAL_ALUT_LIBRARIES, the libraries needed to use OpenAL and ALUT.
#  OPENAL_ALUT_FOUND, If false, do not try to use OpenAL or ALUT.
#
# You can define OPENAL_ALUT_FIND_QUITELY to suppress any messages from this module.
#
# Copyright (c) 2008, Lucas Beyer, <pompei2@gmail.com>
#
# Redistribution and use is allowed according to the terms of the GNU GPLv2 license.
#
set(OPENAL_ALUT_FOUND "NO")

# The part to find OpenAL is the original one from CMake but has been modified
# because of some problems if the lib exists, but not the includes.
# Here is the original comment:
#
# - Locate OpenAL
# This module defines
#  OPENAL_LIBRARY
#  OPENAL_FOUND, if false, do not try to link to OpenAL
#  OPENAL_INCLUDE_DIR, where to find the headers
#
# $OPENALDIR is an environment variable that would
# correspond to the ./configure --prefix=$OPENALDIR
# used in building OpenAL.
#
# Created by Eric Wing. This was influenced by the FindSDL.cmake module.
# On OSX, this will prefer the Framework version (if found) over others.
# People will have to manually change the cache values of
# OPENAL_LIBRARY to override this selection.
# Tiger will include OpenAL as part of the System.
# But for now, we have to look around.
# Other (Unix) systems should be able to utilize the non-framework paths.
#

############################
# Now look for OpenAL
############################

FIND_PATH(_OPENAL_INCLUDE_DIR_ al.h
  $ENV{OPENALDIR}/include
  ~/Library/Frameworks/OpenAL.framework/Headers
  /Library/Frameworks/OpenAL.framework/Headers
  /System/Library/Frameworks/OpenAL.framework/Headers # Tiger
  /usr/local/include/AL
  /usr/local/include/al
  /usr/local/include/OpenAL
  /usr/local/include
  /usr/include/AL
  /usr/include/al
  /usr/include/OpenAL
  /usr/include
  /sw/include/AL # Fink
  /sw/include/al
  /sw/include/OpenAL
  /sw/include
  /opt/local/include/AL # DarwinPorts
  /opt/local/include/al
  /opt/local/include/OpenAL
  /opt/local/include
  /opt/csw/include/AL # Blastwave
  /opt/csw/include/al
  /opt/csw/include/OpenAL
  /opt/csw/include
  /opt/include/AL
  /opt/include/al
  /opt/include/OpenAL
  /opt/include
  )
# I'm not sure if I should do a special casing for Apple. It is
# unlikely that other Unix systems will find the framework path.
# But if they do ([Next|Open|GNU]Step?),
# do they want the -framework option also?
IF(${_OPENAL_INCLUDE_DIR_} MATCHES ".framework")
  STRING(REGEX REPLACE "(.*)/.*\\.framework/.*" "\\1" OPENAL_FRAMEWORK_PATH_TMP ${_OPENAL_INCLUDE_DIR_})
  IF("${OPENAL_FRAMEWORK_PATH_TMP}" STREQUAL "/Library/Frameworks"
      OR "${OPENAL_FRAMEWORK_PATH_TMP}" STREQUAL "/System/Library/Frameworks"
      )
    # String is in default search path, don't need to use -F
    SET (_OPENAL_LIBRARY_ "-framework OpenAL" CACHE STRING "OpenAL framework for OSX")
  ELSE("${OPENAL_FRAMEWORK_PATH_TMP}" STREQUAL "/Library/Frameworks"
      OR "${OPENAL_FRAMEWORK_PATH_TMP}" STREQUAL "/System/Library/Frameworks"
      )
    # String is not /Library/Frameworks, need to use -F
    SET(_OPENAL_LIBRARY_ "-F${OPENAL_FRAMEWORK_PATH_TMP} -framework OpenAL" CACHE STRING "OpenAL framework for OSX")
  ENDIF("${OPENAL_FRAMEWORK_PATH_TMP}" STREQUAL "/Library/Frameworks"
    OR "${OPENAL_FRAMEWORK_PATH_TMP}" STREQUAL "/System/Library/Frameworks"
    )
  # Clear the temp variable so nobody can see it
  SET(OPENAL_FRAMEWORK_PATH_TMP "" CACHE INTERNAL "")

ELSE(${_OPENAL_INCLUDE_DIR_} MATCHES ".framework")
  FIND_LIBRARY(_OPENAL_LIBRARY_
    NAMES openal al OpenAL32
    PATHS
    $ENV{OPENALDIR}/lib
    $ENV{OPENALDIR}/libs
    /usr/local/lib
    /usr/lib
    /sw/lib
    /opt/local/lib
    /opt/csw/lib
    /opt/lib
    )
ENDIF(${_OPENAL_INCLUDE_DIR_} MATCHES ".framework")

############################
# Now look for ALUT
############################
find_path(_ALUT_INCLUDE_DIR_ alut.h
                             /include/OpenAL
                             /include/ALUT
                             /include/alut
                             /include/AL
                             /include/al
                             /include
                             /usr/include/OpenAL
                             /usr/include/ALUT
                             /usr/include/alut
                             /usr/include/AL
                             /usr/include/al
                             /usr/include
                             /usr/local/include/OpenAL
                             /usr/local/include/ALUT
                             /usr/local/include/alut
                             /usr/local/include/AL
                             /usr/local/include/al
                             /usr/local/include
                             /usr/opt/include/OpenAL
                             /usr/opt/include/ALUT
                             /usr/opt/include/alut
                             /usr/opt/include/AL
                             /usr/opt/include/al
                             /usr/opt/include
         )

find_library(_ALUT_LIBRARY_ NAMES alut
                            PATHS /lib
                                  /usr/lib
                                  /usr/local/lib
                                  /opt/lib
            )

############################
# Now Combine the search results.
############################
if(_OPENAL_INCLUDE_DIR_ AND _OPENAL_LIBRARY_ AND _ALUT_INCLUDE_DIR_ AND _ALUT_LIBRARY_)
    set(OPENAL_ALUT_FOUND "YES")
    set(OPENAL_ALUT_INCLUDE_DIR ${_OPENAL_INCLUDE_DIR_} ${_ALUT_INCLUDE_DIR_})
    set(OPENAL_ALUT_LIBRARIES ${_OPENAL_LIBRARY_} ${_ALUT_LIBRARY_}) # TODO: Don't know if this is good ?
    if(NOT OPENAL_ALUT_FIND_QUITELY)
        message(STATUS "Found OpenAL and ALUT: ${OPENAL_ALUT_INCLUDE_DIR}, ${OPENAL_ALUT_LIBRARIES}")
    endif(NOT OPENAL_ALUT_FIND_QUITELY)
else(_OPENAL_INCLUDE_DIR_ AND _OPENAL_LIBRARY_ AND _ALUT_INCLUDE_DIR_ AND _ALUT_LIBRARY_)
    if(NOT OPENAL_ALUT_FIND_QUITELY)
    message(STATUS "OpenAL and/or ALUT not found.")
    endif(NOT OPENAL_ALUT_FIND_QUITELY)
endif(_OPENAL_INCLUDE_DIR_ AND _OPENAL_LIBRARY_ AND _ALUT_INCLUDE_DIR_ AND _ALUT_LIBRARY_)
