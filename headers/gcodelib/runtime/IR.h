#ifndef GCODELIB_RUNTIME_IR_H_
#define GCODELIB_RUNTIME_IR_H_

#include "gcodelib/Base.h"
#include "gcodelib/runtime/Value.h"
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <iosfwd>

namespace GCodeLib::Runtime {

  enum class GCodeIROpcode {
    // Syscall-related
    Prologue,
    SetArg,
    Syscall,
    // Flow control
    Invoke,
    Jump,
    JumpIf,
    Call,
    Ret,
    // Variables
    LoadNumbered,
    StoreNumbered,
    LoadNamed,
    StoreNamed,
    // Stack manipulation
    Push,
    Dup,
    // Arithmetical-logical operations
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
    Xor,
    Not
  };

  std::ostream &operator<<(std::ostream &, GCodeIROpcode);

  enum class GCodeSyscallType {
    General = 'G',
    Misc = 'O',
    FeedRate = 'F',
    SpindleSpeed = 'S',
    ToolSelection = 'T'
  };

  class GCodeIRModule; // Forward referencing

  class GCodeIRInstruction {
   public:
    GCodeIRInstruction(GCodeIROpcode, const GCodeRuntimeValue & = GCodeRuntimeValue::Empty);

    GCodeIROpcode getOpcode() const;
    const GCodeRuntimeValue &getValue() const;
    void setValue(const GCodeRuntimeValue &);

    void dump(std::ostream &, const GCodeIRModule &) const;
   private:
    GCodeIROpcode opcode;
    GCodeRuntimeValue value;
  };

  class GCodeIRLabel {
   public:
    GCodeIRLabel(GCodeIRModule &);
    void bind();
    bool bound() const;
    void jump();
    void jumpIf();
    void call();
    std::size_t getAddress() const;
   private:
    GCodeIRModule &module;
    std::optional<std::size_t> address;
    std::vector<std::size_t> patched;
  };

  class GCodeIRModule {
   public:
    std::size_t length() const;
    const GCodeIRInstruction &at(std::size_t) const;

    std::size_t getSymbolId(const std::string &);
    const std::string &getSymbol(std::size_t) const;
    std::unique_ptr<GCodeIRLabel> newLabel();
    GCodeIRLabel &getNamedLabel(const std::string &);
    void registerProcedure(int64_t, const std::string &);
    GCodeIRLabel &getProcedure(int64_t) const;
    void appendInstruction(GCodeIROpcode, const GCodeRuntimeValue & = GCodeRuntimeValue::Empty);
    bool linked() const;

    friend std::ostream &operator<<(std::ostream &, const GCodeIRModule &);
    friend class GCodeIRLabel;
   private:
    std::vector<GCodeIRInstruction> code;
    std::map<std::size_t, std::string> symbols;
    std::map<std::string, std::size_t> symbolIdentifiers;
    std::map<std::string, std::shared_ptr<GCodeIRLabel>> labels;
    std::map<int64_t, std::shared_ptr<GCodeIRLabel>> procedures;
  };
}

#endif