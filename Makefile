CC=g++
CFLAGS=-c -Wall
V8_INCLUDES=-I../v8 ../v8/out/native/*.a 

build-cli-script: clean
	$(CC) $(V8_INCLUDES) src/cli-script.cpp -o out/cli-script

build-expose-functions: clean
	$(CC) $(V8_INCLUDES) src/expose-functions.cpp -o out/expose-functions

clean:
	-rm -rf out/*
