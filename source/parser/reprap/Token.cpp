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

#include "gcodelib/parser/reprap/Token.h"
#include <map>
#include <iostream>

namespace GCodeLib::Parser::RepRap {

  GCodeToken::GCodeToken(const SourcePosition &position)
    : token_type(GCodeToken::Type::NewLine), token_position(position) {}

  GCodeToken::GCodeToken(int64_t value, const SourcePosition &position)
    : token_type(GCodeToken::Type::IntegerContant), value(value), token_position(position) {}
  
  GCodeToken::GCodeToken(double value, const SourcePosition &position)
    : token_type(GCodeToken::Type::FloatConstant), value(value), token_position(position) {}
  
  GCodeToken::GCodeToken(const std::string &value, bool literal, const SourcePosition &position)
    : token_type(literal ? GCodeToken::Type::StringConstant : GCodeToken::Type::Comment), value(value), token_position(position) {}

  GCodeToken::GCodeToken(GCodeOperator value, const SourcePosition &position)
    : token_type(GCodeToken::Type::Operator), value(value), token_position(position) {}

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

  const std::string &GCodeToken::getString() const {
    if (this->is(Type::StringConstant)) {
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

  std::ostream &operator<<(std::ostream &os, const GCodeToken &token) {
    switch (token.token_type) {
      case GCodeToken::Type::IntegerContant:
        os << token.getInteger();
        break;
      case GCodeToken::Type::FloatConstant:
        os << token.getFloat();
        break;
      case GCodeToken::Type::StringConstant:
        os << token.getString();
        break;
      case GCodeToken::Type::Operator:
        os << static_cast<char>(token.getOperator());
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