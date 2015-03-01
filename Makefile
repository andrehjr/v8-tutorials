CC=g++
CFLAGS=-c -Wall
V8_INCLUDES=-I../v8 ../v8/out/native/*.a 

build-cli-script: clean
	$(CC) $(V8_INCLUDES) src/cli-script.cpp -o out/cli-script

build-expose-functions: clean
	$(CC) $(V8_INCLUDES) src/expose-functions.cpp -o out/expose-functions

build-expose-objects: clean
	$(CC) $(V8_INCLUDES) src/expose-objects.cpp -o out/expose-objects

build-expose-types: clean
	$(CC) $(V8_INCLUDES) src/expose-types.cpp -o out/expose-types

clean:
	-rm -rf out/*
