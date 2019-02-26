#include <cstdlib>
#include <iostream>
#include "gcodelib/parser/Parser.h"
#include "gcodelib/runtime/Translator.h"
#include "gcodelib/runtime/Interpreter.h"
#include <fstream>

using namespace GCodeLib;

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
  GCodeDefaultScanner scanner(is);
  GCodeParser parser(scanner);
  auto root = parser.parse();
  GCodeIRTranslator translator;
  auto ir = translator.translate(*root);
  TestInterpreter interp(*ir);
  interp.execute();
  return EXIT_SUCCESS;
}