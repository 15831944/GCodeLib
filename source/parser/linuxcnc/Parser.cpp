#include "gcodelib/parser/linuxcnc/Parser.h"
#include "gcodelib/parser/Error.h"
#include <algorithm>
#include <iostream>

namespace GCodeLib::Parser::LinuxCNC  {

  static const std::set<GCodeOperator> GCodeCommandOperators = {
    GCodeOperator::G,
    GCodeOperator::M,
    GCodeOperator::F,
    GCodeOperator::S,
    GCodeOperator::T
  };

  static const std::set<GCodeOperator> GCodeParameterLetters = {
    GCodeOperator::A,
    GCodeOperator::B,
    GCodeOperator::C,
    GCodeOperator::D,
    GCodeOperator::H,
    GCodeOperator::I,
    GCodeOperator::J,
    GCodeOperator::K,
    GCodeOperator::L,
    GCodeOperator::P,
    GCodeOperator::Q,
    GCodeOperator::R,
    GCodeOperator::U,
    GCodeOperator::V,
    GCodeOperator::W,
    GCodeOperator::X,
    GCodeOperator::Y,
    GCodeOperator::Z
  };


  class GCodeParser::FilteredScanner : public GCodeScanner {
   public:
    FilteredScanner(GCodeScanner &scanner)
      : scanner(scanner) {}

    std::optional<GCodeToken> next() override {
      std::optional<GCodeToken> token;
      while (!token.has_value() && !this->finished()) {
        token = this->scanner.next();
        if (!token.has_value()) {
          continue;
        }
        GCodeToken &tok = token.value();
        if (tok.is(GCodeToken::Type::Comment) ||
          (tok.is(GCodeToken::Type::Keyword) && (
            tok.getKeyword() == GCodeKeyword::None)) ||
          (tok.is(GCodeToken::Type::Operator) &&
            (tok.getOperator() == GCodeOperator::Percent ||
            tok.getOperator() == GCodeOperator::None))) {
          token.reset();
        }
      }
      return token;
    }
    bool finished() override {
      return this->scanner.finished();
    }
   private:
    GCodeScanner &scanner;
  };

  GCodeParser::GCodeParser(GCodeScanner &scanner)
    : scanner(std::make_unique<FilteredScanner>(scanner)) {
    this->tokens[0] = this->scanner->next();
    this->tokens[1] = this->scanner->next();
    this->tokens[2] = this->scanner->next();
  }

  std::unique_ptr<GCodeBlock> GCodeParser::parse() {
    return this->nextBlock();
  }

  void GCodeParser::error(const std::string &msg) {
    if (this->tokens[0].has_value()) {
      throw GCodeParseException(msg, this->tokens[0].value().getPosition());
    } else {
      throw GCodeParseException(msg);
    }
  }

  void GCodeParser::shift() {
    this->tokens[0] = std::move(this->tokens[1]);
    this->tokens[1] = this->tokens[2];
    this->tokens[2] = this->scanner->next();
  }

  std::optional<SourcePosition> GCodeParser::position() {
    if (this->tokens[0].has_value()) {
      return this->tokens[0].value().getPosition();
    } else {
      return std::optional<SourcePosition>();
    }
  }

  GCodeToken GCodeParser::tokenAt(std::size_t idx) {
    if (this->tokens[idx].has_value()) {
      return this->tokens[idx].value();
    } else {
      throw GCodeParseException("Expected token");
    }
  }

  bool GCodeParser::expectToken(GCodeToken::Type type, std::size_t idx) {
    return this->tokens[idx].has_value() && this->tokens[idx].value().is(type);
  }

  bool GCodeParser::expectOperator(GCodeOperator oper, std::size_t idx) {
    return this->expectToken(GCodeToken::Type::Operator, idx) &&
      this->tokenAt(idx).getOperator() == oper;
  }

  bool GCodeParser::expectOperators(const std::set<GCodeOperator> &opers, std::size_t idx) {
    return this->expectToken(GCodeToken::Type::Operator, idx) &&
      opers.count(this->tokenAt(idx).getOperator()) != 0;
  }

