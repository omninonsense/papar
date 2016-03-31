CC=gcc
CFLAGS=-Wall -Wpedantic
BUILD_DIR=./build

build: prepare
	$(CC) $(CLAFGS) $(CCFLAGS) -I./include -c lexer.c -o $(BUILD_DIR)/lexer.o
	$(CC) $(CLAFGS) $(CCFLAGS) -I./include -c parser.c -o $(BUILD_DIR)/parser.o
	$(CC) $(CLAFGS) $(CCFLAGS) -I./include main.c $(BUILD_DIR)/*.o -o $(BUILD_DIR)/papar

prepare:
	mkdir -p $(BUILD_DIR)

clean:
	rm -rfv $(BUILD_DIR)
