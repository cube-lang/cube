#include "codegen.h"
#include "node.h"

// NDouble::codeGen returns IR stuff for floats
Value* NDouble::codeGen(CodeGenContext& context)
{
  return ConstantFP::get(Type::getDoubleTy(MyContext), value);
}
