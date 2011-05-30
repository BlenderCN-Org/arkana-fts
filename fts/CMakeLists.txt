# Bla
cmake_minimum_required(VERSION 2.6)

# The following lines will be used later :-)
# set(CMAKE_CXX_COMPILER "g++-4.6")
# set(CMAKE_C_COMPILER "gcc-4.6")

project(fts)

message(STATUS "               ")
message(STATUS "===============")
message(STATUS "CONFIGURING FTS")
message(STATUS "===============")
message(STATUS "               ")

# Setup the make environement. #
################################



# Setup the cmake directory containing numerous cmake scripts and macros.
set(CMAKE_MODULE_PATH "${fts_SOURCE_DIR}/cmake")

# Allow else() and endif() constructs (with empty paranthesis)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

# Activate very verbose makefiles.
set(CMAKE_VERBOSE_MAKEFILE ON)

# Disallow in-source builds. #
##############################
EXECUTE_PROCESS(COMMAND pwd OUTPUT_VARIABLE CURR_DIR)
if("${CURR_DIR}" STREQUAL "${fts_SOURCE_DIR}\n")
    message(FATAL_ERROR "In-source-builds are not allowed to build the Arkana-FTS server. Please go into the \"linux\" directory and type \"cmake ..\" there.\nThank you.")
endif()

# Put all sourcefiles into one variable. #
##########################################
set(SOURCES_COMMON
            3rdparty/scrap.cpp
            3rdparty/lodepng.cpp
            tools/server2/checksum/md5.cpp
            tools/server2/checksum/sha2.cpp
            3d/math/AxisAlignedBoundingBox.cpp
            3d/math/Matrix.cpp
            3d/math/Quaternion.cpp
            3d/math/Vector.cpp
            3d/Movers/Mover.cpp
            3d/Movers/Orbiter.cpp
            3d/Movers/Translator.cpp
            3d/Movers/Rotator.cpp
            3d/3d.cpp
            3d/camera.cpp
            3d/DefaultShaders.cpp
            3d/ErrorMesh.cpp
            3d/ModelInstance.cpp
            3d/ModelManager.cpp
            3d/HardwareModel.cpp
            3d/light.cpp
            3d/opengl_wrapper.cpp
            3d/Renderer.cpp
            3d/Resolution.cpp
            3d/Shader.cpp
            3d/VertexArrayObject.cpp
            dLib/dArchive/dArchive.cpp
            dLib/dConf/configuration.cpp
            dLib/dConf/ArkanaDefaultSettings.cpp
            dLib/dConf/DefaultOptions.cpp
            dLib/dFile/dFile.cpp
            dLib/dProcess/dProcess.cpp
            dLib/dString/dString.cpp
            dLib/dString/dPath.cpp
            dLib/dCompressor/dCompressor.cpp
            dLib/dCompressor/minilzo/minilzo.c
            dLib/dCompressor/minilzo_compressor.cpp
            game/objects/objects.cpp
            game/objects/tree.cpp
            game/game_rlv.cpp
            game/loadgame_rlv.cpp
            game/player.cpp
            game/player_og.cpp
            graphic/anim.cpp
            graphic/cegui_ftsimg_codec.cpp
            graphic/errtex.cpp
            graphic/graphic.cpp
            graphic/image.cpp
            graphic/Color.cpp
            input/Cursor.cpp
            input/InputManager.cpp
            input/keys.cpp
            logging/Chronometer.cpp
            logging/ftslogger.cpp
            logging/logger.cpp
            main/Clock.cpp
            main/Exception.cpp
            main/load_fts_rlv.cpp
            main/runlevels.cpp
            main/Updateable.cpp
            main/version.cpp
            map/DecorativeMO.cpp
            map/forest.cpp
            map/map.cpp
            map/MapObject.cpp
            map/mapinfo.cpp
            map/quad.cpp
            map/terrain.cpp
            map/tile.cpp
            mdlviewer/mdlviewer_main.cpp
            net/connection.cpp
            net/packet.cpp
            sound/sndobjpool.cpp
            sound/SndFile.cpp
            sound/SndGrp.cpp
            sound/SndObjOpenAL.cpp
            sound/SndSys.cpp
            sound/SndSysOpenAL.cpp
            sound/SndPlayList.cpp
            ui/cegui_items/progressbar/ArkanaProgressBarWR.cpp
            ui/cegui_items/progressbar/ArkanaProgressBarWRProperties.cpp
            ui/cegui_items/ArkanaResourceProvider.cpp
            ui/cegui_items/chat.cpp
            ui/cegui_items/ColorListItem.cpp
            ui/cegui_items/imaged_list_item.cpp
            ui/cegui_items/simple_list_item.cpp
            ui/confirm_dialog.cpp
            ui/dlg_online_acctInfo.cpp
            ui/dlg_online_findGame.cpp
            ui/dlg_online_joinchannel.cpp
            ui/dlg_online_newGame.cpp
            ui/dlg_online_feedback.cpp
            ui/dlg_options.cpp
            ui/dlg_options_adv_video.cpp
            ui/file_dialog.cpp
            ui/ui_commands.cpp
            ui/ui_menu.cpp
            ui/ui_menu_online.cpp
            ui/ui_menu_online_main.cpp
            ui/ui.cpp
            ui/ScriptConsole.cpp
            utilities/DataContainer.cpp
            utilities/Singleton.cpp
            utilities/StreamedDataContainer.cpp
            utilities/command.cpp
            utilities/console.cpp
            utilities/fps_calculator.cpp
            utilities/radix.cpp
            utilities/string_file.cpp
            utilities/parse.cpp
            utilities/utilities.cpp
            utilities/threading.cpp
            scripting/dao_snd.cpp
            scripting/dao_snd2.cpp
            scripting/dao_snd3.cpp
            scripting/DaoVm.cpp
            scripting/Music.cpp
            scripting/DaoFunction.cpp
            scripting/hotkey.cpp
   )

