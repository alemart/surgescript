.PHONY: default
default: all

TARGET = surgescript
CC = gcc
CFLAGS = -Wall -std=c11
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

tag_system: runtime/tag_system.c runtime/tag_system.h utils
	$(CC) $(CFLAGS) -c runtime/tag_system.c

vm: runtime/vm.c runtime/vm.h sslib stack program_pool object_manager program object variable
	$(CC) $(CFLAGS) -c runtime/vm.c

object: runtime/object.c runtime/object.h program tag_system utils
	$(CC) $(CFLAGS) -c runtime/object.c

utils: util/util.c util/util.h
	$(CC) $(CFLAGS) -c util/util.c

utf8: util/utf8.c util/utf8.h
	$(CC) $(CFLAGS) -c util/utf8.c

transform: util/transform.c util/transform.h utils
	$(CC) $(CFLAGS) -c util/transform.c

sslib: runtime/sslib/sslib.h runtime/sslib/object.c runtime/sslib/array.c runtime/sslib/application.c runtime/sslib/system.c runtime/sslib/string.c runtime/sslib/number.c runtime/sslib/boolean.c runtime/sslib/console.c runtime/sslib/transform2d.c heap utils utf8
	$(CC) $(CFLAGS) -c runtime/sslib/object.c -o sslib_object.o
	$(CC) $(CFLAGS) -c runtime/sslib/array.c -o sslib_array.o
	$(CC) $(CFLAGS) -c runtime/sslib/application.c -o sslib_application.o
	$(CC) $(CFLAGS) -c runtime/sslib/system.c -o sslib_system.o
	$(CC) $(CFLAGS) -c runtime/sslib/string.c -o sslib_string.o
	$(CC) $(CFLAGS) -c runtime/sslib/number.c -o sslib_number.o
	$(CC) $(CFLAGS) -c runtime/sslib/boolean.c -o sslib_boolean.o
	$(CC) $(CFLAGS) -c runtime/sslib/console.c -o sslib_console.o
	$(CC) $(CFLAGS) -c runtime/sslib/transform2d.c -o sslib_transform2d.o
	ar -cvq sslib.a sslib_object.o sslib_array.o sslib_application.o sslib_system.o sslib_string.o sslib_number.o sslib_boolean.o sslib_console.o sslib_transform2d.o

token: compiler/token.h compiler/token.c utils
	$(CC) $(CFLAGS) -c compiler/token.c

lexer: compiler/lexer.h compiler/lexer.c token utils
	$(CC) $(CFLAGS) -c compiler/lexer.c

parser: compiler/parser.h compiler/parser.c lexer token utils codegen
	$(CC) $(CFLAGS) -c compiler/parser.c

symtable: compiler/symtable.h compiler/symtable.c utils heap stack program
	$(CC) $(CFLAGS) -c compiler/symtable.c

codegen: compiler/codegen.h compiler/codegen.c utils symtable program_pool program
	$(CC) $(CFLAGS) -c compiler/codegen.c

main: main.c utils variable
	$(CC) $(CFLAGS) -c main.c

all: main utils variable heap stack program program_pool object renv tag_system vm sslib token lexer parser symtable codegen utf8 transform
	$(CC) $(CFLAGS) -o $(TARGET) main.o util.o utf8.o variable.o heap.o stack.o program.o program_pool.o object_manager.o tag_system.o transform.o renv.o object.o vm.o sslib.a token.o lexer.o parser.o symtable.o codegen.o -lm

clean:
	$(RM) $(TARGET) *.o *.a
