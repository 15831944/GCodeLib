#include "gcodelib/parser/AST.h"
#include <iostream>

namespace GCodeLib {

  template <typename T>
  static void copy_arguments(const std::vector<std::unique_ptr<T>> &source, std::vector<std::reference_wrapper<T>> &destination) {
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

  std::ostream &operator<<(std::ostream &os, const GCodeNode &node) {
    node.dump(os);
    return os;
  }

  GCodeConstantValue::GCodeConstantValue(int64_t value, const SourcePosition &position)
    : GCodeNode::GCodeNode(Type::IntegerContant, position), value(value) {}
  
  GCodeConstantValue::GCodeConstantValue(double value, const SourcePosition &position)
    : GCodeNode::GCodeNode(Type::FloatContant, position), value(value) {}

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

  void GCodeConstantValue::visit(Visitor &v) {
    v.visit(*this);
  }

  void GCodeConstantValue::dump(std::ostream &os) const {
    if (this->is(Type::IntegerContant)) {
      os << this->asInteger();
    } else {
      os << this->asFloat();
    }
  }

  GCodeUnaryOperation::GCodeUnaryOperation(Operation operation, std::unique_ptr<GCodeNode> argument, const SourcePosition &position)
    : GCodeNode::GCodeNode(Type::UnaryOperation, position), operation(operation), argument(std::move(argument)) {}
  
  GCodeUnaryOperation::Operation GCodeUnaryOperation::getOperation() const {
    return this->operation;
  }

  GCodeNode &GCodeUnaryOperation::getArgument() const {
    return *this->argument;
  }

  void GCodeUnaryOperation::visit(Visitor &v) {
    v.visit(*this);
  }

  void GCodeUnaryOperation::dump(std::ostream &os) const {
    os << '[' << static_cast<char>(this->operation) << this->getArgument() << ']';
  }

  GCodeBinaryOperation::GCodeBinaryOperation(Operation operation, std::unique_ptr<GCodeNode> left, std::unique_ptr<GCodeNode> right, const SourcePosition &position)
    : GCodeNode::GCodeNode(Type::BinaryOperation, position), operation(operation), leftArgument(std::move(left)), rightArgument(std::move(right)) {}
  
  GCodeBinaryOperation::Operation GCodeBinaryOperation::getOperation() const {
    return this->operation;
  }

  GCodeNode &GCodeBinaryOperation::getLeftArgument() const {
    return *this->leftArgument;
  }

  GCodeNode &GCodeBinaryOperation::getRightArgument() const {
    return *this->rightArgument;
  }

  void GCodeBinaryOperation::visit(Visitor &v) {
    v.visit(*this);
  }

  void GCodeBinaryOperation::dump(std::ostream &os) const {
    os << '[' << this->getLeftArgument() << static_cast<char>(this->operation) << this->getRightArgument() << ']';
  }

  GCodeFunctionCall::GCodeFunctionCall(const std::string &functionId, std::vector<std::unique_ptr<GCodeNode>> args, const SourcePosition &position)
    : GCodeNode::GCodeNode(GCodeNode::Type::FunctionCall, position), functionIdentifier(functionId), arguments(std::move(args)) {}

  const std::string &GCodeFunctionCall::getFunctionIdentifier() const {
    return this->functionIdentifier;
  }

  void GCodeFunctionCall::getArguments(std::vector<std::reference_wrapper<GCodeNode>> &args) const {
    copy_arguments(this->arguments, args);
  }

  void GCodeFunctionCall::visit(Visitor &v) {
    v.visit(*this);
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
    : GCodeNode::GCodeNode(Type::Word, position), field(field), value(std::move(value)) {}

  unsigned char GCodeWord::getField() const {
    return this->field;
  }

  GCodeNode &GCodeWord::getValue() const {
    return *this->value;
  }

  void GCodeWord::visit(Visitor &v) {
    v.visit(*this);
  }

  void GCodeWord::dump(std::ostream &os) const {
    os << this->field << this->getValue();
  }

  GCodeLabel::GCodeLabel(int64_t label, std::unique_ptr<GCodeNode> stmt, const SourcePosition &position)
    : GCodeNode::GCodeNode(Type::Label, position), label(label), statement(std::move(stmt)) {}
  
  int64_t GCodeLabel::getLabel() const {
    return this->label;
  }

  GCodeNode &GCodeLabel::getStatement() const {
    return *this->statement;
  }

  void GCodeLabel::visit(Visitor &v) {
    v.visit(*this);
  }

  void GCodeLabel::dump(std::ostream &os) const {
    os << "[Label:" << this->label << ' ' << this->getStatement() << ']';
  }

  GCodeCommand::GCodeCommand(std::unique_ptr<GCodeWord> command, std::vector<std::unique_ptr<GCodeWord>> parameters, const SourcePosition &position)
    : GCodeNode::GCodeNode(Type::Command, position), command(std::move(command)), parameters(std::move(parameters)) {}

  GCodeWord &GCodeCommand::getCommand() const {
    return *this->command;
  }

  void GCodeCommand::getParameters(std::vector<std::reference_wrapper<GCodeWord>> &prms) const {
    copy_arguments(this->parameters, prms);
  }

  void GCodeCommand::visit(Visitor &v) {
    v.visit(*this);
  }
  
  void GCodeCommand::dump(std::ostream &os) const {
    os << this->getCommand();
    for (std::size_t i = 0; i < this->parameters.size(); i++) {
      os << ' ' << *this->parameters.at(i);
    }
  }
  

  GCodeBlock::GCodeBlock(std::vector<std::unique_ptr<GCodeNode>> content, const SourcePosition &position)
    : GCodeNode::GCodeNode(Type::Block, position), content(std::move(content)) {}
  
  void GCodeBlock::getContent(std::vector<std::reference_wrapper<GCodeNode>> &content) const {
    copy_arguments(this->content, content);
  }

  void GCodeBlock::visit(Visitor &v) {
    v.visit(*this);
  }

  void GCodeBlock::dump(std::ostream &os) const {
    for (std::size_t i = 0; i < this->content.size(); i++) {
      os << *this->content.at(i);
      if (i + 1 < this->content.size()) {
        os << std::endl;
      }
    }
  }

  GCodeProcedureDefinition::GCodeProcedureDefinition(int64_t id, std::unique_ptr<GCodeNode> body, std::vector<std::unique_ptr<GCodeNode>> rets, const SourcePosition &position)
    : GCodeNode::GCodeNode(Type::ProcedureDefinition, position), identifier(id), body(std::move(body)), retValues(std::move(rets)) {}
  
  int64_t GCodeProcedureDefinition::getIdentifier() const {
    return this->identifier;
  }

  GCodeNode &GCodeProcedureDefinition::getBody() const {
    return *this->body;
  }

  void GCodeProcedureDefinition::getReturnValues(std::vector<std::reference_wrapper<GCodeNode>> &rets) const {
    copy_arguments(this->retValues, rets);
  }

  void GCodeProcedureDefinition::visit(Visitor &v) {
    v.visit(*this);
  }

  void GCodeProcedureDefinition::dump(std::ostream &os) const {
    os << this->identifier << '{' << *this->body << "}[";
    for (auto &ret : this->retValues) {
      os << *ret << ';';
    }
    os << ']';
  }

  GCodeProcedureReturn::GCodeProcedureReturn(std::vector<std::unique_ptr<GCodeNode>> retVals, const SourcePosition &position)
    : GCodeNode::GCodeNode(Type::ProcedureReturn, position), returnValues(std::move(retVals)) {}
  
  void GCodeProcedureReturn::getReturnValues(std::vector<std::reference_wrapper<GCodeNode>> &rets) const {
    copy_arguments(this->returnValues, rets);
  }

  void GCodeProcedureReturn::visit(Visitor &v) {
    v.visit(*this);
  }

  void GCodeProcedureReturn::dump(std::ostream &os) const {
    os << "[return";
    std::for_each(this->returnValues.begin(), this->returnValues.end(), [&](auto &value) {
      os << ' ' << *value;
    });
    os << ']';
  }

  GCodeProcedureCall::GCodeProcedureCall(std::unique_ptr<GCodeNode> pid, std::vector<std::unique_ptr<GCodeNode>> args, const SourcePosition &position)
    : GCodeNode::GCodeNode(Type::ProcedureCall, position), procedureId(std::move(pid)), args(std::move(args)) {}
  
  GCodeNode &GCodeProcedureCall::getProcedureId() const {
    return *this->procedureId;
  }

  void GCodeProcedureCall::getArguments(std::vector<std::reference_wrapper<GCodeNode>> &args) const {
    copy_arguments(this->args, args);
  }

  void GCodeProcedureCall::visit(Visitor &v) {
    v.visit(*this);
  }

  void GCodeProcedureCall::dump(std::ostream &os) const {
    os << "Call:" << *this->procedureId;
  }

  GCodeConditional::GCodeConditional(std::unique_ptr<GCodeNode> condition, std::unique_ptr<GCodeNode> thenBody, std::unique_ptr<GCodeNode> elseBody, const SourcePosition &position)
    : GCodeNode::GCodeNode(Type::Conditional, position), condition(std::move(condition)), thenBody(std::move(thenBody)), elseBody(std::move(elseBody)) {}
  
  GCodeNode &GCodeConditional::getCondition() const {
    return *this->condition;
  }

  GCodeNode &GCodeConditional::getThenBody() const {
    return *this->thenBody;
  }

  GCodeNode *GCodeConditional::getElseBody() const {
    return this->elseBody.get();
  }

  void GCodeConditional::visit(Visitor &v) {
    v.visit(*this);
  }
  
  void GCodeConditional::dump(std::ostream &os) const {
    if (this->elseBody) {
      os << "[if " << *this->condition << " then " << *this->thenBody << " else " << *this->elseBody << ']';
    } else {
      os << "[if " << *this->condition << " then " << *this->thenBody << ']';
    }
  }

  GCodeWhileLoop::GCodeWhileLoop(std::unique_ptr<GCodeNode> condition, std::unique_ptr<GCodeNode> body, const SourcePosition &position)
    : GCodeNode::GCodeNode(Type::WhileLoop, position), condition(std::move(condition)), body(std::move(body)) {}

  GCodeNode &GCodeWhileLoop::getCondition() const {
    return *this->condition;
  }

  GCodeNode &GCodeWhileLoop::getBody() const {
    return *this->body;
  }

  void GCodeWhileLoop::visit(Visitor &v) {
    v.visit(*this);
  }

  void GCodeWhileLoop::dump(std::ostream &os) const {
    os << "[while " << this->getCondition() << " do " << this->getBody() << ']';
  }
}