#ifndef GCODELIB_RUNTIME_VALUE_H_
#define GCODELIB_RUNTIME_VALUE_H_

#include "gcodelib/Base.h"
#include <variant>
#include <string>
#include <iosfwd>

namespace GCodeLib::Runtime {

  class GCodeRuntimeValue {
   public:
    enum class Type {
      None,
      Integer,
      Float,
      String
    };

    GCodeRuntimeValue();
    GCodeRuntimeValue(int64_t);
    GCodeRuntimeValue(double);
    GCodeRuntimeValue(const std::string &);
    GCodeRuntimeValue(const GCodeRuntimeValue &);
    GCodeRuntimeValue &operator=(const GCodeRuntimeValue &);

    Type getType() const;
    bool is(Type) const;

    int64_t getInteger(int64_t = 0) const;
    double getFloat(double = 0.0) const;
    const std::string &getString(const std::string & = "") const;
    int64_t asInteger() const;
    double asFloat() const;
    std::string asString() const;

    bool isNumeric() const;
    const GCodeRuntimeValue &assertType(Type) const;
    const GCodeRuntimeValue &assertNumeric() const;

    friend std::ostream &operator<<(std::ostream &, const GCodeRuntimeValue &);

    static const GCodeRuntimeValue Empty;
   private:
    Type type;
    std::variant<int64_t, double, std::string> value;
  };

  enum class GCodeCompare {
    Equals = 1,
    NotEquals = 1 << 1,
    Greater = 1 << 2,
    Lesser = 1 << 3
  };
}

#endif