  bool GCodeParser::expectKeyword(GCodeKeyword kw, std::size_t idx) {
    return this->expectToken(GCodeToken::Type::Keyword, idx) &&
      this->tokenAt(idx).getKeyword() == kw;
  }

  bool GCodeParser::expectInteger(int64_t value, std::size_t idx) {
    return this->expectToken(GCodeToken::Type::IntegerContant, idx) &&
      this->tokenAt(idx).getInteger() == value;
  }

  void GCodeParser::assert(bool (GCodeParser::*assertion)(), const std::string &message) {
    if (!(this->*assertion)()) {
      this->error(message);
    }
  }

  bool GCodeParser::checkBlock() {
    return this->checkStatement();
  }

  std::unique_ptr<GCodeBlock> GCodeParser::nextBlock() {
    auto position = this->position();
    std::vector<std::unique_ptr<GCodeNode>> block;
    while (this->checkStatement()) {
      auto stmt = this->nextStatement();
      if (stmt) {
        block.push_back(std::move(stmt));
      } else {
        break;
      }
    }
    return std::make_unique<GCodeBlock>(std::move(block), position.value());
  }
   
  bool GCodeParser::checkStatement() {
    return this->checkCommand() ||
      this->checkFlowCommand() ||
      this->checkProcedureCall() ||
      this->checkAssignment() ||
      this->expectToken(GCodeToken::Type::NewLine);
  }

  std::unique_ptr<GCodeNode> GCodeParser::nextStatement() {
    if (this->expectToken(GCodeToken::Type::NewLine)) {
      auto position = this->position();
      this->shift();
      if (this->checkStatement()) {
        auto stmt = this->nextStatement();
        if (stmt) {
          return std::make_unique<GCodeLabel>(position.value().getLine(), std::move(stmt), position.value());
        }
      }
      return nullptr;
    } else if (this->checkFlowCommand()) {
      return this->nextFlowCommand();
    } else if (this->checkProcedureCall()) {
      return this->nextProcedureCall();
    } else if (this->checkAssignment()) {
      return this->nextAssignment();
    } else {
      return this->nextCommand();
    }
  }

  bool GCodeParser::checkAssignment() {
    return this->expectOperator(GCodeOperator::Hash);
  }

  std::unique_ptr<GCodeNode> GCodeParser::nextAssignment() {
    auto position = this->position();
    this->assert(&GCodeParser::checkAssignment, "Assignment expected");
    this->shift();
    bool enclosed = false;
    if (this->expectOperator(GCodeOperator::LessThan)) {
      enclosed = true;
      this->shift();
    }
    std::variant<int64_t, std::string> key;
    if (this->expectToken(GCodeToken::Type::IntegerContant)) {
      key = this->tokenAt().getInteger();
      this->shift();
    } else if (this->expectToken(GCodeToken::Type::Literal)) {
      key = this->tokenAt().getLiteral();
      this->shift();
    } else {
      this->error("Expected numbered or named variable reference");
    }
    if (enclosed) {
      if (!this->expectOperator(GCodeOperator::GreaterThan)) {
        this->error("\'>\' expected");
      }
      this->shift();
    }
    if (!this->expectOperator(GCodeOperator::Equal)) {
      this->error("\'=\' expected");
    }
    this->shift();
    std::unique_ptr<GCodeNode> value = this->nextLogical();
    if (key.index() == 0) {
      return std::make_unique<GCodeNumberedVariableAssignment>(std::get<0>(key), std::move(value), position.value());
    } else {
      return std::make_unique<GCodeNamedVariableAssignment>(std::get<1>(key), std::move(value), position.value());
    }
  }

