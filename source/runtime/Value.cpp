/*
  SPDX short identifier: MIT
  Copyright 2019 Jevgēnijs Protopopovs
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/

#include "gcodelib/runtime/Value.h"
#include "gcodelib/runtime/Error.h"
#include <iostream>
#include <sstream>

namespace GCodeLib::Runtime {

  static std::ostream &operator<<(std::ostream &os, GCodeRuntimeValue::Type type) {
    switch (type) {
      case GCodeRuntimeValue::Type::Integer:
        os << "integer";
        break;
      case GCodeRuntimeValue::Type::Float:
        os << "float";
        break;
      case GCodeRuntimeValue::Type::String:
        os << "string";
        break;
      case GCodeRuntimeValue::Type::None:
        os << "none";
        break;
    }
    return os;
  }
  
  const GCodeRuntimeValue GCodeRuntimeValue::Empty;

  GCodeRuntimeValue::GCodeRuntimeValue()
    : type(Type::None) {}
	
  GCodeRuntimeValue::GCodeRuntimeValue(double value)
    : type(Type::Float), value(value) {}

  GCodeRuntimeValue::GCodeRuntimeValue(const std::string &string)
    : type(Type::String), value(string) {}

  GCodeRuntimeValue::GCodeRuntimeValue(const char *string)
	: type(Type::String), value(string ? std::string(string) : "") {}

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

  const std::string &GCodeRuntimeValue::getString(const std::string &defaultValue) const {
    if (this->is(Type::String)) {
      return std::get<std::string>(this->value);
    } else {
      return defaultValue;
    }
  }

  int64_t GCodeRuntimeValue::asInteger() const {
    if (this->is(Type::Integer)) {
      return this->getInteger();
    } else if (this->is(Type::Float)) {
      return static_cast<int64_t>(this->getFloat());
    } else {
      return 0;
    }
  }

  double GCodeRuntimeValue::asFloat() const {
    if (this->is(Type::Float)) {
      return this->getFloat();
    } else if (this->is(Type::Integer)) {
      return static_cast<double>(this->getInteger());
    } else {
      return 0.0;
    }
  }

  std::string GCodeRuntimeValue::asString() const {
    if (this->is(Type::String)) {
      return this->getString();
    } else if (this->is(Type::Integer)) {
      return std::to_string(this->getInteger());
    } else {
      return std::to_string(this->getFloat());
    }
  }

  bool GCodeRuntimeValue::isNumeric() const {
    return this->is(Type::Integer) || this->is(Type::Float);
  }

  const GCodeRuntimeValue &GCodeRuntimeValue::assertType(Type type) const {
    if (!this->is(type)) {
      std::stringstream ss;
      ss << "Type mismatch: expected " << type << ", got " << this->getType();
      throw GCodeRuntimeError(ss.str());
    }
    return *this;
  }

  const GCodeRuntimeValue &GCodeRuntimeValue::assertNumeric() const {
    if (!this->isNumeric()) {
      throw GCodeRuntimeError("Expected integer or floating point value");
    }
    return *this;
  }

  std::ostream &operator<<(std::ostream &os, const GCodeRuntimeValue &cnst) {
    if (cnst.is(GCodeRuntimeValue::Type::Integer)) {
      os << cnst.getInteger();
    } else if (cnst.is(GCodeRuntimeValue::Type::Float)) {
      os << cnst.getFloat();
    } else {
      os << cnst.getString();
    }
    return os;
  }
}