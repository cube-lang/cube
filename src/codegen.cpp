#include "node.h"
#include "codegen.h"
#include "parser.hpp"

using namespace std;

std::string _package {};
Function *_main {};
Function *_real_main {};

/* Compile the AST into a module */
void CodeGenContext::generateCode(NProgram& root)
{
  std::cout << "Generating code...\n";

  // Ensure package is 'main'
  _package = root.package.name;

  if (_package != "main") {
    throw std::runtime_error("invalid package " + _package);
  }

  /* Create the top level interpreter function to call as entry */
  llvm::ArrayRef<Type*> argTypes;

  FunctionType *ftype = FunctionType::get(Type::getInt64Ty(MyContext), makeArrayRef(argTypes), false);
  _real_main = Function::Create(ftype, GlobalValue::InternalLinkage, RealMainName, module);

  BasicBlock *bblock = BasicBlock::Create(MyContext, "entry", _real_main, 0);

  /* Push a new variable/block context */
  pushBlock(bblock);

  root.codeGen(*this); /* emit bytecode for the toplevel block */

  if (_main == NULL) {
    throw std::runtime_error("no main function - generateCode");
  }

  llvm::ArrayRef<Value*> argValues;
  CallInst *call = CallInst::Create(_main, makeArrayRef(argValues), "", bblock);

  std::cout << "Creating return inst" << std::endl;
  ReturnInst::Create(MyContext, call, bblock);
  std::cout << "popBlock()" << std::endl;
  popBlock();

  /* Print the bytecode in a human-readable format
     to see if our program compiled properly
  */
  std::cout << "Code is generated" << std::endl;

  legacy::PassManager pm;

  pm.add(createPrintModulePass(outs()));
  pm.add(createInstructionCombiningPass());
  pm.add(createReassociatePass());
  pm.add(createGVNPass());
  pm.add(createCFGSimplificationPass());

  std::cout << "Passes added <3" << std::endl;
  std::cout << "Running" << std::endl;

  pm.run(*module);

  std::cout << "Runned" << std::endl;
}

/* Executes the AST by running the main function */

GenericValue CodeGenContext::runCode() {
  if (_main == NULL) {
    throw std::runtime_error("no main function - runCode");
  }

  std::cout << "Running code...\n";
  ExecutionEngine *ee = EngineBuilder( unique_ptr<Module>(module) ).create();
  ee->finalizeObject();
  vector<GenericValue> noargs;
  GenericValue v = ee->runFunction(_real_main, noargs);
  std::cout << "Code was run.\n";
  return v;
}

/* Returns an LLVM type based on the identifier */
static Type *typeOf(NIdentifier& type)
{
  if (type.name.compare("int") == 0) {
    return Type::getInt64Ty(MyContext);
  }
  else if (type.name.compare("float") == 0) {
    return Type::getDoubleTy(MyContext);
  }
  // else if (type.name.compare("string") == 0) {
  //   return
  // }
  return Type::getVoidTy(MyContext);
}

/* -- Code Generation -- */

Value* NInteger::codeGen(CodeGenContext& context)
{
  std::cout << "Creating integer: " << value << std::endl;
  return ConstantInt::get(Type::getInt64Ty(MyContext), value, true);
}

Value* NDouble::codeGen(CodeGenContext& context)
{
  std::cout << "Creating double: " << value << std::endl;
  return ConstantFP::get(Type::getDoubleTy(MyContext), value);
}

Value* NString::codeGen(CodeGenContext& context)
{
  std::cout << "Creating string: " << value << std::endl;
  return ConstantFP::get(Type::getDoubleTy(MyContext), value);
}

Value* NIdentifier::codeGen(CodeGenContext& context)
{
  std::cout << "Creating identifier reference: " << name << std::endl;

  if (context.locals().find(name) == context.locals().end()) {
    std::cerr << "undeclared local variable " << name << ". Trying globals" << std::endl;

    // Get from global; or return NULL
    return context.module->getGlobalVariable(name);
  }
  return new LoadInst(context.locals()[name], "", false, context.currentBlock());
}

Value* NMethodCall::codeGen(CodeGenContext& context)
{
  Function *function = context.module->getFunction(id.name.c_str());
  if (function == NULL) {
    std::cerr << "no such function " << id.name << endl;
  }
  std::vector<Value*> args;
  ExpressionList::const_iterator it;
  for (it = arguments.begin(); it != arguments.end(); it++) {
    args.push_back((**it).codeGen(context));
  }
  CallInst *call = CallInst::Create(function, makeArrayRef(args), "", context.currentBlock());
  std::cout << "Creating method call: " << id.name << endl;
  return call;
}

Value* NBinaryOperator::codeGen(CodeGenContext& context)
{
  std::cout << "Creating binary operation " << op << std::endl;
  Instruction::BinaryOps instr;
  switch (op) {
  case TPLUS:     instr = Instruction::Add; goto math;
  case TMINUS:    instr = Instruction::Sub; goto math;
  case TMUL:      instr = Instruction::Mul; goto math;
  case TDIV:      instr = Instruction::SDiv; goto math;

    /* TODO comparison */
  }

  return NULL;
 math:
  return BinaryOperator::Create(instr, lhs.codeGen(context),
                                rhs.codeGen(context), "", context.currentBlock());
}

