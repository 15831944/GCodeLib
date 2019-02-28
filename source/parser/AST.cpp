#include "gcodelib/parser/AST.h"
#include <map>
#include <iostream>

namespace GCodeLib::Parser {

  static const std::map<GCodeUnaryOperation::Operation, std::string> UnaryMnemonics = {
    { GCodeUnaryOperation::Operation::Negate, "Negate" }
  };

  static const std::map<GCodeBinaryOperation::Operation, std::string> BinaryMnemonics = {
    { GCodeBinaryOperation::Operation::Add, "Add" },
    { GCodeBinaryOperation::Operation::Subtract, "Subtract" },
    { GCodeBinaryOperation::Operation::Multiply, "Multiply" },
    { GCodeBinaryOperation::Operation::Divide, "Divide" },
    { GCodeBinaryOperation::Operation::Power, "Power" },
    { GCodeBinaryOperation::Operation::Modulo, "Modulo" },
    { GCodeBinaryOperation::Operation::Equals, "Equals" },
    { GCodeBinaryOperation::Operation::NotEquals, "NotEquals" },
    { GCodeBinaryOperation::Operation::Greater, "Greater" },
    { GCodeBinaryOperation::Operation::GreaterOrEquals, "GreaterOrEquals" },
    { GCodeBinaryOperation::Operation::Lesser, "Less" },
    { GCodeBinaryOperation::Operation::LesserOrEquals, "LessOrEquals" },
    { GCodeBinaryOperation::Operation::And, "And" },
    { GCodeBinaryOperation::Operation::Or, "Or" },
    { GCodeBinaryOperation::Operation::Xor, "Xor" }
  };

  template <typename T>
  static void copy_arguments(const std::vector<std::unique_ptr<T>> &source, std::vector<std::reference_wrapper<const T>> &destination) {
    for (const auto &element : source) {
      destination.push_back(*element);
    }
  }

  GCodeNode::GCodeNode(Type type, const SourcePosition &position)
    : type(type), position(position) {}

  GCodeNode::Type GCodeNode::getType() const {
    return this->type;
  }

  bool GCodeNode::is(Type type) const {
    return this->type == type;
  }

  const SourcePosition &GCodeNode::getPosition() const {
    return this->position;
  }

  std::ostream &operator<<(std::ostream &os, const GCodeNode &node) {
    node.dump(os);
    return os;
  }

  GCodeConstantValue::GCodeConstantValue(int64_t value, const SourcePosition &position)
    : GCodeVisitableNode(Type::IntegerContant, position), value(value) {}
  
  GCodeConstantValue::GCodeConstantValue(double value, const SourcePosition &position)
    : GCodeVisitableNode(Type::FloatContant, position), value(value) {}

  int64_t GCodeConstantValue::asInteger(int64_t defaultValue) const {
    if (this->is(Type::IntegerContant)) {
      return std::get<int64_t>(this->value);
    } else {
      return defaultValue;
    }
  }

  double GCodeConstantValue::asFloat(double defaultValue) const {
    if (this->is(Type::FloatContant)) {
      return std::get<double>(this->value);
    } else {
      return defaultValue;
    }
  }

  void GCodeConstantValue::dump(std::ostream &os) const {
    if (this->is(Type::IntegerContant)) {
      os << this->asInteger();
    } else {
      os << this->asFloat();
    }
  }

  void GCodeNamedVariable::dump(std::ostream &os) const {
    os << '#' << this->getIdentifier();
  }

  void GCodeNumberedVariable::dump(std::ostream &os) const {
    os << '#' << this->getIdentifier();
  }

  void GCodeNamedVariableAssignment::dump(std::ostream &os) const {
    os << "[store " << this->getIdentifier() << " = " << this->getValue() << ']';
  }

  void GCodeNumberedVariableAssignment::dump(std::ostream &os) const {
    os << "[store " << this->getIdentifier() << " = " << this->getValue() << ']';
  }

  GCodeUnaryOperation::GCodeUnaryOperation(Operation operation, std::unique_ptr<GCodeNode> argument, const SourcePosition &position)
    : GCodeVisitableNode(Type::UnaryOperation, position), operation(operation), argument(std::move(argument)) {}
  
  GCodeUnaryOperation::Operation GCodeUnaryOperation::getOperation() const {
    return this->operation;
  }

  const GCodeNode &GCodeUnaryOperation::getArgument() const {
    return *this->argument;
  }

  void GCodeUnaryOperation::dump(std::ostream &os) const {
    os << '[' << UnaryMnemonics.at(this->getOperation()) << ' ' << this->getArgument() << ']';
  }

  GCodeBinaryOperation::GCodeBinaryOperation(Operation operation, std::unique_ptr<GCodeNode> left, std::unique_ptr<GCodeNode> right, const SourcePosition &position)
    : GCodeVisitableNode(Type::BinaryOperation, position), operation(operation), leftArgument(std::move(left)), rightArgument(std::move(right)) {}
  
