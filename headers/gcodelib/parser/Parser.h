#ifndef GCODELIB_PARSER_PARSER_H_
#define GCODELIB_PARSER_PARSER_H_

#include "gcodelib/parser/AST.h"
#include "gcodelib/parser/Scanner.h"
#include <functional>

namespace GCodeLib {

  class GCodeFilteredScanner : public GCodeScanner {
   public:
    GCodeFilteredScanner(GCodeScanner &);
    std::optional<GCodeToken> next() override;
    bool finished() override;
   private:
    GCodeScanner &scanner;
  };

  class GCodeParser {
   public:
    GCodeParser(GCodeScanner &);
    std::unique_ptr<GCodeNode> parse();
   private:
    void shift();
    bool checkBlock();
    std::unique_ptr<GCodeNode> nextBlock();
    bool checkCommand();
    std::unique_ptr<GCodeNode> nextCommand();
    bool checkCommandWord();
    std::unique_ptr<GCodeNode> nextCommandWord();
    bool checkParameterWord();
    std::unique_ptr<GCodeNode> nextParameterWord();
    bool checkConstant();
    std::unique_ptr<GCodeNode> nextConstant();

    GCodeFilteredScanner scanner;
    std::optional<GCodeToken> tokens[2];
  };
}

#endif