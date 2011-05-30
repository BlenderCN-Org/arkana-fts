# Bla
cmake_minimum_required(VERSION 2.8)
cmake_policy(VERSION 2.8)
cmake_policy(SET CMP0015 NEW)

project(test)

message(STATUS "                 ")
message(STATUS "=================")
message(STATUS "CONFIGURING tests")
message(STATUS "=================")
message(STATUS "                 ")

# Setup the make environement. #
################################

# Allow else() and endif() constructs (with empty paranthesis)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

# Activate very verbose makefiles.
# set(CMAKE_VERBOSE_MAKEFILE ON)

# Disallow in-source builds. #
##############################
EXECUTE_PROCESS(COMMAND pwd OUTPUT_VARIABLE CURR_DIR)
if("${CURR_DIR}" STREQUAL "${test_SOURCE_DIR}\n")
    message(FATAL_ERROR "In-source-builds are not allowed to build the tests. Please go into the \"build\" directory and type \"cmake ..\" there.\nThank you.")
endif()

# Search for the different needed libs. #
#########################################
include_directories(".." "../main" "../dLib")
link_directories("../../dLib/aTest/build")

# Put all sourcefiles into one variable. #
##########################################
set(SOURCES dLib/dString/dStringTest.cpp ../dLib/dString/dString.cpp
            dLib/dString/dPathTest.cpp ../dLib/dString/dPath.cpp
            3d/ResolutionTest.cpp ../3d/Resolution.cpp
   )

add_definitions(-DD_NO_CONF)
add_definitions(-DD_NO_SDL)

# Debug or release ?
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring for debug")
    add_definitions(-DDEBUG=1)
else()
    message(STATUS "Configuring for release")
endif()

# Compiler-dependent and build-dependend flags:
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++0x -Wno-non-virtual-dtor -Wall -O0 -pg -fstack-protector-all -fpermissive")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++0x -Wall -s -O2")
elseif(MSVC)
#     set(SOURCES ${SOURCES} TestResultDebugOut.cpp)
endif()

# And show the flags.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "CXX Flags for the tests (Debug) are: ${CMAKE_CXX_FLAGS_DEBUG}")
else()
    message(STATUS "CXX Flags for the tests (Release) are: ${CMAKE_CXX_FLAGS_RELEASE}")
endif()

# The compiling process. #
##########################
#add_executable(testsCompilerOutput mainCompiler.cpp ${SOURCES})
add_executable(testsNiceOutput mainNice.cpp ${SOURCES})
#target_link_libraries(testsCompilerOutput aTest)
target_link_libraries(testsNiceOutput aTest)

# run the test as a post-build step. #
######################################
add_custom_command(TARGET testsNiceOutput POST_BUILD COMMAND testsNiceOutput fts VERBATIM)
