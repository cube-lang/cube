#include <filesystem>
#include <iostream>
#include "codegen.h"
#include "node.h"

using namespace std;
namespace fs = std::filesystem;

extern int yyparse();
extern FILE* yyin;
extern NProgram* programBlock;

llvm::Function* createPrintfFunction(CodeGenContext& context);

int main(int argc, char **argv)
{
  int loaded = 0;
  for (const auto& entry : fs::directory_iterator(".")) {
    fs::path f = entry.path();

    if (f.has_extension() && f.extension() == extension) {
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
  std::cout << isDebug << std::endl;

  if (isDebug != NULL && *isDebug == *debugEnvTrue) {
    std::cout << "set debug to true" << std::endl;
    context.debug = true;
  }

  createPrintfFunction(context);
  context.generateCode(*programBlock);
  context.buildAndWriteObject();

  return 0;
}
