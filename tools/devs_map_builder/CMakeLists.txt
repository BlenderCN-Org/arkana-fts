cmake_minimum_required(VERSION 3.1)

project(devs_map_builder)

message(STATUS "============================")
message(STATUS "CONFIGURING devs_map_builder")
message(STATUS "============================")

# Setup the make environement. #
################################

# Allow else() and endif() constructs (with empty paranthesis)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

# De-Activate very verbose makefiles.
set(CMAKE_VERBOSE_MAKEFILE OFF)

# Remove unwanted project types in MSVC
if(MSVC)
    list(REMOVE_ITEM CMAKE_CONFIGURATION_TYPES MinSizeRel RelWithDebInfo)
endif()

# Disallow in-source builds. #
##############################
EXECUTE_PROCESS(COMMAND pwd OUTPUT_VARIABLE CURR_DIR)
if("${CURR_DIR}" STREQUAL "${fts_SOURCE_DIR}\n")
    message(FATAL_ERROR "In-source-builds are not allowed to build the project. Please go into the \"build\" directory and type \"cmake ..\" there.\nThank you.")
endif()

if(MSVC)
    FOREACH(flag_var CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE)
        IF(${flag_var} MATCHES "/MD")
            STRING(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        ENDIF()
    ENDFOREACH(flag_var)
    FOREACH(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE )
        IF(${flag_var} MATCHES "/MD")
            STRING(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        ENDIF()
    ENDFOREACH(flag_var)
endif()

##########################################

add_executable(devs_map_builder main.cpp)

set_property(TARGET devs_map_builder PROPERTY CXX_STANDARD 17)
set_property(TARGET devs_map_builder PROPERTY CXX_STANDARD_REQUIRED ON)

if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions("-Wall")
    target_compile_options(devs_map_builder PRIVATE $<$<CONFIG:Debug>:-O0>)
    target_compile_options(devs_map_builder PRIVATE $<$<CONFIG:Debug>:-pg>)
    target_compile_options(devs_map_builder PRIVATE $<$<CONFIG:Debug>:-fstack-protector-all>)
    target_compile_options(devs_map_builder PRIVATE $<$<CONFIG:Debug>:-Wno-non-virtual-dtor>)
    target_compile_options(devs_map_builder PRIVATE $<$<CONFIG:Release>:-s>)
endif()

target_compile_definitions(devs_map_builder PRIVATE $<$<CONFIG:Debug>:DEBUG=1>)
target_compile_definitions(devs_map_builder PRIVATE $<$<CONFIG:Debug>:_DEBUG>)

# Add additional defines:
if(WIN32)
    target_compile_definitions(devs_map_builder PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(devs_map_builder PRIVATE _CRT_SECURE_NO_DEPRECATE)
    target_compile_definitions(devs_map_builder PRIVATE WIN32_LEAN_AND_MEAN=1)
    target_compile_definitions(devs_map_builder PRIVATE _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
endif()

