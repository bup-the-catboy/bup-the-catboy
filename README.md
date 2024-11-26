# Bup the Catboy

A Mario-like 2D platformer game written in C with SDL

## Licenses

This repository is licensed under DWTFYWT license, but some parts are licensed under other licenses.

### MIT License

Parts licensed under the MIT License are:
- `include/stb_image.h`
  - (C) 2017 Sean Barrett
- Part of `src/io/audio/sfxr.c`
  - (C) 2007 Tomas Pettersson

### LGPL2.1 License

Parts licensed under the LGPL2.1 license are:
- `lib/gme`, `include/gme.h`
  - A fork of libgme with additional changes
  - The full LGPL2.1 License can be found at `lib/gme/license.txt` (after cloning the submodule)
  - (C) 2009 Shay Green

## Source Tree

```
include       - Headers for various libraries
lib
  gme         - Modified version of libgme
  lunarengine - A custom 2D platformer engine built for this game
  bupscript   - Source code for the BupScript language
                interpreter used for modding
src
  io          - Input and output
    assets    - Reading & parsing assets
    audio     - Audio engine
    graphics  - Different graphics API implementations for the RENDERER config option
  font        - Text/font renderer
  game        - Core game source code
    data      - Game data (tile, tileset and entity metadata)
    entities  - Entity interaction and update code
    network   - Server, client and packet handing for online multiplayer
    overlay   - HUD, title screen
    tiles     - Tile interaction
tools         - Programs used during compilation
  assetgen    - Generates an asset archive from the "assets"
                directory that gets embedded into the
                game's executable
```

## Building

### Windows

1. Install the [MSYS2 Toolchain](https://msys2.org)
2. Launch the **MINGW64** shell
3. Install dependencies
```sh
pacman -S git make mingw-w64-x86_64-gcc mingw-w64-x86_64-SDL2 mingw-w64-x86_64-glew
```
4. Follow the [General compile steps](#general-compile-steps)

### Linux

1. Install dependencies

**Debian**
```sh
sudo apt install git make gcc libsdl2-dev libglew-dev pkgconf
```
**Arch**
```sh
sudo pacman -S git make gcc sdl2 glew pkgconf
```
2. Follow the [General compile steps](#general-compile-steps)

### macOS (not tested)

1. Install [Homebrew](https://brew.sh/)
2. Install dependencies
```sh
brew install gcc make sdl2 glew pkgconf
```
3. Follow the [General compile steps](#general-compile-steps)

### Cross compiling for Windows on Linux

1. Setup `quasi-msys2` on your distro using the following command:
```
curl https://raw.githubusercontent.com/bup-the-catboy/bup-the-catboy/main/quasi-msys2-setup.sh | sh
```
2. After that, run `~/quasi-msys2/env/shell.sh`
3. Install dependencies
```sh
pacmake install _gcc _SDL2 _glew _pkgconf
```
4. Follow the [General compile steps](#general-compile-steps)

### Cross compiling for macOS on Linux

1. Download Xcode from [Apple's website](https://developer.apple.com/download/all/?q=xcode)
2. Setup `osxcross` on your distro using the following command:
```sh
curl https://raw.githubusercontent.com/bup-the-catboy/bup-the-catboy/main/osxcross-setup.sh | XCODE_PATH=<path to xcode xip> sh
```
3. Install dependencies
```sh
omp install libsdl2 glew mesa mesa-glu pkgconf
```
4. Turn on macOS cross compile mode for the `Makefile`
```sh
export MACOS_CROSS=1
export MACOS_ARCH=x86_64 # optional
```
5. Follow the [General compile steps](#general-compile-steps)

### General compile steps

1. Clone the repository
```sh
git clone --recursive https://github.com/bup-the-catboy/bup-the-catboy && cd bup-the-catboy
```
2. [Acquire assets](#acquiring-assets)*
3. Compile the game using the `make -j$(nproc)` command
4. The executable should be located at `build/btcb`

*Assets aren't required for the game to compile, but it won't be able to run correctly as level, texture and sound data is missing. 

## Acquiring assets

This game isn't published yet so assets are impossible to get for now.
