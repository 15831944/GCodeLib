#ifndef GCODELIB_RUNTIME_TRANSLATOR_H_
#define GCODELIB_RUNTIME_TRANSLATOR_H_

#include "gcodelib/runtime/IR.h"
#include "gcodelib/parser/AST.h"

namespace GCodeLib::Runtime {

  class GCodeIRTranslator  {
   public:
    GCodeIRTranslator();
    std::unique_ptr<GCodeIRModule> translate(const Parser::GCodeBlock &);
   private:
    class Impl;
    std::shared_ptr<Impl> impl;
  };
}

#endif