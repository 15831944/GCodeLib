#ifndef GCODELIB_PARSER_REPRAP_PARSER_H_
#define GCODELIB_PARSER_REPRAP_PARSER_H_

#include "gcodelib/parser/AST.h"
#include "gcodelib/parser/Mangling.h"
#include "gcodelib/parser/reprap/Scanner.h"
#include "gcodelib/parser/Parser.h"
#include <functional>
#include <set>
#include <memory>

namespace GCodeLib::Parser::RepRap {

  class GCodeRepRapMangler : public GCodeNameMangler {
   public:
    std::string getProcedureName(const std::string &) const override;
    std::string getStatementStart(const std::string &) const override;
    std::string getStatementEnd(const std::string &) const override;
    std::string getLoop(const std::string &) const override;
  };

  class GCodeFilteredScanner : public GCodeScanner<GCodeToken> {
   public:
    GCodeFilteredScanner(GCodeScanner<GCodeToken> &);

    std::optional<GCodeToken> next() override;
    bool finished() override;
   private:
    GCodeScanner<GCodeToken> &scanner;
  };

  class GCodeParser;

  class GCodeParser : public GCodeParserBase<GCodeParser, std::unique_ptr<GCodeFilteredScanner>, GCodeToken, 3> {
   public:
    GCodeParser(GCodeScanner<GCodeToken> &, GCodeNameMangler &);
    ~GCodeParser();
    std::unique_ptr<GCodeBlock> parse();
   private:
    bool expectToken(GCodeToken::Type, std::size_t = 0);
    bool expectOperator(GCodeOperator, std::size_t = 0);
    bool expectOperators(const std::set<GCodeOperator> &, std::size_t = 0);

    bool checkBlock();
    std::unique_ptr<GCodeBlock> nextBlock();
    bool checkStatement();
    std::unique_ptr<GCodeNode> nextStatement();
    bool checkChecksum();
    std::unique_ptr<GCodeNode> nextChecksum();
    bool checkCommand();
    std::unique_ptr<GCodeCommand> nextCommand();
    bool checkCommandWord();
    std::unique_ptr<GCodeWord> nextCommandWord();
    bool checkParameterWord();
    std::unique_ptr<GCodeWord> nextParameterWord();
    bool checkParameter();
    std::unique_ptr<GCodeNode> nextParameter();
    bool checkConstant();
    std::unique_ptr<GCodeConstantValue> nextConstant();
    bool checkString();
    std::unique_ptr<GCodeConstantValue> nextString();
  };
}

#endif