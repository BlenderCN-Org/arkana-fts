# Bla
cmake_minimum_required(VERSION 2.4)

project(aTest)

message(STATUS "                 ")
message(STATUS "=================")
message(STATUS "CONFIGURING aTest")
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
if("${CURR_DIR}" STREQUAL "${aTest_SOURCE_DIR}\n")
    message(FATAL_ERROR "In-source-builds are not allowed to build aTest. Please go into the \"build\" directory and type \"cmake ..\" there.\nThank you.")
endif()

# Search for the different needed libs. #
#########################################

# Put all sourcefiles into one variable. #
##########################################
set(SOURCES Test.cpp
            TestRegistry.cpp
            TestResult.cpp
            TestResultCompiler.cpp
            TestResultCombiner.cpp
            TestResultNiceOutput.cpp
            TestResultSQL.cpp
            TestSuite.cpp
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
    set(SOURCES ${SOURCES} TestResultDebugOut.cpp)
endif()

# And show the flags.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "CXX Flags for the aTest (Debug) are: ${CMAKE_CXX_FLAGS_DEBUG}")
    set(OUT_NAME "aTest")
else()
    message(STATUS "CXX Flags for the aTest (Release) are: ${CMAKE_CXX_FLAGS_RELEASE}")
    set(OUT_NAME "aTest")
endif()

enable_testing()

# Build the examples and tests #
################################
# add an option for building the examples
if(NOT DEFINED ATEST_BUILD_EXAMPLES)
    set(ATEST_BUILD_EXAMPLES TRUE)
endif()
set(BUILD_EXAMPLES ${ATEST_BUILD_EXAMPLES} CACHE BOOL "TRUE to build the aTest examples, FALSE to ignore them")

if(${ATEST_BUILD_EXAMPLES})
    add_subdirectory(examples/pizza)

    add_test(PeetsaGeneral examples/pizza/peetsa -TS)
    add_test(PeetsaDough examples/pizza/peetsa TS_Dough)
    add_test(PeetsaSauce examples/pizza/peetsa TS_Sauce)
    add_test(PeetsaIngredients examples/pizza/peetsa TS_Ingredients)
endif()

# The compiling process. #
##########################
add_library(${OUT_NAME} STATIC ${SOURCES})
