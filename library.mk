MACOS_ARCH ?= $(shell uname -m)
MACOS_CROSS ?= 0
ifeq ($(MACOS_CROSS),1)
	COMPILER ?= clang
	MACOS_TOOL := $(MACOS_ARCH)-apple-$(OSXCROSS_TARGET)
	CC := $(MACOS_TOOL)-$(COMPILER)
	AR := $(MACOS_TOOL)-ar
else
	COMPILER ?= gcc
	CC := $(COMPILER)
	AR := ar
endif

SRC_DIR := .
OBJ_DIR := build/objs
BIN_DIR := build
EXECUTABLE := $(BIN_DIR)/out.a

SRCS := $(shell find $(SRC_DIR) -type f -name "*.c")
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
CFLAGS = -Wall -g -fdiagnostics-color=always -fPIC
LIBS =

ifeq ($(OS),Windows_NT)
	CFLAGS += -DWINDOWS
endif

.PHONY: all clean

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	@printf "\033[1m\033[32m  Linking \033[36m$(OBJ_DIR) \033[32m-> \033[34m$(EXECUTABLE)\033[0m\n"
	@mkdir -p $(BIN_DIR)
	@$(AR) rcs $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@printf "\033[1m\033[32m  Compiling \033[36m$< \033[32m-> \033[34m$@\033[0m\n"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@printf "\033[1m\033[32m  Deleting \033[36m$(BIN_DIR) \033[32m-> \033[31mX\033[0m\n"
	@rm -rf $(BIN_DIR)

-include $(OBJS:.o=.d)

$(OBJ_DIR)/%.d: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -MM -MT $(@:.d=.o) $< -o $@

-include $(OBJS:.o=.d)