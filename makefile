# Declaration of variables
CC = gcc
CC_FLAGS = -w -I ./include

# File names
TARGET_EXEC = run
MAIN_DIR = ./
SRC_DIR = ./src
OBJ_DIR = ./obj
BUILD_DIR = ./bin

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

SOURCES := $(shell find $(SRC_DIR) -name '*.c')
OBJECTS = $(patsubst %, $(OBJ_DIR)/%, $(notdir $(SOURCES:.c=.o)))
# HEADERS := $(shell find $(./include) -name *.h)
# Main target
$(BUILD_DIR)/$(TARGET_EXEC): $(OBJECTS)
	$(CC) -g $(OBJECTS) -o $(TARGET_EXEC) -lm

# To obtain object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -g -c $(CC_FLAGS) $< -o $@

# To remove generated files
clean:
	rm -f $(TARGET_EXEC) $(OBJECTS)
