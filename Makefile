.PHONY: default
default: all

TARGET = surgescript
CC = gcc
CFLAGS = -Wall
RM = rm -rf

variable: runtime/variable.c runtime/variable.h utils
	$(CC) $(CFLAGS) -c runtime/variable.c

heap: runtime/heap.c runtime/heap.h variable utils
	$(CC) $(CFLAGS) -c runtime/heap.c

stack: runtime/stack.c runtime/stack.h variable utils
	$(CC) $(CFLAGS) -c runtime/stack.c

program: runtime/program.c runtime/program.h variable renv utils
	$(CC) $(CFLAGS) -c runtime/program.c

program_pool: runtime/program_pool.c runtime/program_pool.h program utils
	$(CC) $(CFLAGS) -c runtime/program_pool.c

object_manager: runtime/object_manager.c runtime/object_manager.h object utils
	$(CC) $(CFLAGS) -c runtime/object_manager.c

renv: runtime/renv.c runtime/renv.h heap stack variable program_pool object_manager utils
	$(CC) $(CFLAGS) -c runtime/renv.c

vm: runtime/vm.c runtime/vm.h sslib stack program_pool object_manager program object variable
	$(CC) $(CFLAGS) -c runtime/vm.c

object: runtime/object.c runtime/object.h program utils
	$(CC) $(CFLAGS) -c runtime/object.c

utils: util/util.c util/util.h
	$(CC) $(CFLAGS) -c util/util.c

sslib: runtime/sslib/sslib.h runtime/sslib/object.c runtime/sslib/array.c heap utils
	$(CC) $(CFLAGS) -c runtime/sslib/object.c -o sslib_object.o
	$(CC) $(CFLAGS) -c runtime/sslib/array.c -o sslib_array.o
	ar -cvq sslib.a sslib_object.o sslib_array.o

main: main.c utils variable
	$(CC) $(CFLAGS) -c main.c

all: main utils variable heap stack program program_pool object renv vm sslib
	$(CC) $(CFLAGS) -o $(TARGET) main.o util.o variable.o heap.o stack.o program.o program_pool.o object_manager.o renv.o object.o vm.o sslib.a -lm

clean:
	$(RM) $(TARGET) *.o
