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

  std::size_t GCodeIRModule::getSymbolId(const std::string &symbol) {
    if (this->symbolIdentifiers.count(symbol) != 0) {
      return this->symbolIdentifiers.at(symbol);
    } else {
      std::size_t symbolId = this->symbols.size();
      this->symbols[symbolId] = symbol;
      this->symbolIdentifiers[symbol] = symbolId;
      return symbolId;
    }
  }

  const std::string &GCodeIRModule::getSymbol(std::size_t symbolId) const {
    static std::string EmptySymbol = "";
    if (this->symbols.count(symbolId) != 0) {
      return this->symbols.at(symbolId);
    } else {
      return EmptySymbol;
    }
  }

  void GCodeIRModule::appendInstruction(GCodeIROpcode opcode, const GCodeRuntimeValue &value) {
    this->code.push_back(GCodeIRInstruction(opcode, value));
  }
}