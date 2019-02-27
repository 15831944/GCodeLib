#ifndef GCODELIB_PARSER_PARSER_H_
#define GCODELIB_PARSER_PARSER_H_

#include "gcodelib/Base.h"
#include "gcodelib/parser/Mangling.h"
#include "gcodelib/parser/Error.h"

namespace GCodeLib::Parser {

  template <class Self, typename Scanner, typename TokenType, std::size_t Lookup>
  class GCodeParserBase {
   public:
    GCodeParserBase(Scanner scanner, GCodeNameMangler &mangler)
      : scanner(std::move(scanner)), mangler(mangler) {
      for (std::size_t i = 0; i < Lookup; i++) {
        this->tokens[i] = this->scanner->next();
      }
    }
   protected:
    [[noreturn]]
    void error(const std::string &msg) {
      if (this->tokens[0].has_value()) {
        throw GCodeParseException(msg, this->tokens[0].value().getPosition());
      } else {
        throw GCodeParseException(msg);
      }
    }

    void shift(std::size_t count = 1) {
      while (count--) {
        for (std::size_t i = 0; i < Lookup - 1; i++) {
          this->tokens[i] = std::move(this->tokens[i + 1]);
        }
        this->tokens[Lookup - 1] = this->scanner->next();
      }
    }

    std::optional<SourcePosition> position() {
      if (this->tokens[0].has_value()) {
        return this->tokens[0].value().getPosition();
      } else {
        return std::optional<SourcePosition>();
      }
    }

    TokenType tokenAt(std::size_t idx = 0) {
      if (this->tokens[idx].has_value()) {
        return this->tokens[idx].value();
      } else {
        throw GCodeParseException("Expected token");
      }
    }

    void assert(bool (Self::*assertion)(), const std::string &message) {
      if (!(static_cast<Self *>(this)->*assertion)()) {
        this->error(message);
      }
    }

    Scanner scanner;
    std::optional<TokenType> tokens[Lookup];
    GCodeNameMangler &mangler;
  };
}

#endif