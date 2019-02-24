#ifndef GCODELIB_RUNTIME_VALUE_H_
#define GCODELIB_RUNTIME_VALUE_H_

#include "gcodelib/Base.h"
#include <variant>
#include <iosfwd>

namespace GCodeLib {

  class GCodeRuntimeValue {
   public:
    enum class Type {
      None,
      Integer,
      Float
    };

    GCodeRuntimeValue();
    GCodeRuntimeValue(int64_t);
    GCodeRuntimeValue(double);

    Type getType() const;
    bool is(Type) const;

    int64_t getInteger(int64_t = 0) const;
    double getFloat(double = 0.0) const;
    int64_t asInteger() const;
    double asFloat() const;

    friend std::ostream &operator<<(std::ostream &, const GCodeRuntimeValue &);

    static const GCodeRuntimeValue Empty;
   private:
    Type type;
    std::variant<int64_t, double> value;
  };
}

#endif