#include <cstdlib>
#include <iostream>
#include "gcodelib/Frontend.h"
#include "gcodelib/runtime/Interpreter.h"
#include <fstream>

using namespace GCodeLib;
using namespace GCodeLib::Runtime;

class TestInterpreter : public GCodeInterpreter {
 public:
  using GCodeInterpreter::GCodeInterpreter;
 protected:
  void syscall(GCodeSyscallType type, const GCodeRuntimeValue &function, const GCodeScopedDictionary<unsigned char> &args) override {
    std::cout << static_cast<unsigned char>(type) << function << '\t';
    for (auto kv : args) {
      std::cout << kv.first << kv.second << ' ';
    }
    std::cout << std::endl;
  }
};

int main(int argc, const char **argv) {
  std::ifstream is(argv[1]);
  GCodeLinuxCNC compiler;
  auto ir = compiler.compile(is);
  TestInterpreter interp(*ir);
  interp.execute();
  return EXIT_SUCCESS;
}