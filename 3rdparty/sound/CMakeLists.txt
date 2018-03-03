CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

PROJECT(fts-sound)

find_package(OpenAL)
if(OpenAL_FOUND)
    set(SND_LIBS ${OPENAL_LIBRARY})
    set(SND_INCLUDE_DIR ${OPENAL_INCLUDE_DIR})
else()
    set(ALSOFT_UTILS OFF)
    set(ALSOFT_EXAMPLES OFF)
    set(ALSOFT_INSTALL OFF)
    add_subdirectory(openal-soft)
    if(WIN32)
        set(SND_LIBS OpenAL32)
    else()
        set(SND_LIBS openal)
    endif()
    set(SND_INCLUDE_DIR ${fts-sound_SOURCE_DIR}/openal-soft/include)
endif()
add_subdirectory(libogg)
add_subdirectory(libvorbis)
# The order is important (Linux) to avoid link errors.
set(SND_LIBS ${SND_LIBS} vorbis ogg PARENT_SCOPE)
set(SND_INCLUDE_DIR ${SND_INCLUDE_DIR} 
                    ${fts-sound_SOURCE_DIR}/libvorbis/include 
                    ${fts-sound_SOURCE_DIR}/libogg/include
                    PARENT_SCOPE)
