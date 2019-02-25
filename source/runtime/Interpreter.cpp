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
        case GCodeIROpcode::Negate:
          frame.negate();
          break;
        case GCodeIROpcode::Add:
          frame.add();
          break;
        case GCodeIROpcode::Subtract:
          frame.subtract();
          break;
        case GCodeIROpcode::Multiply:
          frame.multiply();
          break;
        case GCodeIROpcode::Divide:
          frame.divide();
          break;
        case GCodeIROpcode::Power:
          frame.power();
          break;
        case GCodeIROpcode::Modulo:
          frame.modulo();
          break;
        case GCodeIROpcode::Compare:
          frame.compare();
          break;
        case GCodeIROpcode::Test: {
          int64_t mask = instr.getValue().asInteger();
          frame.test(mask);
        } break;
        case GCodeIROpcode::And:
          frame.iand();
          break;
        case GCodeIROpcode::Or:
          frame.ior();
          break;
        case GCodeIROpcode::Xor:
          frame.ixor();
          break;
      }
    }
    this->stack.pop();
    this->work = false;
  }
  
  GCodeRuntimeState &GCodeInterpreter::getFrame() {
    return this->stack.top();
  }
}