#compiler flag
CC=g++

#compiling flags
CFLAGS=-Wall -Werror -ansi -pedantic


all: bin rshell

bin: bin
	mkdir bin;

rshell: bin
	$(CC) src/rshell.cpp $(CFLAGS) -o bin/rshell

clean:
	rm -r bin
