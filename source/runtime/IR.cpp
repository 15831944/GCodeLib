#include "gcodelib/runtime/IR.h"
#include <iostream>

namespace GCodeLib {

  GCodeIRInstruction::GCodeIRInstruction(GCodeIROpcode opcode, const GCodeRuntimeValue &value)
    : opcode(opcode), value(value) {}
  
  GCodeIROpcode GCodeIRInstruction::getOpcode() const {
    return this->opcode;
  }

  const GCodeRuntimeValue &GCodeIRInstruction::getValue() const {
    return this->value;
  }

  std::size_t GCodeIRModule::length() const {
    return this->code.size();
  }

  const GCodeIRInstruction &GCodeIRModule::at(std::size_t index) const {
    return this->code.at(index);
  }

  void GCodeIRModule::appendInstruction(GCodeIROpcode opcode, const GCodeRuntimeValue &value) {
    this->code.push_back(GCodeIRInstruction(opcode, value));
  }
}