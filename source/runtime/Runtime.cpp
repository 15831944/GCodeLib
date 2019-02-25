#include "gcodelib/runtime/Runtime.h"
#include "gcodelib/runtime/Error.h"
#include <cmath>

namespace GCodeLib {

  static const int64_t GCodeTrue = 1;
  static const int64_t GCodeFalse = 0;

  GCodeRuntimeState::GCodeRuntimeState()
    : pc(0) {}

  std::size_t GCodeRuntimeState::getPC() const {
    return this->pc;
  }

  std::size_t GCodeRuntimeState::nextPC() {
    return this->pc++;
  }

  void GCodeRuntimeState::jump(std::size_t pc) {
    this->pc = pc;
  }

  void GCodeRuntimeState::push(const GCodeRuntimeValue &value) {
    this->stack.push(value);
  }

  GCodeRuntimeValue GCodeRuntimeState::pop() {
    if (this->stack.empty()) {
      throw GCodeRuntimeError("Stack underflow");
    }
    GCodeRuntimeValue value = this->stack.top();
    this->stack.pop();
    return value;
  }

  const GCodeRuntimeValue &GCodeRuntimeState::peek() {
    if (this->stack.empty()) {
      throw GCodeRuntimeError("Stack underflow");
    }
    return this->stack.top();
  }

  void GCodeRuntimeState::dup() {
    this->stack.push(this->stack.top());
  }

  void GCodeRuntimeState::swap() {
    GCodeRuntimeValue upper = this->pop();
    GCodeRuntimeValue bottom = this->pop();
    this->push(upper);
    this->push(bottom);
  }

  void GCodeRuntimeState::negate() {
    GCodeRuntimeValue value = this->pop();
    if (value.is(GCodeRuntimeValue::Type::Integer)) {
      this->push(-value.getInteger());
    } else {
      this->push(-value.getFloat());
    }
  }

  static bool both_integers(const GCodeRuntimeValue &v1, const GCodeRuntimeValue &v2) {
    return v1.is(GCodeRuntimeValue::Type::Integer) && v2.is(GCodeRuntimeValue::Type::Integer);
  }

  void GCodeRuntimeState::add() {
    GCodeRuntimeValue v2 = this->pop();
    GCodeRuntimeValue v1 = this->pop();
    if (both_integers(v1, v2)) {
      this->push(v1.getInteger() + v2.getInteger());
    } else {
      this->push(v1.asFloat() + v2.asFloat());
    }
  }

  void GCodeRuntimeState::subtract() {
    GCodeRuntimeValue v2 = this->pop();
    GCodeRuntimeValue v1 = this->pop();
    if (both_integers(v1, v2)) {
      this->push(v1.getInteger() - v2.getInteger());
    } else {
      this->push(v1.asFloat() - v2.asFloat());
    }
  }

  void GCodeRuntimeState::multiply() {
    GCodeRuntimeValue v2 = this->pop();
    GCodeRuntimeValue v1 = this->pop();
    if (both_integers(v1, v2)) {
      this->push(v1.getInteger() * v2.getInteger());
    } else {
      this->push(v1.asFloat() * v2.asFloat());
    }
  }

  void GCodeRuntimeState::divide() {
    GCodeRuntimeValue v2 = this->pop();
    GCodeRuntimeValue v1 = this->pop();
    this->push(v1.asFloat() / v2.asFloat());
  }

  void GCodeRuntimeState::power() {
    GCodeRuntimeValue v2 = this->pop();
    GCodeRuntimeValue v1 = this->pop();
    this->push(pow(v1.asFloat(), v2.asFloat()));
  }

  void GCodeRuntimeState::modulo() {
    GCodeRuntimeValue v2 = this->pop();
    GCodeRuntimeValue v1 = this->pop();
    if (both_integers(v1, v2)) {
      this->push(v1.getInteger() % v2.getInteger());
    } else {
      this->push(fmod(v1.asFloat(), v2.asFloat()));
    }
  }

  void GCodeRuntimeState::compare() {
    GCodeRuntimeValue v2 = this->pop();
    GCodeRuntimeValue v1 = this->pop();
    int64_t result = 0;
    if (both_integers(v1, v2)) {
      int64_t i1 = v1.getInteger();
      int64_t i2 = v2.getInteger();
      if (i1 == i2) {
        result |= static_cast<int64_t>(GCodeCompare::Equals);
      } else {
        result |= static_cast<int64_t>(GCodeCompare::NotEquals);
        if (i1 > i2) {
          result |= static_cast<int64_t>(GCodeCompare::Greater);
        } else {
          result |= static_cast<int64_t>(GCodeCompare::Lesser);
        }
      }
    } else {
      double d1 = v1.asFloat();
      double d2 = v2.asFloat();
      if (d1 == d2) {
        result |= static_cast<int64_t>(GCodeCompare::Equals);
      } else {
        result |= static_cast<int64_t>(GCodeCompare::NotEquals);
        if (d1 > d2) {
          result |= static_cast<int64_t>(GCodeCompare::Greater);
        } else {
          result |= static_cast<int64_t>(GCodeCompare::Lesser);
        }
      }
    }
    this->push(result);
  }

  void GCodeRuntimeState::test(int64_t mask) {
    int64_t value = this->pop().asInteger();
    if ((value & mask) != 0) {
      this->push(GCodeTrue);
    } else {
      this->push(GCodeFalse);
    }
  }

  void GCodeRuntimeState::iand() {
    GCodeRuntimeValue v2 = this->pop();
    GCodeRuntimeValue v1 = this->pop();
    this->push(v1.asInteger() & v2.asInteger());
  }

  void GCodeRuntimeState::ior() {
    GCodeRuntimeValue v2 = this->pop();
    GCodeRuntimeValue v1 = this->pop();
    this->push(v1.asInteger() | v2.asInteger());
  }

  void GCodeRuntimeState::ixor() {
    GCodeRuntimeValue v2 = this->pop();
    GCodeRuntimeValue v1 = this->pop();
    this->push(v1.asInteger() ^ v2.asInteger());
  }
}