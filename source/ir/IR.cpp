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

  std::ostream &operator<<(std::ostream &os, const GCodeIRConstant &cnst) {
    if (cnst.is(GCodeIRConstant::Type::Integer)) {
      os << cnst.getInteger();
    } else {
      os << cnst.getFloat();
    }
    return os;
  }

  GCodeIRInstruction::GCodeIRInstruction(Type type)
   : type(type) {}

  GCodeIRInstruction::Type GCodeIRInstruction::getType() const {
    return this->type;
  }

  bool GCodeIRInstruction::is(Type type) const {
    return this->type == type;
  }

  std::ostream &operator<<(std::ostream &os, const GCodeIRInstruction &instr) {
    instr.dump(os);
    return os;
  }

  GCodeIRCommand::GCodeIRCommand(FunctionType type, GCodeIRConstant function, const std::map<unsigned char, GCodeIRConstant> &parameters)
    : GCodeIRInstruction::GCodeIRInstruction(Type::Command), type(type), function(function), parameters(parameters) {}
  
  GCodeIRCommand::FunctionType GCodeIRCommand::getFunctionType() const {
    return this->type;
  }

  const GCodeIRConstant &GCodeIRCommand::getFunction() const {
    return this->function;
  }

  bool GCodeIRCommand::hasParameter(unsigned char key) const {
    return this->parameters.count(key) != 0;
  }

  GCodeIRConstant GCodeIRCommand::getParameter(unsigned char key) const {
    if (this->hasParameter(key)) {
      return this->parameters.at(key);
    } else {
      return GCodeIRConstant(0L);
    }
  }

  void GCodeIRCommand::dump(std::ostream &os) const {
    os << static_cast<char>(this->type) << this->function;
    for (const auto &kv : this->parameters) {
      os << ' ' << kv.first << kv.second;
    }
  }

  GCodeIRModule::GCodeIRModule(std::vector<std::unique_ptr<GCodeIRInstruction>> code)
    : code(std::move(code)) {}

  std::size_t GCodeIRModule::size() {
    return this->code.size();
  }

  std::optional<std::reference_wrapper<GCodeIRInstruction>> GCodeIRModule::at(std::size_t index) {
    if (index < this->code.size()) {
      return std::ref(*this->code.at(index));
    } else {
      return std::optional<std::reference_wrapper<GCodeIRInstruction>>();
    }
  }

  std::ostream &operator<<(std::ostream &os, const GCodeIRModule &module) {
    for (auto &instr : module.code) {
      os << *instr << std::endl;
    }
    return os;
  }
}