  bool GCodeParser::checkFlowCommand() {
    return this->expectOperator(GCodeOperator::O) &&
      this->expectToken(GCodeToken::Type::IntegerContant, 1) &&
      (this->expectKeyword(GCodeKeyword::Sub, 2) ||
      this->expectKeyword(GCodeKeyword::If, 2) ||
      (this->expectKeyword(GCodeKeyword::While, 2) &&
        (this->openedStatements.empty() || this->openedStatements.top() != this->tokenAt(1).getInteger())) ||
      this->expectKeyword(GCodeKeyword::Do, 2) ||
      this->expectKeyword(GCodeKeyword::Repeat, 2) ||
      this->expectKeyword(GCodeKeyword::Return, 2) ||
      this->expectKeyword(GCodeKeyword::Break, 2) ||
      this->expectKeyword(GCodeKeyword::Continue, 2));
  }

  bool GCodeParser::checkFlowCommandFinalizer() {
    if (!this->expectOperator(GCodeOperator::O) ||
      !this->expectToken(GCodeToken::Type::IntegerContant, 1) ||
      this->openedStatements.empty()) {
      return false;
    }
    int64_t id = this->openedStatements.top();
    return id == this->tokenAt(1).getInteger() &&
      (this->expectKeyword(GCodeKeyword::Endsub, 2) ||
      this->expectKeyword(GCodeKeyword::Elseif, 2) ||
      this->expectKeyword(GCodeKeyword::Else, 2) ||
      this->expectKeyword(GCodeKeyword::Endif, 2) ||
      this->expectKeyword(GCodeKeyword::While, 2) ||
      this->expectKeyword(GCodeKeyword::Endwhile, 2) ||
      this->expectKeyword(GCodeKeyword::Endrepeat, 2));
  }

  std::unique_ptr<GCodeNode> GCodeParser::nextFlowCommand() {
    this->assert(&GCodeParser::checkFlowCommand, "Flow command expected");
    this->shift();
    if (!this->expectToken(GCodeToken::Type::IntegerContant)) {
      this->error("Integer constant expected");
    }
    int64_t id = this->tokenAt().getInteger();
    this->openedStatements.push(id);
    this->shift();
    std::unique_ptr<GCodeNode> stmt;
    if (this->checkProcedure()) {
      stmt = this->nextProcedure(id);
    } else if (this->checkConditional()) {
      stmt = this->nextConditional(id);
    } else if (this->checkWhileLoop()) {
      stmt = this->nextWhileLoop(id);
    } else if (this->checkRepeatLoop()) {
      stmt = this->nextRepeatLoop(id);
    } else if (this->checkProcedureReturn()) {
      stmt = this->nextProcedureReturn();
    } else if (this->checkLoopControl()) {
      stmt = this->nextLoopControl(id);
    } else {
      this->openedStatements.pop();
      this->error("Unknown flow command");
    }
    this->openedStatements.pop();
    return stmt;
  }

  bool GCodeParser::checkProcedureReturn() {
    return this->expectKeyword(GCodeKeyword::Return);
  }

  std::unique_ptr<GCodeNode> GCodeParser::nextProcedureReturn() {
    auto position = this->position();
    this->assert(&GCodeParser::checkProcedureReturn, "Expected return statement");
    this->shift();
    std::vector<std::unique_ptr<GCodeNode>> values;
    while (this->checkExpression()) {
      values.push_back(this->nextExpression());
    }
    return std::make_unique<GCodeProcedureReturn>(std::move(values), position.value());
  }

  bool GCodeParser::checkConditional() {
    return this->expectKeyword(GCodeKeyword::If);
  }

