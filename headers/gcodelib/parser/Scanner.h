#ifndef GCODELIB_PARSER_SCANNER_H_
#define GCODELIB_PARSER_SCANNER_H_

#include "gcodelib/Base.h"
#include "gcodelib/parser/Token.h"
#include <iosfwd>
#include <string>
#include <optional>

namespace GCodeLib {

  class GCodeScanner {
   public:
    virtual ~GCodeScanner() = default;
    virtual std::optional<GCodeToken> next() = 0;
    virtual bool finished() = 0;
  };

  class GCodeDefaultScanner : public GCodeScanner {
   public:
    GCodeDefaultScanner(std::istream &, unsigned int = GCodeDefaultScanner::DefaultOptions);
    std::optional<GCodeToken> next() override;
    bool finished() override;

    static constexpr unsigned int FilterEnd = 1;
    static constexpr unsigned int FilterComments = 1 << 1;
    static constexpr unsigned int DefaultOptions = 0;
   private:
    std::optional<GCodeToken> raw_next();
    void next_line();
    void shift(std::size_t);
    void skipWhitespaces();

    std::istream &input;
    std::string buffer;
    GCodeToken::Position source_position;
    unsigned int options;
  };
}

#endif