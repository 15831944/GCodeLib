#include "gcodelib/parser/Source.h"
#include <iostream>

namespace GCodeLib::Parser {

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

  std::ostream &operator<<(std::ostream &os, const SourcePosition &position) {
    if (!position.getTag().empty()) {
      os << position.getTag() << '@';
    }
    os << position.getLine() << ':' << position.getColumn();
    return os;
  }
}