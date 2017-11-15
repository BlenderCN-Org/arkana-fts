cmake_minimum_required(VERSION 3.1)

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

# De-Activate very verbose makefiles.
set(CMAKE_VERBOSE_MAKEFILE OFF)

# Disallow in-source builds. #
##############################
EXECUTE_PROCESS(COMMAND pwd OUTPUT_VARIABLE CURR_DIR)
if("${CURR_DIR}" STREQUAL "${fts_SOURCE_DIR}\n")
    message(FATAL_ERROR "In-source-builds are not allowed to build the Arkana-FTS server. Please go into the \"linux\" directory and type \"cmake ..\" there.\nThank you.")
endif()

if(MSVC)
    FOREACH(flag_var
        CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
        )
        IF(${flag_var} MATCHES "/MD")
            STRING(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        ENDIF()
    ENDFOREACH(flag_var)
    FOREACH(flag_var
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        )
        IF(${flag_var} MATCHES "/MD")
            STRING(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        ENDIF()
    ENDFOREACH(flag_var)
endif()

# Put all sourcefiles into one variable. #
##########################################
set(SRC_3rdparty
    3rdparty/lodepng.cpp
    )
set(SRC_3D
    3d/math/AxisAlignedBoundingBox.cpp
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
    )
set(SRC_dLib
    dLib/dArchive/dArchive.cpp
    dLib/dBrowse/dBrowse.cpp
    dLib/dConf/configuration.cpp
    dLib/dConf/ArkanaDefaultSettings.cpp
    dLib/dConf/DefaultOptions.cpp
    dLib/dFile/dFile.cpp
    dLib/dProcess/dProcess.cpp
    dLib/dString/dString.cpp
    dLib/dString/dPath.cpp
    dLib/dString/dTranslation.cpp
    dLib/dCompressor/dCompressor.cpp
    dLib/dCompressor/minilzo/minilzo.c
    dLib/dCompressor/minilzo_compressor.cpp
    )
set(SRC_game
    game/objects/objects.cpp
    game/objects/tree.cpp
    game/game_rlv.cpp
    game/loadgame_rlv.cpp
    game/player.cpp
    game/player_og.cpp
    )
set(SRC_graphic
    graphic/anim.cpp
    graphic/cegui_ftsimg_codec.cpp
    graphic/errtex.cpp
    graphic/graphic.cpp
    graphic/image.cpp
    graphic/Color.cpp
    )
set(SRC_input
    input/Cursor.cpp
    input/InputManager.cpp
    input/keys.cpp
    )
set(SRC_logging
    logging/Chronometer.cpp
    logging/ftslogger.cpp
    logging/logger.cpp
    )
set(SRC_main
    main/main.cpp
    main/Clock.cpp
    main/Exception.cpp
    main/load_fts_rlv.cpp
    main/runlevels.cpp
    main/Updateable.cpp
    main/version.cpp
    )
set(SRC_map
    map/DecorativeMO.cpp
    map/forest.cpp
    map/map.cpp
    map/MapObject.cpp
    map/mapinfo.cpp
    map/quad.cpp
    map/terrain.cpp
    map/tile.cpp
    )
set(SRC_mdlviewer
    mdlviewer/mdlviewer_main.cpp
    )
set(SRC_sound
    sound/sndobjpool.cpp
    sound/SndFile.cpp
    sound/SndGrp.cpp
    sound/SndObjOpenAL.cpp
    sound/SndSys.cpp
    sound/SndSysOpenAL.cpp
    sound/SndPlayList.cpp
    )
set(SRC_tests
    tests/mainNice.cpp
    tests/3d/ResolutionTest.cpp
    tests/Scripting/DaoVmTest.cpp
    tests/dLib/dFile/dFileArchiveTest.cpp
    tests/dLib/dBrowseTest.cpp
    tests/dLib/dString/dPathTest.cpp
    tests/dLib/dString/dStringTest.cpp
    tests/dLib/dString/dTranslationTest.cpp
    tests/Configuration/Settings.cpp
    tests/Configuration/FTSConfiguration.cpp
    tests/main/ClockTest.cpp
    )
set(SRC_ui
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
    )
set(SRC_utilities
    utilities/DataContainer.cpp
    utilities/Singleton.cpp
    utilities/StreamedDataContainer.cpp
    utilities/command.cpp
    utilities/console.cpp
    utilities/fps_calculator.cpp
    utilities/radix.cpp
    utilities/parse.cpp
    utilities/utilities.cpp
    utilities/threading.cpp
    utilities/DateTime.cpp
    utilities/md5.cpp
    utilities/sha2.cpp
    )
set(SRC_scripting
    scripting/dao_snd.cpp
    scripting/dao_snd2.cpp
    scripting/dao_snd3.cpp
    scripting/DaoVm.cpp
    scripting/Music.cpp
    scripting/DaoFunction.cpp
    scripting/hotkey.cpp
)

set(SOURCES_COMMON ${SRC_3rdparty} ${SRC_3D} ${SRC_dLib} ${SRC_game} ${SRC_graphic} ${SRC_input} 
                   ${SRC_logging} ${SRC_main} ${SRC_map} ${SRC_mdlviewer} ${SRC_sound} 
                   ${SRC_tests} ${SRC_ui} ${SRC_utilities} ${SRC_scripting}
                   )

if(MSVC)
    source_group( 3rdparty FILES ${SRC_3rdparty} )
    source_group( 3d FILES ${SRC_3D} )
    source_group( dLib FILES ${SRC_dLib} )
    source_group( game FILES ${SRC_game} )
    source_group( graphic FILES ${SRC_graphic} )
    source_group( input FILES ${SRC_input} )
    source_group( logging FILES ${SRC_logging} )
    source_group( main FILES ${SRC_main} )
    source_group( map FILES ${SRC_map} )
    source_group( mdlviewer FILES ${SRC_mdlviewer} )
    source_group( sound FILES ${SRC_sound} )
    source_group( test FILES ${SRC_tests} )
    source_group( ui FILES ${SRC_ui} )
    source_group( utilities FILES ${SRC_utility} )
    source_group( scripting FILES ${SRC_scripting} )
endif()

# Check for various language-stuff and compiler versions #
##########################################################


# Compiler-dependent and build-dependend flags. Doing this before adding any 3rd party lib
# In order to have them built with the same flags.
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    add_definitions("-Wall -Winvalid-pch")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_definitions("-O0 -pg -fstack-protector-all")
    if(CMAKE_COMPILER_IS_GNUCXX)
        add_definitions("-Wno-non-virtual-dtor")
    endif()
    else()
    add_definitions("-s")
    endif()
elseif(MSVC)
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Debug or release ?
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring for debug")
    if(WIN32)
        add_definitions(-D_DEBUG)
        link_directories("3rdparty/vclib/")
    else()
        add_definitions(-DDEBUG=1)
    endif()
else()
    if(WIN32)
        add_definitions(-DNDEBUG)
    endif()
    message(STATUS "Configuring for release")
endif()

# Search for the different needed libs. #
#########################################

set(FTS_LIBS)
set(FTS_INCLUDES)

# The different standard libraries already supported by CMake.
find_package(Threads REQUIRED)
list(APPEND FTS_LIBS ${CMAKE_THREAD_LIBS_INIT})

find_package(OpenGL REQUIRED)
list(APPEND FTS_LIBS ${OPENGL_LIBRARIES})
list(APPEND FTS_INCLUDES ${OPENGL_INCLUDE_DIR})
if(WIN32)
    add_subdirectory(${fts_SOURCE_DIR}/3rdparty)
    list(APPEND FTS_INCLUDES ${fts_SOURCE_DIR}/3rdparty/CEGUI/include)
    list(APPEND FTS_INCLUDES ${fts_SOURCE_DIR}/3rdparty/CEGUI)
    list(APPEND FTS_INCLUDES ${fts_SOURCE_DIR}/3rdparty/sound/libogg/include)
    list(APPEND FTS_INCLUDES ${fts_SOURCE_DIR}/3rdparty/sound/libvorbis/include)
    list(APPEND FTS_INCLUDES ${fts_SOURCE_DIR}/3rdparty/sound/openal-soft/include)
    list(APPEND FTS_INCLUDES ${fts_SOURCE_DIR}/3rdparty/sound/openal-soft/include/AL)
    list(APPEND FTS_INCLUDES ${fts_SOURCE_DIR}/3rdparty/SDL2/include)
    list(APPEND FTS_INCLUDES ${fts_SOURCE_DIR}/3rdparty/dao/src)
    list(APPEND FTS_INCLUDES ${fts_SOURCE_DIR}/3rdparty/bouge/include)

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        list(APPEND FTS_LIBS OpenAL32_d.lib)
        list(APPEND FTS_LIBS vorbis_d.lib)
        list(APPEND FTS_LIBS ogg_d.lib)
        list(APPEND FTS_LIBS dao_d.lib)
        list(APPEND FTS_LIBS bouge-s-d.lib)
        list(APPEND FTS_LIBS bouge-math-s-d.lib)
        list(APPEND FTS_LIBS bouge-tinyxml-s-d.lib)
        list(APPEND FTS_LIBS bouge-xml-common-s-d.lib)
        list(APPEND FTS_LIBS bouge-xmlio-s-d.lib)
        list(APPEND FTS_LIBS freetype_d.lib)
        list(APPEND FTS_LIBS pcred.lib)
        list(APPEND FTS_LIBS SDL2_d.lib)
        list(APPEND FTS_LIBS SDL2main_d.lib)
        list(APPEND FTS_LIBS CEGUIBased_d.lib)
        list(APPEND FTS_LIBS CEGUITinyXMLParserd_d.lib)
        list(APPEND FTS_LIBS CEGUIOpenGLRendererd_d.lib)
        list(APPEND FTS_LIBS CEGUIFalagardWRBased_d.lib)
    else()
        list(APPEND FTS_LIBS OpenAL32.lib)
        list(APPEND FTS_LIBS vorbis.lib)
        list(APPEND FTS_LIBS ogg.lib)
        list(APPEND FTS_LIBS dao.lib)
        list(APPEND FTS_LIBS bouge-s.lib)
        list(APPEND FTS_LIBS bouge-math-s.lib)
        list(APPEND FTS_LIBS bouge-tinyxml-s.lib)
        list(APPEND FTS_LIBS bouge-xml-common-s.lib)
        list(APPEND FTS_LIBS bouge-xmlio-s.lib)
        list(APPEND FTS_LIBS freetype.lib)
        list(APPEND FTS_LIBS pcre.lib)
        list(APPEND FTS_LIBS SDL2.lib)
        list(APPEND FTS_LIBS SDL2main.lib)
        list(APPEND FTS_LIBS CEGUIBased.lib)
        list(APPEND FTS_LIBS CEGUITinyXMLParserd.lib)
        list(APPEND FTS_LIBS CEGUIOpenGLRendererd.lib)
        list(APPEND FTS_LIBS CEGUIFalagardWRBased.lib)
    endif()
    list(APPEND FTS_LIBS wsock32.lib)
    list(APPEND FTS_LIBS winmm.lib)
    list(APPEND FTS_LIBS dsound.lib)
    list(APPEND FTS_LIBS dxguid.lib)
    list(APPEND FTS_LIBS imm32.lib)
    list(APPEND FTS_LIBS version.lib)
else()
    set(SDL2_FIND_QUIETLY 1)
    find_package(SDL2)
    if(NOT SDL2_FOUND)
        message(FATAL_ERROR "Could not find SDL installed on your system. This means you have to install the SDL development files. This can be done by installing a package called something in the lines of SDL-devel or SDL-dev.")
    else()
        message(STATUS "Found SDL in ${SDL2_INCLUDE_DIR}: ${SDL2_LIBRARY}")
    endif()
    list(APPEND FTS_INCLUDES ${SDL2_INCLUDE_DIR})
    list(APPEND FTS_LIBS ${SDL2_LIBRARY})

    set(PCRE_FIND_QUIETLY 1)
    find_package(PCRE)
    if(NOT PCRE_FOUND)
        message(WARNING "Could not find PCRE installed on your system. Using the bundled PCRE version. You may want to install the PCRE development files by installing a package called something in the lines of PCRE-devel or PCRE-dev.")
        set(PCRE_SUPPORT_UTF8 ON CACHE BOOL "Enable support for the Unicode UTF-8 encoding.")
        set(PCRE_SUPPORT_UNICODE_PROPERTIES ON CACHE BOOL "Enable support for Unicode properties. (If set, UTF-8 support will be enabled as well)")
        option(PCRE_BUILD_PCRECPP "Build the PCRE C++ library (pcrecpp)." OFF)
        add_subdirectory(${fts_SOURCE_DIR}/3rdparty/pcre)
        set(PCRE_INCLUDE_DIR "${PCRE_BINARY_DIR}")
        set(PCRE_LIBRARIES "pcre")
    else()
        message(STATUS "Found PCRE in ${PCRE_INCLUDE_DIR}: ${PCRE_LIBRARY}")
    endif()

    set(FreeType_FIND_QUIETLY 1)
    find_package(FreeType)
    if(NOT FreeType_FOUND)
        message(FATAL_ERROR "Could not find FreeType installed on your system. This means you have to install the FreeType development files. This can be done by installing a package called something in the lines of FreeType-devel or FreeType-dev.")
    else()
        message(STATUS "Found FreeType in ${FreeType_INCLUDE_DIR}: ${FreeType_LIBRARY}")
    endif()
    list(APPEND FTS_INCLUDES ${FreeType_INCLUDE_DIR})
    list(APPEND FTS_LIBS ${FreeType_LIBRARIES})

    # X11 is required (at least). I'm sure there's more.
    set(X11_FIND_QUIETLY 1)
    find_package(X11)
    if(${X11_FOUND} STREQUAL "NO")
        message(FATAL_ERROR "Could not find X11 libraries on your system. This means you have to install the x11/xorg development files. This can be done by installing a package called something along the lines of xorg-x11-dev.")
    else()
        message(STATUS "Found X11 in ${X11_INCLUDE_DIR}: ${X11_LIBRARIES}")
        list(APPEND FTS_INCLUDES ${X11_INCLUDE_DIR})
        list(APPEND FTS_LIBS ${X11_LIBRARIES})
    endif()

    # Add the different subdirectories needed for CEGUI. #
    ######################################################

    # Add our own CEGUI.
    add_subdirectory(${fts_SOURCE_DIR}/3rdparty/CEGUI)
    list(APPEND FTS_INCLUDES "3rdparty/CEGUI/include" "3rdparty/CEGUI")
    list(APPEND FTS_LIBS "dl")
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

    # Add PCRE, needs to be added after CEGUI because CEGUI depends on it.
    list(APPEND FTS_INCLUDES ${PCRE_INCLUDE_DIR})
    list(APPEND FTS_LIBS ${PCRE_LIBRARIES})

    # Add bouge
    set(BOUGE_BUILD_EXAMPLES FALSE)
    set(BOUGE_BUILD_STATICCRT TRUE)
    set(BOUGE_BUILD_CAL3DXMLIO FALSE)
    set(BOUGE_COMPILE_FLAGS ${CMAKE_CXX_FLAGS})
    add_subdirectory(${fts_SOURCE_DIR}/3rdparty/bouge)
    list(APPEND FTS_INCLUDES "3rdparty/bouge/include")
    list(APPEND FTS_LIBS "bouge" "bouge-xmlio" "bouge-tinyxml" "bouge-math")
    unset(BOUGE_COMPILE_FLAGS)
    unset(BOUGE_BUILD_CAL3DXMLIO)
    unset(BOUGE_BUILD_STATICCRT)
    unset(BOUGE_BUILD_EXAMPLES)

    # Add OpenAL-soft.
    add_subdirectory(${fts_SOURCE_DIR}/3rdparty/sound/openal-soft)
    add_definitions(-DD_SND_SYS=1)
    list(APPEND FTS_INCLUDES "3rdparty/sound/openal-soft/include")
    list(APPEND FTS_LIBS "openal")

    # Add libogg and libvorbis
    add_subdirectory(${fts_SOURCE_DIR}/3rdparty/sound/libogg)
    add_subdirectory(${fts_SOURCE_DIR}/3rdparty/sound/libvorbis)
    list(APPEND FTS_INCLUDES "3rdparty/sound/libogg/include" "3rdparty/sound/libvorbis/include")
    list(APPEND FTS_LIBS "vorbis" "ogg")

    # Add Dao scripting stuff
    add_definitions(-DDAO_DIRECT_API)
    add_subdirectory(${fts_SOURCE_DIR}/3rdparty/dao)
    list(APPEND FTS_INCLUDES "3rdparty/dao/src")
    list(APPEND FTS_LIBS "dao")
endif()

# Add fts-networking stuff
set(fts-networking_SOURCE_DIR CACHE STRING "Location of the FTS networking library.")
add_subdirectory(${fts-networking_SOURCE_DIR} "${CMAKE_CURRENT_BINARY_DIR}/net")
list(APPEND FTS_INCLUDES "${fts-networking_SOURCE_DIR}/include")
list(APPEND FTS_LIBS "fts-net")

# Add unit testing stuff
set(ATEST_BUILD_EXAMPLES FALSE)
add_subdirectory(${fts_SOURCE_DIR}/dLib/aTest)
list(APPEND FTS_INCLUDES dLib)
list(APPEND FTS_LIBS aTest)

# Add additional include search directories.
include_directories(${fts_SOURCE_DIR})
include_directories("${fts_SOURCE_DIR}/main")
include_directories(${FTS_INCLUDES})

# Add additional defines:
add_definitions(-DD_STRING_FTS_LIBS)
add_definitions(-DD_USE_CEGUI)
add_definitions(-DD_STRING_FTS_PATHS)
if(WIN32)
    add_definitions(-DDAO_DIRECT_API)
    add_definitions(-DDAO_SND_STATIC)
    add_definitions(-DD_SND_SYS=1)
    add_definitions(-DBOUGE_STATIC)
    add_definitions(-DAL_LIBTYPE_STATIC)
    add_definitions(-DALL_STATIC)
    add_definitions(-DCEGUI_STATIC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()
# And show the flags.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "CXX Flags for FTS (Debug) are: ${CMAKE_CXX_FLAGS_DEBUG}")
else()
    message(STATUS "CXX Flags for FTS (Release) are: ${CMAKE_CXX_FLAGS_RELEASE}")
endif()

# The compiling process. #
##########################
if(CMAKE_COMPILER_IS_GNUCXX)
    # gcc (even 7.2) needs to link a special lib for the filesystem support.
    list(APPEND FTS_LIBS stdc++fs)
    # Pre compiled header
    include(${fts_SOURCE_DIR}/cmake/PCH_GCC4_v2.cmake)
    ADD_PCH_RULE(${fts_SOURCE_DIR}/3d/3d.h SOURCES_COMMON)
    ADD_PCH_RULE(${fts_SOURCE_DIR}/dLib/dString/dString.h SOURCES_COMMON)
    ADD_PCH_RULE(${fts_SOURCE_DIR}/main/main.h SOURCES_GAME)
endif()

execute_process(COMMAND git describe --dirty WORKING_DIRECTORY ${fts_SOURCE_DIR} RESULT_VARIABLE FTS_VERSION_RESULT OUTPUT_VARIABLE FTS_VERSION_FULL_RAW)
if(${FTS_VERSION_RESULT} EQUAL 0)
    string(STRIP ${FTS_VERSION_FULL_RAW} FTS_VERSION_FULL)
else()
    set(FTS_VERSION_FULL "Error determining version. Please check if git has tags.")
endif()
message(STATUS "Detected Arkana-FTS version to be ${FTS_VERSION_FULL}")
configure_file(${fts_SOURCE_DIR}/main/revision.h.in ${fts_SOURCE_DIR}/main/revision.h)

add_executable(fts ${SOURCES_COMMON} ${SOURCES_GAME})
target_link_libraries(fts ${FTS_LIBS} )

# copy the executable as a post-build step. #
#############################################
if(NOT WIN32)
    add_custom_command(TARGET fts POST_BUILD COMMAND cp ${fts_BINARY_DIR}/fts ${fts_SOURCE_DIR} VERBATIM)
endif()

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
