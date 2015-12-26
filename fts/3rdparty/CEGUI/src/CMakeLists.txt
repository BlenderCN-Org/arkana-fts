# Bla

project(CEGUIBase)

set(_CEGUIBase_CEGUI_INCLUDE_DIR "${CEGUIBase_SOURCE_DIR}/../include")

# Setup the make environement. #
################################

# Setup the cmake directory containing numerous cmake scripts and macros.
set(CMAKE_MODULE_PATH "${CEGUIBase_SOURCE_DIR}/../../../cmake")

# Allow else() and endif() constructs (with empty paranthesis)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

# Activate very verbose makefiles.
# set(CMAKE_VERBOSE_MAKEFILE ON)

# Disallow in-source builds. #
##############################
EXECUTE_PROCESS(COMMAND pwd OUTPUT_VARIABLE CURR_DIR)
if("${CURR_DIR}" STREQUAL "${CEGUIBase_SOURCE_DIR}\n")
    message(FATAL_ERROR "In-source-builds are not allowed to build the CEGUIBase. Please go into the \"build\" directory and type \"cmake ..\" there.\nThank you.")
endif()

# Search for the different needed libs. #
#########################################

# The std:: namespace.
include(${CMAKE_ROOT}/Modules/TestForSTDNamespace.cmake)

if(${CMAKE_NO_STD_NAMESPACE})
    message(SEND_ERROR "No std:: Namespace is available, This usually means you have no c++ support or no libstdc++")
endif()

find_package(PCRE)
if(${PCRE_FOUND} STREQUAL "NO")
    message(STATUS "TODO: Could not find PCRE installed on your system. Taking our own version.")
    add_definitions(-DPCRE_STATIC)
endif()

find_package(FreeType)
if(${FreeType_FOUND} STREQUAL "NO")
    message(STATUS "TODO: Could not find FreeType installed on your system. Taking our own version.")
endif()

