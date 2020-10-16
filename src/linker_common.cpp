#include "codegen.h"
#include "node.h"

using namespace llvm;

/* Returns an LLVM type based on the identifier */
Type* typeOf(NIdentifier& type)
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
