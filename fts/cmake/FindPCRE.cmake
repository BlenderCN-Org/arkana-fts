# - Find PCRE
# Find the PCRE includes and library
# This module defines
#  PCRE_INCLUDE_DIR, where to find pcre.h
#  PCRE_LIBRARY, the PCRE library.
#  PCRE_LIBRARIES, the libraries needed to use PCRE.
#  PCRE_FOUND, If false, do not try to use PCRE.
#
# Copyright (c) 2008, Lucas Beyer, <pompei2@gmail.com>
#
# Redistribution and use is allowed according to the terms of the GNU GPLv2 license.
#
# Note: PCRE_LIBRARIES is usually the same as PCRE_LIBRARY.
#

set(PCRE_FOUND "NO")

find_path(PCRE_INCLUDE_DIR pcre.h
                           /include
                           /usr/include
                           /usr/local/include
                           /usr/opt/include
         )

find_library(PCRE_LIBRARY NAMES pcre
                          PATHS /lib
                                /lib64
                                /usr/lib
                                /usr/lib64
                                /usr/local/lib
                                /usr/local/lib64
                                /opt/lib
                                /opt/lib64
            )

if(PCRE_INCLUDE_DIR AND PCRE_LIBRARY)
    set(PCRE_FOUND "YES")
    set(PCRE_LIBRARIES ${PCRE_LIBRARY})
    if(NOT PCRE_FIND_QUIETLY)
        message(STATUS "Found PCRE: ${PCRE_INCLUDE_DIR}, ${PCRE_LIBRARY}")
    endif(NOT PCRE_FIND_QUIETLY)
else(PCRE_INCLUDE_DIR AND PCRE_LIBRARY)
    if(NOT PCRE_FIND_QUIETLY)
        message(STATUS "PCRE not found.")
    endif(NOT PCRE_FIND_QUIETLY)
endif(PCRE_INCLUDE_DIR AND PCRE_LIBRARY)
