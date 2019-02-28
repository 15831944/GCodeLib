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

#include "gcodelib/parser/reprap/Parser.h"
#include "gcodelib/parser/Error.h"
#include <algorithm>

namespace GCodeLib::Parser::RepRap  {

  static const std::set<GCodeOperator> GCodeCommandOperators = {
    GCodeOperator::G,
    GCodeOperator::M,
    GCodeOperator::F,
    GCodeOperator::S,
    GCodeOperator::T
  };

  static const std::set<GCodeOperator> GCodeParameterLetters = {
    GCodeOperator::A,
    GCodeOperator::B,
    GCodeOperator::C,
    GCodeOperator::D,
    GCodeOperator::E,
    GCodeOperator::H,
    GCodeOperator::I,
    GCodeOperator::J,
    GCodeOperator::K,
    GCodeOperator::L,
    GCodeOperator::P,
    GCodeOperator::Q,
    GCodeOperator::R,
    GCodeOperator::U,
    GCodeOperator::V,
    GCodeOperator::W,
    GCodeOperator::X,
    GCodeOperator::Y,
    GCodeOperator::Z
  };

  GCodeFilteredScanner::GCodeFilteredScanner(GCodeScanner<GCodeToken> &scanner)
    : scanner(scanner) {}

  std::optional<GCodeToken> GCodeFilteredScanner::next() {
    std::optional<GCodeToken> token;
    while (!token.has_value() && !this->finished()) {
      token = this->scanner.next();
      if (!token.has_value()) {
        continue;
      }
      GCodeToken &tok = token.value();
      if (tok.is(GCodeToken::Type::Comment) ||
        (tok.is(GCodeToken::Type::Operator) &&
          (tok.getOperator() == GCodeOperator::Percent ||
          tok.getOperator() == GCodeOperator::None))) {
        token.reset();
      }
    }
    return token;
  }

  bool GCodeFilteredScanner::finished() {
    return this->scanner.finished();
  }

  GCodeParser::GCodeParser(GCodeScanner<GCodeToken> &scanner, GCodeNameMangler &mangler)
    : GCodeParserBase(std::make_unique<GCodeFilteredScanner>(scanner), mangler) {}
    
  GCodeParser::~GCodeParser() = default;

  std::unique_ptr<GCodeBlock> GCodeParser::parse() {
    return this->nextBlock();
  }

  bool GCodeParser::expectToken(GCodeToken::Type type, std::size_t idx) {
    return this->tokens[idx].has_value() && this->tokens[idx].value().is(type);
  }

  bool GCodeParser::expectOperator(GCodeOperator oper, std::size_t idx) {
    return this->expectToken(GCodeToken::Type::Operator, idx) &&
      this->tokenAt(idx).getOperator() == oper;
  }

  bool GCodeParser::expectOperators(const std::set<GCodeOperator> &opers, std::size_t idx) {
    return this->expectToken(GCodeToken::Type::Operator, idx) &&
      opers.count(this->tokenAt(idx).getOperator()) != 0;
  }

  bool GCodeParser::checkBlock() {
    return this->checkStatement();
  }

  std::unique_ptr<GCodeBlock> GCodeParser::nextBlock() {
    auto position = this->position();
    std::vector<std::unique_ptr<GCodeNode>> block;
    while (this->checkStatement()) {
      auto stmt = this->nextStatement();
      if (stmt) {
        block.push_back(std::move(stmt));
      } else {
        break;
      }
    }
    return std::make_unique<GCodeBlock>(std::move(block), position.value());
  }
   
  bool GCodeParser::checkStatement() {
    return this->checkCommand() ||
      this->checkChecksum() ||
      this->expectToken(GCodeToken::Type::NewLine);
  }

  std::unique_ptr<GCodeNode> GCodeParser::nextStatement() {
    if (this->expectToken(GCodeToken::Type::NewLine)) {
      auto position = this->position();
      this->shift();
      if (this->checkStatement()) {
        return this->nextStatement();
      }
      return nullptr;
    }
    if (this->checkChecksum()) {
      return this->nextChecksum();
    } else {
      return this->nextCommand();
    }
  }

