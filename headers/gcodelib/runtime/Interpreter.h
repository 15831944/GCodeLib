#ifndef GCODELIB_RUNTIME_INTERPRETER_H_
#define GCODELIB_RUNTIME_INTERPRETER_H_

#include "gcodelib/runtime/IR.h"
#include "gcodelib/runtime/Runtime.h"
#include <stack>
#include <map>

namespace GCodeLib {

  class GCodeInterpreter {
   public:
    GCodeInterpreter(GCodeIRModule &);
    virtual ~GCodeInterpreter() = default;
    void execute();
   protected:
    GCodeRuntimeState &getFrame();

    virtual void syscall(GCodeSyscallType, const GCodeRuntimeValue &, const GCodeScopedDictionary<unsigned char> &) = 0;

    GCodeFunctionScope functions;
   private:
    GCodeIRModule &module;
    bool work;
    std::stack<GCodeRuntimeState> stack;
  };
}

#endif