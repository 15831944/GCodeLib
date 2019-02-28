#ifndef GCODELIB_PARSER_ERROR_H_
#define GCODELIB_PARSER_ERROR_H_

#include "gcodelib/Error.h"

namespace GCodeLib::Parser {

  class GCodeParseException : public GCodeLibException {
   public:
    using GCodeLibException::GCodeLibException;
  };
}

#endif