# Bla

project(CEGUIOpenGLRenderer)

set(_CEGUIOpenGLRenderer_CEGUI_INCLUDE_DIR "${CEGUIOpenGLRenderer_SOURCE_DIR}/../../include")

# Setup the make environement. #
################################

# Allow else() and endif() constructs (with empty paranthesis)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

# Activate very verbose makefiles.
# set(CMAKE_VERBOSE_MAKEFILE ON)

# Disallow in-source builds. #
##############################
EXECUTE_PROCESS(COMMAND pwd OUTPUT_VARIABLE CURR_DIR)
if("${CURR_DIR}" STREQUAL "${CEGUIOpenGLRenderer_SOURCE_DIR}\n")
    message(FATAL_ERROR "In-source-builds are not allowed to build the CEGUIOpenGLRenderer. Please go into the \"build\" directory and type \"cmake ..\" there.\nThank you.")
endif()

# Search for the different needed libs. #
#########################################

# The std:: namespace.
include(${CMAKE_ROOT}/Modules/TestForSTDNamespace.cmake)

if(${CMAKE_NO_STD_NAMESPACE})
    message(SEND_ERROR "No std:: Namespace is available, This usually means you have no c++ support or no libstdc++")
endif()

find_package(OpenGL REQUIRED)
find_package(GLU REQUIRED)

# Put all sourcefiles into one variable. #
##########################################
set(SOURCES openglrenderer.cpp
            opengltexture.cpp
   )

# Debug or release ?
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring for debug")
    add_definitions(-DDEBUG=1)
else()
    message(STATUS "Configuring for release")
endif()

# Add additional include search directories.
include_directories(${CEGUIOpenGLRenderer_SOURCE_DIR}
                    ${_CEGUIOpenGLRenderer_CEGUI_INCLUDE_DIR}
                    ${OPENGL_INCLUDE_DIR}
                   )

# Compiler-dependent and build-dependend flags:
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -pg")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")
elseif(MSVC)
endif()

# And show the flags.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "CXX Flags for the CEGUIOpenGLRenderer (Debug) are: ${CMAKE_CXX_FLAGS_DEBUG}")
    set(OUT_NAME "CEGUIOpenGLRendererd")
else()
    message(STATUS "CXX Flags for the CEGUIOpenGLRenderer (Release) are: ${CMAKE_CXX_FLAGS_RELEASE}")
    set(OUT_NAME "CEGUIOpenGLRenderer")
endif()

# The compiling process. #
##########################
add_library(${OUT_NAME} ${SOURCES})
# target_link_libraries(fts ${FTS_LIBS})

