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

#ifndef GCODELIB_PARSER_LINUXCNC_TOKEN_H_
#define GCODELIB_PARSER_LINUXCNC_TOKEN_H_

#include "gcodelib/Base.h"
#include "gcodelib/parser/Source.h"
#include <string>
#include <variant>
#include <iosfwd>

namespace GCodeLib::Parser::LinuxCNC {

  enum class GCodeOperator {
// Commands
    G = 'G',
    M = 'M',
    N = 'N',
    O = 'O',
    F = 'F',
    S = 'S',
    T = 'T',
// Parameters
    A = 'A',
    B = 'B',
    C = 'C',
    D = 'D',
    E = 'E',
    H = 'H',
    I = 'I',
    J = 'J',
    K = 'K',
    L = 'L',
    P = 'P',
    Q = 'Q',
    R = 'R',
    U = 'U',
    V = 'V',
    W = 'W',
    X = 'X',
    Y = 'Y',
    Z = 'Z',
// Operations
    Plus = '+',
    Minus = '-',
    Star = '*',
    Slash = '/',
    Percent = '%',
    Hash = '#',
    Equal = '=',
    OpeningBracket = '[',
    ClosingBracket = ']',
    GreaterThan = '>',
    LessThan = '<',
    None = '\0'
  };

  enum class GCodeKeyword {
    Mod,
    Eq,
    Ne,
    Gt,
    Ge,
    Lt,
    Le,
    And,
    Or,
    Xor,
    None,
    Sub,
    Endsub,
    Return,
    Call,
    If,
    Elseif,
    Else,
    Endif,
    While,
    Endwhile,
    Do,
    Repeat,
    Endrepeat,
    Break,
    Continue
  };

  class GCodeToken {
   public:
    enum class Type {
      IntegerContant,
      FloatConstant,
      Operator,
      Keyword,
      Literal,
      Comment,
      NewLine
    };

    GCodeToken(const SourcePosition &);
    GCodeToken(int64_t, const SourcePosition &);
    GCodeToken(double, const SourcePosition &);
    GCodeToken(const std::string &, bool, const SourcePosition &);
    GCodeToken(GCodeOperator, const SourcePosition &);
    GCodeToken(GCodeKeyword, const SourcePosition &);
    GCodeToken(const GCodeToken &);
    GCodeToken(GCodeToken &&);
    GCodeToken &operator=(const GCodeToken &);
    GCodeToken &operator=(GCodeToken &&);

    Type getType() const;
    bool is(Type) const;
    const SourcePosition &getPosition() const;

    int64_t getInteger() const;
    double getFloat() const;
    const std::string &getLiteral() const;
    const std::string &getComment() const;
    GCodeOperator getOperator() const;
    GCodeKeyword getKeyword() const;

    friend std::ostream &operator<<(std::ostream &, const GCodeToken &);

   private:
    Type token_type;
    std::variant<int64_t, double, std::string, GCodeOperator, GCodeKeyword> value;
    SourcePosition token_position;
  };
}

#endif