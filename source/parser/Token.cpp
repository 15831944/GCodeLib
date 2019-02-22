#include "gcodelib/parser/Token.h"
#include <iostream>

namespace GCodeLib {

  GCodeToken::GCodeToken(const Position &position)
    : token_type(GCodeToken::Type::End), token_position(std::make_unique<GCodeToken::Position>(position)) {}

  GCodeToken::GCodeToken(int64_t value, const Position &position)
    : token_type(GCodeToken::Type::IntegerContant), value(value), token_position(std::make_unique<GCodeToken::Position>(position)) {}
  
  GCodeToken::GCodeToken(double value, const Position &position)
    : token_type(GCodeToken::Type::FloatConstant), value(value), token_position(std::make_unique<GCodeToken::Position>(position)) {}
  
  GCodeToken::GCodeToken(const std::string &value, bool literal, const Position &position)
    : token_type(literal ? GCodeToken::Type::Literal : GCodeToken::Type::Comment), value(value), token_position(std::make_unique<GCodeToken::Position>(position)) {}

  GCodeToken::GCodeToken(GCodeOperator value, const Position &position)
    : token_type(GCodeToken::Type::Operator), value(value), token_position(std::make_unique<GCodeToken::Position>(position)) {}

  GCodeToken::GCodeToken(GCodeKeyword value, const Position &position)
    : token_type(GCodeToken::Type::Keyword), value(value), token_position(std::make_unique<GCodeToken::Position>(position)) {}

  GCodeToken::GCodeToken(const GCodeToken &token)
    : token_type(token.getType()), value(token.value), token_position(std::make_unique<GCodeToken::Position>(token.getPosition())) {}

  GCodeToken::GCodeToken(GCodeToken &&token)
    : token_type(token.token_type), value(std::move(token.value)), token_position(std::move(token.token_position)) {}
  
  GCodeToken &GCodeToken::operator=(const GCodeToken &token) {
    this->token_type = token.getType();
    this->value = token.value;
    this->token_position = std::make_unique<GCodeToken::Position>(token.getPosition());
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

  const GCodeToken::Position &GCodeToken::getPosition() const {
    return *this->token_position;
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

  GCodeToken::Position::Position(const std::string &tag, uint32_t line, uint16_t column)
    : tag(tag), line(line), column(column) {}

  const std::string &GCodeToken::Position::getTag() const {
    return this->tag;
  }

  uint32_t GCodeToken::Position::getLine() const {
    return this->line;
  }

  uint16_t GCodeToken::Position::getColumn() const {
    return this->column;
  }

  void GCodeToken::Position::update(uint32_t line, uint16_t column) {
    this->line = line;
    this->column = column;
  }

  std::ostream &operator<<(std::ostream &os, const GCodeToken &token) {
    switch (token.token_type) {
      case GCodeToken::Type::IntegerContant:
        os << "(Integer: " << token.getInteger();
        break;
      case GCodeToken::Type::FloatConstant:
        os << "(Float: " << token.getFloat();
        break;
      case GCodeToken::Type::Literal:
        os << "(Literal: " << token.getLiteral();
        break;
      case GCodeToken::Type::Operator:
        os << "(Operator: " << static_cast<char>(token.getOperator());
        break;
      case GCodeToken::Type::Keyword:
        os << "(Keyword";
        break;
      case GCodeToken::Type::Comment:
        os << "(Comment: " << token.getComment();
        break;
      case GCodeToken::Type::End:
        os << "(End";
        break;
    }
    os << "; " << token.getPosition().getLine() << ':' << token.getPosition().getColumn() << ")";
    return os;
  }
}