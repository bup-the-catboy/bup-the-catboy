#!/bin/bash

DEST=$OSXCROSS_TARGET_DIR/macports/pkgs
cd ~
git clone https://github.com/libgme/game-music-emu
cd game-music-emu
mkdir build 2> /dev/null
cd build
$(uname -m)-apple-$OSXCROSS_TARGET-cmake ..
make
cd gme
mkdir -p $DEST/opt/local/lib/
mkdir -p $DEST/opt/local/lib/pkgconfig/../../include
cp libgme.dylib $DEST/opt/local/lib/libgme.dylib
cp libgme.pc $DEST/opt/local/lib/pkgconfig/libgme.pc
mkdir $DEST/opt/local/include/gme/ 2> /dev/null
cp ../../gme/gme.h $DEST/opt/local/include/gme/gme.h