set(SOURCES_GAME main/main.cpp)

if(UNIX)
    set(SOURCES_COMMON ${SOURCES_COMMON} dLib/dBrowse/linux.cpp)
else()
    set(SOURCES_COMMON ${SOURCES_COMMON} dLib/dBrowse/windows.cpp)
endif()

# Check for various language-stuff and compiler versions #
##########################################################

# GCC does not yet support nullptr. Later we need to make this version-dependent.
# Compiler-dependent and build-dependend flags:
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
    string(STRIP "${GCC_VERSION}" GCC_VERSION)
    message(STATUS "GCC Version ${GCC_VERSION} detected")
    message(STATUS "GCC is ${CMAKE_CXX_COMPILER} detected")

    # GCC below 4.4 does not have C++0x features we use.
    if(${GCC_VERSION} VERSION_LESS "4.4")
        message(FATAL_ERROR "You need at least GCC version 4.4 in order to compile Arkana-FTS")
    endif()

    # Note that GCC 4.5 has a known bug with our nullptr workaround.
    if(${GCC_VERSION} VERSION_EQUAL "4.5")
        add_definitions(-DD_GCC_IS_45)
        message(WARNING "Some GCC 4.5 subversions are known to be broken. Please use GCC 4.4 or 4.6 or higher. Good luck!")
    endif()

    # Do we need to emulate C++0x nullptr constant?
    if(${GCC_VERSION} VERSION_LESS "4.6")
        add_definitions(-DD_NEED_NULLPTR_WORKAROUND)
        message(STATUS "GCC Version is less than 4.6, using nullptr workaround")
    endif()
elseif(MSVC)
endif()

# Search for the different needed libs. #
#########################################

set(FTS_LIBS)
set(FTS_INCLUDES)

# The std:: namespace.
include(${CMAKE_ROOT}/Modules/TestForSTDNamespace.cmake)

if(${CMAKE_NO_STD_NAMESPACE})
    message(SEND_ERROR "No std:: Namespace is available, This usually means you have no c++ support or no libstdc++")
endif()

# The different standard libraries already supported by CMake.
find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLU REQUIRED)

find_package(SDL)
if(${SDL_FOUND} STREQUAL "NO")
    message(FATAL_ERROR "Could not find SDL installed on your system. This means you have to install the SDL development files. This can be done by installing a package called something in the lines of SDL-devel or SDL-dev.")
else()
    message(STATUS "Found SDL in ${SDL_INCLUDE_DIR}: ${SDL_LIBRARY}")
    list(APPEND FTS_INCLUDES ${SDL_INCLUDE_DIR})
    list(APPEND FTS_LIBS ${SDL_LIBRARY})
endif()

set(PCRE_FIND_QUIETLY 1)
find_package(PCRE)
if(${PCRE_FOUND} STREQUAL "NO")
    message(FATAL_ERROR "Could not find PCRE installed on your system. This means you have to install the PCRE development files. This can be done by installing a package called something in the lines of PCRE-devel or PCRE-dev.")
