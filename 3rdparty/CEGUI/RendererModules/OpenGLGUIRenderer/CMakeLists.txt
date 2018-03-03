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
set(SOURCES openglrenderer.cpp opengltexture.cpp)

add_library(CEGUIOpenGLRenderer ${SOURCES})

target_compile_definitions(CEGUIOpenGLRenderer PRIVATE $<$<CONFIG:Debug>:DEBUG=1>)
# Add additional include search directories.
target_include_directories(CEGUIOpenGLRenderer PUBLIC ${CEGUIOpenGLRenderer_SOURCE_DIR})
target_include_directories(CEGUIOpenGLRenderer PRIVATE
                    ${_CEGUIOpenGLRenderer_CEGUI_INCLUDE_DIR}
                    ${OPENGL_INCLUDE_DIR}
                   )

# Compiler-dependent and build-dependend flags:
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(CEGUIOpenGLRenderer PRIVATE $<$<CONFIG:Debug>:-pg>)
    target_compile_options(CEGUIOpenGLRenderer PRIVATE -Wall)
endif()
