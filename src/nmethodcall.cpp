#include "codegen.h"
#include "node.h"

// NMethodCall returns IR code for calling functions
Value* NMethodCall::codeGen(CodeGenContext& context)
{
  Function *function = context.module->getFunction(id.name.c_str());
  if (function == NULL) {
    return NULL;
  }

  std::vector<Value*> args;
  ExpressionList::const_iterator it;
  for (it = arguments.begin(); it != arguments.end(); it++) {
    args.push_back((**it).codeGen(context));
  }

  return CallInst::Create(function, makeArrayRef(args), "", context.currentBlock());
}
