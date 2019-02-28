#ifndef GCODELIB_PARSER_LINUXCNC_PARSER_H_
#define GCODELIB_PARSER_LINUXCNC_PARSER_H_

#include "gcodelib/parser/AST.h"
#include "gcodelib/parser/Mangling.h"
#include "gcodelib/parser/linuxcnc/Scanner.h"
#include "gcodelib/parser/Parser.h"
#include <functional>
#include <set>
#include <memory>
#include <stack>

namespace GCodeLib::Parser::LinuxCNC {

  class GCodeLCNCMangler : public GCodeNameMangler {
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
    bool expectKeyword(GCodeKeyword, std::size_t = 0);
    bool expectInteger(int64_t, std::size_t = 0);

    bool checkBlock();
    std::unique_ptr<GCodeBlock> nextBlock();
    bool checkStatement();
    std::unique_ptr<GCodeNode> nextStatement();
    bool checkChecksum();
    std::unique_ptr<GCodeNode> nextChecksum();
    bool checkAssignment();
    std::unique_ptr<GCodeNode> nextAssignment();
    bool checkFlowCommand();
    bool checkFlowCommandFinalizer();
    std::unique_ptr<GCodeNode> nextFlowCommand();
    bool checkNumberedStatement();
    std::unique_ptr<GCodeNode> nextNumberedStatement();
    bool checkConditional();
    std::unique_ptr<GCodeNode> nextConditional(int64_t);
    bool checkWhileLoop();
    std::unique_ptr<GCodeNode> nextWhileLoop(int64_t);
    bool checkRepeatLoop();
    std::unique_ptr<GCodeNode> nextRepeatLoop(int64_t);
    bool checkProcedure();
    std::unique_ptr<GCodeNode> nextProcedure(int64_t);
    bool checkProcedureCall();
    std::unique_ptr<GCodeNode> nextProcedureCall();
    bool checkProcedureReturn();
    std::unique_ptr<GCodeNode> nextProcedureReturn();
    bool checkLoopControl();
    std::unique_ptr<GCodeNode> nextLoopControl(int64_t);
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
    bool checkIdentifier();
    std::unique_ptr<GCodeNode> nextIdentifier();
    bool checkVariable();
    std::unique_ptr<GCodeNode> nextVariableReference();
    bool checkConstant();
    std::unique_ptr<GCodeConstantValue> nextConstant();

    std::stack<int64_t> openedStatements;
  };
}

#endif