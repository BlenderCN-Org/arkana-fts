#include <stack>
#include <map>
#include <set>
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/Type.h"
#include "llvm/DerivedTypes.h"
#include "llvm/LLVMContext.h"
#include "llvm/PassManager.h"
#include "llvm/Instructions.h"
#include "llvm/CallingConv.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Assembly/PrintModulePass.h"
#include "llvm/Support/IRBuilder.h"
#include "llvm/Target/TargetSelect.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/Support/raw_ostream.h"
#include <llvm/Support/ManagedStatic.h>

namespace AST {
    
class Block;

class VarDef
{
public:
    VarDef(const std::string& name, const std::string& typeName, llvm::Value* value)
    : name(name), tyName(typeName), value(value) {};
private:
    std::string name;
    std::string tyName;
    llvm::Value* value;
};


typedef std::map<std::string, llvm::AllocaInst*> ValueNames;

class CodeGenBlock {
    llvm::BasicBlock* bblock;
    ValueNames locals;
public:
    CodeGenBlock(llvm::BasicBlock* bb) { bblock = bb;}
    ~CodeGenBlock() { }
    void setCodeBlock(llvm::BasicBlock* bb) {bblock = bb;}
    llvm::BasicBlock* currentBlock() {return bblock;}
    ValueNames& getValueNames() {return locals;}
};

class CodeGenContext {
    std::stack<CodeGenBlock *> codeBlocks;
    llvm::Function *mainFunction;
    llvm::Module *module;
    llvm::LLVMContext llvmContext;
    void setCurrentBlock(llvm::BasicBlock *block) {
        codeBlocks.top()->setCodeBlock(block); 
    }
    void setupBuiltIns();
public:
    CodeGenContext() { 
        llvm::InitializeNativeTarget();
        module = new llvm::Module("main", llvmContext);
    }
    ~CodeGenContext() {
        llvm::llvm_shutdown();
    }
        
    llvm::Module * getModule() {return module;}
    llvm::LLVMContext& getGlobalContext() {return llvmContext;}
    void newScope() { 
        llvm::BasicBlock* bblock = llvm::BasicBlock::Create(getGlobalContext(),"scope");
        codeBlocks.push(new CodeGenBlock(bblock)); 
    }
    void newScope(llvm::BasicBlock* bb) {codeBlocks.push(new CodeGenBlock(bb));}
    void endScope() {CodeGenBlock* top = codeBlocks.top(); codeBlocks.pop();delete top;}
    void setInsertPoint(llvm::BasicBlock* bblock) {setCurrentBlock(bblock);}
    llvm::BasicBlock* getInsertPoint() {return currentBlock();}
    void generateCode(Block& root);
    llvm::GenericValue runCode();
    ValueNames& locals() { return codeBlocks.top()->getValueNames(); }
    llvm::BasicBlock *currentBlock() { return codeBlocks.top()->currentBlock(); }
    void optimize();
};

}