# Put all sourcefiles into one variable. #
##########################################
set(SOURCES elements/CEGUIScrollbar.cpp
            elements/CEGUICheckbox.cpp
            elements/CEGUIPopupMenu.cpp
            elements/CEGUIItemEntryProperties.cpp
            elements/CEGUIFrameWindowProperties.cpp
            elements/CEGUIScrollbarProperties.cpp
            elements/CEGUIItemListbox.cpp
            elements/CEGUIMultiColumnList.cpp
            elements/CEGUITitlebar.cpp
            elements/CEGUITabControl.cpp
            elements/CEGUIRadioButtonProperties.cpp
            elements/CEGUIThumb.cpp
            elements/CEGUIItemListboxProperties.cpp
            elements/CEGUIMultiLineEditbox.cpp
            elements/CEGUISlider.cpp
            elements/CEGUIMenuItem.cpp
            elements/CEGUIClippedContainer.cpp
            elements/CEGUITabControlProperties.cpp
            elements/CEGUIComboboxProperties.cpp
            elements/CEGUIScrollablePane.cpp
            elements/CEGUIProgressBar.cpp
            elements/CEGUIMenubar.cpp
            elements/CEGUIThumbProperties.cpp
            elements/CEGUIDragContainerProperties.cpp
            elements/CEGUIMultiLineEditboxProperties.cpp
            elements/CEGUIListHeaderProperties.cpp
            elements/CEGUIListboxProperties.cpp
            elements/CEGUIGroupBox.cpp
            elements/CEGUIProgressBarProperties.cpp
            elements/CEGUIListHeaderSegment.cpp
            elements/CEGUITooltipProperties.cpp
            elements/CEGUIPushButton.cpp
            elements/CEGUIEditboxProperties.cpp
            elements/CEGUIScrolledItemListBase.cpp
            elements/CEGUIItemEntry.cpp
            elements/CEGUIFrameWindow.cpp
            elements/CEGUIListHeaderSegmentProperties.cpp
            elements/CEGUIItemListBase.cpp
            elements/CEGUIScrolledContainerProperties.cpp
            elements/CEGUIScrolledItemListBaseProperties.cpp
            elements/CEGUIRadioButton.cpp
            elements/CEGUISpinnerProperties.cpp
            elements/CEGUITree.cpp
            elements/CEGUITabButton.cpp
            elements/CEGUIItemListBaseProperties.cpp
            elements/CEGUICheckboxProperties.cpp
            elements/CEGUIPopupMenuProperties.cpp
            elements/CEGUIButtonBase.cpp
            elements/CEGUICombobox.cpp
            elements/CEGUIListboxTextItem.cpp
            elements/CEGUIBaseFactories.cpp
            elements/CEGUITreeProperties.cpp
            elements/CEGUIGUISheet.cpp
            elements/CEGUITreeItem.cpp
            elements/CEGUIDragContainer.cpp
            elements/CEGUIMultiColumnListProperties.cpp
            elements/CEGUITitlebarProperties.cpp
            elements/CEGUIListHeader.cpp
            elements/CEGUIListbox.cpp
            elements/CEGUITooltip.cpp
            elements/CEGUIEditbox.cpp
            elements/CEGUISliderProperties.cpp
            elements/CEGUIMenuBase.cpp
            elements/CEGUIScrollablePaneProperties.cpp
            elements/CEGUIListboxItem.cpp
            elements/CEGUIScrolledContainer.cpp
            elements/CEGUIMenuBaseProperties.cpp
            elements/CEGUISpinner.cpp
            elements/CEGUIComboDropList.cpp
            falagard/CEGUIFalStateImagery.cpp
            falagard/CEGUIFalWidgetLookManager.cpp
            falagard/CEGUIFalWidgetLookFeel.cpp
            falagard/CEGUIFalSectionSpecification.cpp
            falagard/CEGUIFalWidgetComponent.cpp
            falagard/CEGUIFalTextComponent.cpp
            falagard/CEGUIFalDimensions.cpp
            falagard/CEGUIFalPropertyLinkDefinition.cpp
            falagard/CEGUIFalNamedArea.cpp
            falagard/CEGUIFalLayerSpecification.cpp
            falagard/CEGUIFalFrameComponent.cpp
            falagard/CEGUIFalImageryComponent.cpp
            falagard/CEGUIFalPropertyInitialiser.cpp
            falagard/CEGUIFalImagerySection.cpp
            falagard/CEGUIFalXMLEnumHelper.cpp
            falagard/CEGUIFalPropertyDefinitionBase.cpp
            falagard/CEGUIFalagard_xmlHandler.cpp
            falagard/CEGUIFalPropertyDefinition.cpp
            falagard/CEGUIFalComponentBase.cpp
            CEGUIEvent.cpp
            CEGUIColourRect.cpp
            CEGUIConfig_xmlHandler.cpp
            CEGUISystem.cpp
            CEGUIPixmapFont.cpp
            CEGUIRect.cpp
            CEGUISchemeManager.cpp
            CEGUITexture.cpp
            CEGUIPropertySet.cpp
            CEGUIWindowManager.cpp
            CEGUITextUtils.cpp
            CEGUIRenderer.cpp
            CEGUIDefaultResourceProvider.cpp
            CEGUIXMLParser.cpp
            CEGUIScheme_xmlHandler.cpp
            CEGUIImage.cpp
            CEGUIWindowRendererManager.cpp
            CEGUIImagesetManager.cpp
            CEGUIXMLSerializer.cpp
            CEGUIEventSignalSet.cpp
            CEGUIScriptWindowHelper.cpp
            CEGUIEventSet.cpp
            CEGUIWindowFactory.cpp
            CEGUIcolour.cpp
            CEGUIFreeTypeFont.cpp
            CEGUIImageset_xmlHandler.cpp
            CEGUIEventSignal.cpp
            CEGUILogger.cpp
            CEGUIImageCodec.cpp
            CEGUIFontProperties.cpp
            CEGUIPropertyHelper.cpp
            CEGUIProperty.cpp
            CEGUIMouseCursor.cpp
            CEGUIXMLHandler.cpp
            CEGUIWindowRenderer.cpp
            CEGUIEventArgs.cpp
            CEGUICoordConverter.cpp
            CEGUIImageset.cpp
            CEGUIFontManager.cpp
            CEGUISubscriberSlot.cpp
            CEGUIWindow.cpp
            CEGUISize.cpp
            CEGUIGlobalEventSet.cpp
            CEGUIExceptions.cpp
            CEGUIWindowProperties.cpp
            CEGUIDefaultLogger.cpp
            CEGUIFont_xmlHandler.cpp
            CEGUIWindowFactoryManager.cpp
            CEGUIDynamicModule.cpp
            CEGUIGUILayout_xmlHandler.cpp
            CEGUIRenderCache.cpp
            CEGUIScheme.cpp
            CEGUIString.cpp
            CEGUIFont.cpp
            CEGUIXMLAttributes.cpp
            CEGUIBoundSlot.cpp
            CEGUIScriptModule.cpp
            CEGUIBase.cpp
            CEGUIDataContainer.cpp
            CEGUIFactoryModule.cpp
   )

# Debug or release ?
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring for debug")
    add_definitions(-DDEBUG=1)
else()
    message(STATUS "Configuring for release")
endif()

# Add additional include search directories.
include_directories(${CEGUIBase_SOURCE_DIR}/include
                    ${_CEGUIBase_CEGUI_INCLUDE_DIR}
                    ${PCRE_INCLUDE_DIR}
                    ${FreeType_INCLUDE_DIR}
                    ${FreeType_INCLUDE_DIR}/..
                   )

# Compiler-dependent and build-dependend flags:
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -pg")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")
elseif(MSVC)
endif()

# And show the flags.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "CXX Flags for the CEGUIBase (Debug) are: ${CMAKE_CXX_FLAGS_DEBUG}")
    set(OUT_NAME "CEGUIBased")
else()
    message(STATUS "CXX Flags for the CEGUIBase (Release) are: ${CMAKE_CXX_FLAGS_RELEASE}")
    set(OUT_NAME "CEGUIBase")
endif()

# The compiling process. #
##########################
add_library(${OUT_NAME} ${SOURCES})
# target_link_libraries(fts ${FTS_LIBS})