  std::unique_ptr<GCodeNode> GCodeParser::nextConditional(int64_t id) {
    auto position = this->position();
    this->assert(&GCodeParser::checkConditional, "Conditional statement expected");
    this->shift();
    std::unique_ptr<GCodeNode> condition = this->nextExpression();
    std::unique_ptr<GCodeNode> body = this->nextBlock();
    std::vector<std::tuple<std::unique_ptr<GCodeNode>, std::unique_ptr<GCodeNode>, SourcePosition>> ifBlocks;
    ifBlocks.push_back(std::make_tuple(std::move(condition), std::move(body), position.value()));
    while (this->expectOperator(GCodeOperator::O) &&
      this->expectInteger(id, 1) &&
      this->expectKeyword(GCodeKeyword::Elseif, 2)) {
      position = this->position();
      this->shift();
      this->shift();
      this->shift();
      std::unique_ptr<GCodeNode> elifCondition = this->nextExpression();
      std::unique_ptr<GCodeNode> elifBody = this->nextBlock();
      ifBlocks.push_back(std::make_tuple(std::move(elifCondition), std::move(elifBody), position.value()));
    }
    std::reverse(ifBlocks.begin(), ifBlocks.end());
    std::unique_ptr<GCodeNode> elseBody = nullptr;
    if (this->expectOperator(GCodeOperator::O) &&
      this->expectInteger(id, 1) &&
      this->expectKeyword(GCodeKeyword::Else, 2)) {
      this->shift();
      this->shift();
      this->shift();
      elseBody = this->nextBlock();
    }
    if (!(this->expectOperator(GCodeOperator::O) &&
      this->expectInteger(id, 1) &&
      this->expectKeyword(GCodeKeyword::Endif, 2))) {
      this->error("Expected \'endif\'");
    }
    this->shift();
    this->shift();
    this->shift();
    condition = std::move(std::get<0>(ifBlocks.at(0)));
    body = std::move(std::get<1>(ifBlocks.at(0)));
    std::unique_ptr<GCodeNode> node = std::make_unique<GCodeConditional>(std::move(condition), std::move(body), std::move(elseBody), std::get<2>(ifBlocks.at(0)));
    for (std::size_t i = 1; i < ifBlocks.size(); i++) {
      condition = std::move(std::get<0>(ifBlocks.at(i)));
      body = std::move(std::get<1>(ifBlocks.at(i)));
      node = std::make_unique<GCodeConditional>(std::move(condition), std::move(body), std::move(node), std::get<2>(ifBlocks.at(i)));
    }
    return node;
  }

  bool GCodeParser::checkWhileLoop() {
    return this->expectKeyword(GCodeKeyword::While) ||
      this->expectKeyword(GCodeKeyword::Do  );
  }

  std::unique_ptr<GCodeNode> GCodeParser::nextWhileLoop(int64_t id) {
    auto position = this->position();
    this->assert(&GCodeParser::checkWhileLoop, "Expected while loop");
    std::string loopName = "while_loop_" + std::to_string(id);
    if (this->expectKeyword(GCodeKeyword::While)) {
      this->shift();
      std::unique_ptr<GCodeNode> conditional = this->nextExpression();
      std::unique_ptr<GCodeNode> body = this->nextBlock();
      if (!(this->expectOperator(GCodeOperator::O) &&
        this->expectInteger(id, 1) &&
        this->expectKeyword(GCodeKeyword::Endwhile, 2))) {
        this->error("\'endwhile\' expected");
      }
      this->shift();
      this->shift();
      this->shift();
      return std::make_unique<GCodeNamedStatement>(loopName, std::make_unique<GCodeWhileLoop>(std::move(conditional), std::move(body), false, position.value()), position.value());
    } else {
      this->shift();
      std::unique_ptr<GCodeNode> body = this->nextBlock();
      if (!(this->expectOperator(GCodeOperator::O) &&
        this->expectInteger(id, 1) &&
        this->expectKeyword(GCodeKeyword::While, 2))) {
        this->error("\'while\' expected");
      }
      this->shift();
      this->shift();
      this->shift();
      std::unique_ptr<GCodeNode> conditional = this->nextExpression();
      return std::make_unique<GCodeNamedStatement>(loopName, std::make_unique<GCodeWhileLoop>(std::move(conditional), std::move(body), true, position.value()), position.value());
    }
  }

  bool GCodeParser::checkRepeatLoop() {
    return this->expectKeyword(GCodeKeyword::Repeat);
  }

