/*
  SPDX short identifier: MIT
  Copyright 2019 Jevgēnijs Protopopovs
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/

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