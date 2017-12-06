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

find_package(OpenGL REQUIRED)
find_package(GLU REQUIRED)

# Put all sourcefiles into one variable. #
##########################################
set(SOURCES openglrenderer.cpp
            opengltexture.cpp
   )

add_library(CEGUIOpenGLRenderer ${SOURCES})

# Debug or release ?
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring for debug")
    target_compile_definitions(CEGUIOpenGLRenderer PRIVATE DEBUG=1)
else()
    message(STATUS "Configuring for release")
endif()

# Add additional include search directories.
target_include_directories(CEGUIOpenGLRenderer PUBLIC ${CEGUIOpenGLRenderer_SOURCE_DIR})
target_include_directories(CEGUIOpenGLRenderer PRIVATE
                    ${_CEGUIOpenGLRenderer_CEGUI_INCLUDE_DIR}
                    ${OPENGL_INCLUDE_DIR}
                   )

# Compiler-dependent and build-dependend flags:
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    add_definitions("-Wall")
    if(CAMKE_BUILD_TYPE STREQUAL "Debug")
        add_definitions("-pg")
    endif()
endif()
