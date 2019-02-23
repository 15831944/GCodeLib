#ifndef GCODELIB_PARSER_PARSER_H_
#define GCODELIB_PARSER_PARSER_H_

#include "gcodelib/parser/AST.h"
#include "gcodelib/parser/Scanner.h"
#include <functional>
#include <set>

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
    std::unique_ptr<GCodeBlock> parse();
   private:
    [[noreturn]]
    void error(const std::string &);
    void shift();

    GCodeToken tokenAt(std::size_t = 0);
    bool expectToken(GCodeToken::Type, std::size_t = 0);
    bool expectOperator(GCodeOperator, std::size_t = 0);
    bool expectOperators(const std::set<GCodeOperator> &, std::size_t = 0);
    void assert(bool (GCodeParser::*)(), const std::string &);

    bool checkBlock();
    std::unique_ptr<GCodeBlock> nextBlock();
    bool checkCommand();
    std::unique_ptr<GCodeCommand> nextCommand();
    bool checkCommandWord();
    std::unique_ptr<GCodeWord> nextCommandWord();
    bool checkParameterWord();
    std::unique_ptr<GCodeWord> nextParameterWord();
    bool checkConstant();
    std::unique_ptr<GCodeConstantValue> nextConstant();

    GCodeFilteredScanner scanner;
    std::optional<GCodeToken> tokens[2];
  };
}

#endif