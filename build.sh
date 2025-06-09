#!/bin/bash

# Author: ivan.yu (ivan.yu@rct-power.com.cn)

set -e

ORGDIR=$(realpath $(dirname $0))
CURDIR=$(pwd)

if test "$1" = "-h" -o "$1" = "--help"; then
    echo "Usage: $0 <release|debug>"
    exit 0
fi

BUILD_TYPE="$1"
if test -z "$BUILD_TYPE"; then
    BUILD_TYPE="release"
fi
BUILD_TYPE=$(echo -n "$BUILD_TYPE" | awk '{print toupper(substr($0, 1, 1)) tolower(substr($0, 2))}')

rm -rf "$CURDIR/build"
mkdir -p "$CURDIR/build"
cd "$CURDIR/build"
cmake -DRCTEMS_USE_MYSQL=True -DCMAKE_BUILD_TYPE=$BUILD_TYPE "$ORGDIR"  \
    && make                                                             \
    && make install                                                     \
    && make pack
cd $CURDIR
