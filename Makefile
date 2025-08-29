CC=gcc
CFLAGS=-Wall -Wextra -O2 -fcommon
LDFLAGS=-pthread

SRC_DIR=./src
BUILD_DIR=./build

TARGET=8085vm

BUILD_OBJS= $(BUILD_DIR)/main.o $(BUILD_DIR)/opcodes.o $(BUILD_DIR)/cpu.o $(BUILD_DIR)/debug.o

all: always build

build: $(BUILD_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@

always:
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf build/* $(TARGET)