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

add_library(CEGUIFalagardWRBase ${SOURCES})

# Debug or release ?
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring for debug")
    target_compile_definitions(CEGUIFalagardWRBase PRIVATE DEBUG=1)
else()
    message(STATUS "Configuring for release")
endif()

# Add additional include search directories.
target_include_directories(CEGUIFalagardWRBase PUBLIC ${_CEGUIFalagardWRBase_CEGUI_INCLUDE_DIR})

target_include_directories(CEGUIFalagardWRBase PRIVATE  ${CEGUIFalagardWRBase_SOURCE_DIR}/include)

# Compiler-dependent and build-dependend flags:
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    add_definitions("-Wall")
    if(CAMKE_BUILD_TYPE STREQUAL "Debug")
        add_definitions("-pg")
    endif()
endif()
