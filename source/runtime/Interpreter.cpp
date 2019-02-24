#include "gcodelib/runtime/Interpreter.h"

namespace GCodeLib {

  GCodeInterpreter::GCodeInterpreter(GCodeIRModule &module)
    : module(module), work(false) {}
  
  void GCodeInterpreter::execute() {
    this->work = true;
    this->stack.push(GCodeRuntimeState());
    GCodeRuntimeState &frame = this->getFrame();
    GCodeVariableScope<unsigned char> args;
    while (this->work && frame.getPC() < this->module.length()) {
      const GCodeIRInstruction &instr = this->module.at(frame.nextPC());
      switch (instr.getOpcode()) {
        case GCodeIROpcode::Push:
          frame.push(instr.getValue());
          break;
        case GCodeIROpcode::Prologue:
          args.clear();
          break;
        case GCodeIROpcode::SetArg: {
          unsigned char key = static_cast<unsigned char>(instr.getValue().asInteger());
          args.put(key, frame.pop());
        } break;
        case GCodeIROpcode::Syscall: {
          GCodeSyscallType type = static_cast<GCodeSyscallType>(instr.getValue().asInteger());
          GCodeRuntimeValue function = frame.pop();
          this->syscall(type, function, args);
        } break;
      }
    }
    this->stack.pop();
    this->work = false;
  }
  
  GCodeRuntimeState &GCodeInterpreter::getFrame() {
    return this->stack.top();
  }
}