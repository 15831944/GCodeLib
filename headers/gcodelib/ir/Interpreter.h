#ifndef GCODELIB_IR_INTERPRETER_H_
#define GCODELIB_IR_INTERPRETER_H_

#include "gcodelib/ir/IR.h"
#include <stack>

namespace GCodeLib {

  struct GCodeRuntimeFrame {
    std::size_t pc;
    std::stack<GCodeIRValue> vstack;
  };

  class GCodeInterpreter {
   public:
    GCodeInterpreter(GCodeIRModule &);
    virtual ~GCodeInterpreter() = default;
    void execute();
   protected:
    GCodeRuntimeFrame &getFrame();

    virtual void syscall(GCodeSyscallType, const GCodeIRValue &, const std::map<unsigned char, GCodeIRValue> &) = 0;
   private:
    GCodeIRModule &module;
    bool work;
    std::stack<GCodeRuntimeFrame> stack;
  };
}

#endif