#include "codegen.h"
#include "node.h"

static std::string rescopeRename(std::string name, int depth);

// NIdentifier::codeGen returns a Value from a given identifier.
//
// In order to do this we:
//  1: Check whether the variable exists in the current block
//    1.1: If so, we return that variable
//  2: Else; we iterate up through the blocks looking for the necessary variable
//    2.1: If we find it, we create a new local variable with the value
//    2.2: We mark that variable as a rescoped variable (for when we want to mutate it later)
//    2.3: We increment the assignment counter and re-run codegen
//       2.3.1: This counter exists as a loop-break to avoid infinite loops in case our assignment messes up
//
// We have a maximum depth we crawl up through, to try and avoid broken loops and circular references. We also
// bomb out if we make it all the way to the top without finding a var
Value* NIdentifier::codeGen(CodeGenContext& context)
{
  if (assCount > 1) {
    throw std::runtime_error("stuck in a re-scope assignement loop, bailing");
  }

  // copy blocks
  std::stack<CodeGenBlock *> blocksCopy;
  blocksCopy = context.blocks;

  int depth;
  for (depth = 0; depth<= maxDepth; depth++) {
    // get top block from contextCopy
    CodeGenBlock* top = blocksCopy.top();
    if (top == NULL) break;

    if (top->locals.find(name) != top->locals.end()) {
      // Create a local variable with this value
      LoadInst* v = new LoadInst(top->locals[name], "", false, top->block);

      if (depth == 0) {
        return v;
      }

      std::string tmpName = rescopeRename(name, depth);

      // Create a temporary variable
      NIdentifier emptyID = NIdentifier("");
      NIdentifier nameID = NIdentifier(tmpName);

      NVariableDeclaration vd(emptyID, v->getType(), nameID);
      vd.assignmentExpr = NULL;

      Value* alloc = vd.codeGen(context);
      if (alloc == NULL) {
        throw std::runtime_error("could not assign re-scoped value " + tmpName);
      }

      StoreInst* store = new StoreInst(v, alloc, false, context.currentBlock());
      if (store == NULL) {
        throw std::runtime_error("could not store temporary value " + tmpName);
      }

      assCount++;
      name = tmpName;
      return this->codeGen(context);
    }

    blocksCopy.pop();
  }

  if (depth == maxDepth) {
    throw std::runtime_error("identifier " + name + " not found within " + std::to_string(depth) + " (maxDepth) blocks of the caller");
  }

  throw std::runtime_error("identifier " + name + " does not exist");
}

static std::string rescopeRename(std::string name, int depth)
{
  return rescopePrefix + rescopeSepr + std::to_string(depth) + rescopeSepr + name;
}
