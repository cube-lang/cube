#include "codegen.h"
#include "node.h"

using namespace llvm;

// NFunctionDeclaration::codeGen returns a function IR
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

  std::vector<Type*> argTypes;
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

  return function;
}
