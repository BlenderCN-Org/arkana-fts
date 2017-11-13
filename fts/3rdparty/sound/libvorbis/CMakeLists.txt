# Bla

project(vorbis C)

message(STATUS "                  ")
message(STATUS "==================")
message(STATUS "CONFIGURING VORBIS")
message(STATUS "==================")
message(STATUS "                  ")

# Setup the make environement. #
################################

# Setup the cmake directory containing numerous cmake scripts and macros.
set(CMAKE_MODULE_PATH "${vorbis_SOURCE_DIR}/../../../cmake")

# Allow else() and endif() constructs (with empty paranthesis)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

# Activate very verbose makefiles.
# set(CMAKE_VERBOSE_MAKEFILE ON)

# Disallow in-source builds. #
##############################
EXECUTE_PROCESS(COMMAND pwd OUTPUT_VARIABLE CURR_DIR)
if("${CURR_DIR}" STREQUAL "${vorbis_SOURCE_DIR}\n")
    message(FATAL_ERROR "In-source-builds are not allowed to build the libvorbis. Please go into the \"build\" directory and type \"cmake ..\" there.\nThank you.")
endif()

# Search for the different needed libs. #
#########################################

# Put all sourcefiles into one variable. #
##########################################
set(SOURCES lib/analysis.c
            lib/bitrate.c
            lib/block.c
            lib/codebook.c
            lib/envelope.c
            lib/floor0.c
            lib/floor1.c
            lib/info.c
            lib/lookup.c
            lib/lpc.c
            lib/lsp.c
            lib/mapping0.c
            lib/mdct.c
            lib/psy.c
            lib/registry.c
            lib/res0.c
            lib/sharedbook.c
            lib/smallft.c
            lib/synthesis.c
            lib/vorbisenc.c
            lib/vorbisfile.c
            lib/window.c
   )

# Debug or release ?
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring for debug")
    add_definitions(-DDEBUG=1)
else()
    message(STATUS "Configuring for release")
endif()

# Add additional include search directories.
include_directories("${vorbis_SOURCE_DIR}/../libogg/include" "${vorbis_SOURCE_DIR}/include" "${vorbis_SOURCE_DIR}/lib")

# Compiler-dependent and build-dependend flags:
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -pg")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")
elseif(MSVC)
    FOREACH(flag_var
        CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
        )
        IF(${flag_var} MATCHES "/MD")
            STRING(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        ENDIF()
    ENDFOREACH(flag_var)
endif()

# And show the flags.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "C Flags for the libvorbis (Debug) are: ${CMAKE_C_FLAGS_DEBUG}")
    set(OUT_NAME "vorbis")
else()
    message(STATUS "C Flags for the libvorbis (Release) are: ${CMAKE_C_FLAGS_RELEASE}")
    set(OUT_NAME "vorbis")
endif()

# The compiling process. #
##########################
add_library(${OUT_NAME} STATIC ${SOURCES})
