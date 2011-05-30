# Bla

project(fts_cgi)

#set(CMAKE_VERBOSE_MAKEFILE ON)

# Setup the make environement. #
################################
cmake_minimum_required(VERSION 2.4)

# Setup the cmake directory containing numerous cmake scripts and macros.
set(CMAKE_MODULE_PATH "${fts_cgi_SOURCE_DIR}/../../cmake")

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

# Disallow in-source builds. #
##############################
EXECUTE_PROCESS(COMMAND pwd OUTPUT_VARIABLE CURR_DIR)
if("${CURR_DIR}" STREQUAL "${fts_cgi_SOURCE_DIR}\n")
    message(FATAL_ERROR "In-source-builds are not allowed to build the Arkana-FTS server cgi. Please go into the \"build\" directory and type \"cmake ..\" there.\nThank you.")
endif()

# Search for the different needed libs. #
#########################################

# The std:: namespace.
include(${CMAKE_ROOT}/Modules/TestForSTDNamespace.cmake)

if(${CMAKE_NO_STD_NAMESPACE})
    message(SEND_ERROR "No std:: Namespace is available, This usually means you have no c++ support or no libstdc++")
endif()

# The threads library to use and the mysql client library.
# find_package(Threads REQUIRED)
# find_package(MySQL REQUIRED)

# Put all sourcefiles into one variable. #
##########################################
set(SOURCES main.cpp
            ../../../dLib/dString/dString.cpp
            ../../../net/connection.cpp
            ../../../net/packet.cpp
            ../../../utilities/threading.cpp
   )

# Debug or release ?
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring for debug")
    add_definitions(-DDEBUG=1)
else()
    message(STATUS "Configuring for release")
endif()

# Add additional source directories.
include_directories(${fts_cgi_SOURCE_DIR}/../../..
                    ${fts_cgi_SOURCE_DIR}/..
                    ${fts_cgi_SOURCE_DIR}
                   )

# Add necessary defines:
add_definitions(-DD_DSTRING
                -DERR_OK=0
                -DD_COMPILES_SERVER
                -DD_STRING_NOT_DEFINE_FTS18N)

# Compiler-dependent and build-dependend flags:
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -pg")
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
add_executable(fts_cgi ${SOURCES})
target_link_libraries(fts_cgi)

