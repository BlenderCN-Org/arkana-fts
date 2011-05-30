# Bla

project(cal3d)

# Setup the make environement. #
################################

# Allow else() and endif() constructs (with empty paranthesis)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

# Activate very verbose makefiles.
# set(CMAKE_VERBOSE_MAKEFILE ON)

# Disallow in-source builds. #
##############################
EXECUTE_PROCESS(COMMAND pwd OUTPUT_VARIABLE CURR_DIR)
if("${CURR_DIR}" STREQUAL "${cal3d_SOURCE_DIR}\n")
    message(FATAL_ERROR "In-source-builds are not allowed to build the cal3d. Please go into the \"build\" directory and type \"cmake ..\" there.\nThank you.")
endif()

# Search for the different needed libs. #
#########################################

# The std:: namespace.
include(${CMAKE_ROOT}/Modules/TestForSTDNamespace.cmake)

if(${CMAKE_NO_STD_NAMESPACE})
    message(SEND_ERROR "No std:: Namespace is available, This usually means you have no c++ support or no libstdc++")
endif()

# Put all sourcefiles into one variable. #
##########################################
set(SOURCES animation.cpp
            animation_action.cpp
            animation_cycle.cpp
            bone.cpp
            buffersource.cpp
            calxmlbindings.cpp
            coreanimatedmorph.cpp
            coreanimation.cpp
            corebone.cpp
            corekeyframe.cpp
            corematerial.cpp
            coremesh.cpp
            coremodel.cpp
            coremorphkeyframe.cpp
            coremorphtrack.cpp
            coreskeleton.cpp
            coresubmesh.cpp
            coresubmorphtarget.cpp
            coretrack.cpp
            dualquaternion.cpp
            error.cpp
            global.cpp
            hardwaremodel.cpp
            loader.cpp
            matrix.cpp
            mesh.cpp
            mixer.cpp
            model.cpp
            morphtargetmixer.cpp
            physique.cpp
            physiquedualquaternion.cpp
            platform.cpp
            quaternion.cpp
            renderer.cpp
            saver.cpp
            skeleton.cpp
            springsystem.cpp
            streamsource.cpp
            submesh.cpp
            tinybind.cpp
            tinyxml.cpp
            tinyxmlerror.cpp
            tinyxmlparser.cpp
            vector.cpp
            xmlformat.cpp
   )

# Debug or release ?
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring for debug")
    add_definitions(-DDEBUG=1)
else()
    message(STATUS "Configuring for release")
endif()

# Add additional include search directories.
include_directories(${cal3d_SOURCE_DIR}/..)
add_definitions(-DCAL3D_USE_STL_INSTEAD_OF_SSCANF=1)

# Compiler-dependent and build-dependend flags:
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -pg")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")
elseif(MSVC)
endif()

# And show the flags.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "CXX Flags for the cal3d (Debug) are: ${CMAKE_CXX_FLAGS_DEBUG}")
    set(OUT_NAME "cal3d")
else()
    message(STATUS "CXX Flags for the cal3d (Release) are: ${CMAKE_CXX_FLAGS_RELEASE}")
    set(OUT_NAME "cal3d")
endif()

# The compiling process. #
##########################
add_library(${OUT_NAME} ${SOURCES})
# target_link_libraries(fts ${FTS_LIBS})
