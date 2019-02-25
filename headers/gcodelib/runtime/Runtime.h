#ifndef GCODELIB_RUNTIME_RUNTIME_H_
#define GCODELIB_RUNTIME_RUNTIME_H_

#include "gcodelib/runtime/Value.h"
#include <stack>
#include <map>

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
  class GCodeVariableScope {
   public:
    GCodeVariableScope(GCodeVariableScope<T> *parent = nullptr)
      : parent(parent) {}

    GCodeVariableScope<T> *getParent() {
      return this->parent;
    }

    const GCodeVariableScope<T> *getParent() const {
      return this->parent;
    }
    
    bool has(const T &key) const {
      return this->scope.count(key) != 0 ||
        (this->parent != nullptr && this->parent->has(key));
    }

    bool hasOwn(const T &key) const {
      return this->scope.count(key) != 0;
    }

    const GCodeRuntimeValue &get(const T &key) const {
      if (this->scope.count(key) != 0) {
        return this->scope.at(key);
      } else if (this->parent != nullptr) {
        return this->parent->get(key);
      } else {
        return GCodeRuntimeValue::Empty;
      }
    }
    
    bool put(const T &key, const GCodeRuntimeValue &value) {
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

    bool remove(const T &key) {
      if (this->scope.count(key) != 0) {
        this->scope.erase(key);
        return true;
      } else if (this->parent != nullptr) {
        return this->parent->remove(key);
      } else {
        return false;
      }
    }

    void clear() {
      this->scope.clear();
    }

    typename std::map<T, GCodeRuntimeValue>::const_iterator begin() const {
      return this->scope.begin();
    }

    typename std::map<T, GCodeRuntimeValue>::const_iterator end() const {
      return this->scope.end();
    }
   private:
    GCodeVariableScope<T> *parent;
    std::map<T, GCodeRuntimeValue> scope;
  };
}

#endif