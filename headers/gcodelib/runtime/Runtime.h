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
    void swap();

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