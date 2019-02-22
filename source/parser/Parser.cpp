#include "gcodelib/parser/Parser.h"

namespace GCodeLib  {

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

  std::unique_ptr<GCodeNode> GCodeParser::parse() {
    return this->nextBlock();
  }

  void GCodeParser::shift() {
    this->tokens[0] = std::move(this->tokens[1]);
    this->tokens[1] = scanner.next();
  }

  bool GCodeParser::checkBlock() {
    return this->checkCommand();
  }

  std::unique_ptr<GCodeNode> GCodeParser::nextBlock() {
    std::vector<std::unique_ptr<GCodeNode>> block;
    while (this->checkCommand()) {
      auto node = this->nextCommand();
      if (node == nullptr) {
        return nullptr;
      }
      block.push_back(std::move(node));
    }
    return std::make_unique<GCodeBlock>(std::move(block));
  }

  bool GCodeParser::checkCommand() {
    return this->checkCommandWord();
  }

  std::unique_ptr<GCodeNode> GCodeParser::nextCommand() {
    auto command = this->nextCommandWord();
    if (command == nullptr) {
      return nullptr;
    }
    std::vector<std::unique_ptr<GCodeNode>> parameters;
    while (this->checkParameterWord()) {
      auto param = this->nextParameterWord();
      if (param == nullptr) {
        return nullptr;
      }
      parameters.push_back(std::move(param));
    }
    return std::make_unique<GCodeCommand>(std::move(command), std::move(parameters));
  }

  bool GCodeParser::checkCommandWord() {
    return this->tokens[0].has_value() &&
      this->tokens[0].value().is(GCodeToken::Type::Operator) &&
      (this->tokens[0].value().getOperator() == GCodeOperator::G ||
      this->tokens[0].value().getOperator() == GCodeOperator::M);
  }

  std::unique_ptr<GCodeNode> GCodeParser::nextCommandWord() {
    if (!this->checkCommandWord()) {
      return nullptr;
    }
    unsigned char field = static_cast<unsigned char>(this->tokens[0].value().getOperator());
    this->shift();
    std::unique_ptr<GCodeNode> value = this->nextConstant();
    if (value) {
      return std::make_unique<GCodeWord>(field, std::move(value));
    } else {
      return nullptr;
    }
  }

  bool GCodeParser::checkParameterWord() {
    return this->tokens[0].has_value() &&
      this->tokens[0].value().is(GCodeToken::Type::Operator) &&
      this->tokens[0].value().getOperator() != GCodeOperator::G &&
      this->tokens[0].value().getOperator() != GCodeOperator::M &&
      this->tokens[0].value().getOperator() != GCodeOperator::Percent &&
      this->tokens[0].value().getOperator() != GCodeOperator::Star &&
      this->tokens[0].value().getOperator() != GCodeOperator::None;
  }

  std::unique_ptr<GCodeNode> GCodeParser::nextParameterWord() {
    if (!this->checkParameterWord()) {
      return nullptr;
    }
    unsigned char field = static_cast<unsigned char>(this->tokens[0].value().getOperator());
    this->shift();
    std::unique_ptr<GCodeNode> value = this->nextConstant();
    if (value) {
      return std::make_unique<GCodeWord>(field, std::move(value));
    } else {
      return nullptr;
    }
  }

  bool GCodeParser::checkConstant() {
    return this->tokens[0].has_value() &&
      (this->tokens[0].value().is(GCodeToken::Type::IntegerContant) ||
      this->tokens[0].value().is(GCodeToken::Type::FloatConstant));
  }

  std::unique_ptr<GCodeNode> GCodeParser::nextConstant() {
    if (this->tokens[0].has_value() && this->tokens[0].value().is(GCodeToken::Type::IntegerContant)) {
      auto res = std::make_unique<GCodeIntegerContant>(this->tokens[0].value().getInteger());
      this->shift();
      return res;
    } else if (this->tokens[0].has_value() && this->tokens[0].value().is(GCodeToken::Type::FloatConstant)) {
      auto res = std::make_unique<GCodeFloatContant>(this->tokens[0].value().getFloat());
      this->shift();
      return res;
    } else {
      return nullptr;
    }
  }
}