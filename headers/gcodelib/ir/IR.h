#ifndef GCODELIB_IR_IR_H_
#define GCODELIB_IR_IR_H_

#include "gcodelib/Base.h"
#include <variant>
#include <map>
#include <memory>
#include <vector>
#include <functional>
#include <optional>
#include <iosfwd>

namespace GCodeLib {

  class GCodeIRValue {
   public:
    enum class Type {
      None,
      Integer,
      Float
    };

    GCodeIRValue();
    GCodeIRValue(int64_t);
    GCodeIRValue(double);

    Type getType() const;
    bool is(Type) const;

    int64_t getInteger(int64_t = 0) const;
    double getFloat(double = 0.0) const;
    int64_t asInteger() const;
    double asFloat() const;

    friend std::ostream &operator<<(std::ostream &, const GCodeIRValue &);
   private:
    Type type;
    std::variant<int64_t, double> value;
  };

  enum class GCodeIROpcode {
    Push,
    Prologue,
    SetArg,
    Syscall
  };

  enum class GCodeSyscallType {
    General = 'G',
    Misc = 'O',
    FeedRate = 'F',
    SpindleSpeed = 'S',
    ToolSelection = 'T'
  };

  class GCodeIRInstruction {
   public:
    GCodeIRInstruction(GCodeIROpcode, const GCodeIRValue & = GCodeIRValue());

    GCodeIROpcode getOpcode() const;
    const GCodeIRValue &getValue() const;
   private:
    GCodeIROpcode opcode;
    GCodeIRValue value;
  };

  class GCodeIRModule {
   public:
    std::size_t length() const;
    const GCodeIRInstruction &at(std::size_t) const;

    void appendInstruction(GCodeIROpcode, const GCodeIRValue & = GCodeIRValue());
   private:
    std::vector<GCodeIRInstruction> code;
  };
}

#endif