  std::unique_ptr<GCodeNode> GCodeParser::nextRepeatLoop(int64_t id) {
    auto position = this->position();
    this->assert(&GCodeParser::checkRepeatLoop, "Expected repeat loop");
    this->shift();
    std::unique_ptr<GCodeNode> counter = this->nextExpression();
    std::unique_ptr<GCodeNode> body = this->nextBlock();
    if (!(this->expectOperator(GCodeOperator::O) &&
      this->expectInteger(id, 1) &&
      this->expectKeyword(GCodeKeyword::Endrepeat, 2))) {
      this->error("\'endrepeat\' expected");
    }
    this->shift();
    this->shift();
    this->shift();
    return std::make_unique<GCodeRepeatLoop>(std::move(counter), std::move(body), position.value());
  }

  bool GCodeParser::checkProcedure() {
    return this->expectKeyword(GCodeKeyword::Sub);
  }

  std::unique_ptr<GCodeNode> GCodeParser::nextProcedure(int64_t pid) {
    auto position = this->position();
    this->assert(&GCodeParser::checkProcedure, "Procedure definition expected");
    this->shift();
    std::unique_ptr<GCodeNode> body = this->nextBlock();
    if (!this->expectOperator(GCodeOperator::O) ||
      !this->expectInteger(pid, 1)) {
      this->error("Procedure close command expected");
    }
    this->shift();
    this->shift();
    if (!this->expectKeyword(GCodeKeyword::Endsub)) {
      this->error("\'endsub\' expected");
    }
    this->shift();
    std::vector<std::unique_ptr<GCodeNode>> rets;
    while (this->checkExpression()) {
      rets.push_back(this->nextExpression());
    }
    return std::make_unique<GCodeProcedureDefinition>(pid, std::move(body), std::move(rets), position.value());
  }
  
  bool GCodeParser::checkProcedureCall() {
    return this->expectOperator(GCodeOperator::O) &&
      !this->checkFlowCommand() &&
      !this->checkFlowCommandFinalizer();
  }

  std::unique_ptr<GCodeNode> GCodeParser::nextProcedureCall() {
    auto position = this->position();
    this->assert(&GCodeParser::checkProcedureCall, "Expected procedure call");
    this->shift();
    std::unique_ptr<GCodeNode> pid = this->nextAtom();
    if (!this->expectKeyword(GCodeKeyword::Call)) {
      return nullptr;
    }
    this->shift();
    std::vector<std::unique_ptr<GCodeNode>> args;
    while (this->checkExpression()) {
      args.push_back(this->nextExpression());
    }
    return std::make_unique<GCodeProcedureCall>(std::move(pid), std::move(args), position.value());
  }

  bool GCodeParser::checkLoopControl() {
    return this->expectKeyword(GCodeKeyword::Break) ||
      this->expectKeyword(GCodeKeyword::Continue);
  }

  std::unique_ptr<GCodeNode> GCodeParser::nextLoopControl(int64_t id) {
    auto position = this->position();
    this->assert(&GCodeParser::checkLoopControl, "Expected \'break\' or \'continue\'");
    std::string loopName = "while_loop_" + std::to_string(id);
    GCodeKeyword kw = this->tokenAt().getKeyword();
    this->shift();
    switch (kw) {
      case GCodeKeyword::Break:
        return std::make_unique<GCodeLoopControl>(loopName, GCodeLoopControl::ControlType::Break, position.value());
      case GCodeKeyword::Continue:
        return std::make_unique<GCodeLoopControl>(loopName, GCodeLoopControl::ControlType::Continue, position.value());
      default:
        return nullptr;
    }
  }

  bool GCodeParser::checkCommand() {
    return this->checkCommandWord();
  }

  std::unique_ptr<GCodeCommand> GCodeParser::nextCommand() {
    auto position = this->position();
    auto command = this->nextCommandWord();
    std::vector<std::unique_ptr<GCodeWord>> parameters;
    while (this->checkParameterWord()) {
      parameters.push_back(this->nextParameterWord());
    }
    return std::make_unique<GCodeCommand>(std::move(command), std::move(parameters), position.value());
  }

  bool GCodeParser::checkCommandWord() {
    return this->expectOperators(GCodeCommandOperators);
  }

