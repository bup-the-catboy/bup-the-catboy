#!/bin/bash

exists() {
    which $1 > /dev/null 2> /dev/null
    return $?
}

SILENT=> /dev/null 2> /dev/null

if exists sudo; then
    SUDO=sudo
elif exists doas; then
    SUDO=doas
else
    echo "No sudo or doas detected"
    return 1
fi

if exists apt; then
    $SUDO apt install -y make wget tar zstd gawk gpg wine
elif exists pacman; then
    $SUDO pacman -S --noconfirm --needed make wget tar zstd gawk gnupg wine clang lld
elif exists dnf; then
    $SUDO dnf install --assumeyes make wget tar zstd gawk gpg wine clang lld
else
    echo "Couldn't detect package manager, continuing anyway"
fi

cd ~
git clone https://github.com/HolyBlackCat/quasi-msys2
cd quasi-msys2
echo MINGW64 >msystem.txt
chmod +x env/shell.sh
