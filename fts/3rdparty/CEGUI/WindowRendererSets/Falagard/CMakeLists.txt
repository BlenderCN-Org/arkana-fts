# Bla

project(CEGUIFalagardWRBase)

set(_CEGUIFalagardWRBase_CEGUI_INCLUDE_DIR "${CEGUIFalagardWRBase_SOURCE_DIR}/../../include")

# Setup the make environement. #
################################

# Allow else() and endif() constructs (with empty paranthesis)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

# Activate very verbose makefiles.
# set(CMAKE_VERBOSE_MAKEFILE ON)

# Disallow in-source builds. #
##############################
EXECUTE_PROCESS(COMMAND pwd OUTPUT_VARIABLE CURR_DIR)
if("${CURR_DIR}" STREQUAL "${CEGUIFalagardWRBase_SOURCE_DIR}\n")
    message(FATAL_ERROR "In-source-builds are not allowed to build the CEGUIFalagardWRBase. Please go into the \"build\" directory and type \"cmake ..\" there.\nThank you.")
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
set(SOURCES src/FalButton.cpp
            src/FalToggleButton.cpp
            src/FalItemEntry.cpp
            src/FalFrameWindow.cpp
            src/FalScrollbar.cpp
            src/FalDefault.cpp
            src/FalPopupMenu.cpp
            src/FalStaticText.cpp
            src/FalTree.cpp
            src/FalScrollbarProperties.cpp
            src/FalTabButton.cpp
            src/FalItemListbox.cpp
            src/FalMultiColumnList.cpp
            src/FalTitlebar.cpp
            src/FalTabControl.cpp
            src/FalStaticTextProperties.cpp
            src/FalSystemButton.cpp
            src/FalMultiLineEditbox.cpp
            src/FalSlider.cpp
            src/FalMenuItem.cpp
            src/FalListHeader.cpp
            src/FalListbox.cpp
            src/FalTabControlProperties.cpp
            src/FalModule.cpp
            src/FalScrollablePane.cpp
            src/FalStatic.cpp
            src/FalProgressBar.cpp
            src/FalMenubar.cpp
            src/FalStaticImage.cpp
            src/FalTooltip.cpp
            src/FalEditbox.cpp
            src/FalSliderProperties.cpp
            src/FalListHeaderProperties.cpp
            src/FalStaticProperties.cpp
            src/FalProgressBarProperties.cpp
            src/FalStaticImageProperties.cpp
            src/FalListHeaderSegment.cpp
   )

# Debug or release ?
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring for debug")
    add_definitions(-DDEBUG=1)
else()
    message(STATUS "Configuring for release")
endif()

# Add additional include search directories.
include_directories(${CEGUIFalagardWRBase_SOURCE_DIR}/include
                    ${_CEGUIFalagardWRBase_CEGUI_INCLUDE_DIR}
                   )

# Compiler-dependent and build-dependend flags:
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -pg")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")
elseif(MSVC)
endif()

# And show the flags.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "CXX Flags for the CEGUIFalagardWRBase (Debug) are: ${CMAKE_CXX_FLAGS_DEBUG}")
    set(OUT_NAME "CEGUIFalagardWRBased")
else()
    message(STATUS "CXX Flags for the CEGUIFalagardWRBase (Release) are: ${CMAKE_CXX_FLAGS_RELEASE}")
    set(OUT_NAME "CEGUIFalagardWRBase")
endif()

# The compiling process. #
##########################
add_library(${OUT_NAME} ${SOURCES})
# target_link_libraries(fts ${FTS_LIBS})

