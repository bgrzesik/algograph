
CC=clang
CFLAGS=-I. -Wall -g -DDEBUG
DEPS=
OBJ=main.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

project1: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)


clean:
	rm $(OBJ) project1

run: project1
	./project1 < ./input.txt

debug: project1
	lldb ./project1 --source lldb.txt

viz: run
	../venv/bin/python3 viz.py

default: project1
