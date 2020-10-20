#include "codegen.h"
#include "node.h"

// ExpressionStatement::codeGen pretty much wraps NExpression::codeGen
Value* NExpressionStatement::codeGen(CodeGenContext& context)
{
  return expression.codeGen(context);
}
