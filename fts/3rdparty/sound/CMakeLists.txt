CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(fts-sound)

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY   "${fts-sound_SOURCE_DIR}../../vclib")
set( LIBTYPE "STATIC")
set( CMAKE_DEBUG_POSTFIX "_d")
add_subdirectory(openal-soft)
add_subdirectory(libvorbis)
add_subdirectory(libogg)
add_subdirectory(freealut)

