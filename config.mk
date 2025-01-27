### BUP THE CATBOY CONFIG

# Valid options: OPENGL, OPENGL_LEGACY, SDL_RENDERER. Anything besides OPENGL disables shaders
RENDERER ?= OPENGL

# Enable cross compilation for macOS with osxcross
MACOS_CROSS ?= 0

# Set target arch for osxcross, defaults to system arch
MACOS_ARCH ?= $(shell uname -m)

# Set a C compiler
COMPILER ?= clang

# Set a C++ compiler
COMPILER_CXX ?= clang++

# The ar command
AR ?= ar

# External libraries to link against
LIBRARIES ?= sdl2 glew

#############################
### APPLY COMPILER CONFIG ###
#############################

ifeq ($(MACOS_CROSS),1)
	ifeq ($(RENDERER),OPENGL)
		RENDERER := OPENGL_LEGACY
	endif
endif

ifeq ($(OS),Windows_NT)
	EXE := .exe
	WINDOWS := 1
else
	EXE :=
	WINDOWS := 0
endif

ifeq ($(MACOS_CROSS),1)
	MACOS_TOOL := $(MACOS_ARCH)-apple-$(OSXCROSS_TARGET)
	CC := $(MACOS_TOOL)-$(COMPILER)
	CXX := $(MACOS_TOOL)-$(COMPILER_CXX)
	AR := $(MACOS_TOOL)-$(AR)
else
	CC := $(COMPILER)
	CXX := $(COMPILER_CXX)
endif