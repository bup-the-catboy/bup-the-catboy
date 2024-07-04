CC := g++
SRC_DIR := src
OBJ_DIR := build/objs
BIN_DIR := build
EXECUTABLE := $(BIN_DIR)/lvledit

PROJ_SRCS := $(shell find $(SRC_DIR) -type f -name "*.cpp")
PROJ_OBJS := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(PROJ_SRCS))
IMGUI_SRCS := imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp imgui/imgui.cpp imgui/backends/imgui_impl_sdl2.cpp imgui/backends/imgui_impl_sdlrenderer2.cpp
IMGUI_OBJS := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(IMGUI_SRCS))
OBJS := $(PROJ_OBJS) $(IMGUI_OBJS)
DEPS := $(patsubst %.o,%.d,$(OBJS))
CFLAGS = -Wall -g -I src/imgui -I src/imgui/backends -fdiagnostics-color=always
LIBS =

ifeq ($(OS),Windows_NT)
	CFLAGS += -DWINDOWS
endif

.PHONY: all clean

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	@printf "\033[1m\033[32mLinking \033[36m$(OBJ_DIR) \033[32m-> \033[34m$(EXECUTABLE)\033[0m\n"
	@mkdir -p $(BIN_DIR)
	@$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

$(OBJS): $(OBJ_DIR)/%.o: %.cpp
	@printf "\033[1m\033[32mCompiling \033[36m$< \033[32m-> \033[34m$@\033[0m\n"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

$(DEPS): %.d: %.o
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -MM -MT $(@:.d=.o) $< -o $@

clean:
	@printf "\033[1m\033[32mDeleting \033[36m$(BIN_DIR) \033[32m-> \033[31mX\033[0m\n"
	@rm -rf $(BIN_DIR)

-include $(OBJS:.o=.d)

-include $(OBJS:.o=.d)