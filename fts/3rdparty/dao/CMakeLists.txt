#

project(dao)

message(STATUS "               ")
message(STATUS "===============")
message(STATUS "CONFIGURING DAO")
message(STATUS "===============")
message(STATUS "               ")

# Setup the make environement. #
################################

# Setup the cmake directory containing numerous cmake scripts and macros.
set(CMAKE_MODULE_PATH "${dao_SOURCE_DIR}/../../../cmake")

# Allow else() and endif() constructs (with empty paranthesis)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

# Activate very verbose makefiles.
# set(CMAKE_VERBOSE_MAKEFILE ON)

# Disallow in-source builds. #
##############################
EXECUTE_PROCESS(COMMAND pwd OUTPUT_VARIABLE CURR_DIR)
if("${CURR_DIR}" STREQUAL "${dao_SOURCE_DIR}\n")
    message(FATAL_ERROR "In-source-builds are not allowed to build the libdao. Please go into the \"build\" directory and type \"cmake ..\" there.\nThank you.")
endif()

# Search for the different needed libs. #
#########################################

# Put all sourcefiles into one variable. #
##########################################
set(SOURCES src/daoType.c
        src/daoNamespace.c
        src/daoGC.c
        src/daoMaindl.c
        src/daoClass.c
        src/daoLexer.c
        src/daoParser.c
        src/daoMacro.c
        src/daoAsmbc.c
        src/daoRegex.c
        src/daoValue.c
        src/daoContext.c
        src/daoProcess.c
        src/daoJit.c
        src/daoStdlib.c
        src/daoArray.c
        src/daoMap.c
        src/daoConst.c
        src/daoRoutine.c
        src/daoObject.c
        src/daoThread.c
        src/daoNetwork.c
        src/daoSched.c
        src/daoStream.c
        src/daoString.c
        src/daoVmspace.c
        src/daoNumtype.c
        src/daoStdtype.c
   )

# Debug or release ?
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring for debug")
    add_definitions(-DDEBUG)
else()
    message(STATUS "Configuring for release")
endif()
if(MSVC)
    add_definitions(-DDAO_WITH_MACRO -DDAO_WITH_THREAD -DDAO_WITH_NUMARRAY -D_LIB -D_CRT_SECURE_NO_WARNINGS -DNO_FENV)
else()
    add_definitions(-DUNIX -DDAO_WITH_MACRO -DDAO_WITH_THREAD -DDAO_WITH_NUMARRAY)
endif()

# Add additional include search directories.
include_directories("${dao_SOURCE_DIR}/src")

# Compiler-dependent and build-dependend flags:
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -fPIC -pg")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -fPIC")
elseif(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /wd4018 /wd4244")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /wd4018 /wd4244")
endif()

# And show the flags.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "CXX Flags for the dao (Debug) are: ${CMAKE_CXX_FLAGS_DEBUG}")
    set(OUT_NAME "dao")
else()
    message(STATUS "CXX Flags for the dao (Release) are: ${CMAKE_CXX_FLAGS_RELEASE}")
    set(OUT_NAME "dao")
endif()

# The compiling process. #
##########################
add_library(${OUT_NAME} STATIC ${SOURCES})
