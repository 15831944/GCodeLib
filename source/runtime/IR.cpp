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

  void GCodeIRInstruction::setValue(const GCodeRuntimeValue &value) {
    this->value = value;
  }

  GCodeIRLabel::GCodeIRLabel(GCodeIRModule &module)
    : module(module) {}

  void GCodeIRLabel::bind() {
    if (!this->address.has_value()) {
      this->address = module.code.size();
      for (std::size_t addr : this->patched) {
        this->module.code[addr].setValue(static_cast<int64_t>(this->address.value()));
      }
      this->patched.clear();
    }
  }
  
  bool GCodeIRLabel::bound() const {
    return this->address.has_value();
  }

  void GCodeIRLabel::jump() {
    if (this->address.has_value()) {
      this->module.appendInstruction(GCodeIROpcode::Jump, static_cast<int64_t>(this->address.value()));
    } else {
      this->module.appendInstruction(GCodeIROpcode::Jump);
      std::size_t addr = this->module.code.size() - 1;
      this->patched.push_back(addr);
    }
  }

  void GCodeIRLabel::jumpIf() {
    if (this->address.has_value()) {
      this->module.appendInstruction(GCodeIROpcode::JumpIf, static_cast<int64_t>(this->address.value()));
    } else {
      this->module.appendInstruction(GCodeIROpcode::JumpIf);
      std::size_t addr = this->module.code.size() - 1;
      this->patched.push_back(addr);
    }
  }

  void GCodeIRLabel::call() {
    if (this->address.has_value()) {
      this->module.appendInstruction(GCodeIROpcode::Call, static_cast<int64_t>(this->address.value()));
    } else {
      this->module.appendInstruction(GCodeIROpcode::Call);
      std::size_t addr = this->module.code.size() - 1;
      this->patched.push_back(addr);
    }
  }
  
  std::size_t GCodeIRLabel::getAddress() const {
    return this->address.value();
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

  GCodeIRLabel &GCodeIRModule::getNamedLabel(const std::string &label) {
    if (this->labels.count(label) == 0) {
      this->labels[label] = std::make_shared<GCodeIRLabel>(*this);
    }
    return *this->labels[label];
  }

  void GCodeIRModule::registerProcedure(int64_t procId, const std::string &label) {
    this->procedures[procId] = this->labels[label];
  }

  GCodeIRLabel &GCodeIRModule::getProcedure(int64_t procId) {
    return *this->procedures[procId];
  }

  void GCodeIRModule::appendInstruction(GCodeIROpcode opcode, const GCodeRuntimeValue &value) {
    this->code.push_back(GCodeIRInstruction(opcode, value));
  }

  std::unique_ptr<GCodeIRLabel> GCodeIRModule::newLabel() {
    return std::make_unique<GCodeIRLabel>(*this);
  }

  std::ostream &operator<<(std::ostream &os, const GCodeIRModule &module) {
    for (auto &instr : module.code) {
      os << &instr << '\t' << static_cast<int64_t>(instr.getOpcode()) << '\t' << instr.getValue() << std::endl;
    }
    return os;
  }
}