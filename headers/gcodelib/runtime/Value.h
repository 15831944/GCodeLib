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