#include "gcodelib/parser/Token.h"
#include <iostream>

namespace GCodeLib {

  GCodeToken::GCodeToken(const SourcePosition &position)
    : token_type(GCodeToken::Type::NewLine), token_position(position) {}

  GCodeToken::GCodeToken(int64_t value, const SourcePosition &position)
    : token_type(GCodeToken::Type::IntegerContant), value(value), token_position(position) {}
  
  GCodeToken::GCodeToken(double value, const SourcePosition &position)
    : token_type(GCodeToken::Type::FloatConstant), value(value), token_position(position) {}
  
  GCodeToken::GCodeToken(const std::string &value, bool literal, const SourcePosition &position)
    : token_type(literal ? GCodeToken::Type::Literal : GCodeToken::Type::Comment), value(value), token_position(position) {}

  GCodeToken::GCodeToken(GCodeOperator value, const SourcePosition &position)
    : token_type(GCodeToken::Type::Operator), value(value), token_position(position) {}

  GCodeToken::GCodeToken(GCodeKeyword value, const SourcePosition &position)
    : token_type(GCodeToken::Type::Keyword), value(value), token_position(position) {}

  GCodeToken::GCodeToken(const GCodeToken &token)
    : token_type(token.getType()), value(token.value), token_position(token.getPosition()) {}

  GCodeToken::GCodeToken(GCodeToken &&token)
    : token_type(token.token_type), value(std::move(token.value)), token_position(std::move(token.token_position)) {}
  
  GCodeToken &GCodeToken::operator=(const GCodeToken &token) {
    this->token_type = token.getType();
    this->value = token.value;
    this->token_position = token.getPosition();
    return *this;
  }

  GCodeToken &GCodeToken::operator=(GCodeToken &&token) {
    this->token_type = token.token_type;
    this->value = token.value;
    this->token_position = std::move(token.token_position);
    return *this;
  }

  GCodeToken::Type GCodeToken::getType() const {
    return this->token_type;
  }

  bool GCodeToken::is(Type type) const {
    return this->token_type == type;
  }

  const SourcePosition &GCodeToken::getPosition() const {
    return this->token_position;
  }

  int64_t GCodeToken::getInteger() const {
    if (this->is(Type::IntegerContant)) {
      return std::get<int64_t>(this->value);
    } else {
      return 0;
    }
  }

  double GCodeToken::getFloat() const {
    if (this->is(Type::FloatConstant)) {
      return std::get<double>(this->value);
    } else {
      return 0.0;
    }
  }

  static std::string EmptyString = "";

  const std::string &GCodeToken::getLiteral() const {
    if (this->is(Type::Literal)) {
      return std::get<std::string>(this->value);
    } else {
      return EmptyString;
    }
  }

  const std::string &GCodeToken::getComment() const {
    if (this->is(Type::Comment)) {
      return std::get<std::string>(this->value);
    } else {
      return EmptyString;
    }
  }

  GCodeOperator GCodeToken::getOperator() const {
    if (this->is(Type::Operator)) {
      return std::get<GCodeOperator>(this->value);
    } else {
      return GCodeOperator::None;
    }
  }

  GCodeKeyword GCodeToken::getKeyword() const {
    if (this->is(Type::Keyword)) {
      return std::get<GCodeKeyword>(this->value);
    } else {
      return GCodeKeyword::None;
    }
  }

  SourcePosition::SourcePosition(const std::string &tag, uint32_t line, uint16_t column, uint8_t checksum)
    : tag(tag), line(line), column(column), checksum(checksum) {}

  const std::string &SourcePosition::getTag() const {
    return this->tag;
  }

  uint32_t SourcePosition::getLine() const {
    return this->line;
  }

  uint16_t SourcePosition::getColumn() const {
    return this->column;
  }
  
  uint8_t SourcePosition::getChecksum() const {
    return this->checksum;
  }

  void SourcePosition::update(uint32_t line, uint16_t column, uint8_t checksum) {
    this->line = line;
    this->column = column;
    this->checksum = checksum;
  }

  std::ostream &operator<<(std::ostream &os, const GCodeToken &token) {
    switch (token.token_type) {
      case GCodeToken::Type::IntegerContant:
        os << token.getInteger();
        break;
      case GCodeToken::Type::FloatConstant:
        os << token.getFloat();
        break;
      case GCodeToken::Type::Literal:
        os << token.getLiteral();
        break;
      case GCodeToken::Type::Operator:
        os << static_cast<char>(token.getOperator());
        break;
      case GCodeToken::Type::Keyword:
        os << "Keyword:" << static_cast<int>(token.getKeyword());
        break;
      case GCodeToken::Type::Comment:
        os << '(' << token.getComment() << ')';
        break;
      case GCodeToken::Type::NewLine:
        os << std::endl;
        break;
      default:
        break;
    }
    return os;
  }
}