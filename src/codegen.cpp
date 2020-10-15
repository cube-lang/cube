#include "node.h"
#include "codegen.h"
#include "parser.hpp"

using namespace std;

/* Compile the AST into a module */
void CodeGenContext::generateCode(NProgram& root)
{
  std::cout << "Generating code...\n";

  // Ensure package is 'main'
  std::string packageName = root.package.name;

  if (packageName != execPackage) {
    throw std::runtime_error("invalid package " + packageName + ". expected " + execPackage);
  }

  /* Create the top level interpreter function to call as entry */
  llvm::ArrayRef<Type*> argTypes;

  FunctionType *ftype = FunctionType::get(Type::getInt32Ty(MyContext), makeArrayRef(argTypes), false);
  Function *main = Function::Create(ftype, GlobalValue::ExternalLinkage, realMain, module);

  BasicBlock *bblock = BasicBlock::Create(MyContext, "entry", main, 0);

  /* Push a new variable/block context */
  pushBlock(bblock);

  root.codeGen(*this); /* emit bytecode for the toplevel block */

  cout << "built" << endl;

  Function* userMainF = module->getFunction(userMain);
  if (userMainF == NULL) {
    throw std::runtime_error("invalid program, missing main() function");
  }

  cout << "add call to " << userMain << ", a function which lives at " << userMainF << endl;
  cout << "and pointing it at " << bblock << endl;

  llvm::ArrayRef<Value*> argValues;

  cout << "args: " << &argValues << endl;
  CallInst *call = CallInst::Create(userMainF, makeArrayRef(argValues), "", bblock);

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
  std::cout << "Running code...\n";
  ExecutionEngine *ee = EngineBuilder( unique_ptr<Module>(module) ).create();
  ee->finalizeObject();
  vector<GenericValue> noargs;

  Function *main = module->getFunction(realMain);
  if (main == NULL) {
    throw std::runtime_error("invalid program, missing main() function");
  }

  GenericValue v = ee->runFunction(main, noargs);
  std::cout << "Code was run.\n";
  return v;
}

void CodeGenContext::buildAndWriteObject() {
  InitializeAllTargetInfos();
  InitializeAllTargets();
  InitializeAllTargetMCs();
  InitializeAllAsmParsers();
  InitializeAllAsmPrinters();

  auto TargetTriple = sys::getDefaultTargetTriple();
  module->setTargetTriple(TargetTriple);

  std::string Error;
  auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);

  // Print an error and exit if we couldn't find the requested target.
  // This generally occurs if we've forgotten to initialise the
  // TargetRegistry or we have a bogus target triple.
  if (!Target) {
    throw std::runtime_error(Error);
  }

  auto CPU = "generic";
  auto Features = "";

  TargetOptions opt;
  Reloc::Model RM = Reloc::Model::PIC_;
  auto TheTargetMachine =
    Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

  module->setDataLayout(TheTargetMachine->createDataLayout());

  auto Filename = "_output.o";
  std::error_code EC;
  raw_fd_ostream dest(Filename, EC, sys::fs::OF_None);

  if (EC) {
    throw std::runtime_error(EC.message());
  }

  legacy::PassManager pass;
  auto FileType = CGFT_ObjectFile;

  if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
    throw std::runtime_error("TheTargetMachine can't emit a file of this type");
  }

  pass.run(*module);
  dest.flush();
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
  /*
    All cube programs define a main() function as their entrypoint.
    In actual fact, though, we create a main() function of our own which
    wraps this call, setting stuff up and configuring globals.

    Thus: we rename the user-defined main() to something else. Now: why bother doing
    this? Why not just have a different entrypoint for users to define?

    A couple of reasons:
    1. main() is a well known convention for entry points, and so it makes no sense to fuck with it
    2. we don't want people creating a main() function which fucks us over here
   */
  std::string functionName = id.name.c_str();
  if (id.name == realMain) {
    functionName = userMain;
  }

  vector<Type*> argTypes;
  VariableList::const_iterator it;
  for (it = arguments.begin(); it != arguments.end(); it++) {
    argTypes.push_back(typeOf((**it).type));
  }
  FunctionType *ftype = FunctionType::get(typeOf(type), makeArrayRef(argTypes), false);
  Function *function = Function::Create(ftype, GlobalValue::InternalLinkage, functionName, context.module);
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
  std::cout << "Creating function: " << functionName << endl;

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
  return context.currentBlock()->getParent()->getName().str() == realMain;
}
