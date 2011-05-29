#include <typeinfo>
#include "AstNode.h"
#include "CodeGenContext.h"
#include "parser.hpp"

using namespace std;
using namespace llvm;


namespace AST {
    

/* Returns an LLVM type based on the identifier */
static const Type *typeOf(const Identifier& type)
{
    if (type.getName().compare("int") == 0) {
        return Type::getInt64Ty(getGlobalContext());
    } else if (type.getName().compare("double") == 0) {
        return Type::getDoubleTy(getGlobalContext());
    } else if (type.getName().compare("string") == 0) {
        return Type::getInt8PtrTy(getGlobalContext());
    } else if (type.getName().compare("boolean") == 0) {
        return Type::getInt1Ty(getGlobalContext());
    }
    return Type::getVoidTy(getGlobalContext());
}


Value* Integer::codeGen(CodeGenContext& context)
{
    std::cout << "Creating integer: " << value << std::endl;
    return ConstantInt::get(Type::getInt64Ty(getGlobalContext()), value, true);
}

Value* Double::codeGen(CodeGenContext& context)
{
    std::cout << "  Creating double: " << value << std::endl;
    return ConstantFP::get(Type::getDoubleTy(getGlobalContext()), value);
}

Value* String::codeGen(CodeGenContext& context)
{
    std::cout << "  Creating string: " << value << std::endl;
    // generate the type for the globale var
    ArrayType* ArrayTy_0 = ArrayType::get(IntegerType::get(context.getModule()->getContext(), 8), value.size() +1 );
    // create global var which holds the constant string.
    GlobalVariable* gvar_array__str = new GlobalVariable(/*Module=*/*context.getModule(), 
                                                         /*Type=*/ArrayTy_0,
                                                         /*isConstant=*/true,
                                                         /*Linkage=*/GlobalValue::PrivateLinkage,
                                                         /*Initializer=*/0, // has initializer, specified below
                                                         /*Name=*/".str");
    gvar_array__str->setAlignment(1);
    // create the contents for the string global.
    Constant* const_array_5 =  ConstantArray::get(getGlobalContext(), value);
    // Initialize the global with the string
    gvar_array__str->setInitializer(const_array_5);
    
    // generate access pointer to the string 
    std::vector<Constant*> const_ptr_8_indices;
    ConstantInt* const_int64_9 = ConstantInt::get(context.getModule()->getContext(), APInt(64, StringRef("0"), 10));
    const_ptr_8_indices.push_back(const_int64_9);
    const_ptr_8_indices.push_back(const_int64_9);
    Constant* const_ptr_8 = ConstantExpr::getGetElementPtr(gvar_array__str, &const_ptr_8_indices[0], const_ptr_8_indices.size());
    
    return const_ptr_8;
}

Value* Boolean::codeGen(CodeGenContext& context)
{
    std::cout << "  Creating boolean " << value << std::endl;
    return ConstantInt::get(Type::getInt1Ty(getGlobalContext()),boolVal);
}

Value* Identifier::codeGen(CodeGenContext& context)
{
    std::cout << "  Creating identifier reference: " << name << std::endl;
    if (context.locals().find(name) == context.locals().end()) {
        std::cerr << "  undeclared variable " << name << std::endl;
        return nullptr;
    }
    return new LoadInst(context.locals()[name], "", false, context.currentBlock());
}

Value* MethodCall::codeGen(CodeGenContext& context)
{
    std::cout << "  Creating method call: " << id->getName() << std::endl;
    Function *function = context.getModule()->getFunction(id->getName().c_str());
    if (function == nullptr) {
        std::cerr << "no such function " << id->getName() << std::endl;
        return nullptr;
    }
    std::vector<Value*> args;
    ExpressionList::const_iterator it;
    for (it = arguments->begin(); it != arguments->end(); it++) {
        args.push_back((**it).codeGen(context));
    }
    CallInst *call = CallInst::Create(function, args.begin(), args.end(), "", context.currentBlock());
    return call;
}

Value* UnaryOperator::codeGen(CodeGenContext& context)
{
    std::cout << "  Creating binary operation " << op << std::endl;
    Instruction::BinaryOps instr;
    switch(op) {
        case TNOT: instr = Instruction::Xor; break;
        default: // TODO user defined operator
            return nullptr;
    }
    
    Value* rhsValue = rhs->codeGen(context);
    if( !rhsValue->getType()->isIntegerTy() ) 
        return nullptr;
    Value* lhsValue = ConstantInt::get(IntegerType::get(getGlobalContext(),64), StringRef("-1"),10); 
    return BinaryOperator::Create(instr, lhsValue, rhsValue, "unarytmp", context.currentBlock());
}

Value* BinaryOp::codeGen(CodeGenContext& context)
{
    std::cout << "  Creating binary operation " << op << std::endl;
    Instruction::BinaryOps instr;
    
    switch (op) {
        case TPLUS: instr = Instruction::Add; break;
        case TMINUS:instr = Instruction::Sub; break;
        case TMUL:  instr = Instruction::Mul; break;
        case TDIV:  instr = Instruction::SDiv; break;
        case TAND:  instr = Instruction::And; break;
        case TOR:  instr = Instruction::Or; break;
        default: return nullptr;
    }
    Value* rhsValue = rhs->codeGen(context);
    Value* lhsValue = lhs->codeGen(context);
    if( rhsValue->getType() != lhsValue->getType() ) {
        std::cout << "Binary operation of incompatible types. Is a cast missing? \n";
        return nullptr;
    }
    
    return BinaryOperator::Create(instr, lhsValue, rhsValue, "mathtmp", context.currentBlock());
}

Value* CompOperator::codeGen(CodeGenContext& context)
{
    std::cout << "  Creating compare operation " << op << std::endl;
    Instruction::OtherOps oinstr;
    unsigned short predicate;
    Instruction::CastOps cinstr;
    Value * rhsVal = rhs->codeGen(context);
    Value * lhsVal = lhs->codeGen(context);
    if( rhsVal->getType() != lhsVal->getType() ) {
        // since we only support double and int, always cast to double in case of differnt types.
        cinstr = CastInst::getCastOpcode(rhsVal,true, Type::getDoubleTy(getGlobalContext()), true);
        rhsVal = CastInst::Create(cinstr, rhsVal , Type::getDoubleTy(getGlobalContext()), "castdb" , context.currentBlock());
        cinstr = CastInst::getCastOpcode(lhsVal,true, Type::getDoubleTy(getGlobalContext()), true);
        lhsVal = CastInst::Create(cinstr,lhsVal, Type::getDoubleTy(getGlobalContext()), "castdb" , context.currentBlock());
    }
    bool isDouble = rhsVal->getType() == Type::getDoubleTy(getGlobalContext());
    if( isDouble ) {
        oinstr = Instruction::FCmp;
    } else {
        oinstr = Instruction::ICmp;
    }
    switch (op) {
        case TCGE:  predicate = isDouble ? CmpInst::FCMP_OGE : CmpInst::ICMP_SGE;break;
        case TCGT:  predicate = isDouble ? CmpInst::FCMP_OGT : CmpInst::ICMP_SGT;break;
        case TCLT:  predicate = isDouble ? CmpInst::FCMP_OLT : CmpInst::ICMP_SLT;break;
        case TCLE:  predicate = isDouble ? CmpInst::FCMP_OLE : CmpInst::ICMP_SLE;break;
        case TCEQ:  predicate = isDouble ? CmpInst::FCMP_OEQ : CmpInst::ICMP_EQ ;break;
        default: return nullptr;
    }
    
    return CmpInst::Create(oinstr, predicate, lhsVal, rhsVal, "cmptmp", context.currentBlock());
}

Value* Assignment::codeGen(CodeGenContext& context)
{
    std::cout << "  Creating assignment for " << lhs->getName() << std::endl;
    if( context.locals().find(lhs->getName()) == context.locals().end() ) {
        std::cerr << "undeclared variable " << lhs->getName() << std::endl;
        return nullptr;
    }
    AllocaInst * var = context.locals()[lhs->getName()] ;
    const Type* varType = var->getType()->getElementType();
    Value* value = rhs->codeGen(context);
    if( value == nullptr ) {
        std::cout << "  Assignment expression results in nothing\n";
        return nullptr;
    }
    if( value->getType() != varType ) {
        std::cout << "  Assignment of incompatible types " 
        << varType->getTypeID() << "(" << varType->getScalarSizeInBits() << ") "
        << " = " 
        << value->getType()->getTypeID()  << "(" << value->getType()->getScalarSizeInBits() << ") "
        << ". Is a cast missing? \n";
        return nullptr;
    }
    
    new StoreInst(value, context.locals()[lhs->getName()], false, context.currentBlock());
    return value;
}

Value* Block::codeGen(CodeGenContext& context)
{
    StatementList::const_iterator it;
    Value *last = nullptr;
    std::cout << "  Creating block" << std::endl;
    for (it = statements.begin(); it != statements.end(); it++) {
        last = (**it).codeGen(context);
    }
    std::cout << "  Creating block done" << std::endl;
    return last;
}

Value* ExpressionStatement::codeGen(CodeGenContext& context)
{
//    std::cout << "NExpressionStatement: Generating code for " << typeid(expression).name() << std::endl;
    return expression->codeGen(context);
}

Value* VariableDeclaration::codeGen(CodeGenContext& context)
{
    std::cout << "  Creating variable declaration " << type->getName() << " " << id->getName() << std::endl;
    AllocaInst *alloc = new AllocaInst(typeOf(*type), id->getName().c_str(), context.currentBlock());
    context.locals()[id->getName()] = alloc;
    if (assignmentExpr != nullptr) {
        Assignment assn(id, assignmentExpr);
        assn.codeGen(context);
        // they are already deleted by assn.
        id = nullptr;
        assignmentExpr = nullptr;
    }
    return alloc;
}

Value* FunctionDeclaration::codeGen(CodeGenContext& context)
{
    std::cout << "  Creating function: " << id->getName() << std::endl;
    vector<const Type*> argTypes;
    VariableList::const_iterator it;
    for (it = arguments->begin(); it != arguments->end(); it++) {
        argTypes.push_back(typeOf((**it).getIdentifierOfVariablenType()));
    }
    FunctionType *ftype = FunctionType::get(typeOf(*type), argTypes, false);
    Function *function = Function::Create(ftype, GlobalValue::InternalLinkage, id->getName().c_str(), context.getModule());
    BasicBlock *bblock = BasicBlock::Create(getGlobalContext(), "entry", function, 0);
    
    context.newScope(bblock);

    Function::arg_iterator args = function->arg_begin();
    for(it = arguments->begin(); it != arguments->end() ; ++args, ++it ) {
        args->setName((**it).getVariablenName());
        AllocaInst * val = dynamic_cast<AllocaInst*>((**it).codeGen(context));
        new StoreInst(args, val, context.currentBlock());
    }
    block->codeGen(context);
    
    context.endScope();;
    return function;
}

Value* Conditional::codeGen(CodeGenContext& context)
{
    std::cout << "  Creating conditional " << std::endl;
    Value* comp = cmpOp->codeGen(context);
    if( comp == nullptr ) return nullptr;
                                            
    Function* function = context.currentBlock()->getParent();
    std::cout << function->getNameStr() << std::endl;
    BasicBlock* thenBlock = BasicBlock::Create(getGlobalContext(), "then",function);
    BasicBlock* elseBlock = BasicBlock::Create(getGlobalContext(), "else");
    BasicBlock* mergeBlock = BasicBlock::Create(getGlobalContext(), "merge");
    BranchInst* br = BranchInst::Create(thenBlock,elseBlock,comp,context.currentBlock());
    
    context.setInsertPoint(thenBlock);
    Value* thenValue = thenExpr->codeGen(context);
    if( thenValue == nullptr ) return nullptr;
    br = BranchInst::Create(mergeBlock,context.currentBlock());

    function->getBasicBlockList().push_back(elseBlock);
    context.setInsertPoint(elseBlock);
    if( elseExpr != nullptr ) {
        Value * elseValue = elseExpr->codeGen(context);
    }
    br = BranchInst::Create(mergeBlock,context.currentBlock());

    function->getBasicBlockList().push_back(mergeBlock);
    context.setInsertPoint(mergeBlock);

    return mergeBlock;
}

Value* WhileLoop::codeGen(CodeGenContext& context)
{
    std::cout << "  Creating while  " << std::endl;

    Function* function = context.currentBlock()->getParent();
    std::cout << function->getNameStr() << std::endl;
    BasicBlock* firstCondBlock = BasicBlock::Create(getGlobalContext(), "firstcond",function);
    BasicBlock* condBlock = BasicBlock::Create(getGlobalContext(), "cond");
    BasicBlock* loopBlock = BasicBlock::Create(getGlobalContext(), "loop");
    BasicBlock* elseBlock = BasicBlock::Create(getGlobalContext(), "else");
    BasicBlock* mergeBlock = BasicBlock::Create(getGlobalContext(), "merge");
    BranchInst* br = BranchInst::Create(firstCondBlock,context.currentBlock());

    context.setInsertPoint(firstCondBlock);
    Value* firstCondValue = this->condition->codeGen(context);
    if( firstCondValue == nullptr ) return nullptr;
    br = BranchInst::Create(loopBlock,elseBlock,firstCondValue,context.currentBlock());
    
    function->getBasicBlockList().push_back(condBlock);
    context.setInsertPoint(condBlock);
    Value* condValue = this->condition->codeGen(context);
    if( condValue == nullptr ) return nullptr;
    br = BranchInst::Create(loopBlock,mergeBlock,condValue,context.currentBlock());
    
    function->getBasicBlockList().push_back(loopBlock);
    context.setInsertPoint(loopBlock);
    Value* loopValue = this->loopBlock->codeGen(context);
    if( loopValue == nullptr ) return nullptr;
    br = BranchInst::Create(condBlock,context.currentBlock());
    
    function->getBasicBlockList().push_back(elseBlock);
    context.setInsertPoint(elseBlock);
    if( this->elseBlock != nullptr ) {
        Value* elseValue = this->elseBlock->codeGen(context);
        if( elseValue == nullptr ) return nullptr;
    }
    br = BranchInst::Create(mergeBlock,context.currentBlock());
    function->getBasicBlockList().push_back(mergeBlock);
    context.setInsertPoint(mergeBlock);
    
    return mergeBlock;
}

Value* Return::codeGen(CodeGenContext& context)
{
    std::cout << "  Creating return statement " << std::endl;
    if( retExpr ) {
    Value* ret = retExpr->codeGen(context);
    if( ret == nullptr ) return nullptr;
        return ReturnInst::Create(getGlobalContext(), ret, context.currentBlock());
    } else {
        return ReturnInst::Create(getGlobalContext(), 0, context.currentBlock());
    }
}

}