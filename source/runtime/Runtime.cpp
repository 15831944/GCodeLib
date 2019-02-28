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

#include "gcodelib/runtime/Runtime.h"
#include "gcodelib/runtime/Error.h"
#include <cmath>

namespace GCodeLib::Runtime {

  static const int64_t GCodeTrue = 1;
  static const int64_t GCodeFalse = 0;

  static bool both_integers(const GCodeRuntimeValue &v1, const GCodeRuntimeValue &v2) {
    return v1.is(GCodeRuntimeValue::Type::Integer) && v2.is(GCodeRuntimeValue::Type::Integer);
  }

  template <typename ... T>
  struct AssertNumericImpl {};

  template <>
  struct AssertNumericImpl<> {
    static void assert() {}
  };

  template <typename A, typename ... B>
  struct AssertNumericImpl<A, B...> {
    static void assert(A &arg, B &... args) {
      arg.assertNumeric();
      AssertNumericImpl<B...>::assert(args...);
    }
  };

  template <typename ... T>
  static void assert_numeric(T &... args) {
    AssertNumericImpl<T...>::assert(args...);
  }

  GCodeRuntimeState::GCodeRuntimeState(GCodeVariableScope &system)
    : pc(0) {
    this->scopes.push(std::make_unique<GCodeCascadeVariableScope>(&system));
    this->globalScope = this->scopes.top().get();
  }

  GCodeVariableScope &GCodeRuntimeState::getScope() {
    return *this->scopes.top();
  }

  std::size_t GCodeRuntimeState::getPC() const {
    return this->pc;
  }

  std::size_t GCodeRuntimeState::nextPC() {
    return this->pc++;
  }

  void GCodeRuntimeState::jump(std::size_t pc) {
    this->pc = pc;
  }

  void GCodeRuntimeState::call(std::size_t pc) {
    this->call_stack.push(this->pc);
    this->scopes.push(std::make_unique<GCodeCascadeVariableScope>(this->globalScope));
    this->pc = pc;
  }

  void GCodeRuntimeState::ret() {
    if (this->call_stack.empty()) {
      throw GCodeRuntimeError("Call stack undeflow");
    }
    this->pc = this->call_stack.top();
    this->call_stack.pop();
    this->scopes.pop();
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
    assert_numeric(value);
    if (value.is(GCodeRuntimeValue::Type::Integer)) {
      this->push(-value.getInteger());
    } else {
      this->push(-value.getFloat());
    }
  }

  void GCodeRuntimeState::add() {
    GCodeRuntimeValue v2 = this->pop();
    GCodeRuntimeValue v1 = this->pop();
    assert_numeric(v1, v2);
    if (both_integers(v1, v2)) {
      this->push(v1.getInteger() + v2.getInteger());
    } else {
      this->push(v1.asFloat() + v2.asFloat());
    }
  }

  void GCodeRuntimeState::subtract() {
    GCodeRuntimeValue v2 = this->pop();
    GCodeRuntimeValue v1 = this->pop();
    assert_numeric(v1, v2);
    if (both_integers(v1, v2)) {
      this->push(v1.getInteger() - v2.getInteger());
    } else {
      this->push(v1.asFloat() - v2.asFloat());
    }
  }

  void GCodeRuntimeState::multiply() {
    GCodeRuntimeValue v2 = this->pop();
    GCodeRuntimeValue v1 = this->pop();
    assert_numeric(v1, v2);
    if (both_integers(v1, v2)) {
      this->push(v1.getInteger() * v2.getInteger());
    } else {
      this->push(v1.asFloat() * v2.asFloat());
    }
  }

  void GCodeRuntimeState::divide() {
    GCodeRuntimeValue v2 = this->pop();
    GCodeRuntimeValue v1 = this->pop();
    assert_numeric(v1, v2);
    this->push(v1.asFloat() / v2.asFloat());
  }

  void GCodeRuntimeState::power() {
    GCodeRuntimeValue v2 = this->pop();
    GCodeRuntimeValue v1 = this->pop();
    assert_numeric(v1, v2);
    this->push(pow(v1.asFloat(), v2.asFloat()));
  }

  void GCodeRuntimeState::modulo() {
    GCodeRuntimeValue v2 = this->pop();
    GCodeRuntimeValue v1 = this->pop();
    assert_numeric(v1, v2);
    if (both_integers(v1, v2)) {
      this->push(v1.getInteger() % v2.getInteger());
    } else {
      this->push(fmod(v1.asFloat(), v2.asFloat()));
    }
  }

  void GCodeRuntimeState::compare() {
    GCodeRuntimeValue v2 = this->pop();
    GCodeRuntimeValue v1 = this->pop();
    assert_numeric(v1, v2);
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
    GCodeRuntimeValue rtvalue = this->pop();
    assert_numeric(rtvalue);
    int64_t value = rtvalue.asInteger();
    if ((value & mask) != 0) {
      this->push(GCodeTrue);
    } else {
      this->push(GCodeFalse);
    }
  }

  void GCodeRuntimeState::iand() {
    GCodeRuntimeValue v2 = this->pop();
    GCodeRuntimeValue v1 = this->pop();
    assert_numeric(v1, v2);
    this->push(v1.asInteger() & v2.asInteger());
  }

  void GCodeRuntimeState::ior() {
    GCodeRuntimeValue v2 = this->pop();
    GCodeRuntimeValue v1 = this->pop();
    assert_numeric(v1, v2);
    this->push(v1.asInteger() | v2.asInteger());
  }

  void GCodeRuntimeState::ixor() {
    GCodeRuntimeValue v2 = this->pop();
    GCodeRuntimeValue v1 = this->pop();
    assert_numeric(v1, v2);
    this->push(v1.asInteger() ^ v2.asInteger());
  }

  void GCodeRuntimeState::inot() {
    GCodeRuntimeValue v = this->pop();
    assert_numeric(v);
    if (v.asInteger() != 0) {
      this->push(GCodeFalse);
    } else {
      this->push(GCodeTrue);
    }
  }

  bool GCodeFunctionScope::hasFunction(const std::string &key) {
    return this->functions.count(key) != 0;
  }

  void GCodeFunctionScope::bindFunction(const std::string &key, FnType impl) {
    this->functions[key] = std::move(impl);
  }

  bool GCodeFunctionScope::unbindFunction(const std::string &key) {
    if (this->functions.count(key) != 0) {
      this->functions.erase(key);
      return true;
    } else {
      return false;
    }
  }

  void GCodeFunctionScope::unbindAll() {
    this->functions.clear();
  }

  GCodeRuntimeValue GCodeFunctionScope::invoke(const std::string &key, const std::vector<GCodeRuntimeValue> &args) const {
    if (this->functions.count(key) != 0) {
      return this->functions.at(key)(args);
    } else {
      return GCodeRuntimeValue::Empty;
    }
  }
}