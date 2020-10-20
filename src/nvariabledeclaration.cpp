#include "codegen.h"
#include "node.h"

// NVariableDeclaration::codeGen returns an IR for variable declaration.
//
// Given the statement `var int a = 1` this codeGen will:
//   1: Create a variable 'a' with type 'int64'
//   2: Pass 'a = 1' to NAssignment::codeGen, for that funtion to store the instance to the address
//      created via the AllocInst call below
Value* NVariableDeclaration::codeGen(CodeGenContext& context)
{
  if (lType == NULL) {
    lType = typeOf(type);
  }

  AllocaInst *alloc = new AllocaInst(lType, 0, id.name.c_str(), context.currentBlock());

  context.locals()[id.name] = alloc;
  if (assignmentExpr != NULL) {
    NAssignment assn(id, *assignmentExpr);
    assn.codeGen(context);
  }

  return alloc;
}
