#include <filesystem>
#include <iostream>
#include "codegen.h"
#include "exceptions.h"
#include "node.h"
#include "parser.h"

using namespace std;
namespace fs = std::filesystem;

extern int yyparse();
extern FILE* yyin;
extern NProgram* programBlock;

std::string currentFile;

llvm::Function* createPrintfFunction(CodeGenContext& context);

int main(int argc, char **argv)
{
  int loaded = 0;
  for (const auto& entry : fs::directory_iterator(".")) {
    fs::path f = entry.path();

    if (f.has_extension() && f.extension() == extension) {
      currentFile = f;
      yyin = fopen(f.c_str(), "r+");

      if (yyin == NULL) {
        throw CompilerParserException("could not read file " + f.string());
      }

      yyparse();

      loaded++;
    }
  }

  if (loaded == 0) {
    throw CompilerParserException("no cube sources found");
  }

  if (programBlock == NULL) {
    throw CompilerParserException("an error occurred in parsing cube sources");
  }

  CodeGenContext context;
  char* isDebug = std::getenv(debugEnvVar);

  if (isDebug != NULL && *isDebug == *debugEnvTrue) {
    context.debug = true;
  }

  createPrintfFunction(context);

  try {
    context.generateCode(*programBlock);
    context.buildAndWriteObject();
  }
  catch(CubeException &e) {
    std::cerr << argv[0] << ": compilation failed with exception " << e.id << std::endl;
    std::cerr << "\t" << e.message << std::endl;

    if (e.knowsWhere()) {
      std::cerr << "\tat " + e.loc() << std::endl;
    }

    return exceptionExit;
  }

  return 0;
}
