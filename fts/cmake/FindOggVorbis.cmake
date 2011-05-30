# - Find The ogg vorbis and co. libraries.
# Find the Ogg Vorbis includes and libraries
# This module defines
#  OGGVORBIS_INCLUDE_DIR, where to find ogg.h and vorbisfile.h
#  OGGVORBIS_LIBRARY, the ogg and the vorbis library (libogg, libvorbis).
#  OGGVORBIS_LIBRARIES, the libraries needed to use Ogg and Vorbis (Currently, the same as above).
#  OGGVORBIS_FOUND, If false, do not try to use Ogg and/or Vorbis.
#
# Copyright (c) 2008, Lucas Beyer, <pompei2@gmail.com>
#
# Redistribution and use is allowed according to the terms of the GNU GPLv2 license.
#

set(OGGVORBIS_FOUND "NO")

find_path(OGG_INCLUDE_DIR ogg.h
                         /include/ogg
                         /include/Ogg
                         /include/OGG
                         /include
                         /usr/include/ogg
                         /usr/include/Ogg
                         /usr/include/OGG
                         /usr/include
                         /usr/local/include/ogg
                         /usr/local/include/Ogg
                         /usr/local/include/OGG
                         /usr/local/include
                         /usr/opt/include/ogg
                         /usr/opt/include/Ogg
                         /usr/opt/include/OGG
                         /usr/opt/include
         )

find_path(VORBIS_INCLUDE_DIR vorbisfile.h
                         /include/vorbis
                         /include/Vorbis
                         /include
                         /usr/include/vorbis
                         /usr/include/Vorbis
                         /usr/include
                         /usr/local/include/vorbis
                         /usr/local/include/Vorbis
                         /usr/local/include
                         /usr/opt/include/vorbis
                         /usr/opt/include/Vorbis
                         /usr/opt/include
         )

find_library(OGG_LIBRARY NAMES ogg
                        PATHS /lib
                              /lib64
                              /usr/lib
                              /usr/lib64
                              /usr/local/lib
                              /usr/local/lib64
                              /opt/lib
                              /opt/lib64
            )

find_library(VORBIS_LIBRARY NAMES vorbis
                          PATHS /lib
                                /lib64
                                /usr/lib
                                /usr/lib64
                                /usr/local/lib
                                /usr/local/lib64
                                /opt/lib
                                /opt/lib64
            )

find_library(VORBISFILE_LIBRARY NAMES vorbisfile
                          PATHS /lib
                                /lib64
                                /usr/lib
                                /usr/lib64
                                /usr/local/lib
                                /usr/local/lib64
                                /opt/lib
                                /opt/lib64
            )

if(OGG_INCLUDE_DIR AND VORBIS_INCLUDE_DIR AND OGG_LIBRARY AND VORBIS_LIBRARY AND VORBISFILE_LIBRARY)
    set(OGGVORBIS_FOUND "YES")
    set(OGGVORBIS_LIBRARY ${OGG_LIBRARY} ${VORBIS_LIBRARY} ${VORBISFILE_LIBRARY})
    set(OGGVORBIS_LIBRARIES ${OGG_LIBRARY} ${VORBIS_LIBRARY} ${VORBISFILE_LIBRARY})
    set(OGGVORBIS_INCLUDE_DIR ${OGG_INCLUDE_DIR} ${VORBIS_INCLUDE_DIR})
    if(NOT OGGVORBIS_FIND_QUIETLY)
        message(STATUS "Found Ogg and Vorbis: ${OGGVORBIS_INCLUDE_DIR}, ${OGGVORBIS_LIBRARY}")
    endif(NOT OGGVORBIS_FIND_QUIETLY)
else(OGG_INCLUDE_DIR AND VORBIS_INCLUDE_DIR AND OGG_LIBRARY AND VORBIS_LIBRARY AND VORBISFILE_LIBRARY)
    if(NOT OGGVORBIS_FIND_QUIETLY)
        message(STATUS "Ogg or/and Vorbis not found.")
    endif(NOT OGGVORBIS_FIND_QUIETLY)
endif(OGG_INCLUDE_DIR AND VORBIS_INCLUDE_DIR AND OGG_LIBRARY AND VORBIS_LIBRARY AND VORBISFILE_LIBRARY)
