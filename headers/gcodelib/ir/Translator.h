#ifndef GCODELIB_IR_TRANSLATOR_H_
#define GCODELIB_IR_TRANSLATOR_H_

#include "gcodelib/ir/IR.h"
#include "gcodelib/parser/AST.h"

namespace GCodeLib {

  class GCodeIRTranslator {
   public:
    std::unique_ptr<GCodeIRModule> translate(const GCodeBlock &);
   private:
    void translateNode(const GCodeNode &);
    void translateBlock(const GCodeBlock &);
    void translateCommand(const GCodeCommand &);
    std::unique_ptr<GCodeIRModule> module;
  };
}

#endif