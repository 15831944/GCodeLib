#ifndef GCODELIB_IR_INTERPRETER_H_
#define GCODELIB_IR_INTERPRETER_H_

#include "gcodelib/ir/IR.h"

namespace GCodeLib {

  class GCodeInterpreter {
   public:
    virtual ~GCodeInterpreter() = default;
    void execute(GCodeIRModule &);
   protected:
    virtual void execute(GCodeIRInstruction &) = 0;
  };
}

#endif