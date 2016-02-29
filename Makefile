CC=gcc
CCFLAGS=-Wall -Wpedantic
BUILD_DIR=./build

build: prepare
	$(CC) $(CCFLAGS) -I./include -c lexer.c -o $(BUILD_DIR)/lexer.o
	$(CC) $(CCFLAGS) -I./include main.c $(BUILD_DIR)/*.o -o $(BUILD_DIR)/papar

prepare:
	mkdir -p $(BUILD_DIR)

clean:
	rm -rfv $(BUILD_DIR)
