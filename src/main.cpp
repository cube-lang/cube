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
      std::cout << "Loaded: " << f << std::endl;
    }
  }

  if (loaded == 0) {
    throw std::runtime_error("No files were found!");
  }

  std::cout << programBlock << std::endl;

  CodeGenContext context;
  createPrintfFunction(context);
  context.generateCode(*programBlock);
  context.buildAndWriteObject();

  std::cout << "Running" << std::endl;
  context.runCode();
  return 0;
}