Value* NAssignment::codeGen(CodeGenContext& context)
{
  std::cout << "Creating assignment for " << lhs.name << std::endl;
  if (context.locals().find(lhs.name) == context.locals().end()) {
    std::cerr << "undeclared variable " << lhs.name << std::endl;
    return NULL;
  }
  return new StoreInst(rhs.codeGen(context), context.locals()[lhs.name], false, context.currentBlock());
}

Value* NBlock::codeGen(CodeGenContext& context)
{
  StatementList::const_iterator it;
  Value *last = NULL;
  for (it = statements.begin(); it != statements.end(); it++) {
    std::cout << "Generating code for " << typeid(**it).name() << std::endl;
    last = (**it).codeGen(context);
  }
  std::cout << "Creating block" << std::endl;
  return last;
}

Value* NExpressionStatement::codeGen(CodeGenContext& context)
{
  std::cout << "Generating code for " << typeid(expression).name() << std::endl;
  return expression.codeGen(context);
}

Value* NVariableDeclaration::codeGen(CodeGenContext& context)
{
  if (isMain(context)) {
    std::cout << "Global <3" << std::endl;

    Value *assn;
    if (assignmentExpr != NULL) {
      std::cout << "Lol not null" << std::endl;
      assn = assignmentExpr->codeGen(context);
    }

    std::cout << "Assignment: " << assn << std::endl;

    return new GlobalVariable(*context.module,
                              typeOf(type),
                              false,
                              llvm::GlobalValue::ExternalLinkage,
                              assignmentExpr == NULL ? ConstantPointerNull::get(PointerType::get(typeOf(type), 1)) : cast<Constant>(assn),
                              id.name);
  }

  AllocaInst *alloc = new AllocaInst(typeOf(type), 1, id.name.c_str(), context.currentBlock());
  std::cout << "Alloc: " << alloc << std::endl;

  context.locals()[id.name] = alloc;
  if (assignmentExpr != NULL) {
    NAssignment assn(id, *assignmentExpr);
    assn.codeGen(context);
  }

  std::map<std::__cxx11::basic_string<char>, llvm::Value*> locals = context.locals();
  std::map<std::__cxx11::basic_string<char>, llvm::Value*>::iterator it = locals.begin();
  while (it != locals.end()) {
    std::cout << it->first << " = " << it->second << std::endl;
    it++;
  }

  return alloc;
}

Value* NFunctionDeclaration::codeGen(CodeGenContext& context)
{
  vector<Type*> argTypes;
  VariableList::const_iterator it;
  for (it = arguments.begin(); it != arguments.end(); it++) {
    argTypes.push_back(typeOf((**it).type));
  }
  FunctionType *ftype = FunctionType::get(typeOf(type), makeArrayRef(argTypes), false);
  Function *function = Function::Create(ftype, GlobalValue::InternalLinkage, id.name.c_str(), context.module);
  BasicBlock *bblock = BasicBlock::Create(MyContext, "entry", function, 0);

  context.pushBlock(bblock);

  Function::arg_iterator argsValues = function->arg_begin();
  Value* argumentValue;

  for (it = arguments.begin(); it != arguments.end(); it++) {
    (**it).codeGen(context);

    argumentValue = &*argsValues++;
    argumentValue->setName((*it)->id.name.c_str());
    StoreInst *inst = new StoreInst(argumentValue, context.locals()[(*it)->id.name], false, bblock);
  }

  block.codeGen(context);
  ReturnInst::Create(MyContext, context.getCurrentReturnValue(), bblock);

  context.popBlock();
  std::cout << "Creating function: " << id.name << endl;

  if (id.name == "main") {
    _main = function;
  }

  return function;
}

Value* NProgram::codeGen(CodeGenContext& context)
{
  return programBlock.codeGen(context);
}

Value* NThingDeclaration::codeGen(CodeGenContext& context)
{
  return NULL;
}

Value* NConstDeclaration::codeGen(CodeGenContext& context)
{
  return NULL;
}

Value* NSuperDeclaration::codeGen(CodeGenContext& context)
{
  return NULL;
}

Value* NInternalDeclaration::codeGen(CodeGenContext& context)
{
  return NULL;
}

Value* NReturn::codeGen(CodeGenContext& context)
{
        std::cout << "Generating return code for " << typeid(expression).name() << endl;
        Value *returnValue = expression.codeGen(context);
        context.setCurrentReturnValue(returnValue);
        return returnValue;
}

/*
  isMain returns a boolean signifying whether or not we're in the
  main entrypoint of a program.

  This function is largely used when determining whether to create a local
  or global variable.
 */
bool isMain(CodeGenContext& context)
{
  std::cout << "Name: " << context.currentBlock()->getParent()->getName().str() << std::endl;
  return context.currentBlock()->getParent()->getName().str() == RealMainName;
}
