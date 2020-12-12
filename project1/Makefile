
CC=clang
CFLAGS=-I. -Wall -g -DDDEEBBUUGG -lprofiler
DEPS=

project1: main.c
	$(CC) -o $@ $^ $(CFLAGS)


clean:
	rm $(OBJ) project1

run: project1
	killall project1 || true
	time ./project1 < ./input.txt

debug: project1
	killall project1 || true
	lldb ./project1 --source lldb.txt

viz: run
	../venv/bin/python3 viz.py

all: project1
default: all
