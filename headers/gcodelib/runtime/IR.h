#ifndef GCODELIB_RUNTIME_IR_H_
#define GCODELIB_RUNTIME_IR_H_

#include "gcodelib/Base.h"
#include "gcodelib/runtime/Value.h"
#include <vector>
#include <map>

namespace GCodeLib {

  enum class GCodeIROpcode {
    Push,
    Prologue,
    SetArg,
    Syscall,
    Invoke,

    Negate,
    Add,
    Subtract,
    Multiply,
    Divide,
    Power,
    Modulo,
    Compare,
    Test,
    And,
    Or,
    Xor
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
    GCodeIRInstruction(GCodeIROpcode, const GCodeRuntimeValue & = GCodeRuntimeValue::Empty);

    GCodeIROpcode getOpcode() const;
    const GCodeRuntimeValue &getValue() const;
   private:
    GCodeIROpcode opcode;
    GCodeRuntimeValue value;
  };

  class GCodeIRModule {
   public:
    std::size_t length() const;
    const GCodeIRInstruction &at(std::size_t) const;

    std::size_t getSymbolId(const std::string &);
    const std::string &getSymbol(std::size_t) const;
    void appendInstruction(GCodeIROpcode, const GCodeRuntimeValue & = GCodeRuntimeValue::Empty);
   private:
    std::vector<GCodeIRInstruction> code;
    std::map<std::size_t, std::string> symbols;
    std::map<std::string, std::size_t> symbolIdentifiers;
  };
}

#endif