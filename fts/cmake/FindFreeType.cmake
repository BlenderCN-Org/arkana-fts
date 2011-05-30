# - Find FreeType
# Find the FreeType includes and library
# This module defines
#  FreeType_INCLUDE_DIR, where to find freetype.h
#  FreeType_LIBRARY, the FreeType library.
#  FreeType_LIBRARIES, the libraries needed to use FreeType.
#  FreeType_FOUND, If false, do not try to use FreeType.
#
# Copyright (c) 2008, Lucas Beyer, <pompei2@gmail.com>
#
# Redistribution and use is allowed according to the terms of the GNU GPLv2 license.
#
# TODO: FreeType_LIBRARIES is maybe not perfect yet.
#

set(FreeType_FOUND "NO")

find_path(FreeType_INCLUDE_DIR freetype/config/ftheader.h
#                                /include/freetype2/freetype
                               /include/freetype2
#                                /include/freetype
                               /include
#                                /usr/include/freetype2/freetype
                               /usr/include/freetype2
#                                /usr/include/freetype
                               /usr/include
#                                /usr/local/include/freetype2/freetype
                               /usr/local/include/freetype2
#                                /usr/local/include/freetype
                               /usr/local/include
#                                /usr/opt/include/freetype2/freetype
                               /usr/opt/include/freetype2
#                                /usr/opt/include/freetype
                               /usr/opt/include
         )

find_library(FreeType_LIBRARY NAMES freetype
                                    freetype2
                              PATHS /lib
                                    /lib64
                                    /usr/lib
                                    /usr/lib64
                                    /usr/local/lib
                                    /usr/local/lib64
                                    /opt/lib
                                    /opt/lib64
            )

if(FreeType_INCLUDE_DIR AND FreeType_LIBRARY)
    set(FreeType_FOUND "YES")
    set(FreeType_LIBRARIES ${FreeType_LIBRARY} "-lz") # TODO: Don't know if this is good ?
    if(NOT FreeType_FIND_QUIETLY)
        message(STATUS "Found FreeType: ${FreeType_INCLUDE_DIR}, ${FreeType_LIBRARY}")
    endif(NOT FreeType_FIND_QUIETLY)
else(FreeType_INCLUDE_DIR AND FreeType_LIBRARY)
    if(NOT FreeType_FIND_QUIETLY)
        message(STATUS "FreeType not found.")
    endif(NOT FreeType_FIND_QUIETLY)
endif(FreeType_INCLUDE_DIR AND FreeType_LIBRARY)