else()
    message(STATUS "Found PCRE in ${PCRE_INCLUDE_DIR}: ${PCRE_LIBRARY}")
    list(APPEND FTS_INCLUDES ${PCRE_INCLUDE_DIR})
    list(APPEND FTS_LIBS ${PCRE_LIBRARIES})
endif()

set(FreeType_FIND_QUIETLY 1)
find_package(FreeType)
if(${FreeType_FOUND} STREQUAL "NO")
    message(FATAL_ERROR "Could not find FreeType installed on your system. This means you have to install the FreeType development files. This can be done by installing a package called something in the lines of FreeType-devel or FreeType-dev.")
else()
    message(STATUS "Found FreeType in ${FreeType_INCLUDE_DIR}: ${FreeType_LIBRARY}")
    list(APPEND FTS_INCLUDES ${FreeType_INCLUDE_DIR})
    list(APPEND FTS_LIBS ${FreeType_LIBRARIES})
endif()

# In debug mode, look for the google perf tools. #
##################################################
# Does crash before entering main on my 64bit machine ...

# if(CMAKE_BUILD_TYPE STREQUAL "Debug")
#     set(GOOGLEPERFMEM_FIND_QUIETLY 1)
#     find_package(GooglePerfMem)
#     if(${GOOGLEPERFMEM_FOUND} STREQUAL "NO")
#         message(STATUS "No google performance memory tools found on your system, not using them.")
#     else()
#         message(STATUS "Found google performance memory tools as ${GOOGLEPERFMEM_LIBRARY}")
#         list(APPEND FTS_LIBS ${GOOGLEPERFMEM_LIBRARY})
#     endif()
# endif()

# Add the different subdirectories needed for CEGUI. #
######################################################

# Add our own CEGUI.
add_subdirectory(${fts_SOURCE_DIR}/3rdparty/CEGUI)
list(APPEND FTS_INCLUDES "3rdparty/CEGUI/include" "3rdparty/CEGUI")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND FTS_LIBS "CEGUIOpenGLRendererd"
                         "CEGUIBased"
                         "CEGUITinyXMLParserd"
                         "CEGUIFalagardWRBased"
        )
else()
    list(APPEND FTS_LIBS "CEGUIOpenGLRenderer"
                         "CEGUIBase"
                         "CEGUITinyXMLParser"
                         "CEGUIFalagardWRBase"
        )
endif()

# Add cal3d
add_subdirectory(${fts_SOURCE_DIR}/3rdparty/cal3d)
list(APPEND FTS_INCLUDES "3rdparty/cal3d/src")
list(APPEND FTS_LIBS "cal3d")

# Add OpenAL-soft and FreeALUT.
add_subdirectory(${fts_SOURCE_DIR}/3rdparty/sound/openal-soft)
add_subdirectory(${fts_SOURCE_DIR}/3rdparty/sound/freealut)
add_definitions(-DD_SND_SYS=1)
list(APPEND FTS_INCLUDES "3rdparty/sound/openal-soft/include" "3rdparty/sound/freealut/include")
list(APPEND FTS_LIBS "openal" "alut")

# Add libogg and libvorbis
add_subdirectory(${fts_SOURCE_DIR}/3rdparty/sound/libogg)
add_subdirectory(${fts_SOURCE_DIR}/3rdparty/sound/libvorbis)
list(APPEND FTS_INCLUDES "3rdparty/sound/libogg/include" "3rdparty/sound/libvorbis/include")
list(APPEND FTS_LIBS "vorbis" "ogg")

# Add Dao scripting stuff
add_subdirectory(${fts_SOURCE_DIR}/3rdparty/dao)
add_definitions(-DDAO_DIRECT_API)
list(APPEND FTS_INCLUDES "3rdparty/dao/src")
list(APPEND FTS_LIBS "dao")

# Add unit testing stuff
add_subdirectory(${fts_SOURCE_DIR}/dLib/aTest)
list(APPEND FTS_INCLUDES dLib)
list(APPEND FTS_LIBS aTest)
set(SOURCES_TEST tests/mainNice.cpp
                 tests/3d/ResolutionTest.cpp
                 tests/dLib/dFile/dFileArchiveTest.cpp
                 tests/Scripting/DaoVmTest.cpp
                 tests/dLib/dString/dPathTest.cpp
                 tests/dLib/dString/dStringTest.cpp
                 )
