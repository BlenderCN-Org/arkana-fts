project(CEGUI)

message(STATUS "                 ")
message(STATUS "=================")
message(STATUS "CONFIGURING CEGUI")
message(STATUS "=================")
message(STATUS "                 ")

# Setup the make environement. #
################################

# Setup the cmake directory containing numerous cmake scripts and macros.
set(CMAKE_MODULE_PATH "${CEGUI_SOURCE_DIR}/../../cmake")

# Allow else() and endif() constructs (with empty paranthesis)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

# Activate very verbose makefiles.
# set(CMAKE_VERBOSE_MAKEFILE ON)

# Disallow in-source builds. #
##############################
EXECUTE_PROCESS(COMMAND pwd OUTPUT_VARIABLE CURR_DIR)
if("${CURR_DIR}" STREQUAL "${fts_SOURCE_DIR}\n")
    message(FATAL_ERROR "In-source-builds are not allowed to CEGUI. Please go into the \"linux\" directory and type \"cmake ..\" there.\nThank you.")
endif()

# Search for the different needed libs. #
#########################################

# Create the config.h file #
############################
set(CEGUI_WITH_TINYXML 1)
set(CEGUI_DEFAULT_XMLPARSER "TinyXMLParser")
set(CEGUI_LOAD_MODULE_APPEND_SUFFIX_FOR_DEBUG 1)
set(CEGUI_SAMPLES_USE_OPENGL 1)
set(CEGUI_FALAGARD_RENDERER 1)
set(CEGUI_LOAD_MODULE_DEBUG_SUFFIX "\"d\"")

configure_file(${CEGUI_SOURCE_DIR}/config.h.cmake ${CEGUI_SOURCE_DIR}/include/config.h)
message(STATUS "Configuration file written to: ${CEGUI_SOURCE_DIR}/include/config.h")
add_definitions(-DHAVE_CONFIG_H -DCEGUI_STATIC)
# Debug or release ?
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring for debug")
    add_definitions(-DDEBUG=1)
else()
    message(STATUS "Configuring for release")
endif()

# Add the different subdirectories needed for CEGUI. #
######################################################
add_subdirectory(${CEGUI_SOURCE_DIR}/RendererModules/OpenGLGUIRenderer)
add_subdirectory(${CEGUI_SOURCE_DIR}/WindowRendererSets/Falagard)
add_subdirectory(${CEGUI_SOURCE_DIR}/XMLParserModules/TinyXMLParser)
add_subdirectory(${CEGUI_SOURCE_DIR}/src)
