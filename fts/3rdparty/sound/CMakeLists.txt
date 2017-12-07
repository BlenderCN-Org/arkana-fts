CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

PROJECT(fts-sound)

set(ALSOFT_UTILS OFF)
set(ALSOFT_EXAMPLES OFF)
set(ALSOFT_INSTALL OFF)
add_subdirectory(openal-soft)
add_subdirectory(libvorbis)
add_subdirectory(libogg)
if(WIN32)
    set(SND_LIBS OpenAL32)
else()
    set(SND_LIBS openal)
endif()
set(SND_LIBS ${SND_LIBS} ogg vorbis PARENT_SCOPE)
set(SND_INCLUDE_DIR ${fts-sound_SOURCE_DIR}/openal-soft/include 
                    ${fts-sound_SOURCE_DIR}/libvorbis/include 
                    ${fts-sound_SOURCE_DIR}/libogg/include
                    PARENT_SCOPE)
