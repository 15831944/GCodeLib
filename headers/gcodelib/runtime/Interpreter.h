#ifndef GCODELIB_RUNTIME_INTERPRETER_H_
#define GCODELIB_RUNTIME_INTERPRETER_H_

#include "gcodelib/runtime/IR.h"
#include "gcodelib/runtime/Runtime.h"
#include <stack>
#include <map>

namespace GCodeLib::Runtime {

  class GCodeInterpreter {
   public:
    GCodeInterpreter(GCodeIRModule &);
    virtual ~GCodeInterpreter() = default;
    void execute();
   protected:
    GCodeFunctionScope &getFunctions();
    GCodeRuntimeState &getState();
    void stop();

    virtual void syscall(GCodeSyscallType, const GCodeRuntimeValue &, const GCodeScopedDictionary<unsigned char> &) = 0;
    virtual GCodeVariableScope &getSystemScope() = 0;
   private:
    GCodeIRModule &module;
    std::optional<GCodeRuntimeState> state;
    GCodeFunctionScope functions;
  };
}

#endif