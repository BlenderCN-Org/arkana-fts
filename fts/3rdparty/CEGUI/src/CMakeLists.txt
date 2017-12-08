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

find_package(PCRE)
if(${PCRE_FOUND} STREQUAL "NO")
    message(STATUS "TODO: Could not find PCRE installed on your system. Taking our own version.")
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
            CEGUIDynamicModule.cpp
   )

add_library(CEGUIBase ${SOURCES})

target_compile_definitions(CEGUIBase PRIVATE $<$<BOOL:PCRE_FOUND>:PCRE_STATIC>)
target_compile_definitions(CEGUIBase PRIVATE $<$<CONFIG:Debug>:DEBUG=1>)

# Add additional include search directories.
target_include_directories(CEGUIBase PUBLIC ${_CEGUIBase_CEGUI_INCLUDE_DIR})
target_include_directories(CEGUIBase PRIVATE ${PCRE_INCLUDE_DIR}
                    ${FreeType_INCLUDE_DIR}
                    ${FreeType_INCLUDE_DIR}/..)
                    
# Compiler-dependent and build-dependend flags:
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(CEGUIBase PRIVATE -Wall)
    target_compile_options(CEGUIBase PRIVATE $<$<CONFIG:Debug>:-pg>)
endif()
