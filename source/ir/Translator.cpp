#include "gcodelib/ir/Translator.h"

namespace GCodeLib {

  static void ir_translate_block(std::vector<std::unique_ptr<GCodeIRInstruction>> &, GCodeBlock &);
  static void ir_translate_command(std::vector<std::unique_ptr<GCodeIRInstruction>> &, GCodeCommand &);

  static void ir_translate_node(std::vector<std::unique_ptr<GCodeIRInstruction>> &ir, GCodeNode &node) {
    if (node.is(GCodeNode::Type::Block)) {
      ir_translate_block(ir, dynamic_cast<GCodeBlock &>(node));
    } else if (node.is(GCodeNode::Type::Command)) {
      ir_translate_command(ir, dynamic_cast<GCodeCommand &>(node));
    }
  }

  static void ir_translate_block(std::vector<std::unique_ptr<GCodeIRInstruction>> &module, GCodeBlock &block) {
    std::vector<std::reference_wrapper<GCodeNode>> content;
    block.getContent(content);
    for (auto node : content) {
      ir_translate_node(module, node.get());
    }
  }

  static std::pair<unsigned char, GCodeIRConstant> ir_extract_word(GCodeWord &word) {
    unsigned char key = word.getField();
    GCodeNode &value = word.getValue();
    if (value.is(GCodeNode::Type::IntegerContant)) {
      return std::make_pair(key, GCodeIRConstant(dynamic_cast<GCodeIntegerContant &>(value).getValue()));
    } else if (value.is(GCodeNode::Type::FloatContant)) {
      return std::make_pair(key, GCodeIRConstant(dynamic_cast<GCodeFloatContant &>(value).getValue()));
    } else {
      return std::make_pair(key, GCodeIRConstant());
    }
  }

  static void ir_translate_command(std::vector<std::unique_ptr<GCodeIRInstruction>> &module, GCodeCommand &cmd) {
    auto command = ir_extract_word(dynamic_cast<GCodeWord &>(cmd.getCommand()));
    std::map<unsigned char, GCodeIRConstant> params;
    std::vector<std::reference_wrapper<GCodeNode>> paramList;
    cmd.getParameters(paramList);
    for (auto param : paramList) {
      auto keyValue = ir_extract_word(dynamic_cast<GCodeWord &>(param.get()));
      params[keyValue.first] = keyValue.second;
    }
    module.push_back(std::make_unique<GCodeIRCommand>(static_cast<GCodeIRCommand::FunctionType>(command.first), command.second, params));
  }

  std::unique_ptr<GCodeIRModule> GCodeIRTranslator::translate(GCodeNode &node) {
    std::vector<std::unique_ptr<GCodeIRInstruction>> code;
    ir_translate_node(code, node);
    return std::make_unique<GCodeIRModule>(std::move(code));
  }
}