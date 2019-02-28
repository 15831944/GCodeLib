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