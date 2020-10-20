#include "codegen.h"
#include "node.h"

std::string svelte = "svelte";

// NProgram::codeGen returns the top level IR for a
// cube prorgam
Value* NProgram::codeGen(CodeGenContext& context)
{
  return programBlock.codeGen(context);
}


// NProgram::isSvelte returns true when a program is set to
// svelte mode.
//
// `svelte' mode is used to avoid compiling in things like:
//  1. cmdline args
//  2. environment variables
//  3. the cube main() wrapper
//  4. various helper functions
//
// And much _much_ more. It is designed to create as tiny a program
// as cube is able to create
bool NProgram::isSvelte()
{
  return (mode != NULL && mode->name == svelte);
}
