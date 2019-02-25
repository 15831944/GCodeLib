#include "gcodelib/runtime/Value.h"
#include <iostream>

namespace GCodeLib {
  
  const GCodeRuntimeValue GCodeRuntimeValue::Empty;

  GCodeRuntimeValue::GCodeRuntimeValue()
    : type(Type::None) {}

  GCodeRuntimeValue::GCodeRuntimeValue(int64_t value)
    : type(Type::Integer), value(value) {}
  
  GCodeRuntimeValue::GCodeRuntimeValue(double value)
    : type(Type::Float), value(value) {}

  GCodeRuntimeValue::GCodeRuntimeValue(const GCodeRuntimeValue &value)
    : type(value.type), value(value.value) {}

  GCodeRuntimeValue &GCodeRuntimeValue::operator=(const GCodeRuntimeValue &value) {
    this->type = value.type;
    this->value = value.value;
    return *this;
  }

  GCodeRuntimeValue::Type GCodeRuntimeValue::getType() const {
    return this->type;
  }

  bool GCodeRuntimeValue::is(Type type) const {
    return this->type == type;
  }

  int64_t GCodeRuntimeValue::getInteger(int64_t defaultValue) const {
    if (this->is(Type::Integer)) {
      return std::get<int64_t>(this->value);
    } else {
      return defaultValue;
    }
  }

  double GCodeRuntimeValue::getFloat(double defaultValue) const {
    if (this->is(Type::Float)) {
      return std::get<double>(this->value);
    } else {
      return defaultValue;
    }
  }

  int64_t GCodeRuntimeValue::asInteger() const {
    if (this->is(Type::Integer)) {
      return this->getInteger();
    } else {
      return static_cast<int64_t>(this->getFloat());
    }
  }

  double GCodeRuntimeValue::asFloat() const {
    if (this->is(Type::Float)) {
      return this->getFloat();
    } else {
      return static_cast<double>(this->getInteger());
    }
  }

  std::ostream &operator<<(std::ostream &os, const GCodeRuntimeValue &cnst) {
    if (cnst.is(GCodeRuntimeValue::Type::Integer)) {
      os << cnst.getInteger();
    } else {
      os << cnst.getFloat();
    }
    return os;
  }
}