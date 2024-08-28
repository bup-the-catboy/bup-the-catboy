ifeq ($(OS),Windows_NT)
	EXE := .exe
	WINDOWS := 1
else
	EXE :=
	WINDOWS := 0
endif

MACOS_ARCH ?= $(shell uname -m)
MACOS_CROSS ?= 0
ifeq ($(MACOS_CROSS),1)
	MACOS_TOOL := $(MACOS_ARCH)-apple-$(OSXCROSS_TARGET)
	CC := $(MACOS_TOOL)-gcc
else
	CC := gcc
endif

SRC_DIR := src
OBJ_DIR := build/objs
BIN_DIR := build
TOOLS_SRCDIR := tools
TOOLS_BINDIR := build/tools
TOOLS_CC := gcc
EXECUTABLE := $(BIN_DIR)/btcb$(EXE)

LIB_NAMES := sdl2 libgme

LIBS_DIR := lib
LIBS_SRC := $(shell find $(LIBS_DIR)/* -maxdepth 0 -type d -name "*")
LIBS_BIN := $(patsubst $(LIBS_DIR)/%,$(LIBS_DIR)/lib%.a,$(LIBS_SRC))
LIBS_FLAGS := $(patsubst $(LIBS_DIR)/%,-l%,$(LIBS_SRC))
LIBS_BUILD := $(patsubst $(LIBS_DIR)/%,$(LIBS_DIR)/%/build,$(LIBS_SRC))

SRCS := $(shell find $(SRC_DIR) -type f -name "*.c")
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
TOOLS_SRC := $(shell find $(TOOLS_SRCDIR) -type f -name "*.c")
TOOLS_EXEC := $(patsubst $(TOOLS_SRCDIR)/%.c,$(TOOLS_BINDIR)/%,$(TOOLS_SRC))
CFLAGS = -Wall -g -I src -I include
LDFLAGS := -L./$(LIBS_DIR)
LIBS :=

BUILD_FILES := $(BIN_DIR) $(LIBS_BIN) $(LIBS_BUILD) src/assets/asset_data.h

ifeq ($(WINDOWS),1)
	CFLAGS += -DWINDOWS
	LIBS += -static $(shell pkg-config --libs --static $(LIB_NAMES)) -lm -lWs2_32 $(LIBS_FLAGS)
else ifeq ($(MACOS_CROSS),1)
	CFLAGS += $(shell $(MACOS_TOOL)-pkg-config --cflags $(LIB_NAMES)) -DMACOS
	LIBS += $(shell $(MACOS_TOOL)-pkg-config --libs $(LIB_NAMES)) -lm $(LIBS_FLAGS)
else
	CFLAGS += $(shell pkg-config --cflags $(LIB_NAMES)) -DLINUX
	LIBS += $(shell pkg-config --libs $(LIB_NAMES)) -lm $(LIBS_FLAGS)
endif

CFLAGS += -DNO_VSCODE

.PHONY: all clean compile-libs compile-tools run-tools tools compile

all:
	@$(MAKE) clean --silent
	@$(MAKE) compile-tools --silent
	@$(MAKE) run-tools --silent
	@$(MAKE) compile-libs --silent
	@$(MAKE) compile --silent

tools:
	@$(MAKE) compile-tools --silent
	@$(MAKE) run-tools --silent

compile-libs: $(LIBS_BIN)

$(LIBS_DIR)/lib%.a: $(LIBS_DIR)/%
	@printf "\033[1m\033[32mCompiling library \033[36m$<\033[0m\n"
	@$(MAKE) -f ../../library.mk -C $<
	@cp $</build/out.a $@

compile-tools: $(TOOLS_EXEC)

$(TOOLS_BINDIR)/%: $(TOOLS_SRCDIR)/%.c
	@printf "\033[1m\033[32mCompiling tool \033[36m$< \033[32m-> \033[34m$@\033[0m\n"
	@mkdir -p $(dir $@)
	@$(TOOLS_CC) $(CFLAGS) $< -o $@

run-tools:
	@for tool in $(TOOLS_EXEC); do \
		printf "\033[1m\033[32mRunning tool \033[36m$$tool\033[0m\n"; \
		$$tool$(EXE); \
	done

compile: $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	@printf "\033[1m\033[32mLinking \033[36m$(OBJ_DIR) \033[32m-> \033[34m$(EXECUTABLE)\033[0m\n"
	@mkdir -p $(BIN_DIR)
	@$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@printf "\033[1m\033[32mCompiling \033[36m$< \033[32m-> \033[34m$@\033[0m\n"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@for i in $(BUILD_FILES); do \
		printf "\033[1m\033[32mDeleting \033[36m$$i \033[32m-> \033[31mX\033[0m\n"; \
		rm -rf $$i; \
	done

-include $(OBJS:.o=.d)

$(OBJ_DIR)/%.d: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -MM -MT $(@:.d=.o) $< -o $@ 2> /dev/null

-include $(OBJS:.o=.d)