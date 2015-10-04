

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(fts-dependencies)

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY   "${fts-dependencies_SOURCE_DIR}/vclib")
set( LIBTYPE "STATIC")
set( CMAKE_DEBUG_POSTFIX "_d")

if(MSVC)
    FOREACH(flag_var
        CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
        )
        IF(${flag_var} MATCHES "/MD")
            STRING(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        ENDIF()
    ENDFOREACH(flag_var)
    FOREACH(flag_var
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        )
        IF(${flag_var} MATCHES "/MD")
            STRING(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        ENDIF()
    ENDFOREACH(flag_var)
endif()

add_subdirectory(sound)

set(BOUGE_BUILD_EXAMPLES FALSE)
set(BOUGE_BUILD_STATICCRT TRUE)
set(BOUGE_BUILD_CAL3DXMLIO FALSE)
set(BOUGE_BUILD_SHARED_LIBS FALSE)
set(BOUGE_BUILD_DOC FALSE)
set(BOUGE_COMPILE_FLAGS ${CMAKE_CXX_FLAGS})
add_subdirectory(bouge)

add_subdirectory(dao)

if(WIN32)
    set(PCRE_BUILD_PCRECPP FALSE)
    set(PCRE_STATIC TRUE)
    set(PCRE_SUPPORT_UTF8 TRUE)
    add_subdirectory(pcre)
    set(PCRE_INCLUDE_DIR "${fts-dependencies_SOURCE_DIR}/pcre/build")
    set(PCRE_LIBRARIES "pcre")
else(WIN32)
    set(PCRE_FIND_QUIETLY 1)
    find_package(PCRE)
    if(${PCRE_FOUND} STREQUAL "NO")
        message(WARNING "Could not find PCRE installed on your system. Using the bundled PCRE version. You may want to install the PCRE development files by installing a package called something in the lines of PCRE-devel or PCRE-dev.")
        set(PCRE_SUPPORT_UTF8 ON CACHE BOOL "Enable support for the Unicode UTF-8 encoding.")
        set(PCRE_SUPPORT_UNICODE_PROPERTIES ON CACHE BOOL "Enable support for Unicode properties. (If set, UTF-8 support will be enabled as well)")
        option(PCRE_BUILD_PCRECPP "Build the PCRE C++ library (pcrecpp)." OFF)
        add_subdirectory(pcre)
        set(PCRE_INCLUDE_DIR "${PCRE_BINARY_DIR}")
        set(PCRE_LIBRARIES "pcre")
    else()
        message(STATUS "Found PCRE in ${PCRE_INCLUDE_DIR}: ${PCRE_LIBRARY}")
    endif()
endif(WIN32)

if(WIN32)
    add_subdirectory(freetype-2.6)
    set(FreeType_INCLUDE_DIR "${fts-dependencies_SOURCE_DIR}/freetype-2.6/include")
else(WIN32)
    set(FreeType_FIND_QUIETLY 1)
    find_package(FreeType)
    if(${FreeType_FOUND} STREQUAL "NO")
        message(FATAL_ERROR "Could not find FreeType installed on your system. This means you have to install the FreeType development files. This can be done by installing a package called something in the lines of FreeType-devel or FreeType-dev.")
    else()
        message(STATUS "Found FreeType in ${FreeType_INCLUDE_DIR}: ${FreeType_LIBRARY}")
        list(APPEND FTS_INCLUDES ${FreeType_INCLUDE_DIR})
        list(APPEND FTS_LIBS ${FreeType_LIBRARIES})
    endif()
endif(WIN32)

add_subdirectory(CEGUI)
# This options will be needed w/ SDL2.
#set(DIRECTX FALSE)
#set(SDL_SHARED OFF)
add_subdirectory(SDL)



