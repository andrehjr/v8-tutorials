CC=g++
CFLAGS=-c -Wall
V8_INCLUDES=-I../v8 ../v8/out/native/*.a src/cli-script.cpp 

build:
	mkdir -p out
	$(CC) $(V8_INCLUDES) -o out/cli-script

clean:
	-rm -rf out/*
