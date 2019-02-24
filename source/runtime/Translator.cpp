#include "gcodelib/runtime/Translator.h"

namespace GCodeLib {

  class GCodeIRTranslator::Impl : public GCodeNode::Visitor {
   public:
    std::unique_ptr<GCodeIRModule> translate(const GCodeBlock &);
    void visit(const GCodeBlock &) override;
    void visit(const GCodeCommand &) override;
    void visit(const GCodeConstantValue &) override;
   private:
    std::unique_ptr<GCodeIRModule> module;
  };

  GCodeIRTranslator::GCodeIRTranslator()
    : impl(std::make_shared<Impl>()) {}

  std::unique_ptr<GCodeIRModule> GCodeIRTranslator::translate(const GCodeBlock &ast) {
    return this->impl->translate(ast);
  }

  std::unique_ptr<GCodeIRModule> GCodeIRTranslator::Impl::translate(const GCodeBlock &ast) {
    this->module = std::make_unique<GCodeIRModule>();
    this->visit(ast);
    return std::move(this->module);
  }

  void GCodeIRTranslator::Impl::visit(const GCodeBlock &block) {
    std::vector<std::reference_wrapper<GCodeNode>> content;
    block.getContent(content);
    for (auto node : content) {
      node.get().visit(*this);
    }
  }

  void GCodeIRTranslator::Impl::visit(const GCodeCommand &cmd) {
    this->module->appendInstruction(GCodeIROpcode::Prologue);
    GCodeSyscallType syscallType = static_cast<GCodeSyscallType>(cmd.getCommand().getField());
    std::vector<std::reference_wrapper<GCodeWord>> paramList;
    cmd.getParameters(paramList);
    for (auto param : paramList) {
      param.get().getValue().visit(*this);
      this->module->appendInstruction(GCodeIROpcode::SetArg, GCodeRuntimeValue(static_cast<int64_t>(param.get().getField())));
    }
    cmd.getCommand().getValue().visit(*this);
    this->module->appendInstruction(GCodeIROpcode::Syscall, GCodeRuntimeValue(static_cast<int64_t>(syscallType)));
  }

  void GCodeIRTranslator::Impl::visit(const GCodeConstantValue &value) {
    if (value.is(GCodeNode::Type::IntegerContant)) {
      this->module->appendInstruction(GCodeIROpcode::Push, GCodeRuntimeValue(value.asInteger()));
    } else if (value.is(GCodeNode::Type::FloatContant)) {
      this->module->appendInstruction(GCodeIROpcode::Push, GCodeRuntimeValue(value.asFloat()));
    }
  }
}