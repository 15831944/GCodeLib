#ifndef GCODELIB_PARSER_GCODELIB_VALIDATOR_H_
#define GCODELIB_PARSER_GCODELIB_VALIDATOR_H_

#include "gcodelib/parser/AST.h"

namespace GCodeLib::Parser::LinuxCNC {

  class GCodeLCNCValidator {
   public:
    GCodeLCNCValidator();
    ~GCodeLCNCValidator();
    void validate(const GCodeNode &);
   private:
    class Impl;
    std::unique_ptr<Impl> impl;
  };
}

#endif