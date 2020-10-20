#include "codegen.h"
#include "node.h"

// NReturn::codeGen returns an IR representing a return
// statement from a function.
//
Value* NReturn::codeGen(CodeGenContext& context)
{
  Value *returnValue = expression.codeGen(context);
  context.setCurrentReturnValue(returnValue);

  return returnValue;
}
