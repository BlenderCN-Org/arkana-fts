#!/bin/bash

if [ $1 ] ; then
    if [ $1 = "-h" -o $1 = "--help" ] ; then
        echo "-d: Enable debug mode"
        echo "-n: do not configure"
        echo "-c: clean"
        exit 0
    fi
fi

DEBUG=0
CONFIG=1
CLEAN=0

while [ $1 ] ; do
    if [ $1 = "-d" ] ; then
        DEBUG=1
        shift
    elif [ $1 = "-n" ] ; then
        CONFIG=0
        shift
    elif [ $1 = "-c" ] ; then
        CLEAN=1
        shift
    else
        echo "WARNING: Unrecognized option $1"
        shift
    fi
done

echo "CONFIG=$CONFIG"
echo "DEBUG=$DEBUG"
echo "CLEAN=$CLEAN"

#####################################################
# BUILD PROCESS
#
# Go into the build directory and build fts, copy the
# executable back here.
#####################################################

mkdir -p linux/release
mkdir -p linux/debug

BUILDDIR=""
if [ $DEBUG = 0 ] ; then
    BUILDDIR="linux/release"
else
    BUILDDIR="linux/debug"
fi

cd $BUILDDIR
EXECUTABLE="$BUILDDIR/fts"

if [ $CLEAN = 1 ] ; then
    rm -Rf *
    cd -
    exit 0
fi

RES=0

if [ $CONFIG = 1 ] ; then
    PARAM=" "
    if [ $DEBUG = 1 ] ; then
        PARAM="-D CMAKE_BUILD_TYPE:STRING=Debug"
    else
        PARAM="-D CMAKE_BUILD_TYPE:STRING=Release"
    fi

    cmake $PARAM ../..
    RES=$?
fi

if [ $RES != 0 ] ; then
    rm -Rf *
    cd -
    exit 1
fi

make
RES=$?

cd -

# If we succeeded in building everything, copy back the executable
# and increase the build number.
if [ $RES = 0 ] ; then
    cp $EXECUTABLE .
fi