  std::unique_ptr<GCodeWord> GCodeParser::nextCommandWord() {
    auto position = this->position();
    this->assert(&GCodeParser::checkCommandWord, "Command expected");
    unsigned char field = static_cast<unsigned char>(this->tokenAt().getOperator());
    this->shift();
    std::unique_ptr<GCodeConstantValue> value = this->nextConstant();
    return std::make_unique<GCodeWord>(field, std::move(value), position.value());
  }

  bool GCodeParser::checkParameterWord() {
    return this->expectOperators(GCodeParameterLetters);
  }

  std::unique_ptr<GCodeWord> GCodeParser::nextParameterWord() {
    auto position = this->position();
    this->assert(&GCodeParser::checkParameterWord, "Parameter expected");
    unsigned char field = static_cast<unsigned char>(this->tokenAt().getOperator());
    this->shift();
    std::unique_ptr<GCodeNode> value = this->nextParameter();
    return std::make_unique<GCodeWord>(field, std::move(value), position.value());
  }

  bool GCodeParser::checkParameter() {
    return this->checkAtom();
  }

  std::unique_ptr<GCodeNode> GCodeParser::nextParameter() {
    this->assert(&GCodeParser::checkParameter, "Expression or constant expected");
    return this->nextAtom();
  }

  bool GCodeParser::checkExpression() {
    return this->expectOperator(GCodeOperator::OpeningBracket);
  }

  std::unique_ptr<GCodeNode> GCodeParser::nextExpression() {
    this->assert(&GCodeParser::checkExpression, "Expression expected");
    this->shift();
    auto expr = this->nextLogical();
    if (!this->expectOperator(GCodeOperator::ClosingBracket)) {
      this->error("\']\' expected");
    }
    this->shift();
    return expr;
  }

  bool GCodeParser::checkLogical() {
    return this->checkComparison();
  }

  std::unique_ptr<GCodeNode> GCodeParser::nextLogical() {
    auto position = this->position();
    this->assert(&GCodeParser::checkLogical, "Expected expression");
    auto expr = this->nextComparison();
    while (this->expectKeyword(GCodeKeyword::And) ||
      this->expectKeyword(GCodeKeyword::Or) ||
      this->expectKeyword(GCodeKeyword::Xor)) {
      GCodeKeyword kw = this->tokenAt().getKeyword();
      this->shift();
      auto right = this->nextComparison();
      switch (kw) {
        case GCodeKeyword::And:
          expr = std::make_unique<GCodeBinaryOperation>(GCodeBinaryOperation::Operation::And, std::move(expr), std::move(right), position.value());
          break;
        case GCodeKeyword::Or:
          expr = std::make_unique<GCodeBinaryOperation>(GCodeBinaryOperation::Operation::Or, std::move(expr), std::move(right), position.value());
          break;
        case GCodeKeyword::Xor:
          expr = std::make_unique<GCodeBinaryOperation>(GCodeBinaryOperation::Operation::Xor, std::move(expr), std::move(right), position.value());
          break;
        default:
          break;
      }
    }
    return expr;
  }

  bool GCodeParser::checkComparison() {
    return this->checkAddSub();
  }

