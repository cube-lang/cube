#include "codegen.h"
#include "node.h"
#include "parser.hpp"

// NBinaryOperator::codeGen returns an IR of some maths stuff
//
// It covers operator precedence and stuff
Value* NBinaryOperator::codeGen(CodeGenContext& context)
{
  Instruction::BinaryOps instr;
  switch (op) {
  case TPLUS:     instr = Instruction::Add; goto math;
  case TMINUS:    instr = Instruction::Sub; goto math;
  case TMUL:      instr = Instruction::Mul; goto math;
  case TDIV:      instr = Instruction::SDiv; goto math;

    /* TODO comparison */
  }

  return NULL;

 math:
  return BinaryOperator::Create(instr, lhs.codeGen(context),
                                rhs.codeGen(context), "", context.currentBlock());
}
