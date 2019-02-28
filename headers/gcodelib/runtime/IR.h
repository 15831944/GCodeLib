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

#ifndef GCODELIB_RUNTIME_IR_H_
#define GCODELIB_RUNTIME_IR_H_

#include "gcodelib/Base.h"
#include "gcodelib/runtime/Value.h"
#include "gcodelib/runtime/SourceMap.h"
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

  class GCodeIRPosition;

  class GCodeIRModule {
   public:
    std::size_t length() const;
    const GCodeIRInstruction &at(std::size_t) const;

    std::unique_ptr<GCodeIRPosition> newPositionRegister(const Parser::SourcePosition &);
    IRSourceMap &getSourceMap();

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
    IRSourceMap sourceMap;
  };

  class GCodeIRPosition {
   public:
    GCodeIRPosition(GCodeIRModule &, const Parser::SourcePosition &);
    ~GCodeIRPosition();
    GCodeIRPosition(const GCodeIRPosition &) = delete;
    GCodeIRPosition &operator=(const GCodeIRPosition &) = delete;
   private:
    GCodeIRModule &module;
    const Parser::SourcePosition &position;
    std::size_t start_address;
  };
}

#endif