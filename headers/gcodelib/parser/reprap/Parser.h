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

#ifndef GCODELIB_PARSER_REPRAP_PARSER_H_
#define GCODELIB_PARSER_REPRAP_PARSER_H_

#include "gcodelib/parser/AST.h"
#include "gcodelib/parser/Mangling.h"
#include "gcodelib/parser/reprap/Scanner.h"
#include "gcodelib/parser/Parser.h"
#include <functional>
#include <set>
#include <memory>

namespace GCodeLib::Parser::RepRap {

  class GCodeRepRapMangler : public GCodeNameMangler {
   public:
    std::string getProcedureName(const std::string &) const override;
    std::string getStatementStart(const std::string &) const override;
    std::string getStatementEnd(const std::string &) const override;
    std::string getLoop(const std::string &) const override;
  };

  class GCodeFilteredScanner : public GCodeScanner<GCodeToken> {
   public:
    GCodeFilteredScanner(GCodeScanner<GCodeToken> &);

    std::optional<GCodeToken> next() override;
    bool finished() override;
   private:
    GCodeScanner<GCodeToken> &scanner;
  };

  class GCodeParser;

  class GCodeParser : public GCodeParserBase<GCodeParser, std::unique_ptr<GCodeFilteredScanner>, GCodeToken, 3> {
   public:
    GCodeParser(GCodeScanner<GCodeToken> &, GCodeNameMangler &);
    ~GCodeParser();
    std::unique_ptr<GCodeBlock> parse();
   private:
    bool expectToken(GCodeToken::Type, std::size_t = 0);
    bool expectOperator(GCodeOperator, std::size_t = 0);
    bool expectOperators(const std::set<GCodeOperator> &, std::size_t = 0);

    bool checkBlock();
    std::unique_ptr<GCodeBlock> nextBlock();
    bool checkStatement();
    std::unique_ptr<GCodeNode> nextStatement();
    bool checkChecksum();
    std::unique_ptr<GCodeNode> nextChecksum();
    bool checkCommand();
    std::unique_ptr<GCodeCommand> nextCommand();
    bool checkCommandWord();
    std::unique_ptr<GCodeWord> nextCommandWord();
    bool checkParameterWord();
    std::unique_ptr<GCodeWord> nextParameterWord();
    bool checkParameter();
    std::unique_ptr<GCodeNode> nextParameter();
    bool checkConstant();
    std::unique_ptr<GCodeConstantValue> nextConstant();
    bool checkString();
    std::unique_ptr<GCodeConstantValue> nextString();
  };
}

#endif