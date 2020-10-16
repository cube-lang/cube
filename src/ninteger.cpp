#include "codegen.h"
#include "node.h"

// NInteger::codeGen generates IR stuff for integers
Value* NInteger::codeGen(CodeGenContext& context)
{
  return ConstantInt::get(Type::getInt64Ty(MyContext), value, true);
}
