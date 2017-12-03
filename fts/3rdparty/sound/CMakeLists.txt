CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

PROJECT(fts-sound)

if(MSVC)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY   "${fts-sound_SOURCE_DIR}../../vclib")
endif()
set(LIBTYPE "STATIC")
set(ALSOFT_UTILS OFF)
set(ALSOFT_EXAMPLES OFF)
set(ALSOFT_INSTALL OFF)
set(CMAKE_DEBUG_POSTFIX "_d")
add_subdirectory(openal-soft)
add_subdirectory(libvorbis)
add_subdirectory(libogg)

