# Bla
cmake_minimum_required(VERSION 2.4)

project(peetsa)

message(STATUS "                  ")
message(STATUS "==================")
message(STATUS "CONFIGURING peetsa")
message(STATUS "==================")
message(STATUS "                  ")

# Setup the make environement. #
################################

# Allow else() and endif() constructs (with empty paranthesis)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

# Activate very verbose makefiles.
# set(CMAKE_VERBOSE_MAKEFILE ON)

# Disallow in-source builds. #
##############################
EXECUTE_PROCESS(COMMAND pwd OUTPUT_VARIABLE CURR_DIR)
if("${CURR_DIR}" STREQUAL "${peetsa_SOURCE_DIR}\n")
    message(FATAL_ERROR "In-source-builds are not allowed to build the peetsa sample. Please go into the \"build\" directory and type \"cmake ..\" there.\nThank you.")
endif()

# Search for the different needed libs. #
#########################################

# Put all sourcefiles into one variable. #
##########################################
set(SOURCES dough.cpp
            ingredients.cpp
            main.cpp
            peetsaTest.cpp
            peetsaTestSuites.cpp
            sauce.cpp
   )

# Debug or release ?
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring for debug")
    add_definitions(-DDEBUG=1)
else()
    message(STATUS "Configuring for release")
endif()

# Compiler-dependent and build-dependend flags:
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g -pg")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")
elseif(MSVC)
#     set(SOURCES ${SOURCES} TestResultDebugOut.cpp)
endif()

# And show the flags.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "CXX Flags for the peetsa (Debug) are: ${CMAKE_CXX_FLAGS_DEBUG}")
    set(OUT_NAME "peetsa")
else()
    message(STATUS "CXX Flags for the peetsa (Release) are: ${CMAKE_CXX_FLAGS_RELEASE}")
    set(OUT_NAME "peetsa")
endif()

# The compiling process. #
##########################
add_executable(${OUT_NAME} ${SOURCES})
target_link_libraries (${OUT_NAME} aTest)
