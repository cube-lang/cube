#include "codegen.h"
#include "node.h"

static std::string descopeName(std::string in);
static bool isRescoped(std::string in);

// NAssignment::codeGen assigns a value to a local variable
//
// In cube we don't really change globals /shrug
Value* NAssignment::codeGen(CodeGenContext& context)
{
  if (context.locals().find(lhs.name) != context.locals().end()) {
    return new StoreInst(rhs.codeGen(context), context.locals()[lhs.name], false, context.currentBlock());
  }

  // If we get this far, our variable is rescoped from somewhere
  if (isRescoped(lhs.name)) {
    lhs.name = descopeName(lhs.name);
  }

  std::stack<CodeGenBlock *> blocksCopy;
  blocksCopy = context.blocks;

  int depth;
  for (depth = 0; depth<= maxDepth; depth++) {
    CodeGenBlock* top = blocksCopy.top();
    if (top == NULL) break;

    if (top->locals.find(lhs.name) != top->locals.end()) {
      return new StoreInst(rhs.codeGen(context), top->locals[lhs.name], false, context.currentBlock());
    }

    blocksCopy.pop();
  }

  if (depth >= maxDepth) {
    throw std::runtime_error("identifier " + lhs.name + " not found within " + std::to_string(depth) + " (maxDepth) blocks of the caller");
  }

  throw std::runtime_error("identifier " + lhs.name + " does not exist");
}


static std::string descopeName(std::string in)
{
  std::smatch matches;

  if (std::regex_search(in, matches, rescopeRegex)) {
    if (matches.size() != 1) {
      throw std::runtime_error("descoping variable name " + in + " failed. " + std::to_string(matches.size()) + " matches found");
    }

    return matches[0].str();
  }

  throw std::runtime_error("descoping variable name " + in + " failed. Match not found");
}

static bool isRescoped(std::string in)
{
  return std::regex_search(in, rescopeRegex);
}
