CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

PROJECT(fts-dependencies)

if(MSVC)
    set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY   "${fts-dependencies_SOURCE_DIR}/vclib")
endif()
set( LIBTYPE "STATIC")
set( CMAKE_DEBUG_POSTFIX "_d")

if(MSVC)
    FOREACH(flag_var CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE)
        IF(${flag_var} MATCHES "/MD")
            STRING(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        ENDIF()
    ENDFOREACH(flag_var)
    FOREACH(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE)
        IF(${flag_var} MATCHES "/MD")
            STRING(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        ENDIF()
    ENDFOREACH(flag_var)
endif()

message(STATUS "=================")
message(STATUS "CONFIGURING sound")
message(STATUS "=================")
add_subdirectory(sound)
list(APPEND FTS_LIBS ${SND_LIBS})
list(APPEND FTS_INCLUDES ${SND_INCLUDE_DIR})

set(BOUGE_BUILD_EXAMPLES FALSE)
set(BOUGE_BUILD_STATICCRT TRUE)
set(BOUGE_BUILD_CAL3DXMLIO FALSE)
set(BOUGE_BUILD_SHARED_LIBS FALSE)
set(BOUGE_BUILD_DOC FALSE)
add_subdirectory(bouge)
list(APPEND FTS_INCLUDES ${BOUGE_INCLUDE_DIR})
list(APPEND FTS_LIBS ${BOUGE_LIBRARIES} bouge-tinyxml bouge-xmlio)

add_subdirectory(dao)
list(APPEND FTS_LIBS dao)

message(STATUS "================")
message(STATUS "CONFIGURING PCRE")
message(STATUS "================")
set(PCRE_FIND_QUIETLY 1)
find_package(PCRE)
if(NOT PCRE_FOUND)
    message(WARNING "Could not find PCRE installed on your system. Using the bundled PCRE version. You may want to install the PCRE development files by installing a package called something in the lines of PCRE-devel or PCRE-dev.")
    set(PCRE_BUILD_PCRECPP OFF)
    set(PCRE_STATIC_RUNTIME ON)
    set(PCRE_SUPPORT_UTF ON)
    set(PCRE_SUPPORT_UNICODE_PROPERTIES ON)
    add_subdirectory(pcre-8.41)
    set(PCRE_INCLUDE_DIR "${PCRE_BINARY_DIR}")
    set(PCRE_LIBRARIES "pcre")
endif()
list(APPEND FTS_INCLUDES ${PCRE_INCLUDE_DIR})
list(APPEND FTS_LIBS ${PCRE_LIBRARIES})

message(STATUS "====================")
message(STATUS "CONFIGURING freetype")
message(STATUS "====================")
set(FreeType_FIND_QUIETLY 1)
find_package(FreeType)
if(NOT FreeType_FOUND)
    message(WARNING "Could not find FreeType installed on your system. This means you have to install the FreeType development files. This can be done by installing a package called something in the lines of FreeType-devel or FreeType-dev.")
    add_subdirectory(freetype-2.6)
    set(FreeType_INCLUDE_DIR "${fts-dependencies_SOURCE_DIR}/freetype-2.6/include")
    set(FreeType_LIBRARY "freetype")
endif()
list(APPEND FTS_INCLUDES ${FreeType_INCLUDE_DIR})
list(APPEND FTS_LIBS ${FreeType_LIBRARY})

add_subdirectory(CEGUI)
list(APPEND FTS_LIBS CEGUIBase)
list(APPEND FTS_LIBS CEGUITinyXMLParser)
list(APPEND FTS_LIBS CEGUIOpenGLRenderer)
list(APPEND FTS_LIBS CEGUIFalagardWRBase)

message(STATUS "================")
message(STATUS "CONFIGURING SDL2")
message(STATUS "================")
if(WIN32)
    # This options will be needed w/ SDL2 on WIN32.
    set(DIRECTX OFF)
    set(SDL_SHARED OFF)
    set(SDL_STATIC ON)
    add_subdirectory(SDL2)
    set(SDL2_LIBRARY "SDL2main" "SDL2-static")
    set(SDL2_INCLUDE_DIR ${fts-dependencies_SOURCE_DIR}/SDL2/include)
    if(MSVC)
        # Workaroud to get rid of the dxerr.lib usage of the SDL2 project.
        get_target_property(ilibs SDL2-static INTERFACE_LINK_LIBRARIES)
        list(REMOVE_ITEM ilibs dxerr)
        set_property(TARGET SDL2-static PROPERTY INTERFACE_LINK_LIBRARIES ${ilibs})
        set_property(TARGET SDL2-static PROPERTY LINK_LIBRARIES ${ilibs})
    endif()
else()
    set(SDL2_FIND_QUIETLY 1)
    find_package(SDL2)
    if(NOT SDL2_FOUND)
        message(WARNING "Could not find SDL installed on your system. This means you have to install the SDL development files. This can be done by installing a package called something in the lines of SDL-devel or SDL-dev. Using our own version of SDL.")
        set(DIRECTX OFF)
        set(SDL_SHARED OFF)
        set(SDL_STATIC ON)
        add_subdirectory(SDL2)
        set(SDL2_LIBRARY "SDL2main" "SDL2")
        set(SDL2_INCLUDE_DIR ${fts-dependencies_SOURCE_DIR}/SDL2/include)
    else()
        # On some systems the names are different.
        # So map them to one.
        if(SDL2_LIBRARIES)
            set(SDL2_LIBRARY ${SDL2_LIBRARIES})
        endif()
        if(SDL2_INCLUDE_DIRS)
            set(SDL2_INCLUDE_DIR ${SDL2_INCLUDE_DIRS})
        endif()
    endif()
endif()
list(APPEND FTS_LIBS ${SDL2_LIBRARY})
list(APPEND FTS_INCLUDES ${SDL2_INCLUDE_DIR})

# Add fts-networking stuff
add_subdirectory(fts-network)
list(APPEND FTS_LIBS fts-net)

# Populate the libs and includes
set(FTS_LIBS "${FTS_LIBS}" PARENT_SCOPE)
set(FTS_INCLUDES "${FTS_INCLUDES}" PARENT_SCOPE)
