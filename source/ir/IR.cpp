#include "gcodelib/ir/IR.h"
#include <iostream>

namespace GCodeLib {

  GCodeIRConstant::GCodeIRConstant()
    : type(Type::Integer), value(0L) {}

  GCodeIRConstant::GCodeIRConstant(int64_t value)
    : type(Type::Integer), value(value) {}
  
  GCodeIRConstant::GCodeIRConstant(double value)
    : type(Type::Float), value(value) {}

  GCodeIRConstant::Type GCodeIRConstant::getType() const {
    return this->type;
  }

  bool GCodeIRConstant::is(Type type) const {
    return this->type == type;
  }

  int64_t GCodeIRConstant::getInteger(int64_t defaultValue) const {
    if (this->is(Type::Integer)) {
      return std::get<int64_t>(this->value);
    } else {
      return defaultValue;
    }
  }

  double GCodeIRConstant::getFloat(double defaultValue) const {
    if (this->is(Type::Float)) {
      return std::get<double>(this->value);
    } else {
      return defaultValue;
    }
  }

  int64_t GCodeIRConstant::asInteger() const {
    if (this->is(Type::Integer)) {
      return this->getInteger();
    } else {
      return static_cast<int64_t>(this->getFloat());
    }
  }

  double GCodeIRConstant::asFloat() const {
    if (this->is(Type::Float)) {
      return this->getFloat();
    } else {
      return static_cast<double>(this->getInteger());
    }
  }

  std::ostream &operator<<(std::ostream &os, const GCodeIRConstant &cnst) {
    if (cnst.is(GCodeIRConstant::Type::Integer)) {
      os << cnst.getInteger();
    } else {
      os << cnst.getFloat();
    }
    return os;
  }

  GCodeIRInstruction::GCodeIRInstruction(const GCodeSystemCommand &cmd)
   : type(GCodeIRInstruction::Type::SystemCommand), value(cmd) {}

  GCodeIRInstruction::Type GCodeIRInstruction::getType() const {
    return this->type;
  }

  bool GCodeIRInstruction::is(Type type) const {
    return this->type == type;
  }

  const GCodeSystemCommand &GCodeIRInstruction::getSystemCommand() const {
    return std::get<GCodeSystemCommand>(this->value);
  }

  std::ostream &operator<<(std::ostream &os, const GCodeIRInstruction &instr) {
    switch (instr.getType()) {
      case GCodeIRInstruction::Type::SystemCommand:
        return os << std::get<GCodeSystemCommand>(instr.value);
      default:
        return os;
    }
  }

  GCodeSystemCommand::GCodeSystemCommand(FunctionType type, GCodeIRConstant function, const std::map<unsigned char, GCodeIRConstant> &parameters)
    : type(type), function(function), parameters(parameters) {}
  
  GCodeSystemCommand::FunctionType GCodeSystemCommand::getFunctionType() const {
    return this->type;
  }

  const GCodeIRConstant &GCodeSystemCommand::getFunction() const {
    return this->function;
  }

  bool GCodeSystemCommand::hasParameter(unsigned char key) const {
    return this->parameters.count(key) != 0;
  }

  GCodeIRConstant GCodeSystemCommand::getParameter(unsigned char key) const {
    if (this->hasParameter(key)) {
      return this->parameters.at(key);
    } else {
      return GCodeIRConstant(0L);
    }
  }

  std::ostream &operator<<(std::ostream &os, const GCodeSystemCommand &cmd) {
    os << static_cast<char>(cmd.type) << cmd.function;
    for (const auto &kv : cmd.parameters) {
      os << ' ' << kv.first << kv.second;
    }
    return os;
  }

  std::size_t GCodeIRModule::size() const {
    return this->code.size();
  }

  const GCodeIRInstruction &GCodeIRModule::at(std::size_t index) const {
    return *this->code.at(index);
  }

  std::size_t GCodeIRModule::appendInstruction(std::unique_ptr<GCodeIRInstruction> instr) {
    this->code.push_back(std::move(instr));
    return this->code.size();
  }

  std::ostream &operator<<(std::ostream &os, const GCodeIRModule &module) {
    for (auto &instr : module.code) {
      os << *instr << std::endl;
    }
    return os;
  }
}