add_definitions(-DGAME_MAIN=main)
add_definitions(-DTEST_MAIN=main)

# Debug or release ?
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring for debug")
    add_definitions(-DDEBUG=1)
    # Only run the tests in debug mode, because the compiler fills up all my ram
    # when trying to optimize them for release mode.
    # TODO: Maybe set a flag to not optimize them in release mode?
    add_subdirectory(${fts_SOURCE_DIR}/tests)
else()
    message(STATUS "Configuring for release")
endif()

# Add additional include search directories.
include_directories(${fts_SOURCE_DIR})
include_directories("${fts_SOURCE_DIR}/main")
include_directories(${FTS_INCLUDES})
include_directories(${OPENGL_INCLUDE_DIR})

# Add additional defines:
add_definitions(-DD_STRING_FTS_LIBS)
add_definitions(-DD_USE_CEGUI)
add_definitions(-DD_CAL3D_INTEGRATION)
add_definitions(-DD_STRING_FTS_PATHS)

# Compiler-dependent and build-dependend flags:
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
# -Wextra -Wold-style-cast -Wfloat-equal -Wswitch-enum -Wundef -Wpointer-arith -Wcast-align
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++0x -Wno-non-virtual-dtor -Wall -O0 -pg -fstack-protector-all -Winvalid-pch")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++0x -Wall -s -O2 -Winvalid-pch")
elseif(MSVC)
endif()

# And show the flags.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "CXX Flags for FTS (Debug) are: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
    # set the flags which are needed by the macro ADD_PCH_RULE
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}" )
else()
    message(STATUS "CXX Flags for FTS (Release) are: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
    # set the flags which are needed by the macro ADD_PCH_RULE
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}" )
endif()

# Collect all library files that are needed. #
##############################################
list(APPEND FTS_LIBS ${OPENGL_LIBRARIES})
list(APPEND FTS_LIBS ${CMAKE_THREAD_LIBS_INIT})

# The compiling process. #
##########################
message(STATUS ${FTS_LIBS})

include(${fts_SOURCE_DIR}/cmake/PCH_GCC4_v2.cmake)
ADD_PCH_RULE(${fts_SOURCE_DIR}/3d/3d.h SOURCES_COMMON)
ADD_PCH_RULE(${fts_SOURCE_DIR}/dLib/dString/dString.h SOURCES_COMMON)
ADD_PCH_RULE(${fts_SOURCE_DIR}/main/main.h SOURCES_GAME)

execute_process(COMMAND svnversion -n WORKING_DIRECTORY ${fts_SOURCE_DIR} OUTPUT_VARIABLE SVN_REVISION)
configure_file(${fts_SOURCE_DIR}/main/revision.h.in ${fts_SOURCE_DIR}/main/revision.h)

add_executable(fts ${SOURCES_COMMON} ${SOURCES_GAME})
target_link_libraries(fts ${FTS_LIBS} )

add_executable(test ${SOURCES_COMMON} ${SOURCES_TEST})
target_link_libraries(test ${FTS_LIBS})

# copy the executable as a post-build step. #
#############################################
add_custom_command(TARGET fts POST_BUILD COMMAND cp ${fts_BINARY_DIR}/fts ${fts_SOURCE_DIR} VERBATIM)
add_custom_command(TARGET test POST_BUILD COMMAND cp ${fts_BINARY_DIR}/test ${fts_SOURCE_DIR} VERBATIM)
add_custom_command(TARGET test POST_BUILD COMMAND test fts WORKING_DIRECTORY ${fts_SOURCE_DIR} VERBATIM)

