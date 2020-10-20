#include "codegen.h"
#include "node.h"

// NBlock::codeGen returns an IR for a given block. A block
// is pretty much any logical grouping of statements
Value* NBlock::codeGen(CodeGenContext& context)
{
  StatementList::const_iterator it;
  Value *last = NULL;

  for (it = statements.begin(); it != statements.end(); it++) {
    last = (**it).codeGen(context);
  }

  return last;
}
