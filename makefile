CC = gcc
CFLAGS = -Wall
LDFLAGS = -lncurses -lm

SRC_DIR = src
BUILD_DIR = build

SRC = $(SRC_DIR)/main.c $(SRC_DIR)/tabUtil.c
TARGET = $(BUILD_DIR)/wte
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC))

all: $(TARGET)

$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
