#ifndef GCODELIB_RUNTIME_STORAGE_H_
#define GCODELIB_RUNTIME_STORAGE_H_

#include "gcodelib/runtime/Value.h"
#include <map>

namespace GCodeLib {

  template <typename T>
  class GCodeDictionary {
   public:
    virtual ~GCodeDictionary() = default;
    virtual bool has(const T&) const = 0;
    virtual const GCodeRuntimeValue &get(const T&) const = 0;
    virtual bool put(const T&, const GCodeRuntimeValue &) = 0;
    virtual bool remove(const T&) = 0;
    virtual void clear() = 0;
  };

  template <typename T>
  class GCodeScopedDictionary : public GCodeDictionary<T> {
   public:
    GCodeScopedDictionary(GCodeDictionary<T> *parent = nullptr)
      : parent(parent) {}

    GCodeDictionary<T> *getParent() {
      return this->parent;
    }

    const GCodeDictionary<T> *getParent() const {
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
    GCodeDictionary<T> *parent;
    std::map<T, GCodeRuntimeValue> scope;
  };

  class GCodeVariableScope {
   public:
    virtual ~GCodeVariableScope() = default;
    virtual GCodeDictionary<int64_t> &getNumbered() = 0;
    virtual GCodeDictionary<std::string> &getNamed() = 0;
  };

  class GCodeCascadeVariableScope : public GCodeVariableScope {
   public:
    GCodeCascadeVariableScope(GCodeVariableScope * = nullptr);
    GCodeDictionary<int64_t> &getNumbered() override;
    GCodeDictionary<std::string> &getNamed() override;
   private:
    GCodeScopedDictionary<int64_t> numbered;
    GCodeScopedDictionary<std::string> named;
  };
}

#endif