# installation to the system. #
#    OUTDATED AND UNTESTED    #
###############################
# install(TARGETS fts DESTINATION bin)
#
# install(FILES Data/newicon.ico
#         DESTINATION share/arkana-fts/Data)
#
# install(FILES Data/Graphics/icon.ftsimg
#               Data/Graphics/preview.ftsimg
#               Data/Graphics/loadscreen.ftsimg
#         DESTINATION share/arkana-fts/Data/Graphics)
#
# install(FILES Data/Graphics/tilesets/OldSu.tileset
#         DESTINATION share/arkana-fts/Data/Graphics/tilesets)
#
# install(FILES Data/Graphics/ui/icp.ftsimg
#               Data/Graphics/ui/rb.ftsimg
#               Data/Graphics/ui/Loading.English.ftsimg
#         DESTINATION share/arkana-fts/Data/Graphics/ui)
#
# install(FILES Data/Graphics/ui/cursors/std.conf
#               Data/Graphics/ui/cursors/std.ftsimg
#         DESTINATION share/arkana-fts/Data/Graphics/ui/cursors)
#
# install(FILES Data/Graphics/ui/fonts/license.txt
#               Data/Graphics/ui/fonts/arkana-normal.ttf
#               Data/Graphics/ui/fonts/normal1.font
#               Data/Graphics/ui/fonts/normal2.font
#               Data/Graphics/ui/fonts/normal3.font
#               Data/Graphics/ui/fonts/normal4.font
#               Data/Graphics/ui/fonts/normal5.font
#               Data/Graphics/ui/fonts/normalNoCol.font
#               Data/Graphics/ui/fonts/arkana-old.ttf
#               Data/Graphics/ui/fonts/old1.font
#               Data/Graphics/ui/fonts/old2.font
#               Data/Graphics/ui/fonts/old3.font
#               Data/Graphics/ui/fonts/old4.font
#               Data/Graphics/ui/fonts/old5.font
#               Data/Graphics/ui/fonts/oldNoCol.font
#         DESTINATION share/arkana-fts/Data/Graphics/ui/fonts)
#
# install(FILES Data/Graphics/ui/imagesets/ArkanaLook.ftsimg
#               Data/Graphics/ui/imagesets/ArkanaLook.imageset
#               Data/Graphics/ui/imagesets/ftsui.ftsimg
#               Data/Graphics/ui/imagesets/ftsui.imageset
#         DESTINATION share/arkana-fts/Data/Graphics/ui/imagesets)
#
# install(FILES Data/Graphics/ui/layouts/dlg_confirm.English.layout
#               Data/Graphics/ui/layouts/dlg_confirm.French.layout
#               Data/Graphics/ui/layouts/dlg_confirm.German.layout
#               Data/Graphics/ui/layouts/dlg_createAccount.English.layout
#               Data/Graphics/ui/layouts/dlg_createAccount.French.layout
#               Data/Graphics/ui/layouts/dlg_createAccount.German.layout
#               Data/Graphics/ui/layouts/dlg_createOnlineAcc.English.layout
#               Data/Graphics/ui/layouts/dlg_createOnlineAcc.French.layout
#               Data/Graphics/ui/layouts/dlg_createOnlineAcc.German.layout
#               Data/Graphics/ui/layouts/dlg_editAccount.English.layout
#               Data/Graphics/ui/layouts/dlg_editAccount.French.layout
#               Data/Graphics/ui/layouts/dlg_editAccount.German.layout
#               Data/Graphics/ui/layouts/dlg_enterAccountName.English.layout
#               Data/Graphics/ui/layouts/dlg_enterAccountName.French.layout
#               Data/Graphics/ui/layouts/dlg_enterAccountName.German.layout
#               Data/Graphics/ui/layouts/dlg_message.English.layout
#               Data/Graphics/ui/layouts/dlg_message.French.layout
#               Data/Graphics/ui/layouts/dlg_message.German.layout
#               Data/Graphics/ui/layouts/dlg_onlineChannelJoin.English.layout
#               Data/Graphics/ui/layouts/dlg_onlineChannelJoin.French.layout
#               Data/Graphics/ui/layouts/dlg_onlineChannelJoin.German.layout
#               Data/Graphics/ui/layouts/dlg_onlineFeedback.English.layout
#               Data/Graphics/ui/layouts/dlg_onlineFeedback.French.layout
#               Data/Graphics/ui/layouts/dlg_onlineFeedback.German.layout
#               Data/Graphics/ui/layouts/dlg_onlineFindGame.English.layout
#               Data/Graphics/ui/layouts/dlg_onlineFindGame.French.layout
#               Data/Graphics/ui/layouts/dlg_onlineFindGame.German.layout
#               Data/Graphics/ui/layouts/dlg_onlineNewGame.English.layout
#               Data/Graphics/ui/layouts/dlg_onlineNewGame.French.layout
#               Data/Graphics/ui/layouts/dlg_onlineNewGame.German.layout
#               Data/Graphics/ui/layouts/dlg_options.English.layout
#               Data/Graphics/ui/layouts/dlg_options.French.layout
#               Data/Graphics/ui/layouts/dlg_options.German.layout
#               Data/Graphics/ui/layouts/dlg_options_btnAudio.English.layout
#               Data/Graphics/ui/layouts/dlg_options_btnAudio.French.layout
#               Data/Graphics/ui/layouts/dlg_options_btnAudio.German.layout
#               Data/Graphics/ui/layouts/dlg_options_btnGeneral.English.layout
#               Data/Graphics/ui/layouts/dlg_options_btnGeneral.French.layout
#               Data/Graphics/ui/layouts/dlg_options_btnGeneral.German.layout
#               Data/Graphics/ui/layouts/dlg_options_btnMisc.English.layout
#               Data/Graphics/ui/layouts/dlg_options_btnMisc.French.layout
#               Data/Graphics/ui/layouts/dlg_options_btnMisc.German.layout
#               Data/Graphics/ui/layouts/dlg_options_btnNetwork.English.layout
#               Data/Graphics/ui/layouts/dlg_options_btnNetwork.French.layout
#               Data/Graphics/ui/layouts/dlg_options_btnNetwork.German.layout
#               Data/Graphics/ui/layouts/dlg_options_btnVideo.English.layout
#               Data/Graphics/ui/layouts/dlg_options_btnVideo.French.layout
#               Data/Graphics/ui/layouts/dlg_options_btnVideo.German.layout
#               Data/Graphics/ui/layouts/dlg_options_btnVideo_advanced.English.layout
#               Data/Graphics/ui/layouts/dlg_options_btnVideo_advanced.French.layout
#               Data/Graphics/ui/layouts/dlg_options_btnVideo_advanced.German.layout
#               Data/Graphics/ui/layouts/dlg_Update.English.layout
#               Data/Graphics/ui/layouts/dlg_Update.French.layout
#               Data/Graphics/ui/layouts/dlg_Update.German.layout
#               Data/Graphics/ui/layouts/file_dlg.English.layout
#               Data/Graphics/ui/layouts/file_dlg.French.layout
#               Data/Graphics/ui/layouts/file_dlg.German.layout
#               Data/Graphics/ui/layouts/fts_updater.layout
#               Data/Graphics/ui/layouts/game.English.layout
#               Data/Graphics/ui/layouts/game.French.layout
#               Data/Graphics/ui/layouts/game.German.layout
#               Data/Graphics/ui/layouts/loadfts.English.layout
#               Data/Graphics/ui/layouts/loadscreen.English.layout
#               Data/Graphics/ui/layouts/mdlviewer_main.English.layout
#               Data/Graphics/ui/layouts/mdlviewer_main.French.layout
#               Data/Graphics/ui/layouts/mdlviewer_main.German.layout
#               Data/Graphics/ui/layouts/menu_main.English.layout
#               Data/Graphics/ui/layouts/menu_main.French.layout
#               Data/Graphics/ui/layouts/menu_main.German.layout
#               Data/Graphics/ui/layouts/menu_online.English.layout
#               Data/Graphics/ui/layouts/menu_online.French.layout
#               Data/Graphics/ui/layouts/menu_online.German.layout
#               Data/Graphics/ui/layouts/menu_online_main.English.layout
#               Data/Graphics/ui/layouts/menu_online_main.French.layout
#               Data/Graphics/ui/layouts/menu_online_main.German.layout
#         DESTINATION share/arkana-fts/Data/Graphics/ui/layouts)
#
# install(FILES Data/Graphics/ui/looknfeel/ArkanaLook.BigProgressbar.looknfeel
#               Data/Graphics/ui/looknfeel/ArkanaLook.Button.looknfeel
#               Data/Graphics/ui/looknfeel/ArkanaLook.CheckboxLabelLeft.looknfeel
#               Data/Graphics/ui/looknfeel/ArkanaLook.CheckboxLabelRight.looknfeel
#               Data/Graphics/ui/looknfeel/ArkanaLook.Combobox.looknfeel
#               Data/Graphics/ui/looknfeel/ArkanaLook.ComboDropList.looknfeel
#               Data/Graphics/ui/looknfeel/ArkanaLook.ContainerBox.looknfeel
#               Data/Graphics/ui/looknfeel/ArkanaLook.DecoButton.looknfeel
#               Data/Graphics/ui/looknfeel/ArkanaLook.DecoButtonTxt.looknfeel
#               Data/Graphics/ui/looknfeel/ArkanaLook.Editbox.looknfeel
#               Data/Graphics/ui/looknfeel/ArkanaLook.EditboxNoRightBorder.looknfeel
#               Data/Graphics/ui/looknfeel/ArkanaLook.FrameWindow.looknfeel
#               Data/Graphics/ui/looknfeel/ArkanaLook.HorizontalDecoScrollbar.looknfeel
#               Data/Graphics/ui/looknfeel/ArkanaLook.Image.looknfeel
#               Data/Graphics/ui/looknfeel/ArkanaLook.Label.looknfeel
#               Data/Graphics/ui/looknfeel/ArkanaLook.Listbox.looknfeel
#               Data/Graphics/ui/looknfeel/ArkanaLook.MenuButton.looknfeel
#               Data/Graphics/ui/looknfeel/ArkanaLook.MenuItem.looknfeel
#               Data/Graphics/ui/looknfeel/ArkanaLook.MultiLineEditbox.looknfeel
#               Data/Graphics/ui/looknfeel/ArkanaLook.PopupMenu.looknfeel
#               Data/Graphics/ui/looknfeel/ArkanaLook.Progressbar.looknfeel
#               Data/Graphics/ui/looknfeel/ArkanaLook.RadioButtonLabelLeft.looknfeel
#               Data/Graphics/ui/looknfeel/ArkanaLook.RadioButtonLabelRight.looknfeel
#               Data/Graphics/ui/looknfeel/ArkanaLook.ShadowedLabel.looknfeel
#               Data/Graphics/ui/looknfeel/ArkanaLook.Spinner.looknfeel
#               Data/Graphics/ui/looknfeel/ArkanaLook.StaticFrame.looknfeel
#               Data/Graphics/ui/looknfeel/ArkanaLook.StaticFrameNoText.looknfeel
#               Data/Graphics/ui/looknfeel/ArkanaLook.StaticText.looknfeel
#               Data/Graphics/ui/looknfeel/ArkanaLook.SystemButton.looknfeel
#               Data/Graphics/ui/looknfeel/ArkanaLook.Titlebar.looknfeel
#               Data/Graphics/ui/looknfeel/ArkanaLook.Tooltip.looknfeel
#               Data/Graphics/ui/looknfeel/ArkanaLook.VerticalDecoScrollbar.looknfeel
#         DESTINATION share/arkana-fts/Data/Graphics/ui/looknfeel)
#
# install(FILES Data/Graphics/ui/menubg/800/main.ftsimg DESTINATION share/arkana-fts/Data/Graphics/ui/menubg/800)
# install(FILES Data/Graphics/ui/menubg/1024/main.ftsimg DESTINATION share/arkana-fts/Data/Graphics/ui/menubg/1024)
# install(FILES Data/Graphics/ui/menubg/1152/main.ftsimg DESTINATION share/arkana-fts/Data/Graphics/ui/menubg/1152)
# install(FILES Data/Graphics/ui/menubg/1280/main.ftsimg DESTINATION share/arkana-fts/Data/Graphics/ui/menubg/1280)
# install(FILES Data/Graphics/ui/menubg/1600/main.ftsimg DESTINATION share/arkana-fts/Data/Graphics/ui/menubg/1600)
#
# install(FILES Data/Graphics/ui/schemes/ArkanaLook.scheme
#         DESTINATION share/arkana-fts/Data/Graphics/ui/schemes)
#
# install(FILES Data/Languages/English/messages.conf
#               Data/Languages/English/ui.conf
#         DESTINATION share/arkana-fts/Data/Languages/English)
#
# install(FILES Data/Languages/French/messages.conf
#               Data/Languages/French/ui.conf
#         DESTINATION share/arkana-fts/Data/Languages/French)
#
# install(FILES Data/Languages/German/messages.conf
#               Data/Languages/German/ui.conf
#         DESTINATION share/arkana-fts/Data/Languages/German)
#
# install(FILES Data/Languages/Thai/messages.conf
#               Data/Languages/Thai/ui.conf
#         DESTINATION share/arkana-fts/Data/Languages/Thai)
#
# install(FILES Data/Maps/test.ftsm
#               Data/Maps/test_full.ftsm
#         DESTINATION share/arkana-fts/Data/Maps)
#
# install(FILES Data/Sound/Hydrate-Kenny_Beltrey.ogg
#               Data/Sound/whisp_recv.ogg
#         DESTINATION share/arkana-fts/Data/Sound)
