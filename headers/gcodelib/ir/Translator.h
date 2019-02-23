#ifndef GCODELIB_IR_TRANSLATOR_H_
#define GCODELIB_IR_TRANSLATOR_H_

#include "gcodelib/ir/IR.h"
#include "gcodelib/parser/AST.h"

namespace GCodeLib {

  class GCodeIRTranslator {
   public:
    static std::unique_ptr<GCodeIRModule> translate(GCodeBlock &);
  };
}

#endif