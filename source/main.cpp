#include <cstdlib>
#include <iostream>
#include "gcodelib/Frontend.h"
#include "gcodelib/runtime/Interpreter.h"
#include "gcodelib/runtime/Error.h"
#include <fstream>

using namespace GCodeLib;
using namespace GCodeLib::Runtime;

class TestInterpreter : public GCodeInterpreter {
 public:
  using GCodeInterpreter::GCodeInterpreter;
  TestInterpreter(GCodeIRModule &module)
    : GCodeInterpreter::GCodeInterpreter(module), systemScope(numbered, named) {
    this->named.putSupplier("_vmajor", []()->GCodeRuntimeValue { return 100L; });
  }

 protected:
  void syscall(GCodeSyscallType type, const GCodeRuntimeValue &function, const GCodeScopedDictionary<unsigned char> &args) override {
    std::cout << static_cast<unsigned char>(type) << function << '\t';
    for (auto kv : args) {
      std::cout << kv.first << kv.second << ' ';
    }
    std::cout << std::endl;
  }

  GCodeVariableScope &getSystemScope() override {
    return this->systemScope;
  }
 private:
  GCodeVirtualDictionary<std::string> named;
  GCodeVirtualDictionary<int64_t> numbered;
  GCodeCustomVariableScope systemScope;
};

int main(int argc, const char **argv) {
  try {
    std::ifstream is(argv[1]);
    GCodeLinuxCNC compiler;
    auto ir = compiler.compile(is, std::string(argv[1]));
    is.close();
    TestInterpreter interp(*ir);
    interp.execute();
  } catch (Parser::GCodeParseException &ex) {
    if (ex.getLocation().has_value()) {
      std::cout << "Compile error at " << ex.getLocation().value() << std::endl << '\t' << ex.getMessage() << std::endl;
    } else {
      std::cout << "Compile error" << std::endl << '\t' << ex.getMessage() << std::endl;
    }
  } catch (GCodeRuntimeError &ex) {
    if (ex.getLocation().has_value()) {
      std::cout << "Runtime error at " << ex.getLocation().value() << std::endl << '\t' << ex.getMessage() << std::endl;
    } else {
      std::cout << "Runtime error" << std::endl << '\t' << ex.getMessage() << std::endl;
    }
  }
  return EXIT_SUCCESS;
}