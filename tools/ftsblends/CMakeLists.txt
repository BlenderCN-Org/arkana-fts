# Bla
#
# Replace ftsblends by your tool's name. Use find&replace plz.

project(ftsblends)

#set(CMAKE_VERBOSE_MAKEFILE ON)

# Setup the make environement. #
################################
cmake_minimum_required(VERSION 2.4)

# Setup the cmake directory containing numerous cmake scripts and macros.
set(CMAKE_MODULE_PATH "${ftsblends_SOURCE_DIR}/../../cmake")

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

# Disallow in-source builds. #
##############################
EXECUTE_PROCESS(COMMAND pwd OUTPUT_VARIABLE CURR_DIR)
if("${CURR_DIR}" STREQUAL "${ftsblends_SOURCE_DIR}\n")
    message(FATAL_ERROR "In-source-builds are not allowed to build the Arkana-FTS image converter. Please go into the \"build\" directory and type \"cmake ..\" there.\nThank you.")
endif()

# Search for the different needed libs. #
#########################################

set(LIBS)

# Append the libraries that should be uset do LIBS. Example:
#
# The SDL:
find_package(SDL)
if(${SDL_FOUND} STREQUAL "NO")
    message(FATAL_ERROR "Could not find SDL installed on your system. TODO: Build our own.")
else()
    message(STATUS "Found SDL in ${SDL_INCLUDE_DIR}: ${SDL_LIBRARY}")
    include_directories(${SDL_INCLUDE_DIR})
    list(APPEND LIBS ${SDL_LIBRARY})
endif()

# The std:: namespace.
include(${CMAKE_ROOT}/Modules/TestForSTDNamespace.cmake)

if(${CMAKE_NO_STD_NAMESPACE})
    message(SEND_ERROR "No std:: Namespace is available, This usually means you have no c++ support or no libstdc++")
endif()

# Put all sourcefiles into one variable. #
##########################################
set(SOURCES main.cpp
            ../toolcompat.cpp
            ../../dLib/dString/dString.cpp
            ../../dLib/dString/dPath.cpp
            ../../dLib/dFile/dFile.cpp
            ../../dLib/dCompressor/dCompressor.cpp
            ../../dLib/dCompressor/minilzo_compressor.cpp
            ../../dLib/dCompressor/minilzo/minilzo.c
            ../../main/Exception.cpp
            ../../logging/logger.cpp
            ../../logging/chronometer.cpp
            ../../utilities/DataContainer.cpp
            ../../utilities/StreamedDataContainer.cpp
            ../../utilities/Singleton.cpp
            ../../graphic/image.cpp
   )

# Debug or release ?
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring for debug")
    add_definitions(-DDEBUG=1)
else()
    message(STATUS "Configuring for release")
endif()

# Add additional source directories.
include_directories(${ftsblends_SOURCE_DIR}/../..
                    ${ftsblends_SOURCE_DIR}/..
                   )

# Add necessary defines:
add_definitions(-DD_STRING_FTS_LIBS)
add_definitions(-DD_NOCEGUI)
add_definitions(-DD_NONET)
add_definitions(-DD_FILE_NO_ARCHMAP)

# Compiler-dependent and build-dependend flags:
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g -pg")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")
elseif(MSVC)
endif()

# And show the flags.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "CXX Flags (Debug) are: ${CMAKE_CXX_FLAGS_DEBUG}")
else()
    message(STATUS "CXX Flags (Release) are: ${CMAKE_CXX_FLAGS_RELEASE}")
endif()

# The compiling process. #
##########################
add_executable(ftsblends ${SOURCES})
target_link_libraries(ftsblends ${LIBS})
