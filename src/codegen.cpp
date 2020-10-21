#include "codegen.h"
#include "exceptions.h"
#include "node.h"
#include "parser.hpp"

using namespace std;

/* Compile the AST into a module */
void CodeGenContext::generateCode(NProgram& root)
{
  // Ensure package is 'main'
  std::string packageName = root.package.name;

  if (packageName != execPackage) {
    throw ExecutablePackageNameException("invalid package " + packageName + ". expected " + execPackage);
  }

  /* Create the top level interpreter function to call as entry */
  llvm::ArrayRef<Type*> argTypes;

  FunctionType *ftype = FunctionType::get(Type::getInt64Ty(MyContext), makeArrayRef(argTypes), false);
  Function *main = Function::Create(ftype, GlobalValue::ExternalLinkage, realMain, module);

  BasicBlock *bblock = BasicBlock::Create(MyContext, "entry", main, 0);

  /* Push a new variable/block context */
  pushBlock(bblock);
  root.codeGen(*this); /* emit bytecode for the toplevel block */

  Function* userMainF = module->getFunction(userMain);
  if (userMainF == NULL) {
    throw ExecutablePackageEntrypointException();
  }

  llvm::ArrayRef<Value*> argValues;
  CallInst *call = CallInst::Create(userMainF, makeArrayRef(argValues), "", bblock);

  ReturnInst::Create(MyContext, call, bblock);
  popBlock();

  legacy::PassManager pm;

  if (debug) {
    cout << "debug = true" << endl;
    pm.add(createPrintModulePass(outs()));
  }

  //pm.add(createInstructionCombiningPass());   // Disable: external function calls fucking shit it here
  pm.add(createReassociatePass());
  pm.add(createGVNPass());
  pm.add(createCFGSimplificationPass());

  pm.run(*module);
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
    throw CompilerObjectException(EC.message());
  }

  legacy::PassManager pass;
  auto FileType = CGFT_ObjectFile;

  if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
    throw CompilerObjectException("TheTargetMachine can't emit a file of this type");
  }

  pass.run(*module);
  dest.flush();
}
