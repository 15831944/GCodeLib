#include "gcodelib/parser/AST.h"

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

  GCodeWord::GCodeWord(unsigned char field, std::unique_ptr<GCodeNode> value)
    : GCodeNode::GCodeNode(Type::Word), field(field), value(std::move(value)) {}

  unsigned char GCodeWord::getField() const {
    return this->field;
  }

  GCodeNode &GCodeWord::getValue() const {
    return *this->value;
  }

  void GCodeWord::dump(std::ostream &os) const {
    os << this->field << this->getValue();
  }

  GCodeCommand::GCodeCommand(std::unique_ptr<GCodeNode> command, std::vector<std::unique_ptr<GCodeNode>> parameters)
    : GCodeNode::GCodeNode(Type::Command), command(std::move(command)), parameters(std::move(parameters)) {}

  GCodeNode &GCodeCommand::getCommand() const {
    return *this->command;
  }

  void GCodeCommand::getParameters(std::vector<std::reference_wrapper<GCodeNode>> &prms) const {
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