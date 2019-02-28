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