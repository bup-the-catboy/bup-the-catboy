### BUP THE CATBOY CONFIG

RENDERER ?= OPENGL               # Valid options: OPENGL, OPENGL_LEGACY, SDL_RENDERER. Anything besides OPENGL disables shaders
MACOS_CROSS ?= 0                 # Enable cross compilation for macOS with osxcross
MACOS_ARCH ?= $(shell uname -m)  # Set target arch for osxcross, defaults to system arch
COMPILER ?= clang                # Set a C compiler
COMPILER_CXX ?= clang++          # Set a C++ compiler
AR ?= ar                         # The ar command
LIBRARIES ?= sdl2 glew           # External libraries to link against
