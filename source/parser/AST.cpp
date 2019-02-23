#include "gcodelib/parser/AST.h"
#include <iostream>

namespace GCodeLib {

  GCodeNode::GCodeNode(Type type)
    : type(type) {}

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

  GCodeConstantValue::GCodeConstantValue(int64_t value)
    : GCodeNode::GCodeNode(Type::IntegerContant), value(value) {}
  
  GCodeConstantValue::GCodeConstantValue(double value)
    : GCodeNode::GCodeNode(Type::FloatContant), value(value) {}

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

  GCodeWord::GCodeWord(unsigned char field, std::unique_ptr<GCodeConstantValue> value)
    : GCodeNode::GCodeNode(Type::Word), field(field), value(std::move(value)) {}

  unsigned char GCodeWord::getField() const {
    return this->field;
  }

  GCodeConstantValue &GCodeWord::getValue() const {
    return *this->value;
  }

  void GCodeWord::dump(std::ostream &os) const {
    os << this->field << this->getValue();
  }

  GCodeCommand::GCodeCommand(std::unique_ptr<GCodeWord> command, std::vector<std::unique_ptr<GCodeWord>> parameters)
    : GCodeNode::GCodeNode(Type::Command), command(std::move(command)), parameters(std::move(parameters)) {}

  GCodeWord &GCodeCommand::getCommand() const {
    return *this->command;
  }

  void GCodeCommand::getParameters(std::vector<std::reference_wrapper<GCodeWord>> &prms) const {
    for (auto &param : this->parameters) {
      prms.push_back(*param);
    }
  }
  
  void GCodeCommand::dump(std::ostream &os) const {
    os << '[' << this->getCommand() << ": ";
    for (auto &param : this->parameters) {
      os << *param << ';';
    }
    os << ']';
  }
  

  GCodeBlock::GCodeBlock(std::vector<std::unique_ptr<GCodeNode>> content)
    : GCodeNode::GCodeNode(Type::Block), content(std::move(content)) {}
  
  void GCodeBlock::getContent(std::vector<std::reference_wrapper<GCodeNode>> &content) const {
    for (auto &cmd : this->content) {
      content.push_back(*cmd);
    }
  }

  void GCodeBlock::dump(std::ostream &os) const {
    os << '{';
    for (auto &cmd : this->content) {
      os << *cmd;
    }
    os << '}';
  }
}