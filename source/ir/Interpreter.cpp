#include "gcodelib/ir/Interpreter.h"

namespace GCodeLib {

  GCodeInterpreter::GCodeInterpreter(GCodeIRModule &module)
    : module(module), work(false) {}
  
  void GCodeInterpreter::execute() {
    this->work = true;
    this->stack.push(GCodeRuntimeFrame());
    GCodeRuntimeFrame &frame = this->getFrame();
    std::map<unsigned char, GCodeIRValue> args;
    while (this->work && frame.pc < this->module.length()) {
      const GCodeIRInstruction &instr = this->module.at(frame.pc++);
      switch (instr.getOpcode()) {
        case GCodeIROpcode::Push:
          frame.vstack.push(instr.getValue());
          break;
        case GCodeIROpcode::Prologue:
          args.clear();
          break;
        case GCodeIROpcode::SetArg: {
          unsigned char key = static_cast<unsigned char>(instr.getValue().asInteger());
          args[key] = frame.vstack.top();
          frame.vstack.pop();
        } break;
        case GCodeIROpcode::Syscall: {
          GCodeSyscallType type = static_cast<GCodeSyscallType>(instr.getValue().asInteger());
          GCodeIRValue function = frame.vstack.top();
          frame.vstack.pop();
          this->syscall(type, function, args);
        } break;
      }
    }
    this->stack.pop();
    this->work = false;
  }
  
  GCodeRuntimeFrame &GCodeInterpreter::getFrame() {
    return this->stack.top();
  }
}