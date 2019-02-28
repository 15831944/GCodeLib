#ifndef GCODELIB_PARSER_SCANNER_H_
#define GCODELIB_PARSER_SCANNER_H_

#include "gcodelib/Base.h"
#include <optional>

namespace GCodeLib::Parser {

  template <typename Token>
  class GCodeScanner {
   public:
    using TokenType = Token;
    virtual ~GCodeScanner() = default;
    virtual std::optional<Token> next() = 0;
    virtual bool finished() = 0;
  };
}

#endif