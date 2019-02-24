#include "gcodelib/parser/AST.h"
#include <iostream>

namespace GCodeLib {

  GCodeNode::GCodeNode(Type type, const SourcePosition &position)
    : type(type), position(position) {}

  GCodeNode::Type GCodeNode::getType() const {
    return this->type;
  }

  bool GCodeNode::is(Type type) const {
    return this->type == type;
  }

  const std::set<uint32_t> &GCodeNode::getLabels() const {
    return this->labels;
  }

  void GCodeNode::addLabel(uint32_t label) {
    this->labels.insert(label);
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

  GCodeCommand::GCodeCommand(std::unique_ptr<GCodeWord> command, std::vector<std::unique_ptr<GCodeWord>> parameters, const SourcePosition &position)
    : GCodeNode::GCodeNode(Type::Command, position), command(std::move(command)), parameters(std::move(parameters)) {}

  GCodeWord &GCodeCommand::getCommand() const {
    return *this->command;
  }

  void GCodeCommand::getParameters(std::vector<std::reference_wrapper<GCodeWord>> &prms) const {
    for (auto &param : this->parameters) {
      prms.push_back(*param);
    }
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
    for (auto &cmd : this->content) {
      content.push_back(*cmd);
    }
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
}