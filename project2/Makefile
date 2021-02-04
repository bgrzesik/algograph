
CC=clang
CFLAGS=-I. -Wall -g -DDDEEBBUUGG -lprofiler
DEPS=

project2: main.c
	$(CC) -o $@ $^ $(CFLAGS)


clean:
	rm $(OBJ) project1

run: project2
	@killall project2 >/dev/null 2>&1 || true
	time ./project2 < ./input2.txt

debug: project2
	@killall project2 >/dev/null 2>&1 || true
	lldb ./project2 --source lldb.txt

all: project2
default: all
