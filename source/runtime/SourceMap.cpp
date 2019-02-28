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