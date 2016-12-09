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

program: runtime/program.c runtime/program.h variable runtime_environment lambda utils
	$(CC) $(CFLAGS) -c runtime/program.c

program_pool: runtime/program_pool.c runtime/program_pool.h program utils
	$(CC) $(CFLAGS) -c runtime/program_pool.c

object_pool: runtime/object_pool.c runtime/object_pool.h object utils
	$(CC) $(CFLAGS) -c runtime/object_pool.c

lambda: runtime/lambda.c runtime/lambda.h program runtime_environment utils
	$(CC) $(CFLAGS) -c runtime/lambda.c

runtime_environment: runtime/runtime_environment.c runtime/runtime_environment.h heap stack variable program_pool object_pool utils
	$(CC) $(CFLAGS) -c runtime/runtime_environment.c

object: runtime/object.c runtime/object.h program utils
	$(CC) $(CFLAGS) -c runtime/object.c

utils: util/util.c util/util.h
	$(CC) $(CFLAGS) -c util/util.c

main: main.c utils variable
	$(CC) $(CFLAGS) -c main.c

all: main utils variable heap stack program program_pool object lambda runtime_environment
	$(CC) $(CFLAGS) -o $(TARGET) main.o util.o variable.o heap.o stack.o program.o program_pool.o object_pool.o lambda.o runtime_environment.o object.o -lm

clean:
	$(RM) $(TARGET) *.o
