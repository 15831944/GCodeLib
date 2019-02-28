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

#include "gcodelib/runtime/SourceMap.h"
#include <functional>

namespace GCodeLib::Runtime {

  IRSourceBlock::IRSourceBlock(const Parser::SourcePosition &position, std::size_t start, std::size_t length)
    : position(position), start(start), length(length) {}
  
  const Parser::SourcePosition &IRSourceBlock::getSourcePosition() const {
    return this->position;
  }

  std::size_t IRSourceBlock::getStartAddress() const {
    return this->start;
  }

  std::size_t IRSourceBlock::getLength() const {
    return this->length;
  }

  bool IRSourceBlock::includes(std::size_t address) const {
    return address >= this->start && address < this->start + this->length;
  }

  void IRSourceMap::addBlock(const Parser::SourcePosition &position, std::size_t start, std::size_t length) {
    this->blocks.push_back(IRSourceBlock(position, start, length));
  }

  std::optional<Parser::SourcePosition> IRSourceMap::locate(std::size_t address) {
    std::optional<std::reference_wrapper<const IRSourceBlock>> result;
    for (const auto &block : this->blocks) {
      if (block.includes(address) && (
        !result.has_value() || result.value().get().getLength() > block.getLength())) {
        result = std::ref(block);
      }
    }
    if (result.has_value()) {
      return result.value().get().getSourcePosition();
    } else {
      return std::optional<Parser::SourcePosition>();
    }
  }
}