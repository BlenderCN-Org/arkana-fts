project(ftsarc)

#set(CMAKE_VERBOSE_MAKEFILE ON)

# Setup the make environement. #
################################
cmake_minimum_required(VERSION 3.9)

# Setup the cmake directory containing numerous cmake scripts and macros.
set(CMAKE_MODULE_PATH "${ftsarc_SOURCE_DIR}/../../cmake")

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

set(CMAKE_VERBOSE_MAKEFILE true)

# Disallow in-source builds. #
##############################
EXECUTE_PROCESS(COMMAND pwd OUTPUT_VARIABLE CURR_DIR)
if("${CURR_DIR}" STREQUAL "${ftsarc_SOURCE_DIR}\n")
    message(FATAL_ERROR "In-source-builds are not allowed to build the Arkana-FTS image converter. Please go into the \"build\" directory and type \"cmake ..\" there.\nThank you.")
endif()

# Search for the different needed libs. #
#########################################

set(LIBS)

# Put all sourcefiles into one variable. #
##########################################
set(SOURCES main.cpp
            archiver.cpp
            dearchiver.cpp
            lister.cpp
            remover.cpp
            sfcompressor.cpp
            internaltester.cpp
            compressorlister.cpp
            ../toolcompat.cpp
            ../../dLib/dArchive/dArchive.cpp
            ../../dLib/dCompressor/dCompressor.cpp
            ../../dLib/dCompressor/minilzo_compressor.cpp
            ../../dLib/dCompressor/minilzo/minilzo.c
            ../../dLib/dFile/dFile.cpp
            ../../dLib/dString/dString.cpp
            ../../dLib/dString/dPath.cpp
            ../../logging/Chronometer.cpp
            ../../logging/logger.cpp
            ../../utilities/DataContainer.cpp
            ../../utilities/StreamedDataContainer.cpp
            ../../utilities/Singleton.cpp
            ../../main/Exception.cpp
            ../../main/Clock.cpp
   )

add_executable(ftsarc ${SOURCES})
target_compile_definitions(ftsarc PRIVATE $<$<CONFIG:Debug>:DEBUG=1>)
set_property(TARGET ftsarc PROPERTY CXX_STANDARD 14)
set_property(TARGET ftsarc PROPERTY CXX_STANDARD_REQUIRED ON)

# Add additional source directories.
include_directories(${ftsarc_SOURCE_DIR}/../..
                    ${ftsarc_SOURCE_DIR}/..
                   )

# Add necessary defines:
target_compile_definitions(ftsarc PRIVATE D_STRING_FTS_LIBS D_NOCEGUI D_NONET)

# Compiler-dependent and build-dependent flags:
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(ftsarc PRIVATE Wall $<$<CONFIG:Debug>:-O0> $<$<CONFIG:Debug>:-g> $<$<CONFIG:Debug>:-pg> )
elseif(MSVC)
    target_compile_definitions(ftsarc PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
endif()

# The compiling process. #
##########################
target_link_libraries(ftsarc ${LIBS})

# The installation process. #
#############################
install(TARGETS ftsarc DESTINATION bin)
