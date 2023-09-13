.PHONY: all test clean

# Platform detection
ifeq ($(OS),Windows_NT)
    mkdir = mkdir $(subst /,\,$(1)) > nul 2>&1 || (exit 0)
    rm = $(wordlist 2,65535,$(foreach FILE,$(subst /,\,$(1)),& del $(FILE) > nul 2>&1)) || (exit 0)
else
    mkdir = mkdir -p $(1)
    rm = rm $(1) > /dev/null 2>&1 || true
endif

# Compiler and compiler flags
CC = gcc
CFLAGS = -O2

# Directories
BUILD_DIR = build
SRC_DIR = src
INCLUDE_DIR = include
TEST_DIR = test
LIB_BIN = libjson.a

CFLAGS += -I$(INCLUDE_DIR)

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.c)
DEPS = $(wildcard $(INCLUDE_DIR)/*.h)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Test source file
TEST_SRC = $(TEST_DIR)/test.c
TEST_BIN = $(TEST_DIR)/test

# Build
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(DEPS)
	@$(call mkdir, $(BUILD_DIR))
	$(CC) -c $(CFLAGS) -o $@ $<

$(LIB_BIN): $(OBJS)
	$(AR) rcs $(LIB_BIN) $(OBJS)

all: $(LIB_BIN)

# Compile and run the test
test: $(LIB_BIN)
	$(CC) $(CFLAGS) $(TEST_SRC) $(LIB_BIN) -o $(TEST_BIN)
	./$(TEST_BIN)

# Format all files
format:
	clang-format -i $(INCLUDE_DIR)/*.h $(SRC_DIR)/*.c

# Clean
clean:
	$(call rm, $(OBJS) $(LIB_BIN) $(TEST_BIN))
