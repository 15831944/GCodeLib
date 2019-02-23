#include "gcodelib/ir/Interpreter.h"

namespace GCodeLib {

  void GCodeInterpreter::execute(GCodeIRModule &module) {
    std::size_t pc = 0;
    while (pc < module.size()) {
      const GCodeIRInstruction &instr = module.at(pc++);
      switch (instr.getType()) {
        case GCodeIRInstruction::Type::SystemCommand:
          this->execute(instr.getSystemCommand());
          break;
      }
    }
  }
}