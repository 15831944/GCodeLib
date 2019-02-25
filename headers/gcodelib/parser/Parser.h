#ifndef GCODELIB_PARSER_PARSER_H_
#define GCODELIB_PARSER_PARSER_H_

#include "gcodelib/parser/AST.h"
#include "gcodelib/parser/Scanner.h"
#include <functional>
#include <set>
#include <memory>

namespace GCodeLib {

  class GCodeParser {
    struct FilteredScanner;
   public:
    GCodeParser(GCodeScanner &);
    std::unique_ptr<GCodeBlock> parse();
   private:

    [[noreturn]]
    void error(const std::string &);
    void shift();

    std::optional<SourcePosition> position();
    GCodeToken tokenAt(std::size_t = 0);
    bool expectToken(GCodeToken::Type, std::size_t = 0);
    bool expectOperator(GCodeOperator, std::size_t = 0);
    bool expectOperators(const std::set<GCodeOperator> &, std::size_t = 0);
    bool expectKeyword(GCodeKeyword, std::size_t = 0);
    void assert(bool (GCodeParser::*)(), const std::string &);

    bool checkBlock();
    std::unique_ptr<GCodeBlock> nextBlock();
    bool checkStatement();
    std::unique_ptr<GCodeNode> nextStatement();
    bool checkCommand();
    std::unique_ptr<GCodeCommand> nextCommand();
    bool checkCommandWord();
    std::unique_ptr<GCodeWord> nextCommandWord();
    bool checkParameterWord();
    std::unique_ptr<GCodeWord> nextParameterWord();
    bool checkParameter();
    std::unique_ptr<GCodeNode> nextParameter();
    bool checkExpression();
    std::unique_ptr<GCodeNode> nextExpression();
    bool checkLogical();
    std::unique_ptr<GCodeNode> nextLogical();
    bool checkComparison();
    std::unique_ptr<GCodeNode> nextComparison();
    bool checkAddSub();
    std::unique_ptr<GCodeNode> nextAddSub();
    bool checkMulDiv();
    std::unique_ptr<GCodeNode> nextMulDiv();
    bool checkPower();
    std::unique_ptr<GCodeNode> nextPower();
    bool checkAtom();
    std::unique_ptr<GCodeNode> nextAtom();
    bool checkConstant();
    std::unique_ptr<GCodeConstantValue> nextConstant();

    std::shared_ptr<GCodeParser::FilteredScanner> scanner;
    std::optional<GCodeToken> tokens[2];
  };
}

#endif