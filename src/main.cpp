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
        std::cerr << "could not open " << entry << std::endl;
        throw std::runtime_error("bailing");
      }

      yyparse();

      loaded++;
    }
  }

  if (loaded == 0) {
    throw std::runtime_error("No files were found!");
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
