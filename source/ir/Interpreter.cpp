#include "gcodelib/ir/Interpreter.h"

namespace GCodeLib {

  void GCodeInterpreter::execute(GCodeIRModule &module) {
    std::size_t pc = 0;
    while (pc < module.size()) {
      std::optional<std::reference_wrapper<GCodeIRInstruction>> instr = module.at(pc++);
      if (instr.has_value()) {
        this->execute(instr.value().get());
      }
    }
  }
}