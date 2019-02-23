#include "gcodelib/ir/Translator.h"

namespace GCodeLib {

  static std::pair<unsigned char, GCodeIRConstant> ir_extract_word(GCodeWord &word) {
    unsigned char key = word.getField();
    GCodeConstantValue &value = word.getValue();
    if (value.is(GCodeNode::Type::IntegerContant)) {
      return std::make_pair(key, GCodeIRConstant(value.asInteger()));
    } else if (value.is(GCodeNode::Type::FloatContant)) {
      return std::make_pair(key, GCodeIRConstant(value.asFloat()));
    } else {
      return std::make_pair(key, GCodeIRConstant());
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
    auto command = ir_extract_word(cmd.getCommand());
    std::map<unsigned char, GCodeIRConstant> params;
    std::vector<std::reference_wrapper<GCodeWord>> paramList;
    cmd.getParameters(paramList);
    for (auto param : paramList) {
      auto keyValue = ir_extract_word(param.get());
      params[keyValue.first] = keyValue.second;
    }
    this->module->appendInstruction(std::make_unique<GCodeIRInstruction>(GCodeSystemCommand(static_cast<GCodeSystemCommand::FunctionType>(command.first), command.second, params)));
  }
}