  std::unique_ptr<GCodeNode> GCodeParser::nextComparison() {
    auto position = this->position();
    this->assert(&GCodeParser::checkComparison, "Expected expression");
    auto expr = this->nextAddSub();
    while (this->expectKeyword(GCodeKeyword::Eq) ||
      this->expectKeyword(GCodeKeyword::Ne) ||
      this->expectKeyword(GCodeKeyword::Ge) ||
      this->expectKeyword(GCodeKeyword::Gt) ||
      this->expectKeyword(GCodeKeyword::Le) ||
      this->expectKeyword(GCodeKeyword::Lt)) {
      GCodeKeyword kw = this->tokenAt().getKeyword();
      this->shift();
      auto right = this->nextAddSub();
      switch (kw) {
        case GCodeKeyword::Eq:
          expr = std::make_unique<GCodeBinaryOperation>(GCodeBinaryOperation::Operation::Equals, std::move(expr), std::move(right), position.value());
          break;
        case GCodeKeyword::Ne:
          expr = std::make_unique<GCodeBinaryOperation>(GCodeBinaryOperation::Operation::NotEquals, std::move(expr), std::move(right), position.value());
          break;
        case GCodeKeyword::Ge:
          expr = std::make_unique<GCodeBinaryOperation>(GCodeBinaryOperation::Operation::GreaterOrEquals, std::move(expr), std::move(right), position.value());
          break;
        case GCodeKeyword::Gt:
          expr = std::make_unique<GCodeBinaryOperation>(GCodeBinaryOperation::Operation::Greater, std::move(expr), std::move(right), position.value());
          break;
        case GCodeKeyword::Le:
          expr = std::make_unique<GCodeBinaryOperation>(GCodeBinaryOperation::Operation::LesserOrEquals, std::move(expr), std::move(right), position.value());
          break;
        case GCodeKeyword::Lt:
          expr = std::make_unique<GCodeBinaryOperation>(GCodeBinaryOperation::Operation::Lesser, std::move(expr), std::move(right), position.value());
          break;
        default:
          break;
      }
    }
    return expr;
  }

  bool GCodeParser::checkAddSub() {
    return this->checkMulDiv();
  }

  std::unique_ptr<GCodeNode> GCodeParser::nextAddSub() {
    auto position = this->position();
    this->assert(&GCodeParser::checkAddSub, "Expected expression");
    auto expr = this->nextMulDiv();
    while (this->expectOperator(GCodeOperator::Plus) ||
      this->expectOperator(GCodeOperator::Minus)) {
      GCodeOperator oper = this->tokenAt().getOperator();
      this->shift();
      auto right = this->nextMulDiv();
      switch (oper) {
        case GCodeOperator::Plus:
          expr = std::make_unique<GCodeBinaryOperation>(GCodeBinaryOperation::Operation::Add, std::move(expr), std::move(right), position.value());
          break;
        case GCodeOperator::Minus:
          expr = std::make_unique<GCodeBinaryOperation>(GCodeBinaryOperation::Operation::Subtract, std::move(expr), std::move(right), position.value());
          break;
        default:
          break;
      }
    }
    return expr;
  }

  bool GCodeParser::checkMulDiv() {
    return this->checkPower();
  }

  std::unique_ptr<GCodeNode> GCodeParser::nextMulDiv() {
    auto position = this->position();
    this->assert(&GCodeParser::checkMulDiv, "Expected expression");
    auto expr = this->nextPower();
    while (this->expectOperator(GCodeOperator::Star) ||
      this->expectOperator(GCodeOperator::Slash) ||
      this->expectKeyword(GCodeKeyword::Mod)) {
      auto oper = this->expectOperator(GCodeOperator::Star)
        ? GCodeBinaryOperation::Operation::Multiply
        : (this->expectOperator(GCodeOperator::Slash)
          ? GCodeBinaryOperation::Operation::Divide
          : GCodeBinaryOperation::Operation::Modulo);
      this->shift();
      auto right = this->nextPower();
      expr = std::make_unique<GCodeBinaryOperation>(oper, std::move(expr), std::move(right), position.value());
    }
    return expr;
  }

  bool GCodeParser::checkPower() {
    return this->checkAtom();
  }

  std::unique_ptr<GCodeNode> GCodeParser::nextPower() {
    auto position = this->position();
    this->assert(&GCodeParser::checkMulDiv, "Expected expression");
    auto expr = this->nextAtom();
    while (this->expectOperator(GCodeOperator::Star) &&
      this->expectOperator(GCodeOperator::Star, 1)) {
      this->shift();
      this->shift();
      auto right = this->nextAtom();
      expr = std::make_unique<GCodeBinaryOperation>(GCodeBinaryOperation::Operation::Power, std::move(expr), std::move(right), position.value());
    }
    return expr;
  }