  bool GCodeParser::checkChecksum() {
    return this->expectOperator(GCodeOperator::Star) &&
      this->expectToken(GCodeToken::Type::IntegerContant, 1) &&
      this->expectToken(GCodeToken::Type::NewLine, 2);
  }

  std::unique_ptr<GCodeNode> GCodeParser::nextChecksum() {
    auto position = this->position();
    this->assert(&GCodeParser::checkChecksum, "Expected checksum");
    int64_t checksum = this->tokenAt(1).getInteger();
    int64_t real_checksum = position.value().getChecksum();
    if (checksum != real_checksum) {
      this->error("Wrong checksum; expected " + std::to_string(checksum) + ", got " + std::to_string(real_checksum));
    }
    this->shift();
    this->shift();
    return std::make_unique<GCodeNoOperation>(position.value());
  }

  bool GCodeParser::checkCommand() {
    return this->checkCommandWord();
  }

  std::unique_ptr<GCodeCommand> GCodeParser::nextCommand() {
    auto position = this->position();
    auto command = this->nextCommandWord();
    std::vector<std::unique_ptr<GCodeWord>> parameters;
    while (this->checkParameterWord()) {
      parameters.push_back(this->nextParameterWord());
    }
    return std::make_unique<GCodeCommand>(std::move(command), std::move(parameters), position.value());
  }

  bool GCodeParser::checkCommandWord() {
    return this->expectOperators(GCodeCommandOperators);
  }

  std::unique_ptr<GCodeWord> GCodeParser::nextCommandWord() {
    auto position = this->position();
    this->assert(&GCodeParser::checkCommandWord, "Command expected");
    unsigned char field = static_cast<unsigned char>(this->tokenAt().getOperator());
    this->shift();
    std::unique_ptr<GCodeConstantValue> value = this->nextConstant();
    return std::make_unique<GCodeWord>(field, std::move(value), position.value());
  }

  bool GCodeParser::checkParameterWord() {
    return this->expectOperators(GCodeParameterLetters);
  }

  std::unique_ptr<GCodeWord> GCodeParser::nextParameterWord() {
    auto position = this->position();
    this->assert(&GCodeParser::checkParameterWord, "Parameter expected");
    unsigned char field = static_cast<unsigned char>(this->tokenAt().getOperator());
    this->shift();
    std::unique_ptr<GCodeNode> value = this->nextParameter();
    return std::make_unique<GCodeWord>(field, std::move(value), position.value());
  }

  bool GCodeParser::checkParameter() {
    return this->checkConstant();
  }

  std::unique_ptr<GCodeNode> GCodeParser::nextParameter() {
    this->assert(&GCodeParser::checkParameter, "Expression or constant expected");
    return this->nextConstant();
  }

  bool GCodeParser::checkConstant() {
    return this->expectToken(GCodeToken::Type::IntegerContant) ||
      this->expectToken(GCodeToken::Type::FloatConstant) ||
      this->checkString();
  }

  std::unique_ptr<GCodeConstantValue> GCodeParser::nextConstant() {
    auto position = this->position();
    std::unique_ptr<GCodeConstantValue> res = nullptr;
    if (this->expectToken(GCodeToken::Type::IntegerContant)) {
      res = std::make_unique<GCodeConstantValue>(this->tokenAt().getInteger(), position.value());
    } else if (this->expectToken(GCodeToken::Type::FloatConstant)) {
      res = std::make_unique<GCodeConstantValue>(this->tokenAt().getFloat(), position.value());
    } else if (this->checkString()) {
      return this->nextString();
    } else {
      this->error("Constant expected");
    }
    this->shift();
    return res;
  }

  bool GCodeParser::checkString() {
    return this->expectToken(GCodeToken::Type::StringConstant);
  }

  std::unique_ptr<GCodeConstantValue> GCodeParser::nextString() {
    auto position = this->position();
    this->assert(&GCodeParser::checkString, "String constant expected");
    std::string str = this->tokenAt().getString();
    this->shift();
    while (this->expectToken(GCodeToken::Type::StringConstant)) {
      str += "\"" + this->tokenAt().getString();
      this->shift();
    }
    return std::make_unique<GCodeConstantValue>(str, position.value());
  }
}