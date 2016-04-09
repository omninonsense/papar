all: build

build_dir = ./build
CFLAGS = -lm -I../include
CC=gcc
STD=c11
SOVERSION=1

prepare: dir_structure libpapar

build: prepare build_msg $(patsubst %.c, %.out, $(wildcard *.c))
run: $(patsubst %.out, %.test, $(wildcard $(build_dir)/bin/*.out))

%.out: %.c Makefile
	$(CC) -std=$(STD) -DPAPAR_DEBUGGING=1 -Wall -L$(build_dir) -lpapar $< -o $(build_dir)/bin/$@ $(CFLAGS)

%.test: %.out
	@./$<

dir_structure:
	mkdir -p $(build_dir)/bin

libpapar:
	@echo " "
	@echo "Compiling libpapar.so"
	$(CC) -shared -Wl,-soname,libpapar.so.$(SOVERSION) -o $(build_dir)/libpapar.so -std=$(STD) $(CFLAGS) -Wall -fPIC ../lexer.c ../parser.c
	@rm $(build_dir)/libpapar.so.$(SOVERSION)
	@ln -s libpapar.so $(build_dir)/libpapar.so.$(SOVERSION)

clean:
	rm -rf $(build_dir)/

build_msg:
	@echo " "
	@echo "Compiling tests"
