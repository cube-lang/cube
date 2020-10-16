#include "codegen.h"
#include "node.h"

// NAssignment::codeGen assigns a value to a local variable
//
// In cube we don't really change globals /shrug
Value* NAssignment::codeGen(CodeGenContext& context)
{
  if (context.locals().find(lhs.name) == context.locals().end()) {
    throw std::runtime_error("undeclared variable " + lhs.name);
  }
  return new StoreInst(rhs.codeGen(context), context.locals()[lhs.name], false, context.currentBlock());
}
