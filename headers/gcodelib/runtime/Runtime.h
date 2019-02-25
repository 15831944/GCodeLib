#ifndef GCODELIB_RUNTIME_RUNTIME_H_
#define GCODELIB_RUNTIME_RUNTIME_H_

#include "gcodelib/runtime/Value.h"
#include <stack>
#include <map>
#include <functional>

namespace GCodeLib {

  class GCodeRuntimeState {
   public:
    GCodeRuntimeState();

    std::size_t getPC() const;
    std::size_t nextPC();
    void jump(std::size_t);

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
   private:
    std::stack<GCodeRuntimeValue> stack;
    std::size_t pc;
  };

  template <typename T>
  class GCodeVariableNamespace {
   public:
    virtual ~GCodeVariableNamespace() = default;
    virtual bool has(const T&) const = 0;
    virtual const GCodeRuntimeValue &get(const T&) const = 0;
    virtual bool put(const T&, const GCodeRuntimeValue &) = 0;
    virtual bool remove(const T&) = 0;
    virtual void clear() = 0;
  };

  template <typename T>
  class GCodeVariableScope : public GCodeVariableNamespace<T> {
   public:
    GCodeVariableScope(GCodeVariableNamespace<T> *parent = nullptr)
      : parent(parent) {}

    GCodeVariableNamespace<T> *getParent() {
      return this->parent;
    }

    const GCodeVariableNamespace<T> *getParent() const {
      return this->parent;
    }
    
    bool has(const T &key) const override {
      return this->scope.count(key) != 0 ||
        (this->parent != nullptr && this->parent->has(key));
    }

    bool hasOwn(const T &key) const {
      return this->scope.count(key) != 0;
    }

    const GCodeRuntimeValue &get(const T &key) const override {
      if (this->scope.count(key) != 0) {
        return this->scope.at(key);
      } else if (this->parent != nullptr) {
        return this->parent->get(key);
      } else {
        return GCodeRuntimeValue::Empty;
      }
    }
    
    bool put(const T &key, const GCodeRuntimeValue &value) override {
      if (this->scope.count(key) != 0 ||
        this->parent == nullptr ||
        !this->parent->has(key)) {
        this->scope[key] = value;
        return true;
      } else {
        this->parent->put(key, value);
        return false;
      }
    }

    bool remove(const T &key) override {
      if (this->scope.count(key) != 0) {
        this->scope.erase(key);
        return true;
      } else if (this->parent != nullptr) {
        return this->parent->remove(key);
      } else {
        return false;
      }
    }

    void clear() override {
      this->scope.clear();
    }

    typename std::map<T, GCodeRuntimeValue>::const_iterator begin() const {
      return this->scope.begin();
    }

    typename std::map<T, GCodeRuntimeValue>::const_iterator end() const {
      return this->scope.end();
    }
   private:
    GCodeVariableNamespace<T> *parent;
    std::map<T, GCodeRuntimeValue> scope;
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