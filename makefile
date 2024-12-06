CC = gcc
CFLAGS = -Wall
LDFLAGS =

SRC_DIR = src
BUILD_DIR = build

SRC = $(SRC_DIR)/main.c
TARGET = $(BUILD_DIR)/wte

all: $(TARGET)

$(TARGET): $(SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
