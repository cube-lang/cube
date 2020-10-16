#include <stack>
#include <typeinfo>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/CallingConv.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Bitstream/BitstreamReader.h>
#include <llvm/Bitstream/BitstreamWriter.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>

using namespace llvm;

static std::string userMain = "__cube_real_main";
static std::string realMain = "main";
static std::string execPackage = "main";
static std::string extension = ".cb";

static const char* debugEnvVar = "CUBE_DEBUG";
static const char* debugEnvTrue = "1";

class NProgram;
class NIdentifier;

static LLVMContext MyContext;

class CodeGenBlock {
public:
    BasicBlock *block;
    Value *returnValue;
    std::map<std::string, Value*> locals;
};

class CodeGenContext {
  std::stack<CodeGenBlock *> blocks;

public:

  bool debug;
  Module *module;
  CodeGenContext() { module = new Module("main", MyContext); }

  void generateCode(NProgram& root);
  GenericValue runCode();
  void buildAndWriteObject();
  std::map<std::string, Value*>& locals() { return blocks.top()->locals; }
  BasicBlock *currentBlock() { return blocks.top()->block; }
  void pushBlock(BasicBlock *block) { blocks.push(new CodeGenBlock()); blocks.top()->returnValue = NULL; blocks.top()->block = block; }
  void popBlock() { CodeGenBlock *top = blocks.top(); blocks.pop(); delete top; }
  void setCurrentReturnValue(Value *value) { blocks.top()->returnValue = value; }
  Value* getCurrentReturnValue() { return blocks.top()->returnValue; }
};

bool isMain(CodeGenContext& context);
Type* typeOf(NIdentifier& type);
