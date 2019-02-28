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

#ifndef GCODELIB_RUNTIME_SOURCEMAP_H_
#define GCODELIB_RUNTIME_SOURCEMAP_H_

#include "gcodelib/parser/Source.h"
#include <vector>
#include <optional>

namespace GCodeLib::Runtime {

  class IRSourceBlock {
   public:
    IRSourceBlock(const Parser::SourcePosition &, std::size_t, std::size_t);
    const Parser::SourcePosition &getSourcePosition() const;
    std::size_t getStartAddress() const;
    std::size_t getLength() const;
    bool includes(std::size_t) const;
   private:
    Parser::SourcePosition position;
    std::size_t start;
    std::size_t length;
  };

  class IRSourceMap {
   public:
    void addBlock(const Parser::SourcePosition &, std::size_t, std::size_t);
    std::optional<Parser::SourcePosition> locate(std::size_t);
   private:
    std::vector<IRSourceBlock> blocks;
  };
}

#endif