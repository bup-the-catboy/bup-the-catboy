#!/bin/bash

set -e

if [ ! -e "$XCODE_PATH" ] || [ ! -n "$XCODE_PATH" ]; then
    echo "Xcode not found at location '$XCODE_PATH'"
    echo "Use XCODE_PATH=<path> $0 to specify a path"
    exit 1
fi

cd ~
git clone https://github.com/tpoechtrager/osxcross
cd osxcross
echo Copying Xcode...
cp "$XCODE_PATH" Xcode.xip
./tools/gen_sdk_package_pbzx.sh Xcode.xip
mv *.sdk.tar.xz tarballs

printf "\7"
clear
while true; do
    echo "Extracted macOS SDKs:"
    find tarballs -name "*.sdk.tar.xz" | sed "s/tarballs\/MacOSX//g" | sed "s/.sdk.tar.xz//g"
    echo
    echo -n "Select version: "
    read SDK_VER
    if [ -e "tarballs/MacOSX$SDK_VER.sdk.tar.xz" ]; then
        break
    fi
    clear
    echo "Version not found"
done

SDK_VERSION=$SDK_VER UNATTENDED=1 ./build.sh
PWD=$(pwd)
echo "
export PATH=\$PATH:$PWD/target/bin
export MACOSX_DEPLOYMENT_TARGET=10.7
\$(osxcross-conf)
" >> ~/.bashrc
source ~/.bashrc