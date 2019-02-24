#include "gcodelib/runtime/Runtime.h"
#include "gcodelib/runtime/Error.h"

namespace GCodeLib {

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
}