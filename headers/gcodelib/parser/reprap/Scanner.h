#ifndef GCODELIB_PARSER_REPRAP_SCANNER_H_
#define GCODELIB_PARSER_REPRAP_SCANNER_H_

#include "gcodelib/Base.h"
#include "gcodelib/parser/Scanner.h"
#include "gcodelib/parser/reprap/Token.h"
#include <iosfwd>
#include <string>
#include <optional>

namespace GCodeLib::Parser::RepRap {

  class GCodeDefaultScanner : public GCodeScanner<GCodeToken> {
   public:
    GCodeDefaultScanner(std::istream &, const std::string &tag = "");
    std::optional<GCodeToken> next() override;
    bool finished() override;
   private:
    void next_line();
    void shift(std::size_t);
    void skipWhitespaces();

    std::istream &input;
    std::string buffer;
    SourcePosition source_position;
    unsigned int options;
  };
}

#endif