  bool GCodeParser::checkAtom() {
    return this->checkConstant() ||
      this->checkExpression() ||
      this->checkIdentifier() ||
      this->checkVariable() ||
      this->expectOperator(GCodeOperator::Plus) ||
      this->expectOperator(GCodeOperator::Minus);
  }

  std::unique_ptr<GCodeNode> GCodeParser::nextAtom() {
    auto position = this->position();
    this->assert(&GCodeParser::checkAtom, "Constant or unary expression exprected");
    if (this->checkConstant()) {
      return this->nextConstant();
    } else if (this->checkExpression()) {
      return this->nextExpression();
    } else if (this->checkIdentifier()) {
      return this->nextIdentifier();
    } else if (this->checkVariable()) {
      return this->nextVariableReference();
    } else {
      GCodeOperator oper = this->tokenAt().getOperator();
      this->shift();
      auto argument = this->nextAtom();
      switch (oper) {
        case GCodeOperator::Plus:
          return argument;
        case GCodeOperator::Minus:
          return std::make_unique<GCodeUnaryOperation>(GCodeUnaryOperation::Operation::Negate, std::move(argument), position.value());
        default:
          return nullptr;
      }
    }
  }

  bool GCodeParser::checkIdentifier() {
    return this->expectToken(GCodeToken::Type::Literal);
  }

  std::unique_ptr<GCodeNode> GCodeParser::nextIdentifier() {
    auto position = this->position();
    this->assert(&GCodeParser::checkIdentifier, "Function call expected");
    std::string identifier = this->tokenAt().getLiteral();
    this->shift();
    if (!this->expectOperator(GCodeOperator::OpeningBracket)) {
      this->error("\'[\' expected");
    }
    this->shift();
    std::vector<std::unique_ptr<GCodeNode>> args;
    while (!this->expectOperator(GCodeOperator::ClosingBracket)) {
      args.push_back(this->nextLogical());
    }
    this->shift();
    return std::make_unique<GCodeFunctionCall>(identifier, std::move(args), position.value());
  }

  bool GCodeParser::checkVariable() {
    return this->expectOperator(GCodeOperator::Hash);
  }

  std::unique_ptr<GCodeNode> GCodeParser::nextVariableReference() {
    auto position = this->position();
    this->assert(&GCodeParser::checkVariable, "Expected variable reference");
    this->shift();
    std::unique_ptr<GCodeNode> node;
    bool enclosed = false;
    if (this->expectOperator(GCodeOperator::LessThan)) {
      enclosed = true;
      this->shift();
    }
    if (this->expectToken(GCodeToken::Type::IntegerContant)) {
      node = std::make_unique<GCodeNumberedVariable>(this->tokenAt().getInteger(), position.value());
      this->shift();
    } else if (this->expectToken(GCodeToken::Type::Literal)) {
      node = std::make_unique<GCodeNamedVariable>(this->tokenAt().getLiteral(), position.value());
      this->shift();
    } else {
      this->error("Expected named or numbered variable reference");
    }
    if (enclosed) {
      if (!this->expectOperator(GCodeOperator::GreaterThan)) {
        this->error("\'>\' expected");
      }
      this->shift();
    }
    return node;
  }

  bool GCodeParser::checkConstant() {
    return this->expectToken(GCodeToken::Type::IntegerContant) ||
      this->expectToken(GCodeToken::Type::FloatConstant);
  }

  std::unique_ptr<GCodeConstantValue> GCodeParser::nextConstant() {
    auto position = this->position();
    std::unique_ptr<GCodeConstantValue> res = nullptr;
    if (this->expectToken(GCodeToken::Type::IntegerContant)) {
      res = std::make_unique<GCodeConstantValue>(this->tokenAt().getInteger(), position.value());
    } else if (this->expectToken(GCodeToken::Type::FloatConstant)) {
      res = std::make_unique<GCodeConstantValue>(this->tokenAt().getFloat(), position.value());
    } else {
      this->error("Constant expected");
    }
    this->shift();
    return res;
  }
}