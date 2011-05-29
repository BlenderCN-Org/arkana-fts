#LLVM = ~/dev/llvm-2.9/install
LLVM = /usr
LLVM-CONF = $(LLVM)/bin/llvm-config
LLVMLIB = `$(LLVM-CONF) --libdir`
LLVM-INC  = `$(LLVM-CONF) --includedir`
LLVM-FLAGS = `$(LLVM-CONF) --libfiles --cxxflags` -lpthread -ldl
CC = g++ -std=c++0x
SOURCE= main.cpp parser.cpp tokens.cpp AstNode.cpp CodeGenContext.cpp
INCS = CodeGenContext.h AstNode.h

all: parser

clean:
	rm parser.cpp parser.hpp parser tokens.cpp

parser.cpp: parser.y
	bison -d -o $@ $^

parser.hpp: parser.cpp

tokens.cpp: tokens.l parser.hpp
	lex -o $@ $^

parser: $(SOURCE) $(INCS)
	$(CC) -g -o $@ *.cpp  $(LLVM-FLAGS) -rdynamic
