### BUP THE CATBOY CONFIG

RENDERER ?= OPENGL               # Valid options: OPENGL, OPENGL_LEGACY, SDL_RENDERER. Anything besides OPENGL disables shaders
MACOS_CROSS ?= 0                 # Enable cross compilation for macOS with osxcross
MACOS_ARCH ?= $(shell uname -m)  # Set target arch for osxcross, defaults to system arch
COMPILER ?= gcc                  # Set a compiler
LIBRARIES ?= sdl2 libgme glew    # Libraries to link against
