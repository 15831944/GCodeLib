#ifndef GCODELIB_PARSER_TOKEN_H_
#define GCODELIB_PARSER_TOKEN_H_

#include "gcodelib/Base.h"
#include <string>
#include <variant>
#include <iosfwd>

namespace GCodeLib {

  class SourcePosition {
   public:
    SourcePosition(const std::string &, uint32_t, uint16_t, uint8_t);

    const std::string &getTag() const;
    uint32_t getLine() const;
    uint16_t getColumn() const;
    uint8_t getChecksum() const;

    void update(uint32_t, uint16_t, uint8_t);
   private:
    std::string tag;
    uint32_t line;
    uint16_t column;
    uint8_t checksum;
  };

  enum class GCodeOperator {
    G = 'G',
    M = 'M',
    T = 'T',
    S = 'S',
    P = 'P',
    X = 'X',
    Y = 'Y',
    Z = 'Z',
    U = 'U',
    V = 'V',
    W = 'W',
    I = 'I',
    J = 'J',
    D = 'D',
    H = 'H',
    F = 'F',
    R = 'R',
    Q = 'Q',
    E = 'E',
    N = 'N',
    Star = '*',
    Percent = '%',
    None = '\0'
  };

  enum class GCodeKeyword {
    None
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
      End
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