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

set(OUT_NAME "dao")
add_library(${OUT_NAME} STATIC ${SOURCES})

# Debug or release ?
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring for debug")
    target_compile_definitions(${OUT_NAME} PRIVATE DEBUG)
else()
    message(STATUS "Configuring for release")
endif()
target_compile_definitions(${OUT_NAME} PRIVATE DAO_WITH_MACRO DAO_WITH_THREAD DAO_WITH_NUMARRAY)
if(MSVC)
    target_compile_definitions(${OUT_NAME} PRIVATE _LIB _CRT_SECURE_NO_WARNINGS NO_FENV)
    target_compile_options(${OUT_NAME} PRIVATE /wd4018 /wd4244)
else()
    target_compile_definitions(${OUT_NAME} PRIVATE UNIX)
    target_compile_options(${OUT_NAME} PRIVATE -Wall -fPIC)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(${OUT_NAME} PRIVATE -pg -o0)
    endif()
endif()

# Add additional include search directories.
target_include_directories(${OUT_NAME} PUBLIC "${dao_SOURCE_DIR}/src")

