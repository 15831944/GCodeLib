#include "gcodelib/parser/Parser.h"
#include "gcodelib/parser/Error.h"

namespace GCodeLib  {

  static const std::set<GCodeOperator> GCodeOperators = {
    GCodeOperator::G,
    GCodeOperator::M,
    GCodeOperator::Percent,
    GCodeOperator::Star
  };

  static const std::set<GCodeOperator> GCodeCommandOperators = {
    GCodeOperator::G,
    GCodeOperator::M
  };

  GCodeFilteredScanner::GCodeFilteredScanner(GCodeScanner &scanner)
    : scanner(scanner) {}
  
  std::optional<GCodeToken> GCodeFilteredScanner::next() {
    std::optional<GCodeToken> token;
    while (!token.has_value() && !this->finished()) {
      token = this->scanner.next();
      if (!token.has_value()) {
        continue;
      }
      GCodeToken &tok = token.value();
      if (tok.is(GCodeToken::Type::End) ||
        tok.is(GCodeToken::Type::Comment) ||
        tok.is(GCodeToken::Type::Keyword) ||
        tok.is(GCodeToken::Type::Literal) ||
        (tok.is(GCodeToken::Type::Operator) &&
          (tok.getOperator() == GCodeOperator::Percent ||
          tok.getOperator() == GCodeOperator::Star ||
          tok.getOperator() == GCodeOperator::None))) {
        token.reset();
      }
    }
    return token;
  }

  bool GCodeFilteredScanner::finished() {
    return this->scanner.finished();
  }

  GCodeParser::GCodeParser(GCodeScanner &scanner)
    : scanner(scanner) {
    this->tokens[0] = this->scanner.next();
    this->tokens[1] = this->scanner.next();
  }

  std::unique_ptr<GCodeBlock> GCodeParser::parse() {
    return this->nextBlock();
  }

  void GCodeParser::error(const std::string &msg) {
    if (this->tokens[0].has_value()) {
      throw GCodeParseException(msg, this->tokens[0].value().getPosition());
    } else {
      throw GCodeParseException(msg);
    }
  }

  void GCodeParser::shift() {
    this->tokens[0] = std::move(this->tokens[1]);
    this->tokens[1] = scanner.next();
  }

  GCodeToken GCodeParser::tokenAt(std::size_t idx) {
    if (this->tokens[idx].has_value()) {
      return this->tokens[idx].value();
    } else {
      throw GCodeParseException("Expected token");
    }
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

  void GCodeParser::assert(bool (GCodeParser::*assertion)(), const std::string &message) {
    if (!(this->*assertion)()) {
      this->error(message);
    }
  }

  bool GCodeParser::checkBlock() {
    return this->checkCommand();
  }

  std::unique_ptr<GCodeBlock> GCodeParser::nextBlock() {
    std::vector<std::unique_ptr<GCodeNode>> block;
    while (this->checkCommand()) {
      block.push_back(this->nextCommand());
    }
    return std::make_unique<GCodeBlock>(std::move(block));
  }

  bool GCodeParser::checkCommand() {
    return this->checkCommandWord();
  }

  std::unique_ptr<GCodeCommand> GCodeParser::nextCommand() {
    auto command = this->nextCommandWord();
    std::vector<std::unique_ptr<GCodeWord>> parameters;
    while (this->checkParameterWord()) {
      parameters.push_back(this->nextParameterWord());
    }
    return std::make_unique<GCodeCommand>(std::move(command), std::move(parameters));
  }

  bool GCodeParser::checkCommandWord() {
    return this->expectOperators(GCodeCommandOperators);
  }

  std::unique_ptr<GCodeWord> GCodeParser::nextCommandWord() {
    this->assert(&GCodeParser::checkCommandWord, "Command expected");
    unsigned char field = static_cast<unsigned char>(this->tokenAt().getOperator());
    this->shift();
    std::unique_ptr<GCodeConstantValue> value = this->nextConstant();
    if (value) {
      return std::make_unique<GCodeWord>(field, std::move(value));
    } else {
      return nullptr;
    }
  }

  bool GCodeParser::checkParameterWord() {
    return this->expectToken(GCodeToken::Type::Operator) &&
      !this->expectOperators(GCodeOperators) &&
      !this->expectOperator(GCodeOperator::None);
  }

  std::unique_ptr<GCodeWord> GCodeParser::nextParameterWord() {
    this->assert(&GCodeParser::checkParameterWord, "Parameter expected");
    unsigned char field = static_cast<unsigned char>(this->tokenAt().getOperator());
    this->shift();
    std::unique_ptr<GCodeConstantValue> value = this->nextConstant();
    return std::make_unique<GCodeWord>(field, std::move(value));
  }

  bool GCodeParser::checkConstant() {
    return this->expectToken(GCodeToken::Type::IntegerContant) ||
      this->expectToken(GCodeToken::Type::FloatConstant);
  }

  std::unique_ptr<GCodeConstantValue> GCodeParser::nextConstant() {
    std::unique_ptr<GCodeConstantValue> res = nullptr;
    if (this->expectToken(GCodeToken::Type::IntegerContant)) {
      res = std::make_unique<GCodeConstantValue>(this->tokenAt().getInteger());
    } else if (this->expectToken(GCodeToken::Type::FloatConstant)) {
      res = std::make_unique<GCodeConstantValue>(this->tokenAt().getFloat());
    } else {
      this->error("Constant expected");
    }
    this->shift();
    return res;
  }
}