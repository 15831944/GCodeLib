#ifndef GCODELIB_PARSER_SOURCE_H_
#define GCODELIB_PARSER_SOURCE_H_

#include "gcodelib/Base.h"
#include <string>
#include <iosfwd>

namespace GCodeLib::Parser {

  class SourcePosition {
   public:
    SourcePosition(const std::string &, uint32_t, uint16_t, uint8_t);

    const std::string &getTag() const;
    uint32_t getLine() const;
    uint16_t getColumn() const;
    uint8_t getChecksum() const;

    void update(uint32_t, uint16_t, uint8_t);

    friend std::ostream &operator<<(std::ostream &, const SourcePosition &);
   private:
    std::string tag;
    uint32_t line;
    uint16_t column;
    uint8_t checksum;
  };
}

#endif