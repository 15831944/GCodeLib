#ifndef GCODELIB_IR_TRANSLATOR_H_
#define GCODELIB_IR_TRANSLATOR_H_

#include "gcodelib/ir/IR.h"
#include "gcodelib/parser/AST.h"

namespace GCodeLib {

  class GCodeIRTranslator  {
   public:
    GCodeIRTranslator();
    std::unique_ptr<GCodeIRModule> translate(const GCodeBlock &);
   private:
    class Impl;
    std::shared_ptr<Impl> impl;
    // void translateNode(const GCodeNode &);
    // void _translate(const GCodeNode &);
    // void _translate(const GCodeBlock &);
    // void _translate(const GCodeCommand &);
    // std::unique_ptr<GCodeIRModule> module;
  };
}

#endif