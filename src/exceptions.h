#include <exception>

static int exceptionExit = 255;

using namespace std;

class CubeException : public exception {
 public:
  std::string id;
  std::string message;

  int line;
  int col;
  std::string file;

  CubeException(std::string message) : message(message) { }

  CubeException(std::string id,
                std::string message) :
    id(id), message(message) { }

  CubeException(std::string id,
                std::string message,
                std::string file,
                int line,
                int col) :
    id(id), message(message), file(file), line(line), col(col) { }


  const char* what() const noexcept { return message.c_str(); }
  std::string loc() { return file + " line: " + std::to_string(line) + ", col: " + std::to_string(col); }
  bool knowsWhere() { return line != 0 && col != 0; }
};

class CompilerParserException : public CubeException {
public:
  explicit CompilerParserException(std::string message) : CubeException("C000", message) { }
};

class RescopeSaveException : public CubeException {
public:
  explicit RescopeSaveException(std::string message) : CubeException("C001", message) { }
  explicit RescopeSaveException(std::string file, int line, int col, std::string message) : CubeException("C001", message, file, line, col) { }
};

class NoSuchIdentifierException : public CubeException {
public:
  explicit NoSuchIdentifierException(std::string message) : CubeException("C002", message) { }
  explicit NoSuchIdentifierException(std::string file, int line, int col, std::string message) : CubeException("C002", message, file, line, col) { }
};

class NameErrorException : public CubeException {
public:
  explicit NameErrorException(std::string message) : CubeException("C003", message) { }
};

class DescopeException : public CubeException {
public:
  explicit DescopeException(std::string message) : CubeException("C004", message) { }
};

class ExecutablePackageNameException : public CubeException {
public:
  explicit ExecutablePackageNameException(std::string message) : CubeException("C005", message) { }
};

class ExecutablePackageEntrypointException : public CubeException {
public:
  explicit ExecutablePackageEntrypointException() : CubeException("C006", "missing main() function") { }
};

class CompilerObjectException : public CubeException {
public:
  explicit CompilerObjectException(std::string message) : CubeException("C007", message) { }
};

class UnexpectedLoopException : public CubeException {
public:
  explicit UnexpectedLoopException(std::string message) : CubeException("C008", message) { }
};
