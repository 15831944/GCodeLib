#include "gcodelib/ir/IR.h"
#include <iostream>

namespace GCodeLib {

  GCodeIRValue::GCodeIRValue()
    : type(Type::None) {}

  GCodeIRValue::GCodeIRValue(int64_t value)
    : type(Type::Integer), value(value) {}
  
  GCodeIRValue::GCodeIRValue(double value)
    : type(Type::Float), value(value) {}

  GCodeIRValue::Type GCodeIRValue::getType() const {
    return this->type;
  }

  bool GCodeIRValue::is(Type type) const {
    return this->type == type;
  }

  int64_t GCodeIRValue::getInteger(int64_t defaultValue) const {
    if (this->is(Type::Integer)) {
      return std::get<int64_t>(this->value);
    } else {
      return defaultValue;
    }
  }

  double GCodeIRValue::getFloat(double defaultValue) const {
    if (this->is(Type::Float)) {
      return std::get<double>(this->value);
    } else {
      return defaultValue;
    }
  }

  int64_t GCodeIRValue::asInteger() const {
    if (this->is(Type::Integer)) {
      return this->getInteger();
    } else {
      return static_cast<int64_t>(this->getFloat());
    }
  }

  double GCodeIRValue::asFloat() const {
    if (this->is(Type::Float)) {
      return this->getFloat();
    } else {
      return static_cast<double>(this->getInteger());
    }
  }

  std::ostream &operator<<(std::ostream &os, const GCodeIRValue &cnst) {
    if (cnst.is(GCodeIRValue::Type::Integer)) {
      os << cnst.getInteger();
    } else {
      os << cnst.getFloat();
    }
    return os;
  }

  GCodeIRInstruction::GCodeIRInstruction(GCodeIROpcode opcode, const GCodeIRValue &value)
    : opcode(opcode), value(value) {}
  
  GCodeIROpcode GCodeIRInstruction::getOpcode() const {
    return this->opcode;
  }

  const GCodeIRValue &GCodeIRInstruction::getValue() const {
    return this->value;
  }

  std::size_t GCodeIRModule::length() const {
    return this->code.size();
  }

  const GCodeIRInstruction &GCodeIRModule::at(std::size_t index) const {
    return this->code.at(index);
  }

  void GCodeIRModule::appendInstruction(GCodeIROpcode opcode, const GCodeIRValue &value) {
    this->code.push_back(GCodeIRInstruction(opcode, value));
  }
}