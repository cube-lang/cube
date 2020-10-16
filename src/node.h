#include <iostream>
#include <vector>
#include <llvm/IR/Value.h>

class CodeGenContext;
class NStatement;
class NExpression;
class NVariableDeclaration;
class NConstDeclaration;
class NInternalDeclaration;
class NFunction;

typedef std::vector<NStatement*> StatementList;
typedef std::vector<NExpression*> ExpressionList;
typedef std::vector<NVariableDeclaration*> VariableList;
typedef std::vector<NConstDeclaration*> ConstList;
typedef std::vector<NInternalDeclaration*> InternalList;
typedef std::vector<NFunction*> FunctionList;

class Node {
 public:
  virtual ~Node() {}
  virtual llvm::Value* codeGen(CodeGenContext& context) { }
};

class NExpression : public Node {
};

class NStatement : public Node {
};

class NInteger : public NExpression {
 public:
  long long value;

 NInteger(long long value) : value(value) { }

  virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NDouble : public NExpression {
 public:
  double value;

 NDouble(double value) : value(value) { }

  virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NString : public NExpression {
 public:
  std::string value;

  NString(std::string value) : value(value) { }

  virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NIdentifier : public NExpression {
 public:
  std::string name;

 NIdentifier(const std::string& name) : name(name) { }

  virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NMethodCall : public NExpression {
 public:
  const NIdentifier& id;
  ExpressionList arguments;

 NMethodCall(const NIdentifier& id,
             ExpressionList& arguments) :
  id(id), arguments(arguments) { }

 NMethodCall(const NIdentifier& id) : id(id) { }

  virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NBinaryOperator : public NExpression {
 public:
  int op;
  NExpression& lhs;
  NExpression& rhs;

 NBinaryOperator(NExpression& lhs,
                 int op,
                 NExpression& rhs) :
  lhs(lhs), rhs(rhs), op(op) { }

  virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NAssignment : public NExpression {
 public:
  NIdentifier& lhs;
  NExpression& rhs;

 NAssignment(NIdentifier& lhs,
             NExpression& rhs) :
  lhs(lhs), rhs(rhs) { }

  virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NBlock : public NExpression {
 public:
  StatementList statements;
  NBlock() { }
  virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NProgram : public NExpression {
 public:
  const NIdentifier& package;
  NIdentifier *mode;
  NBlock programBlock;

  NProgram(NIdentifier& package,
           NBlock& programBlock) :
    package(package), programBlock(programBlock) { }

  NProgram(NIdentifier& package,
           NIdentifier *mode,
           NBlock& programBlock):
    package(package), mode(mode), programBlock(programBlock) { }

  virtual llvm::Value* codeGen(CodeGenContext& context);
  bool isSvelte();
};

class NExpressionStatement : public NStatement {
 public:
  NExpression& expression;

 NExpressionStatement(NExpression& expression) :
  expression(expression) { }

  virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NVariableDeclaration : public NStatement {
 public:
  NIdentifier& type;
  NIdentifier& id;
  NExpression *assignmentExpr;

 NVariableDeclaration(NIdentifier& type,
                      NIdentifier& id) :
  type(type), id(id) { }

 NVariableDeclaration(NIdentifier& type,
                      NIdentifier& id,
                      NExpression *assignmentExpr) :
  type(type), id(id), assignmentExpr(assignmentExpr) { }

  virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NConstDeclaration : public NStatement {
 public:
  const NIdentifier& type;
  NIdentifier& id;
  NExpression *assignmentExpr;

 NConstDeclaration(const NIdentifier& type,
                   NIdentifier& id) :
  type(type), id(id) { }

 NConstDeclaration(const NIdentifier& type,
                   NIdentifier& id,
                   NExpression *assignmentExpr) :
  type(type), id(id), assignmentExpr(assignmentExpr) { }

  virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NInternalDeclaration : public NStatement {
 public:
  const NIdentifier& type;
  NIdentifier& id;
  NExpression *assignmentExpr;

 NInternalDeclaration(const NIdentifier& type,
                      NIdentifier& id) :
  type(type), id(id) { }

 NInternalDeclaration(const NIdentifier& type,
                      NIdentifier& id,
                      NExpression *assignmentExpr) :
  type(type), id(id), assignmentExpr(assignmentExpr) { }

  virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NFunctionDeclaration : public NStatement {
 public:
  NIdentifier& type;
  const NIdentifier& id;
  VariableList arguments;
  NBlock& block;

 NFunctionDeclaration(const NIdentifier& id,
                      const VariableList& arguments,
                      NIdentifier& type,
                      NBlock& block) :
   id(id), arguments(arguments), type(type), block(block) { }

  virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NThingDeclaration : public NStatement {
 public:
  const NIdentifier& id;
  NBlock& block;

 NThingDeclaration(const NIdentifier& id,
                   NBlock& block) :
  id(id), block(block) { }
  virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NSuperDeclaration : public NStatement {
 public:
  const NIdentifier& thingID;

  NSuperDeclaration(const NIdentifier& thingID):
    thingID(thingID) { }

  virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NStdlibRequirementDeclaration : public NStatement {
 public:
  const NIdentifier& packageID;

  NStdlibRequirementDeclaration(const NIdentifier& packageID):
    packageID(packageID) { }

  virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NRelativeRequirementDeclaration : public NStatement {
 public:
  const NIdentifier& packageID;

  NRelativeRequirementDeclaration(const NIdentifier& packageID):
    packageID(packageID) { }

  virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NReturn : public NStatement {
 public:
  NExpression& expression;

  NReturn(NExpression& expression):
    expression(expression) { }

  virtual llvm::Value* codeGen(CodeGenContext& context);
};
