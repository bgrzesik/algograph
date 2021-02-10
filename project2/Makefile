
CC=clang
CPP=clang++
CFLAGS=-I. -Wall -g -DDDEEBBUUGG -lprofiler
DEPS=

project2: main.c
	$(CC) -o $@ $^ $(CFLAGS)

project2cpp: main.cpp
	$(CPP) -o $@ $^ $(CFLAGS)

clean:
	rm $(OBJ) project1

run: project2
	@killall project2 >/dev/null 2>&1 || true
	time ./project2 < ./input.txt

run-cpp: project2cpp
	@killall project2cpp >/dev/null 2>&1 || true
	time ./project2cpp < ./input.txt

run2: project2
	@killall project2 >/dev/null 2>&1 || true
	time ./project2 < ./input2.txt

debug: project2
	@killall project2 >/dev/null 2>&1 || true
	lldb ./project2 --source lldb.txt

all: project2 project2cpp
default: all
