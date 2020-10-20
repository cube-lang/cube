#include "codegen.h"
#include "node.h"

// NString::codeGen returns an array IR representing
// characters which can be stringified
Value* NString::codeGen(CodeGenContext& context)
{
  return ConstantFP::get(Type::getDoubleTy(MyContext), value);
}
