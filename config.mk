### BUP THE CATBOY CONFIG

LEGACY_GL ?= 0                   # Use legacy OpenGL (1.x), will disable shaders
MACOS_CROSS ?= 0                 # Enable cross compilation for macOS with osxcross
MACOS_ARCH ?= $(shell uname -m)  # Set target arch for osxcross, defaults to system arch
COMPILER ?= gcc                  # Set a compiler
LIBRARIES ?= sdl2 libgme glew    # Libraries to link against