  GCodeBinaryOperation::Operation GCodeBinaryOperation::getOperation() const {
    return this->operation;
  }

  const GCodeNode &GCodeBinaryOperation::getLeftArgument() const {
    return *this->leftArgument;
  }

  const GCodeNode &GCodeBinaryOperation::getRightArgument() const {
    return *this->rightArgument;
  }

  void GCodeBinaryOperation::dump(std::ostream &os) const {
    os << '[' << BinaryMnemonics.at(this->operation) << ' ' << this->getLeftArgument() << "; " << this->getRightArgument() << ']';
  }

  GCodeFunctionCall::GCodeFunctionCall(const std::string &functionId, std::vector<std::unique_ptr<GCodeNode>> args, const SourcePosition &position)
    : GCodeVisitableNode(GCodeNode::Type::FunctionCall, position), functionIdentifier(functionId), arguments(std::move(args)) {}

  const std::string &GCodeFunctionCall::getFunctionIdentifier() const {
    return this->functionIdentifier;
  }

  void GCodeFunctionCall::getArguments(std::vector<std::reference_wrapper<const GCodeNode>> &args) const {
    copy_arguments(this->arguments, args);
  }

  void GCodeFunctionCall::dump(std::ostream &os) const {
    os << this->functionIdentifier << '[';
    for (std::size_t i = 0; i < this->arguments.size(); i++) { 
      os << *this->arguments.at(i);
      if (i + 1 < this->arguments.size()) {
        os << ';';
      }
    }
    os << ']';
  }

  GCodeWord::GCodeWord(unsigned char field, std::unique_ptr<GCodeNode> value, const SourcePosition &position)
    : GCodeVisitableNode(Type::Word, position), field(field), value(std::move(value)) {}

  unsigned char GCodeWord::getField() const {
    return this->field;
  }

  const GCodeNode &GCodeWord::getValue() const {
    return *this->value;
  }

  void GCodeWord::dump(std::ostream &os) const {
    os << this->field << this->getValue();
  }

  GCodeCommand::GCodeCommand(std::unique_ptr<GCodeWord> command, std::vector<std::unique_ptr<GCodeWord>> parameters, const SourcePosition &position)
    : GCodeVisitableNode(Type::Command, position), command(std::move(command)), parameters(std::move(parameters)) {}

  const GCodeWord &GCodeCommand::getCommand() const {
    return *this->command;
  }

  void GCodeCommand::getParameters(std::vector<std::reference_wrapper<const GCodeWord>> &prms) const {
    copy_arguments(this->parameters, prms);
  }
  
  void GCodeCommand::dump(std::ostream &os) const {
    os << '[' << this->getCommand();
    for (std::size_t i = 0; i < this->parameters.size(); i++) {
      os << ' ' << *this->parameters.at(i);
    }
    os << ']';
  }
  

  GCodeBlock::GCodeBlock(std::vector<std::unique_ptr<GCodeNode>> content, const SourcePosition &position)
    : GCodeVisitableNode(Type::Block, position), content(std::move(content)) {}
  
  void GCodeBlock::getContent(std::vector<std::reference_wrapper<const GCodeNode>> &content) const {
    copy_arguments(this->content, content);
  }

  void GCodeBlock::dump(std::ostream &os) const {
    os << '[';
    for (std::size_t i = 0; i < this->content.size(); i++) {
      os << *this->content.at(i);
      if (i + 1 < this->content.size()) {
        os << "; ";
      }
    }
    os << ']';
  }

  GCodeNamedStatement::GCodeNamedStatement(const std::string &identifier, std::unique_ptr<GCodeNode> stmt, const SourcePosition &position)
    : GCodeVisitableNode(Type::NamedStatement, position), identifier(identifier), statement(std::move(stmt)) {}
  
  const std::string &GCodeNamedStatement::getIdentifier() const {
    return this->identifier;
  }

  const GCodeNode &GCodeNamedStatement::getStatement() const {
    return *this->statement;
  }
  
  void GCodeNamedStatement::dump(std::ostream &os) const {
    os << "[" << this->identifier << ": " << this->getStatement() << ']';
  }

  GCodeProcedureDefinition::GCodeProcedureDefinition(int64_t id, std::unique_ptr<GCodeNode> body, std::vector<std::unique_ptr<GCodeNode>> rets, const SourcePosition &position)
    : GCodeVisitableNode(Type::ProcedureDefinition, position), identifier(id), body(std::move(body)), retValues(std::move(rets)) {}
  
  int64_t GCodeProcedureDefinition::getIdentifier() const {
    return this->identifier;
  }

  const GCodeNode &GCodeProcedureDefinition::getBody() const {
    return *this->body;
  }

  void GCodeProcedureDefinition::getReturnValues(std::vector<std::reference_wrapper<const GCodeNode>> &rets) const {
    copy_arguments(this->retValues, rets);
  }

