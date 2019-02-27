#ifndef GCODELIB_PARSER_LINUXCNC_SCANNER_H_
#define GCODELIB_PARSER_LINUXCNC_SCANNER_H_

#include "gcodelib/Base.h"
#include "gcodelib/parser/linuxcnc/Token.h"
#include <iosfwd>
#include <string>
#include <optional>

namespace GCodeLib::Parser::LinuxCNC {

  class GCodeScanner {
   public:
    virtual ~GCodeScanner() = default;
    virtual std::optional<GCodeToken> next() = 0;
    virtual bool finished() = 0;
  };

  class GCodeDefaultScanner : public GCodeScanner {
   public:
    GCodeDefaultScanner(std::istream &);
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