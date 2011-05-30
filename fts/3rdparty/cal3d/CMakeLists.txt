# Bla

project(cal3dmeta)

message(STATUS "                     ")
message(STATUS "=====================")
message(STATUS "CONFIGURING cal3dmeta")
message(STATUS "=====================")
message(STATUS "                     ")

# Setup the make environement. #
################################

# Setup the cmake directory containing numerous cmake scripts and macros.
set(CMAKE_MODULE_PATH "${cal3dmeta_SOURCE_DIR}/../../cmake")

# Allow else() and endif() constructs (with empty paranthesis)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

# Activate very verbose makefiles.
# set(CMAKE_VERBOSE_MAKEFILE ON)

# Disallow in-source builds. #
##############################
EXECUTE_PROCESS(COMMAND pwd OUTPUT_VARIABLE CURR_DIR)
if("${CURR_DIR}" STREQUAL "${cal3dmeta_SOURCE_DIR}\n")
    message(FATAL_ERROR "In-source-builds are not allowed to cal3dmeta. Please go into the \"linux\" directory and type \"cmake ..\" there.\nThank you.")
endif()

# Search for the different needed libs. #
#########################################

# The std:: namespace.
include(${CMAKE_ROOT}/Modules/TestForSTDNamespace.cmake)

if(${CMAKE_NO_STD_NAMESPACE})
    message(SEND_ERROR "No std:: Namespace is available, This usually means you have no c++ support or no libstdc++")
endif()

# The different standard libraries already supported by CMake.
# find_package(Threads REQUIRED)

# Some additional headers #
###########################
# Debug or release ?
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring for debug")
    add_definitions(-DDEBUG=1)
else()
    message(STATUS "Configuring for release")
endif()

# Add the different subdirectories needed for cal3dmeta. #
######################################################

add_subdirectory(${cal3dmeta_SOURCE_DIR}/src/cal3d)
# add_subdirectory(${cal3d_SOURCE_DIR}/src)

# Setup the compiler flags. #
#############################

# Compiler-dependent and build-dependend flags:
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -pg")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")
elseif(MSVC)
endif()

# And show the flags.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "CXX Flags for cal3dmeta (Debug) are: ${CMAKE_CXX_FLAGS_DEBUG}")
else()
    message(STATUS "CXX Flags for cal3dmeta (Release) are: ${CMAKE_CXX_FLAGS_RELEASE}")
endif()
