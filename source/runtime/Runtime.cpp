#include "gcodelib/runtime/Runtime.h"

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
    GCodeRuntimeValue value = this->stack.top();
    this->stack.pop();
    return value;
  }

  const GCodeRuntimeValue &GCodeRuntimeState::peek() {
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
}