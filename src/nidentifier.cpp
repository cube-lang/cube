#include "codegen.h"
#include "node.h"

// NIdentifier::codeGen returns a Value from a given identifier.
// If a variable doesn't exist, this Value is taken from the globals
// list.
//
// TODO: rather than reading directly from globals should the var not exist
// in locals, iterate up through parents looking for variables until hitting
// the top/ global level.
// At this point return either NULL, or maybe even error
Value* NIdentifier::codeGen(CodeGenContext& context)
{
  if (context.locals().find(name) == context.locals().end()) {
    // Get from global; or return NULL
    return context.module->getGlobalVariable(name);
  }
  return new LoadInst(context.locals()[name], "", false, context.currentBlock());
}
