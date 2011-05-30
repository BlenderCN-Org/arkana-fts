# Bla

project(fts_server)

#set(CMAKE_VERBOSE_MAKEFILE ON)

# Setup the make environement. #
################################
cmake_minimum_required(VERSION 2.4)
if(POLICY CMP0005)
  cmake_policy(SET CMP0005 OLD)
endif(POLICY CMP0005)


# Setup the cmake directory containing numerous cmake scripts and macros.
set(CMAKE_MODULE_PATH "${fts_server_SOURCE_DIR}/../../cmake")

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

# Disallow in-source builds. #
##############################
EXECUTE_PROCESS(COMMAND pwd OUTPUT_VARIABLE CURR_DIR)
if("${CURR_DIR}" STREQUAL "${fts_server_SOURCE_DIR}\n")
    message(FATAL_ERROR "In-source-builds are not allowed to build the Arkana-FTS server. Please go into the \"build\" directory and type \"cmake ..\" there.\nThank you.")
endif()

# Search for the different needed libs. #
#########################################

# The std:: namespace.
include(${CMAKE_ROOT}/Modules/TestForSTDNamespace.cmake)

if(${CMAKE_NO_STD_NAMESPACE})
    message(SEND_ERROR "No std:: Namespace is available, This usually means you have no c++ support or no libstdc++")
endif()

# The threads library to use and the mysql client library.
find_package(Threads REQUIRED)
find_package(MYSQL REQUIRED)

# Put all sourcefiles into one variable. #
##########################################
set(SOURCES main.cpp channel.cpp client.cpp db.cpp db_build.cpp dummy_types.cpp
            game.cpp server_log.cpp socket_connection_waiter.cpp
            checksum/sha2.cpp
            checksum/md5.cpp
            ../toolcompat.cpp
            ../../dLib/dString/dString.cpp
            ../../net/connection.cpp
            ../../net/packet.cpp
            ../../utilities/threading.cpp
            ../../utilities/DataContainer.cpp
            ../../utilities/Singleton.cpp
            ../../logging/logger.cpp
            ../../main/Exception.cpp
   )

# Debug or release ?
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring for debug")
    add_definitions(-DDEBUG=1)
else()
    message(STATUS "Configuring for release")
endif()

if(DSRV_ROOT_IS_BAD)
    message(STATUS "Server will start as user ${DSRV_ROOT_IS_BAD} if started as root")
    add_definitions(-DDSRV_ROOT_IS_BAD="\\"${DSRV_ROOT_IS_BAD}\\"")
endif()

if(D_USE_ANTIFLOOD)
    message(STATUS "Using anti-flooding system")
    add_definitions(-DD_USE_ANTIFLOOD)
endif()

if(DSRV_MAX_LOGFILE_BYTES)
    message(STATUS "Setting maximum logfile length to ${DSRV_MAX_LOGFILE_BYTES}")
    add_definitions(-DDSRV_MAX_LOGFILE_BYTES=${DSRV_MAX_LOGFILE_BYTES})
endif()

if(DSRV_LOG_DIR)
    message(STATUS "Setting default logfiles directory to ${DSRV_LOG_DIR}")
    add_definitions(-DDSRV_LOG_DIR="\\"${DSRV_LOG_DIR}\\"")
endif()

if(DSRV_FILE_NPLAYERS)
    message(STATUS "Setting player count filename to ${DSRV_FILE_NPLAYERS}")
    add_definitions(-DDSRV_FILE_NPLAYERS="\\"${DSRV_FILE_NPLAYERS}\\"")
endif()

if(DSRV_FILE_NGAMES)
    message(STATUS "Setting game count filename to ${DSRV_FILE_NGAMES}")
    add_definitions(-DDSRV_FILE_NGAMES="\\"${DSRV_FILE_NGAMES}\\"")
endif()

if(DSRV_LOGFILE_ERR)
    message(STATUS "Setting error log filename to ${DSRV_LOGFILE_ERR}")
    add_definitions(-DDSRV_LOGFILE_ERR="\\"${DSRV_LOGFILE_ERR}\\"")
endif()

if(DSRV_LOGFILE_LOG)
    message(STATUS "Setting log filename to ${DSRV_LOGFILE_LOG}")
    add_definitions(-DDSRV_LOGFILE_LOG="\\"${DSRV_LOGFILE_LOG}\\"")
endif()

if(DSRV_LOGFILE_NETLOG)
    message(STATUS "Setting net log filename to ${DSRV_LOGFILE_NETLOG}")
    add_definitions(-DDSRV_LOGFILE_NETLOG="\\"${DSRV_LOGFILE_NETLOG}\\"")
endif()

if(DSRV_PORT_FIRST)
    message(STATUS "Setting first port of the range to ${DSRV_PORT_FIRST}")
    add_definitions(-DDSRV_PORT_FIRST=${DSRV_PORT_FIRST}\\"")
endif()

if(DSRV_PORT_LAST)
    message(STATUS "Setting last port of the range to ${DSRV_PORT_LAST}")
    add_definitions(-DDSRV_PORT_LAST=${DSRV_PORT_LAST}\\"")
endif()

if(DSRV_MYSQL_HOST)
    message(STATUS "Setting MySQL host to ${DSRV_MYSQL_HOST}")
    add_definitions(-DDSRV_MYSQL_HOST="\\"${DSRV_MYSQL_HOST}\\"")
endif()

if(DSRV_MYSQL_USER)
    message(STATUS "Setting MySQL Username to ${DSRV_MYSQL_USER}")
    add_definitions(-DDSRV_MYSQL_USER="\\"${DSRV_MYSQL_USER}\\"")
endif()

if(DSRV_MYSQL_PASS)
    message(STATUS "Setting MySQL Password to ${DSRV_MYSQL_PASS}")
    add_definitions(-DDSRV_MYSQL_PASS="\\"${DSRV_MYSQL_PASS}\\"")
endif()

if(DSRV_MYSQL_DB)
    message(STATUS "Setting MySQL Database to ${DSRV_MYSQL_DB}")
    add_definitions(-DDSRV_MYSQL_DB="\\"${DSRV_MYSQL_DB}\\"")
endif()

# Add additional source directories.
include_directories(${fts_server_SOURCE_DIR}/../..
                    ${fts_server_SOURCE_DIR}/..
                    ${fts_server_SOURCE_DIR}
                    ${MYSQL_INCLUDE_DIR}
                   )

# Add necessary defines:
add_definitions(-DD_STRING_MYSQL
                -DD_DSTRING
                -DD_COMPILES_SERVER
                -DD_NOCEGUI
                -DD_OWN_ENDIAN
                -DSDL_BYTEORDER=SDL_LIL_ENDIAN)

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
add_executable(fts-server ${SOURCES})
target_link_libraries(fts-server ${MYSQL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
