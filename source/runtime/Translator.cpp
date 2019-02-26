#include "gcodelib/runtime/Translator.h"
#include <algorithm>

namespace GCodeLib {

  class GCodeIRTranslator::Impl : public GCodeNode::Visitor {
   public:
    std::unique_ptr<GCodeIRModule> translate(const GCodeBlock &);
    void visit(const GCodeBlock &) override;
    void visit(const GCodeCommand &) override;
    void visit(const GCodeUnaryOperation &) override;
    void visit(const GCodeBinaryOperation &) override;
    void visit(const GCodeFunctionCall &) override;
    void visit(const GCodeProcedureDefinition &) override;
    void visit(const GCodeProcedureCall &) override;
    void visit(const GCodeConditional &) override;
    void visit(const GCodeWhileLoop &) override;
    void visit(const GCodeConstantValue &) override;
    void visit(const GCodeLabel &) override;
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

  void GCodeIRTranslator::Impl::visit(const GCodeUnaryOperation &node) {
    node.getArgument().visit(*this);
    switch (node.getOperation()) {
      case GCodeUnaryOperation::Operation::Negate:
        this->module->appendInstruction(GCodeIROpcode::Negate);
        break;
    }
  }

  void GCodeIRTranslator::Impl::visit(const GCodeBinaryOperation &node) {
    node.getLeftArgument().visit(*this);
    node.getRightArgument().visit(*this);
    switch (node.getOperation()) {
      case GCodeBinaryOperation::Operation::Add:
        this->module->appendInstruction(GCodeIROpcode::Add);
        break;
      case GCodeBinaryOperation::Operation::Subtract:
        this->module->appendInstruction(GCodeIROpcode::Subtract);
        break;
      case GCodeBinaryOperation::Operation::Multiply:
        this->module->appendInstruction(GCodeIROpcode::Multiply);
        break;
      case GCodeBinaryOperation::Operation::Divide:
        this->module->appendInstruction(GCodeIROpcode::Divide);
        break;
      case GCodeBinaryOperation::Operation::Power:
        this->module->appendInstruction(GCodeIROpcode::Power);
        break;
      case GCodeBinaryOperation::Operation::Modulo:
        this->module->appendInstruction(GCodeIROpcode::Modulo);
        break;
      case GCodeBinaryOperation::Operation::Equals:
        this->module->appendInstruction(GCodeIROpcode::Compare);
        this->module->appendInstruction(GCodeIROpcode::Test, static_cast<int64_t>(GCodeCompare::Equals));
        break;
      case GCodeBinaryOperation::Operation::NotEquals:
        this->module->appendInstruction(GCodeIROpcode::Compare);
        this->module->appendInstruction(GCodeIROpcode::Test, static_cast<int64_t>(GCodeCompare::NotEquals));
        break;
      case GCodeBinaryOperation::Operation::Greater:
        this->module->appendInstruction(GCodeIROpcode::Compare);
        this->module->appendInstruction(GCodeIROpcode::Test, static_cast<int64_t>(GCodeCompare::Greater));
        break;
      case GCodeBinaryOperation::Operation::GreaterOrEquals:
        this->module->appendInstruction(GCodeIROpcode::Compare);
        this->module->appendInstruction(GCodeIROpcode::Test, static_cast<int64_t>(GCodeCompare::Equals) | static_cast<int64_t>(GCodeCompare::Greater));
        break;
      case GCodeBinaryOperation::Operation::Lesser:
        this->module->appendInstruction(GCodeIROpcode::Compare);
        this->module->appendInstruction(GCodeIROpcode::Test, static_cast<int64_t>(GCodeCompare::Lesser));
        break;
      case GCodeBinaryOperation::Operation::LesserOrEquals:
        this->module->appendInstruction(GCodeIROpcode::Compare);
        this->module->appendInstruction(GCodeIROpcode::Test, static_cast<int64_t>(GCodeCompare::Equals) | static_cast<int64_t>(GCodeCompare::Lesser));
        break;
      case GCodeBinaryOperation::Operation::And:
        this->module->appendInstruction(GCodeIROpcode::And);
        break;
      case GCodeBinaryOperation::Operation::Or:
        this->module->appendInstruction(GCodeIROpcode::Or);
        break;
      case GCodeBinaryOperation::Operation::Xor:
        this->module->appendInstruction(GCodeIROpcode::Xor);
        break;
    }
  }

  void GCodeIRTranslator::Impl::visit(const GCodeFunctionCall &call) {
    std::vector<std::reference_wrapper<GCodeNode>> args;
    call.getArguments(args);
    std::reverse(args.begin(), args.end());
    for (auto arg : args) {
      arg.get().visit(*this);
    }
    this->module->appendInstruction(GCodeIROpcode::Push, static_cast<int64_t>(args.size()));
    std::size_t symbol = this->module->getSymbolId(call.getFunctionIdentifier());
    this->module->appendInstruction(GCodeIROpcode::Invoke, static_cast<int64_t>(symbol));
  }

  void GCodeIRTranslator::Impl::visit(const GCodeProcedureDefinition &definition) {
    auto label = this->module->newLabel();
    label->jump();
    auto &proc = this->module->getProcedure(definition.getIdentifier());
    proc.bind();
    definition.getBody().visit(*this);
    // std::vector<std::reference_wrapper<GCodeNode>> rets;
    // definition.getReturnValues(rets);
    // std::reverse(rets.begin(), rets.end());
    // for (auto ret : rets) {
    //   ret.get().visit(*this);
    // }
    this->module->appendInstruction(GCodeIROpcode::Ret);
    label->bind();
  }

  void GCodeIRTranslator::Impl::visit(const GCodeProcedureCall &call) {
    call.getProcedureId().visit(*this);
    this->module->appendInstruction(GCodeIROpcode::Call);
  }

  void GCodeIRTranslator::Impl::visit(const GCodeConditional &conditional) {
    auto endifLabel = this->module->newLabel();
    conditional.getCondition().visit(*this);
    this->module->appendInstruction(GCodeIROpcode::Not);
    if (conditional.getElseBody() != nullptr) {
      auto elseLabel = this->module->newLabel();
      elseLabel->jumpIf();
      conditional.getThenBody().visit(*this);
      endifLabel->jump();
      elseLabel->bind();
      conditional.getElseBody()->visit(*this);
    } else {
      endifLabel->jumpIf();
      conditional.getThenBody().visit(*this);
    }
    endifLabel->bind();
  }

  void GCodeIRTranslator::Impl::visit(const GCodeWhileLoop &loop) {
    auto loopStart = this->module->newLabel();
    auto loopEnd = this->module->newLabel();
    loopEnd->jump();
    loopStart->bind();
    loop.getBody().visit(*this);
    loopEnd->bind();
    loop.getCondition().visit(*this);
    loopStart->jumpIf();
  }

  void GCodeIRTranslator::Impl::visit(const GCodeConstantValue &value) {
    if (value.is(GCodeNode::Type::IntegerContant)) {
      this->module->appendInstruction(GCodeIROpcode::Push, GCodeRuntimeValue(value.asInteger()));
    } else if (value.is(GCodeNode::Type::FloatContant)) {
      this->module->appendInstruction(GCodeIROpcode::Push, GCodeRuntimeValue(value.asFloat()));
    }
  }

  void GCodeIRTranslator::Impl::visit(const GCodeLabel &label) {
    label.getStatement().visit(*this);
  }
}