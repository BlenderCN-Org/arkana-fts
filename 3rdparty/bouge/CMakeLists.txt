cmake_minimum_required(VERSION 3.1)

message(STATUS "                 ")
message(STATUS "=================")
message(STATUS "CONFIGURING bouge")
message(STATUS "=================")
message(STATUS "                 ")

# set a default build type if none was provided
# this has to be done before the project() instruction!
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

# project name
project(bouge)

# include the configuration file
include(${PROJECT_SOURCE_DIR}/cmake/Config.cmake)

# setup version numbers
set(BOUGE_VERSION_MAJOR 0)
set(BOUGE_VERSION_MINOR 1)
set(BOUGE_VERSION_PATCH 0)

# add the SFML header path
include_directories(${PROJECT_SOURCE_DIR}/include)

# add an option for choosing the build type (shared or static)
if(NOT DEFINED BOUGE_BUILD_SHARED_LIBS)
    set(BOUGE_BUILD_SHARED_LIBS FALSE)
endif()
set(BUILD_SHARED_LIBS ${BOUGE_BUILD_SHARED_LIBS} CACHE BOOL "TRUE to build bouge as shared libraries, FALSE to build it as static libraries")

# add an option for building the examples
if(NOT DEFINED BOUGE_BUILD_EXAMPLES)
    set(BOUGE_BUILD_EXAMPLES TRUE)
else()
    set(BUILD_EXAMPLES ${BOUGE_BUILD_EXAMPLES})
endif()
set(BUILD_EXAMPLES ${BOUGE_BUILD_EXAMPLES} CACHE BOOL "TRUE to build the bouge examples, FALSE to ignore them")

# add an option for building the API documentation
if(NOT DEFINED BOUGE_BUILD_DOC)
    set(BOUGE_BUILD_DOC FALSE)
else()
    set(BUILD_DOC ${BOUGE_BUILD_DOC})
endif()
set(BUILD_DOC ${BOUGE_BUILD_DOC} CACHE BOOL "TRUE to generate the API documentation, FALSE to ignore it")

# add an option for choosing to statically link to the CRT
if(NOT DEFINED BOUGE_BUILD_STATICCRT)
    set(BOUGE_BUILD_STATICCRT FALSE)
else()
    set(BUILD_STATICCRT ${BOUGE_BUILD_STATICCRT})
endif()
set(BUILD_STATICCRT ${BOUGE_BUILD_STATICCRT} CACHE BOOL "TRUE to statically link against the C Runtime, FALSE to dynamically link agains the C Runtime")

# define BOUGE_STATIC if the build type is not set to 'shared'
if(BUILD_SHARED_LIBS)
    add_definitions(-DBOUGE_EXPORT)
else()
    add_definitions(-DBOUGE_STATIC)
endif()

# disable the rpath stuff
set(CMAKE_SKIP_BUILD_RPATH TRUE)

# add the subdirectories
add_subdirectory(src/bouge)
set(BOUGE_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include PARENT_SCOPE)
set(BOUGE_LIBRARIES bouge PARENT_SCOPE)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
if(BUILD_DOC)
    add_subdirectory(doc)
endif()

#We may want to statically link to CRT to reduce dependancies
if(BUILD_STATICCRT)
    if(MSVC)
        foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
            string(REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        endforeach()
    endif()
endif()

# setup the install rules
install(DIRECTORY include
        DESTINATION .
        COMPONENT devel
        PATTERN ".svn" EXCLUDE)
install(FILES cmake/Modules/Findbouge.cmake DESTINATION ${CMAKE_ROOT}/Modules)
