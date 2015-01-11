# Bla

project(fts_server)

#set(CMAKE_VERBOSE_MAKEFILE ON)

# Setup the make environement. #
################################
cmake_minimum_required(VERSION 3.0)


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
            ../../utilities/GetOpt.cpp
            ../../logging/logger.cpp
            ../../logging/Chronometer.cpp
            ../../main/Clock.cpp
            ../../main/Exception.cpp
   )

# Debug or release ?
if( NOT MSCV)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(STATUS "Configuring for debug")
        add_definitions(-DDEBUG=1)
    else()
        message(STATUS "Configuring for release")
    endif()
endif()
# The configuration settings.
set(RUN_AS_USER "fts" CACHE STRING "The user as which to run the server, in case root starts it.")
#set(USE_ANTIFLOOD True cache STRING "Use a flood-protection?")

set(LOG_MAX_LOGFILE_KBYTES 10240 CACHE STRING "Maximum lenght of a logfile, in kb, before a new one is used.")
if(MSVC)
    set(LOG_DIR "$ENV{LOCALAPPDATA}/fts/log" CACHE PATH "Where to write the logfiles?")
else()
    set(LOG_DIR "/var/log" CACHE PATH "Where to write the logfiles?")
endif()
set(LOG_PLAYERCOUNT_FILE "nplayers.txt" CACHE FILEPATH "File to write the amount of players currently online to.")
set(LOG_GAMECOUNT_FILE "ngames.txt" CACHE FILEPATH "File to write the amount of games currently being played to.")

set(PORT_FIRST "0xAF75" CACHE STRING "First port in the range of ports to listen to.")
set(PORT_LAST "0xAF7F" CACHE STRING "Last port in the range of ports to listen to.")

set(MYSQL_HOST "localhost" CACHE STRING "The server holding the MySQL tables to use.")
set(MYSQL_USER "fts" CACHE STRING "The MySQL username that should access the db.")
set(MYSQL_PASS "" CACHE STRING "The password of the MySQL user.")
set(MYSQL_DB "arkana_game" CACHE STRING "The name of the database to use.")

set(CHAT_DEFAULT_CHANNEL_NAME "Talk To Survive (main channel)" CACHE STRING "The name of the channel new users land in.")
set(CHAT_DEFAULT_CHANNEL_MOTTO "This is the main channel in Arkana-FTS where everybody meets" CACHE STRING "The motto of the channel new users land in.")
set(CHAT_DEFAULT_CHANNEL_ADMIN "Pompei2" CACHE STRING "The user who should be admin of the default channel.")
set(CHAT_DEVS_CHANNEL_NAME "Dev's Channel" CACHE STRING "The name of the developer's channel.")
set(CHAT_DEVS_CHANNEL_MOTTO "The channel for the developers of Arkana-FTS and the players wanting to know more." CACHE STRING "The motto of the dev's channel")
set(CHAT_DEVS_CHANNEL_ADMIN "Pompei2" CACHE STRING "The user who should be admin of the dev's channel.")
set(CHAT_DEFAULT_MOTTO "Arkana-FTS Rules!" CACHE STRING "The motto any newly created channel has by default.")
set(CHAT_MAX_CHANS_PER_USER "3" CACHE STRING "The maximum amount of channes a user can create/own.")

configure_file("${fts_server_SOURCE_DIR}/config.h.in" "config.h" ESCAPE_QUOTES)

# Add additional source directories.
include_directories(${fts_server_SOURCE_DIR}/../..
                    ${fts_server_SOURCE_DIR}/..
                    ${fts_server_SOURCE_DIR}
                    ${fts_server_BINARY_DIR}
                    ${MYSQL_INCLUDE_DIR}
                   )

# Add necessary defines:
add_definitions(-DD_DSTRING
                -DD_COMPILES_SERVER
                -DD_NOCEGUI
                -DD_OWN_ENDIAN
                -DSDL_BYTEORDER=SDL_LIL_ENDIAN)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS
                    -D_WINSOCK_DEPRECATED_NO_WARNINGS
    )
endif(MSVC)

# Compiler-dependent and build-dependend flags:
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++0x -Wall -pg")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++0x -Wall")
    # And show the flags.
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(STATUS "CXX Flags (Debug) are: ${CMAKE_CXX_FLAGS_DEBUG}")
    else()
        message(STATUS "CXX Flags (Release) are: ${CMAKE_CXX_FLAGS_RELEASE}")
    endif()
elseif(MSVC)
    link_directories( ${MYSQL_LIBRARIES_DIR}  )	
    set( MYSQL_LIBRARIES "libmysql.lib;mysqlclient.lib" )
endif()


# The compiling process. #
##########################

add_executable(fts-server ${SOURCES})
target_link_libraries(fts-server ${MYSQL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} )

install(TARGETS fts-server DESTINATION bin)
