#include "main.h"

#include <iostream>
#include <stdio.h>
#include "CodeGenContext.h"
#include "AstNode.h"

#include "llvm/Support/ManagedStatic.h"

extern int yyparse();
extern int yylex_destroy();
extern FILE* yyin;
extern AST::Block* programBlock;


int main(int argc, char **argv)
{
    std::string fileName;
    
    if( argc == 2 ) {
        fileName = argv[1] ;
    } else {
        fileName = "test.myl";
    }
    yyin = fopen(fileName.c_str(), "r+") ;
    yyparse();
    assert(programBlock);
    AST::CodeGenContext context;
    context.generateCode(*programBlock);
    context.runCode();

    fclose(yyin);
    delete programBlock;
    yylex_destroy();
    return 0;
}
