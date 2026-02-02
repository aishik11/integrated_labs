CC=gcc
CFLAGS=-std=c11 -Wall -Wextra -g
TEST_LDFLAGS=-lrt -lm
SRC_DIR=src
TEST_DIR=tests
BUILD_DIR=build
MAIN=bareshell
TEST=test

all: $(MAIN)

$(MAIN): $(BUILD_DIR)/main.o $(BUILD_DIR)/input.o $(BUILD_DIR)/utils.o $(BUILD_DIR)/command.o $(BUILD_DIR)/looper.o
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/main.o: $(SRC_DIR)/main.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/input.o: $(SRC_DIR)/io/input.c $(SRC_DIR)/io/input.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/utils.o: $(SRC_DIR)/utils.c $(SRC_DIR)/utils.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/command.o: $(SRC_DIR)/parser/command.c $(SRC_DIR)/parser/command.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/looper.o: $(SRC_DIR)/repl/looper.c $(SRC_DIR)/repl/looper.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/command_test.o: $(TEST_DIR)/command_test.c $(SRC_DIR)/parser/command.h $(TEST_DIR)/minunit.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

test: $(BUILD_DIR)/command_test.o $(BUILD_DIR)/command.o
	$(CC) $(CFLAGS) $(TEST_LDFLAGS) -o $(TEST) $^
	./$(TEST)

clean:
	rm -rf $(BUILD_DIR) $(MAIN) $(TEST)

.PHONY: all clean test
