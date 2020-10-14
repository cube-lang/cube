#include <iostream>
#include "codegen.h"
#include "node.h"

using namespace std;

extern int yyparse();
extern NProgram* programBlock;

llvm::Function* createPrintfFunction(CodeGenContext& context);

int main(int argc, char **argv)
{
    yyparse();
    std::cout << programBlock << std::endl;

    CodeGenContext context;
    createPrintfFunction(context);
    context.generateCode(*programBlock);

    std::cout << "Running" << std::endl;

    context.runCode();
    return 0;
}
