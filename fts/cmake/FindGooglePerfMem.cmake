# - Find GooglePerf (The google performance tools, only the tcmalloc needed for
#   heap memory checking) library
# This module defines
#  GOOGLEPERFMEM_LIBRARY, the tcmalloc library needed for heap checking.
#  GOOGLEPERFMEM_FOUND, If NO, do not try to use the tcmalloc.
#
# Copyright (c) 2008, Lucas Beyer, <pompei2@gmail.com>
#
# Redistribution and use is allowed according to the terms of the GNU GPLv2 license.
#

set(GOOGLEPERFMEM_FOUND "NO")

find_library(GOOGLEPERFMEM_LIBRARY NAMES tcmalloc
                                   PATHS /lib
                                         /lib64
                                         /usr/lib
                                         /usr/lib64
                                         /usr/local/lib
                                         /usr/local/lib64
                                         /opt/lib
                                         /opt/lib64
            )

if(GOOGLEPERFMEM_LIBRARY)
    set(GOOGLEPERFMEM_FOUND "YES")
    if(NOT GOOGLEPERFMEM_FIND_QUIETLY)
        message(STATUS "Found Google Performance Tools tcmalloc: ${GOOGLEPERFMEM_LIBRARY}")
    endif(NOT GOOGLEPERFMEM_FIND_QUIETLY)
else(GOOGLEPERFMEM_LIBRARY)
    if(NOT GOOGLEPERFMEM_FIND_QUIETLY)
        message(STATUS "Google Performance Tools tcmalloc not found.")
    endif(NOT GOOGLEPERFMEM_FIND_QUIETLY)
endif(GOOGLEPERFMEM_LIBRARY)
