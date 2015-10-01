

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(fts-sound)

message(SATUS "${fts-sound_SOURCE_DIR}")
#set( ARCHIVE_OUTPUT_PATH  "${fts-sound_SOURCE_DIR}../../vclib")
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY   "${fts-sound_SOURCE_DIR}../../vclib")
set( LIBTYPE "STATIC")
set( CMAKE_DEBUG_POSTFIX "_d")
add_subdirectory(openal-soft)
add_subdirectory(libvorbis)
add_subdirectory(libogg)

