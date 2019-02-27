#ifndef GCODELIB_RUNTIME_STORAGE_H_
#define GCODELIB_RUNTIME_STORAGE_H_

#include "gcodelib/runtime/Value.h"
#include <map>
#include <functional>

namespace GCodeLib::Runtime {

  template <typename T>
  class GCodeDictionary {
   public:
    virtual ~GCodeDictionary() = default;
    virtual bool has(const T &) const = 0;
    virtual GCodeRuntimeValue get(const T &) const = 0;
    virtual bool put(const T &, const GCodeRuntimeValue &) = 0;
    virtual bool remove(const T &) = 0;
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

    GCodeRuntimeValue get(const T &key) const override {
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

  template <typename T>
  class GCodeVirtualDictionary : public GCodeDictionary<T> {
   public:
    using Supplier = std::function<GCodeRuntimeValue()>;
    bool has(const T &key) const override {
      return this->dictionary.count(key) != 0;
    }

    GCodeRuntimeValue get(const T &key) const override {
      if (this->has(key)) {
        return this->dictionary.at(key)();
      } else {
        return GCodeRuntimeValue::Empty;
      }
    }

    bool put(const T &key, const GCodeRuntimeValue &value) override {
      return false;
    }

    bool remove(const T &key) override {
      return false;
    }

    void clear() override {}

    void putSupplier(const T &key, Supplier sup) {
      this->dictionary[key] = sup;
    }

    bool hasSupplier(const T &key) const {
      return this->dictionary.count(key) != 0;
    }

    void removeSupplier(const T &key) {
      if (this->hasSupplier(key)) {
        this->dictionary.erase(key);
      }
    }

    void clearSuppliers() {
      this->dictionary.clear();
    }
   private:
    std::map<T, Supplier> dictionary;
  };

  template <typename T>
  class GCodeDictionaryMultiplexer : public GCodeDictionary<T> {
   public:
    GCodeDictionaryMultiplexer(GCodeDictionary<T> &master, GCodeDictionary<T> *slave = nullptr)
      : master(master), slave(slave) {}

    bool has(const T &key) const override {
      return this->master.has(key) ||
        (this->slave != nullptr && this->slave->has(key));
    }

    GCodeRuntimeValue get(const T &key) const override {
      if (this->master.has(key)) {
        return this->master.get(key);
      } else if (this->slave) {
        return this->slave->get(key);
      } else {
        return GCodeRuntimeValue::Empty;
      }
    }

    bool put(const T &key, const GCodeRuntimeValue &value) override {
      return this->master.put(key, value);
    }

    bool remove(const T &key) override {
      return this->master.remove(key);
    }

    void clear() override {
      this->master.clear();
    }
   private:
    GCodeDictionary<T> &master;
    GCodeDictionary<T> *slave;
  };

  class GCodeVariableScope {
   public:
    virtual ~GCodeVariableScope() = default;
    virtual GCodeDictionary<int64_t> &getNumbered() = 0;
    virtual GCodeDictionary<std::string> &getNamed() = 0;
  };

  class GCodeCustomVariableScope : public GCodeVariableScope {
   public:
    GCodeCustomVariableScope(GCodeDictionary<int64_t> &, GCodeDictionary<std::string> &);
    GCodeDictionary<int64_t> &getNumbered() override;
    GCodeDictionary<std::string> &getNamed() override;
   private:
    GCodeDictionary<int64_t> &numbered;
    GCodeDictionary<std::string> &named;
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