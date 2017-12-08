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

# Remove unwanted project types in MSVC
if(MSVC)
    list(REMOVE_ITEM CMAKE_CONFIGURATION_TYPES MinSizeRel RelWithDebInfo)
endif()

# Disallow in-source builds. #
##############################
EXECUTE_PROCESS(COMMAND pwd OUTPUT_VARIABLE CURR_DIR)
if("${CURR_DIR}" STREQUAL "${fts_SOURCE_DIR}\n")
    message(FATAL_ERROR "In-source-builds are not allowed to build the Arkana-FTS game. Please go into the \"build\" directory and type \"cmake ..\" there.\nThank you.")
endif()

if(MSVC)
    FOREACH(flag_var CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE)
        IF(${flag_var} MATCHES "/MD")
            STRING(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        ENDIF()
    ENDFOREACH(flag_var)
    FOREACH(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE )
        IF(${flag_var} MATCHES "/MD")
            STRING(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        ENDIF()
    ENDFOREACH(flag_var)
endif()

# Put all sourcefiles into one variable. #
##########################################
set(SRC_3rdparty
    3rdparty/lodepng.cpp
    3rdparty/freetype_debug.cpp
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
    dLib/dConf/configuration.cpp
    dLib/dConf/ArkanaDefaultSettings.cpp
    dLib/dConf/DefaultOptions.cpp
    dLib/dFile/dFile.cpp
    dLib/dFile/dBrowse.cpp
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
    logging/MinimalLogger.cpp
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
    tests/dLib/dFile/dFileTest.cpp
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
    source_group( TREE ${fts_SOURCE_DIR} FILES ${SRC_3D} )
    source_group( TREE ${fts_SOURCE_DIR} FILES ${SRC_dLib} )
    source_group( TREE ${fts_SOURCE_DIR} FILES ${SRC_tests} )
    source_group( TREE ${fts_SOURCE_DIR} FILES ${SRC_ui} )
    source_group( TREE ${fts_SOURCE_DIR} FILES ${SRC_game} )
    source_group( graphic FILES ${SRC_graphic} )
    source_group( input FILES ${SRC_input} )
    source_group( logging FILES ${SRC_logging} )
    source_group( main FILES ${SRC_main} )
    source_group( map FILES ${SRC_map} )
    source_group( mdlviewer FILES ${SRC_mdlviewer} )
    source_group( sound FILES ${SRC_sound} )
    source_group( utilities FILES ${SRC_utilities} )
    source_group( scripting FILES ${SRC_scripting} )
endif()

add_executable(fts ${SOURCES_COMMON} ${SOURCES_GAME})

# Search for the different needed libs. #
#########################################

set(FTS_LIBS)
set(FTS_INCLUDES)

if(MSVC)
    # This must be set prior to the target in order to be populated to the vcxproject.
    link_directories(3rdparty/vclib)
endif()


# The different standard libraries already supported by CMake.
find_package(Threads REQUIRED)
list(APPEND FTS_LIBS ${CMAKE_THREAD_LIBS_INIT})

find_package(OpenGL REQUIRED)
list(APPEND FTS_LIBS ${OPENGL_LIBRARIES})
list(APPEND FTS_INCLUDES ${OPENGL_INCLUDE_DIR})

# Check for various language-stuff and compiler versions #
##########################################################

# Compiler-dependent and build-dependend flags. Doing this before adding any 3rd party lib
# In order to have them built with the same flags.
set_property(TARGET fts PROPERTY CXX_STANDARD 17)
set_property(TARGET fts PROPERTY CXX_STANDARD_REQUIRED ON)

if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions("-Wall")
    target_compile_options(fts PRIVATE $<$<CONFIG:Debug>:-O0>)
    target_compile_options(fts PRIVATE $<$<CONFIG:Debug>:-pg>)
    target_compile_options(fts PRIVATE $<$<CONFIG:Debug>:-fstack-protector-all>)
    target_compile_options(fts PRIVATE $<$<CONFIG:Debug>:-Wno-non-virtual-dtor>)
    target_compile_options(fts PRIVATE $<$<CONFIG:Release>:-s>)
endif()

# Debug or release ?
target_compile_definitions(fts PRIVATE $<$<CONFIG:Debug>:DEBUG=1>)
target_compile_definitions(fts PRIVATE $<$<CONFIG:Debug>:_DEBUG>)

add_subdirectory(${fts_SOURCE_DIR}/3rdparty)
if(WIN32)
    list(APPEND FTS_LIBS wsock32.lib)
    list(APPEND FTS_LIBS winmm.lib)
    list(APPEND FTS_LIBS dsound.lib)
    list(APPEND FTS_LIBS dxguid.lib)
    list(APPEND FTS_LIBS imm32.lib)
    list(APPEND FTS_LIBS version.lib)
else()
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
    list(APPEND FTS_LIBS dl)
endif()

# Add fts-networking stuff
set(fts-networking_SOURCE_DIR CACHE STRING "Location of the FTS networking library.")
add_subdirectory(${fts-networking_SOURCE_DIR} "${CMAKE_CURRENT_BINARY_DIR}/net")
list(APPEND FTS_3rdparty_LIBS fts-net)

# Add unit testing stuff
set(ATEST_BUILD_EXAMPLES FALSE)
add_subdirectory(${fts_SOURCE_DIR}/dLib/aTest)
list(APPEND FTS_3rdparty_LIBS aTest)

# Add additional include search directories.
target_include_directories(fts PRIVATE ${fts_SOURCE_DIR} ${fts_SOURCE_DIR}/main ${FTS_INCLUDES})

# Add additional defines:
target_compile_definitions(fts PRIVATE D_STRING_FTS_LIBS)
target_compile_definitions(fts PRIVATE D_USE_CEGUI)
target_compile_definitions(fts PRIVATE D_STRING_FTS_PATHS)
target_compile_definitions(fts PRIVATE _USE_MATH_DEFINES) # Using M_PI etc.
target_compile_definitions(fts PRIVATE HAVE_M_PI) # Avoid redefinition through SDL_stdinc.h
target_compile_definitions(fts PRIVATE DAO_DIRECT_API)
target_compile_definitions(fts PRIVATE D_SND_SYS=1)
target_compile_definitions(fts PRIVATE DAO_SND_STATIC)
if(WIN32)
    target_compile_definitions(fts PRIVATE BOUGE_STATIC)
    target_compile_definitions(fts PRIVATE AL_LIBTYPE_STATIC)
    target_compile_definitions(fts PRIVATE ALL_STATIC)
    target_compile_definitions(fts PRIVATE CEGUI_STATIC)
    target_compile_definitions(fts PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(fts PRIVATE _CRT_SECURE_NO_DEPRECATE)
    target_compile_definitions(fts PRIVATE WIN32_LEAN_AND_MEAN=1)
    target_compile_definitions(fts PRIVATE NOMINMAX)
    target_compile_definitions(fts PRIVATE _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
endif()

# The compiling process. #
##########################
if(CMAKE_COMPILER_IS_GNUCXX)
    # gcc (even 7.2) needs to link a special lib for the filesystem support.
    list(APPEND FTS_LIBS stdc++fs)
endif()

execute_process(COMMAND git describe --dirty WORKING_DIRECTORY ${fts_SOURCE_DIR} RESULT_VARIABLE FTS_VERSION_RESULT OUTPUT_VARIABLE FTS_VERSION_FULL_RAW)
if(${FTS_VERSION_RESULT} EQUAL 0)
    string(STRIP ${FTS_VERSION_FULL_RAW} FTS_VERSION_FULL)
else()
    set(FTS_VERSION_FULL "Error determining version. Please check if git has tags.")
endif()
message(STATUS "Detected Arkana-FTS version to be ${FTS_VERSION_FULL}")
configure_file(${fts_SOURCE_DIR}/main/revision.h.in ${fts_SOURCE_DIR}/main/revision.h)

target_link_libraries(fts ${FTS_LIBS} ${FTS_3rdparty_LIBS})
# copy the executable as a post-build step. #
#############################################
if(NOT WIN32)
    add_custom_command(TARGET fts POST_BUILD COMMAND cp ${fts_BINARY_DIR}/fts ${fts_SOURCE_DIR} VERBATIM)
endif()
