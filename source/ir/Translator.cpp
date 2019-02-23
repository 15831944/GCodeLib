#include "gcodelib/ir/Translator.h"

namespace GCodeLib {

  static std::pair<unsigned char, GCodeIRValue> ir_extract_word(GCodeWord &word) {
    unsigned char key = word.getField();
    GCodeConstantValue &value = word.getValue();
    if (value.is(GCodeNode::Type::IntegerContant)) {
      return std::make_pair(key, GCodeIRValue(value.asInteger()));
    } else if (value.is(GCodeNode::Type::FloatContant)) {
      return std::make_pair(key, GCodeIRValue(value.asFloat()));
    } else {
      return std::make_pair(key, GCodeIRValue());
    }
  }

  std::unique_ptr<GCodeIRModule> GCodeIRTranslator::translate(const GCodeBlock &ast) {
    this->module = std::make_unique<GCodeIRModule>();
    this->translateBlock(ast);
    return std::move(this->module);
  }

  void GCodeIRTranslator::translateNode(const GCodeNode &node) {
    if (node.is(GCodeNode::Type::Block)) {
      this->translateBlock(dynamic_cast<const GCodeBlock &>(node));
    } else if (node.is(GCodeNode::Type::Command)) {
      this->translateCommand(dynamic_cast<const GCodeCommand &>(node));
    }
  }

  void GCodeIRTranslator::translateBlock(const GCodeBlock &block) {
    std::vector<std::reference_wrapper<GCodeNode>> content;
    block.getContent(content);
    for (auto node : content) {
      this->translateNode(node.get());
    }
  }

  void GCodeIRTranslator::translateCommand(const GCodeCommand &cmd) {
    this->module->appendInstruction(GCodeIROpcode::Prologue);
    std::pair<unsigned char, GCodeIRValue> command = ir_extract_word(cmd.getCommand());
    GCodeSyscallType syscallType = static_cast<GCodeSyscallType>(command.first);
    GCodeIRValue syscallFunction = command.second;
    std::vector<std::reference_wrapper<GCodeWord>> paramList;
    cmd.getParameters(paramList);
    for (auto param : paramList) {
      std::pair<unsigned char, GCodeIRValue> parameter = ir_extract_word(param.get());
      this->module->appendInstruction(GCodeIROpcode::Push, parameter.second);
      this->module->appendInstruction(GCodeIROpcode::SetArg, GCodeIRValue(static_cast<int64_t>(parameter.first)));
    }
    this->module->appendInstruction(GCodeIROpcode::Push, syscallFunction);
    this->module->appendInstruction(GCodeIROpcode::Syscall, GCodeIRValue(static_cast<int64_t>(syscallType)));
  }
}