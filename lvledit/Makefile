CC := g++
SRC_DIR := src
BIN_DIR := build
OBJ_DIR := $(BIN_DIR)/objs
EXECUTABLE := $(BIN_DIR)/lvledit

SRCS := $(shell find $(SRC_DIR) -type f -name "*.cpp")
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
DEPS := $(patsubst %.o,%.d,$(OBJS))
CFLAGS = -Wall -g -I src -fdiagnostics-color=always
LIBS =

ifeq ($(OS),Windows_NT)
	CFLAGS += -DWINDOWS
	LIBS += -static $(shell pkg-config --libs --static sdl2) -lm $(LIBS_FLAGS)
else
	LIBS += -lSDL2 -lSDL2main -lm $(LIBS_FLAGS)
endif

.PHONY: all clean

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	@printf "\033[1m\033[32mLinking \033[36m$(OBJ_DIR) \033[32m-> \033[34m$(EXECUTABLE)\033[0m\n"
	@mkdir -p $(BIN_DIR)
	@$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

$(OBJS): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@printf "\033[1m\033[32mCompiling \033[36m$< \033[32m-> \033[34m$@\033[0m\n"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

$(DEPS): $(OBJ_DIR)/%.d: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -MM -MT $(@:.d=.o) $< -o $@

clean:
	@printf "\033[1m\033[32mDeleting \033[36m$(BIN_DIR) \033[32m-> \033[31mX\033[0m\n"
	@rm -rf $(BIN_DIR)

-include $(OBJS:.o=.d)

-include $(OBJS:.o=.d)
