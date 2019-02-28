#include "gcodelib/runtime/IR.h"
#include "gcodelib/runtime/Error.h"
#include <map>
#include <bitset>
#include <iostream>
#include <iomanip>

namespace GCodeLib::Runtime {

  static const std::map<GCodeIROpcode, std::string> OpcodeMnemonics = {
    { GCodeIROpcode::Prologue, "SyscallPrologue" },
    { GCodeIROpcode::SetArg, "SyscalArgument" },
    { GCodeIROpcode::Syscall, "Syscall" },
    { GCodeIROpcode::Invoke, "Invoke" },
    { GCodeIROpcode::Jump, "Jump" },
    { GCodeIROpcode::JumpIf, "JumpIf" },
    { GCodeIROpcode::Call, "Invoke" },
    { GCodeIROpcode::Ret, "Return" },
    { GCodeIROpcode::LoadNumbered, "LoadNumbered" },
    { GCodeIROpcode::StoreNumbered, "StoreNumbered" },
    { GCodeIROpcode::LoadNamed, "LoadNamed" },
    { GCodeIROpcode::StoreNamed, "StoreNamed" },
    { GCodeIROpcode::Push, "Push" },
    { GCodeIROpcode::Dup, "Duplicate" },
    { GCodeIROpcode::Negate, "Negate" },
    { GCodeIROpcode::Add, "Add" },
    { GCodeIROpcode::Subtract, "Subtract" },
    { GCodeIROpcode::Multiply, "Multiply" },
    { GCodeIROpcode::Divide, "Divide" },
    { GCodeIROpcode::Power, "Power" },
    { GCodeIROpcode::Modulo, "Modulo" },
    { GCodeIROpcode::Compare, "Compare" },
    { GCodeIROpcode::Test, "Test" },
    { GCodeIROpcode::And, "And" },
    { GCodeIROpcode::Or, "Or" },
    { GCodeIROpcode::Xor, "Xor" },
    { GCodeIROpcode::Not, "Not" }
  };

  std::ostream &operator<<(std::ostream &os, GCodeIROpcode opcode) {
    os << OpcodeMnemonics.at(opcode);
    return os;
  }

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

  void GCodeIRInstruction::dump(std::ostream &os, const GCodeIRModule &module) const {
    os << std::left << std::setw(20);
    switch (this->getOpcode()) {
      case GCodeIROpcode::SetArg: {
        unsigned char key = static_cast<unsigned char>(this->getValue().asInteger());
        os << this->getOpcode() << key;
      } break;
      case GCodeIROpcode::Syscall: {
        GCodeSyscallType type = static_cast<GCodeSyscallType>(this->getValue().asInteger());
        os << this->getOpcode() << static_cast<char>(type);
      } break;
      case GCodeIROpcode::Invoke: {
        const std::string &functionId = module.getSymbol(this->getValue().getInteger());
        os << this->getOpcode() << functionId;
      } break;
      case GCodeIROpcode::LoadNamed: {
        const std::string &symbol = module.getSymbol(static_cast<std::size_t>(this->getValue().getInteger()));
        os << this->getOpcode() << symbol;
      } break;
      case GCodeIROpcode::StoreNamed: {
        const std::string &symbol = module.getSymbol(static_cast<std::size_t>(this->getValue().getInteger()));
        os << this->getOpcode() << symbol;
      } break;
      case GCodeIROpcode::Test: {
        int64_t mask = this->getValue().asInteger();
        std::bitset<8> bs(static_cast<int8_t>(mask));
        os << this->getOpcode() << bs;
      } break;
      default:
        if (this->getValue().is(GCodeRuntimeValue::Type::None)) {
          os << this->getOpcode();
        } else {
          os << this->getOpcode() << this->getValue();
        }
        break;
    }
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
    } else {
      throw GCodeRuntimeError("Address already bound to the label");
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
    if (this->address.has_value()) {
      return this->address.value();
    } else {
      throw GCodeRuntimeError("Address is not bound to the label");
    }
  }

  std::size_t GCodeIRModule::length() const {
    return this->code.size();
  }

  const GCodeIRInstruction &GCodeIRModule::at(std::size_t index) const {
    if (index < this->code.size()) {
      return this->code.at(index);
    } else {
      throw GCodeRuntimeError("Instruction at offset " + std::to_string(index) + " not found");
    }
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

  GCodeIRLabel &GCodeIRModule::getProcedure(int64_t procId) const {
    if (this->procedures.count(procId)) {
      return *this->procedures.at(procId);
    } else {
      throw GCodeRuntimeError("Procedure " + std::to_string(procId) + " not found");
    }
  }

  bool GCodeIRModule::linked() const {
    for (auto &kv : this->labels) {
      if (!kv.second->bound()) {
        return false;
      }
    }
    return true;
  }

  void GCodeIRModule::appendInstruction(GCodeIROpcode opcode, const GCodeRuntimeValue &value) {
    this->code.push_back(GCodeIRInstruction(opcode, value));
  }

  std::unique_ptr<GCodeIRLabel> GCodeIRModule::newLabel() {
    return std::make_unique<GCodeIRLabel>(*this);
  }

  std::ostream &operator<<(std::ostream &os, const GCodeIRModule &module) {
    if (!module.symbols.empty()) {
      os << "Symbols:" << std::endl;
      for (auto kv : module.symbols) {
        os << std::left << std::setw(10) << std::setfill(' ') << kv.first << kv.second << std::endl;
      }
      os << std::endl;
    }
    if (!module.procedures.empty()) {
      os << "Procedures:" << std::endl;
      for (auto kv : module.procedures) {
        os << std::left << std::setw(10) << std::setfill(' ') << kv.first << kv.second->getAddress() << std::endl;
      }
      os << std::endl;
    }
    os << "Code:" << std::endl;
    std::size_t offset = 0;
    for (auto &instr : module.code) {
      os << std::left << std::setw(10) << std::setfill(' ') << (std::to_string(offset++) + ":");
      instr.dump(os, module);
      os << std::endl;
    }
    return os;
  }
}