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

#ifndef GCODELIB_RUNTIME_RUNTIME_H_
#define GCODELIB_RUNTIME_RUNTIME_H_

#include "gcodelib/runtime/Storage.h"
#include <stack>
#include <map>
#include <functional>
#include <memory>

namespace GCodeLib::Runtime {

  class GCodeRuntimeState {
   public:
    GCodeRuntimeState(GCodeVariableScope &);

    std::size_t getPC() const;
    std::size_t nextPC();
    void jump(std::size_t);
    void call(std::size_t);
    void ret();

    void push(const GCodeRuntimeValue &);
    GCodeRuntimeValue pop();
    const GCodeRuntimeValue &peek();
    void dup();

    void negate();
    void add();
    void subtract();
    void multiply();
    void divide();
    void power();
    void modulo();
    void compare();
    void test(int64_t);
    void iand();
    void ior();
    void ixor();
    void inot();

    GCodeVariableScope &getScope();
   private:
    std::stack<GCodeRuntimeValue> stack;
    std::stack<std::size_t> call_stack;
    std::stack<std::unique_ptr<GCodeVariableScope>> scopes;
    GCodeVariableScope *globalScope;
    std::size_t pc;
  };

  class GCodeFunctionNamespace {
   public:
    virtual ~GCodeFunctionNamespace() = default;
    virtual GCodeRuntimeValue invoke(const std::string &, const std::vector<GCodeRuntimeValue> &) const = 0;
  };

  class GCodeFunctionScope : public GCodeFunctionNamespace {
   public:
    using FnType = std::function<GCodeRuntimeValue(const std::vector<GCodeRuntimeValue> &)>;
    bool hasFunction(const std::string &);
    void bindFunction(const std::string &, FnType);
    bool unbindFunction(const std::string &);
    void unbindAll();
    GCodeRuntimeValue invoke(const std::string &, const std::vector<GCodeRuntimeValue> &) const override;
   private:
    std::map<std::string, FnType> functions;
  };
}

#endif