#include "gcodelib/parser/linuxcnc/Token.h"
#include <iostream>

namespace GCodeLib::Parser::LinuxCNC {

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