  void GCodeProcedureDefinition::dump(std::ostream &os) const {
    os << "[procedure " << this->identifier << ' ' << *this->body << " returns [";
    for (auto &ret : this->retValues) {
      os << *ret << ';';
    }
    os << ']';
  }

  GCodeProcedureReturn::GCodeProcedureReturn(std::vector<std::unique_ptr<GCodeNode>> retVals, const SourcePosition &position)
    : GCodeVisitableNode(Type::ProcedureReturn, position), returnValues(std::move(retVals)) {}
  
  void GCodeProcedureReturn::getReturnValues(std::vector<std::reference_wrapper<const GCodeNode>> &rets) const {
    copy_arguments(this->returnValues, rets);
  }

  void GCodeProcedureReturn::dump(std::ostream &os) const {
    os << "[return";
    std::for_each(this->returnValues.begin(), this->returnValues.end(), [&](auto &value) {
      os << ' ' << *value << ';';
    });
    os << ']';
  }

  GCodeProcedureCall::GCodeProcedureCall(std::unique_ptr<GCodeNode> pid, std::vector<std::unique_ptr<GCodeNode>> args, const SourcePosition &position)
    : GCodeVisitableNode(Type::ProcedureCall, position), procedureId(std::move(pid)), args(std::move(args)) {}
  
  const GCodeNode &GCodeProcedureCall::getProcedureId() const {
    return *this->procedureId;
  }

  void GCodeProcedureCall::getArguments(std::vector<std::reference_wrapper<const GCodeNode>> &args) const {
    copy_arguments(this->args, args);
  }

  void GCodeProcedureCall::dump(std::ostream &os) const {
    os << "[call " << *this->procedureId << ']';
  }

  GCodeConditional::GCodeConditional(std::unique_ptr<GCodeNode> condition, std::unique_ptr<GCodeNode> thenBody, std::unique_ptr<GCodeNode> elseBody, const SourcePosition &position)
    : GCodeVisitableNode(Type::Conditional, position), condition(std::move(condition)), thenBody(std::move(thenBody)), elseBody(std::move(elseBody)) {}
  
  const GCodeNode &GCodeConditional::getCondition() const {
    return *this->condition;
  }

  const GCodeNode &GCodeConditional::getThenBody() const {
    return *this->thenBody;
  }

  const GCodeNode *GCodeConditional::getElseBody() const {
    return this->elseBody.get();
  }
  
  void GCodeConditional::dump(std::ostream &os) const {
    if (this->elseBody) {
      os << "[if " << *this->condition << " then " << *this->thenBody << " else " << *this->elseBody << ']';
    } else {
      os << "[if " << *this->condition << " then " << *this->thenBody << ']';
    }
  }

  GCodeWhileLoop::GCodeWhileLoop(int64_t label, std::unique_ptr<GCodeNode> condition, std::unique_ptr<GCodeNode> body, bool doWhile, const SourcePosition &position)
    : GCodeLoop(label, position), condition(std::move(condition)), body(std::move(body)), doWhileLoop(doWhile) {}

  const GCodeNode &GCodeWhileLoop::getCondition() const {
    return *this->condition;
  }

  const GCodeNode &GCodeWhileLoop::getBody() const {
    return *this->body;
  }

  bool GCodeWhileLoop::isDoWhile() const {
    return this->doWhileLoop;
  }

  void GCodeWhileLoop::dump(std::ostream &os) const {
    if (!this->doWhileLoop) {
      os << '[' << this->getLabel() << ": while " << this->getCondition() << " do " << this->getBody() << ']';
    } else {
      os << '[' << this->getLabel() << ": do " << this->getBody() << " while " << this->getCondition() << ']';
    }
  }

  GCodeRepeatLoop::GCodeRepeatLoop(int64_t label, std::unique_ptr<GCodeNode> counter, std::unique_ptr<GCodeNode> body, const SourcePosition &position)
    : GCodeLoop(label, position), counter(std::move(counter)), body(std::move(body)) {}
  
  const GCodeNode &GCodeRepeatLoop::getCounter() const {
    return *this->counter;
  }

  const GCodeNode &GCodeRepeatLoop::getBody() const {
    return *this->body;
  }

  void GCodeRepeatLoop::dump(std::ostream &os) const {
    os << '[' << this->getLabel() << ": repeat " << this->getBody() << ' ' << this->getCounter() << ']';
  }

  GCodeLoopControl::GCodeLoopControl(int64_t identifier, ControlType controlType, const SourcePosition &position)
    : GCodeVisitableNode(Type::LoopControl, position), identifier(identifier), controlType(controlType) {}
  
  int64_t GCodeLoopControl::getLoopIdentifier() const {
    return this->identifier;
  }

  GCodeLoopControl::ControlType GCodeLoopControl::getControlType() const {
    return this->controlType;
  }
  
  void GCodeLoopControl::dump(std::ostream &os) const {
    if (this->controlType == ControlType::Break) {
      os << "[break ";
    } else {
      os << "[continue ";
    }
    os << this->identifier << ']';
  }
}