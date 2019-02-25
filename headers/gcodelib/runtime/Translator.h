#ifndef GCODELIB_RUNTIME_TRANSLATOR_H_
#define GCODELIB_RUNTIME_TRANSLATOR_H_

#include "gcodelib/runtime/IR.h"
#include "gcodelib/parser/AST.h"

namespace GCodeLib {

  class GCodeIRTranslator  {
   public:
    GCodeIRTranslator();
    std::unique_ptr<GCodeIRModule> translate(const GCodeBlock &);
   private:
    class Impl;
    std::shared_ptr<Impl> impl;
  };
}

#endif