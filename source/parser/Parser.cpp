#include "gcodelib/parser/Parser.h"
#include "gcodelib/parser/Error.h"

namespace GCodeLib  {

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
          tok.is(GCodeToken::Type::Keyword) ||
          tok.is(GCodeToken::Type::Literal) ||
          (tok.is(GCodeToken::Type::Operator) &&
            (tok.getOperator() == GCodeOperator::Percent ||
            tok.getOperator() == GCodeOperator::Star ||
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
    this->tokens[1] = this->scanner->next();
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
      }
    }
    return std::make_unique<GCodeBlock>(std::move(block), position.value());
  }
   
  bool GCodeParser::checkStatement() {
    return this->checkCommand() ||
      this->expectToken(GCodeToken::Type::NewLine);
  }

  std::unique_ptr<GCodeNode> GCodeParser::nextStatement() {
    if (this->expectToken(GCodeToken::Type::NewLine)) {
      auto position = this->position();
      this->shift();
      if (this->checkStatement()) {
        auto stmt = this->nextStatement();
        if (stmt) {
          stmt->addLabel(position.value().getLine());
          return stmt;
        }
      }
      return nullptr;
    } else {
      return this->nextCommand();
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
    std::unique_ptr<GCodeConstantValue> value = this->nextConstant();
    return std::make_unique<GCodeWord>(field, std::move(value), position.value());
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