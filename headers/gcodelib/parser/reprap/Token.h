#ifndef GCODELIB_PARSER_REPRAP_TOKEN_H_
#define GCODELIB_PARSER_REPRAP_TOKEN_H_

#include "gcodelib/Base.h"
#include "gcodelib/parser/Source.h"
#include <string>
#include <variant>
#include <iosfwd>

namespace GCodeLib::Parser::RepRap {

  enum class GCodeOperator {
// Commands
    G = 'G',
    M = 'M',
    N = 'N',
    F = 'F',
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
    O = 'O',
    P = 'P',
    Q = 'Q',
    R = 'R',
    S = 'S',
    U = 'U',
    V = 'V',
    W = 'W',
    X = 'X',
    Y = 'Y',
    Z = 'Z',
// Operations
    Star = '*',
    Percent = '%',
    None = '\0'
  };

  class GCodeToken {
   public:
    enum class Type {
      IntegerContant,
      FloatConstant,
      StringConstant,
      Operator,
      Comment,
      NewLine
    };

    GCodeToken(const SourcePosition &);
    GCodeToken(int64_t, const SourcePosition &);
    GCodeToken(double, const SourcePosition &);
    GCodeToken(const std::string &, bool, const SourcePosition &);
    GCodeToken(GCodeOperator, const SourcePosition &);
    GCodeToken(const GCodeToken &);
    GCodeToken(GCodeToken &&);
    GCodeToken &operator=(const GCodeToken &);
    GCodeToken &operator=(GCodeToken &&);

    Type getType() const;
    bool is(Type) const;
    const SourcePosition &getPosition() const;

    int64_t getInteger() const;
    double getFloat() const;
    const std::string &getString() const;
    const std::string &getComment() const;
    GCodeOperator getOperator() const;

    friend std::ostream &operator<<(std::ostream &, const GCodeToken &);

   private:
    Type token_type;
    std::variant<int64_t, double, std::string, GCodeOperator> value;
    SourcePosition token_position;
  };
}

#endif