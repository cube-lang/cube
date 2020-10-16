#include "codegen.h"
#include "node.h"

// NVariableDeclaration::codeGen returns an IR for variable declaration.
//
// This IR is one of two things:
//  1. When inside the main() function, variables are created as globals
//  2. When inside any other function, variables are created as locals.
//
// TODO: alongside the TODO in nidentifier.cpp, remove globals and always store
// as a local. That way NIdentifier::codeGen can walk up the IR until it finds
// variables.
//
// This will allow us to have functions in functions, which have access to vars in
// the toplevel function. It'll also make it much simpler when we come to write our
// `thing' type
Value* NVariableDeclaration::codeGen(CodeGenContext& context)
{
  // When inside our main() function, create everything as a global
  if (isMain(context)) {
    Value *assn;

    // Create an assignment expression which we can save. Otherwise
    // we create a null pointer which can be assigned to later
    if (assignmentExpr != NULL) {
      assn = assignmentExpr->codeGen(context);
    }

    return new GlobalVariable(*context.module,
                              typeOf(type),
                              false,
                              llvm::GlobalValue::ExternalLinkage,
                              assignmentExpr == NULL ? ConstantPointerNull::get(PointerType::get(typeOf(type), 1)) : cast<Constant>(assn),
                              id.name);
  }

  // Not in main(), don't need a global. Create a local instead
  AllocaInst *alloc = new AllocaInst(typeOf(type), 1, id.name.c_str(), context.currentBlock());

  context.locals()[id.name] = alloc;
  if (assignmentExpr != NULL) {
    NAssignment assn(id, *assignmentExpr);
    assn.codeGen(context);
  }

  return alloc;
}
