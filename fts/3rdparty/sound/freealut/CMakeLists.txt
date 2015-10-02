# cmake project file by Prakash Punnoor
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

message(STATUS "                    ")
message(STATUS "====================")
message(STATUS "CONFIGURING FreeALUT")
message(STATUS "====================")
message(STATUS "                    ")

PROJECT(Alut C)
SET(CMAKE_MODULE_PATH "${Alut_SOURCE_DIR}/admin/CMakeModules")
SET(PACKAGE "freealut")
SET(PACKAGE_TARNAME "freealut")
SET(PACKAGE_NAME "freealut library")
SET(PACKAGE_MAJOR_VERSION "1")
SET(PACKAGE_MINOR_VERSION "0")
SET(PACKAGE_BUILD_VERSION "1")
SET(PACKAGE_VERSION "${PACKAGE_MAJOR_VERSION}.${PACKAGE_MINOR_VERSION}.${PACKAGE_BUILD_VERSION}")
SET(MAJOR_VERSION "0")
SET(MINOR_VERSION "0")
SET(BUILD_VERSION "0")
SET(VERSION "${MAJOR_VERSION}.${MINOR_VERSION}.${BUILD_VERSION}")
SET(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
SET(PACKAGE_BUGREPORT "openal-devel@opensource.creative.com")
SET(OPERATING_SYSTEM "${CMAKE_SYSTEM_NAME}")
if(MSVC)
SET(OPENAL_INCLUDE_DIR "${Alut_SOURCE_DIR}/../openal-soft/include/AL")
add_definitions(-DALL_STATIC)
add_definitions(-DHAVE__STAT)
add_definitions(-DHAVE_BASETSD_H)
add_definitions(-DHAVE_SLEEP)
add_definitions(-DHAVE_WINDOWS_H)
add_definitions(-DAL_LIBTYPE_STATIC)

else()
SET(OPENAL_INCLUDE_DIR "${Alut_SOURCE_DIR}/../openal-soft/include")
endif()
INCLUDE(${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/CheckIncludeFiles.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/CheckCSourceCompiles.cmake)

FIND_PACKAGE(ConfigHelper)

SET(ALUT_SOURCES src/alutBufferData.c
                 src/alutCodec.c
                 src/alutError.c
                 src/alutInit.c
                 src/alutInputStream.c
                 src/alutInternal.h
                 src/alutLoader.c
                 src/alutOutputStream.c
                 src/alutUtil.c
                 src/alutVersion.c
                 src/alutWaveform.c)

INCLUDE_DIRECTORIES(${Alut_SOURCE_DIR}/include)

SET(ADD_WFLAGS "")
SET(ADD_CFLAGS "")
SET(ADD_LDFLAGS "")

OPTION(PROFILE         "enable profile"            OFF)
OPTION(OPTIMIZATION    "enable optimization"        ON)
OPTION(WARNINGS        "enable warnings"            ON)
IF(WARNINGS)
  OPTION(MORE_WARNINGS "enable more warnings"       OFF)
  OPTION(USE_WERROR    "enable fail on all warning" OFF)
ENDIF(WARNINGS)

ADD_DEFINE(ALUT_BUILD_LIBRARY)
# We could possibly need struct timespec and random(), which are not ANSI.
# Define to 500 if Single Unix conformance is wanted, 600 for sixth revision.
ADD_DEFINE("_XOPEN_SOURCE 500")
# We might need nanosleep, which is a POSIX IEEE Std 1003.1b-1993 feature.
# Define to the POSIX version that should be used.
ADD_DEFINE("_POSIX_C_SOURCE 199309")
# Without __NO_CTYPE tolower and friends are macros which introduce a GLIBC 2.3
# dependency. By defining this identifier we are currently backwards compatible
# to GLIBC 2.1.3, which is a good thing. In addition, the macros lead to code
# which triggers warnings with -Wunreachable-code.
ADD_DEFINE("__NO_CTYPE 1")

ADD_DEFINITIONS(-DHAVE_CONFIG_H)
ADD_DEFINITIONS(-DNDEBUG)

SET(CMAKE_REQUIRED_INCLUDES ${OPENAL_INCLUDE_DIR})
CHECK_INCLUDE_FILES("AL/alc.h;AL/al.h" AL_HEADERS)
IF(NOT AL_HEADERS)
  MESSAGE(FATAL_ERROR "OpenAL header files not found - aborting.")
ENDIF(NOT AL_HEADERS)

IF(DEFINED OPENAL_INCLUDE_DIR)
  INCLUDE_DIRECTORIES(${OPENAL_INCLUDE_DIR})
ENDIF(DEFINED OPENAL_INCLUDE_DIR)

FIND_PACKAGE(SleepFunction)

CHECK_INCLUDE_FILE_DEFINE(stdint.h HAVE_STDINT_H)

IF(WIN32)
  CHECK_INCLUDE_FILE_DEFINE(basetsd.h HAVE_BASETSD_H)
ENDIF(WIN32)

# FindSleepFunction will check for unistd.h if time.h wasn't found
IF(HAVE_TIME_H)
  CHECK_INCLUDE_FILE_DEFINE(unistd.h HAVE_UNISTD_H)
ENDIF(HAVE_TIME_H)

IF(HAVE_UNISTD_H)
  CHECK_FUNCTION_DEFINE("
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <unistd.h>
  " stat "(\"\", (struct stat*)0)" HAVE_STAT)
ENDIF(HAVE_UNISTD_H)

IF(NOT HAVE_STAT)
  CHECK_FUNCTION_DEFINE("
  #include <sys/types.h>
  #include <sys/stat.h>
  " _stat "(\"\", (struct _stat*)0)" HAVE__STAT)
ENDIF(NOT HAVE_STAT)

# compiler specific settings
FIND_PACKAGE(CompilerAttribute)
FIND_PACKAGE(CompilerVisibility)
FIND_PACKAGE(CompilerFlagsSet)

GENERATE_CONFIG_H()

SET(CMAKE_C_FLAGS "${ADD_WFLAGS} ${ADD_CFLAGS} ${CMAKE_C_FLAGS}")
SET(CMAKE_CXX_FLAGS "${ADD_CFLAGS} ${CMAKE_CXX_FLAGS}")
SET(CMAKE_SHARED_LINKER_FLAGS "${ADD_LDFLAGS} ${CMAKE_SHARED_LINKER_FLAGS}")
SET(CMAKE_MODULE_LINKER_FLAGS "${ADD_LDFLAGS} ${CMAKE_MODULE_LINKER_FLAGS}")

ADD_LIBRARY(alut STATIC ${ALUT_SOURCES})
