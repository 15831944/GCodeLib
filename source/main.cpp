/*
  SPDX short identifier: MIT
  Copyright 2019 Jevgēnijs Protopopovs
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/

#include <cstdlib>
#include <iostream>
#include "gcodelib/Frontend.h"
#include "gcodelib/runtime/Interpreter.h"
#include "gcodelib/runtime/Error.h"
#include <fstream>

using namespace GCodeLib;
using namespace GCodeLib::Runtime;

class EchoInterpreter : public GCodeInterpreter {
 public:
  using GCodeInterpreter::GCodeInterpreter;
  EchoInterpreter(GCodeIRModule &module)
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

static constexpr auto CommandRepRap = "reprap";
static constexpr auto CommandLinuxCNC = "linuxcnc";
static constexpr auto CommandAST = "print-ast";
static constexpr auto CommandBytecode = "print-bytecode";

int main(int argc, const char **argv) {
  if (argc < 2) {
    std::cout << "Provide filename" << std::endl;
    return EXIT_FAILURE;
  }
  std::string fileName(argv[1]);
  std::unique_ptr<GCodeCompilerFrontend> compiler;
  if (argc > 2 && std::string(argv[2]).compare(CommandRepRap) == 0) {
    compiler = std::make_unique<GCodeRepRap>();
  } else if (argc == 2 || std::string(argv[2]).compare(CommandLinuxCNC) == 0) {
    compiler = std::make_unique<GCodeLinuxCNC>();
  } else {
    std::cout << "Unknown compiler engine" << std::endl;
    return EXIT_FAILURE;
  }
  std::string mode = "";
  if (argc > 3) {
    mode = std::string(argv[3]);
  }
  try {
    std::ifstream is(argv[1]);
    if (mode.compare(CommandAST) != 0) {
      auto ir = compiler->compile(is, fileName);
      is.close();
      if (mode.compare(CommandBytecode) == 0) {
        std::cout << *ir << std::endl;
      } else {
        EchoInterpreter interp(*ir);
        interp.execute();
      }
    } else {
      auto ast = compiler->parse(is, fileName);
      std::cout << *ast << std::endl;
    }
  } catch (Parser::GCodeParseException &ex) {
    if (ex.getLocation().has_value()) {
      std::cout << "Compile error at " << ex.getLocation().value() << std::endl << '\t' << ex.getMessage() << std::endl;
    } else {
      std::cout << "Compile error" << std::endl << '\t' << ex.getMessage() << std::endl;
    }
    return EXIT_FAILURE;
  } catch (GCodeRuntimeError &ex) {
    if (ex.getLocation().has_value()) {
      std::cout << "Runtime error at " << ex.getLocation().value() << std::endl << '\t' << ex.getMessage() << std::endl;
    } else {
      std::cout << "Runtime error" << std::endl << '\t' << ex.getMessage() << std::endl;
    }
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}