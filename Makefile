CC = clang
STD = c11
CFLAGS = -Wall -Wpedantic
CFLAGS += -Iinclude
LDFLAGS += -lm
BUILD_DIR = ./build
SOVERSION = 1

all: build
build: shared
shared: prepare
	$(CC) -shared -Wl,-soname,libpapar.so.$(SOVERSION) -o $(BUILD_DIR)/libpapar.so -std=$(STD) $(CFLAGS) -Wall -fPIC lexer.c parser.c $(LDFLAGS)
	@rm -f $(BUILD_DIR)/libpapar.so.$(SOVERSION)
	@ln -s libpapar.so $(BUILD_DIR)/libpapar.so.$(SOVERSION)

prepare:
	mkdir -p $(BUILD_DIR)

clean:
	rm -rfv $(BUILD_DIR)
