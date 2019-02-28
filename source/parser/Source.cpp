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