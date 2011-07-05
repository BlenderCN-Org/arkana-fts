#include <typeinfo>
#include "AstNode.h"
#include "CodeGenContext.h"
#include "parser.hpp"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Transforms/Scalar.h"

#include <stdarg.h>
#include <stdio.h>
#include <algorithm>

using namespace std;
using namespace llvm;


extern "C"
int printvalue(int val)
{
    std::cout << "IDEBUG: " << val << "\n";
    return 1;
}

extern "C"
double printdouble(double val)
{
    std::cout << "DDEBUG: " << val << "\n";
    return 1.;
}

extern "C"
void printstring(char * str)
{
    std::cout << "SDEBUG: " << str << "\n";
}

extern "C"
void display(char * str, ...)
{
    va_list argp;
    va_start(argp,str);
    vprintf(str, argp);
    va_end(argp);
}

namespace AST {
void CodeGenContext::setupBuiltIns()
{
    std::vector<const Type *> argTypesOneInt(1, Type::getInt64Ty(getGlobalContext()));
    FunctionType * ft = FunctionType::get(Type::getInt64Ty(getGlobalContext()), argTypesOneInt, false);
    Function * f = Function::Create(ft, Function::ExternalLinkage,"printvalue",getModule());
    Function::arg_iterator i = f->arg_begin();
    if( i != f->arg_end() )
        i->setName("val");

    std::vector<const Type *> argTypesOneDouble(1, Type::getDoubleTy(getGlobalContext()));
    ft = FunctionType::get(Type::getDoubleTy(getGlobalContext()), argTypesOneDouble,false);
    f = Function::Create(ft, Function::ExternalLinkage,"printdouble",getModule());
    i = f->arg_begin();
    if( i != f->arg_end() )
        i->setName("val");

    ft = FunctionType::get(Type::getDoubleTy(getGlobalContext()), argTypesOneDouble,false);
    f = Function::Create(ft, Function::ExternalLinkage,"sin",getModule());
    i = f->arg_begin();
    if( i != f->arg_end() )
        i->setName("val");

    std::vector<const Type *> argTypesInt8Ptr(1, Type::getInt8PtrTy(getGlobalContext()));
    ft = FunctionType::get(Type::getVoidTy(getGlobalContext()), argTypesInt8Ptr,false);
    f = Function::Create(ft, Function::ExternalLinkage,"printstring",getModule());
    i = f->arg_begin();
    if( i != f->arg_end() )
        i->setName("str");

    ft = FunctionType::get(Type::getVoidTy(getGlobalContext()),argTypesInt8Ptr, true);
    f = Function::Create(ft, Function::ExternalLinkage,"display",getModule());
    i = f->arg_begin();
    if( i != f->arg_end() )
        i->setName("format_str");
}

/* Compile the AST into a module */
void CodeGenContext::generateCode(Block& root)
{
    std::cout << "Generating code...\n";

    /* Create the top level interpreter function to call as entry */
    vector<const Type*> argTypes;
    FunctionType *ftype = FunctionType::get(Type::getVoidTy(getGlobalContext()), argTypes, false);
    mainFunction = Function::Create(ftype, GlobalValue::InternalLinkage, "main", getModule());
    BasicBlock *bblock = BasicBlock::Create(getGlobalContext(), "entry", mainFunction, 0);
    setupBuiltIns();
    /* Push a new variable/block context */
    newScope(bblock);
    Value * retVal = root.codeGen(*this); /* emit bytecode for the toplevel block */
    if( currentBlock()->getTerminator() == nullptr ) {
        ReturnInst::Create(getGlobalContext(),0, currentBlock());
    }
    endScope();

#if !defined(_DEBUG)
    optimize();
#endif
    /* Print the bytecode in a human-readable format
     *     to see if our program compiled properly
     */
    std::cout << "Code is generated.\n";
    PassManager pm;
    pm.add(createPrintModulePass(&outs()));
    pm.run(*module);
}

/* Executes the AST by running the main function */
GenericValue CodeGenContext::runCode() {
    std::cout << "Running code...\n";
    ExecutionEngine *ee = EngineBuilder(module).create();
    assert(ee);
    vector<GenericValue> noargs;
    GenericValue v = ee->runFunction(mainFunction, noargs);
    std::cout << "Code was run.\n";
    std::cout << v.IntVal.toString(10,true) << std::endl;
    ee->freeMachineCodeForFunction(mainFunction);
    delete ee;
    return v;
}

void CodeGenContext::optimize()
{
    FunctionPassManager fpm(getModule());
    fpm.add(createBasicAliasAnalysisPass());
    fpm.add(createPromoteMemoryToRegisterPass());
    fpm.add(createCFGSimplificationPass());
    fpm.add(createInstructionCombiningPass());
    fpm.add(createGVNPass());
    fpm.add(createReassociatePass());
    fpm.doInitialization();
    std::for_each(getModule()->getFunctionList().begin(), getModule()->getFunctionList().end(),
                  [&fpm] (Function& i) {
                      fpm.run(i);
                  }
    );
    
    fpm.run(*mainFunction);
}

}