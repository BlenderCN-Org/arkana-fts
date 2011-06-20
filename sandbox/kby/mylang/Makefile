LLVM = ~/dev/llvm-3.0/build
#LLVM = /usr
LLVM-CONF = $(LLVM)/bin/llvm-config
LLVMLIB = `$(LLVM-CONF) --libdir`
LLVM-INC  = `$(LLVM-CONF) --includedir`
LLVM-FLAGS = `$(LLVM-CONF) --libfiles --cxxflags` -lpthread -ldl
CC = g++ -ggdb -pg -fno-inline -O0 -std=c++0x
SOURCE= main.cpp parser.cpp tokens.cpp AstNode.cpp CodeGenContext.cpp
INCS = CodeGenContext.h AstNode.h
DEFS = -D_DEBUG

all: myl

clean:
	rm parser.cpp parser.hpp parser tokens.cpp

parser.cpp: parser.y
	bison -v -d -o $@ $^

parser.hpp: parser.cpp

tokens.cpp: tokens.l parser.hpp
	lex -o $@ $^

myl: $(SOURCE) $(INCS) Makefile
	$(CC) $(DEFS) -o $@ *.cpp  $(LLVM-FLAGS) -rdynamic
