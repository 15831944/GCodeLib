#include "gcodelib/runtime/Translator.h"
#include <algorithm>

namespace GCodeLib::Runtime {

  class GCodeIRTranslator::Impl : public Parser::GCodeNode::Visitor {
   public:
    std::unique_ptr<GCodeIRModule> translate(const Parser::GCodeBlock &);
    void visit(const Parser::GCodeBlock &) override;
    void visit(const Parser::GCodeCommand &) override;
    void visit(const Parser::GCodeUnaryOperation &) override;
    void visit(const Parser::GCodeBinaryOperation &) override;
    void visit(const Parser::GCodeFunctionCall &) override;
    void visit(const Parser::GCodeProcedureDefinition &) override;
    void visit(const Parser::GCodeProcedureCall &) override;
    void visit(const Parser::GCodeConditional &) override;
    void visit(const Parser::GCodeWhileLoop &) override;
    void visit(const Parser::GCodeRepeatLoop &) override;
    void visit(const Parser::GCodeConstantValue &) override;
    void visit(const Parser::GCodeLabel &) override;
    void visit(const Parser::GCodeNumberedVariable &) override;
    void visit(const Parser::GCodeNamedVariable &) override;
    void visit(const Parser::GCodeNumberedVariableAssignment &) override;
    void visit(const Parser::GCodeNamedVariableAssignment &) override;
    void visit(const Parser::GCodeProcedureReturn &) override;
    void visit(const Parser::GCodeNamedStatement &) override;
    void visit(const Parser::GCodeLoopControl &) override;
   private:
    std::unique_ptr<GCodeIRModule> module;
  };

  GCodeIRTranslator::GCodeIRTranslator()
    : impl(std::make_shared<Impl>()) {}

  std::unique_ptr<GCodeIRModule> GCodeIRTranslator::translate(const Parser::GCodeBlock &ast) {
    return this->impl->translate(ast);
  }

  std::unique_ptr<GCodeIRModule> GCodeIRTranslator::Impl::translate(const Parser::GCodeBlock &ast) {
    this->module = std::make_unique<GCodeIRModule>();
    this->visit(ast);
    return std::move(this->module);
  }

  void GCodeIRTranslator::Impl::visit(const Parser::GCodeBlock &block) {
    std::vector<std::reference_wrapper<Parser::GCodeNode>> content;
    block.getContent(content);
    for (auto node : content) {
      node.get().visit(*this);
    }
  }

  void GCodeIRTranslator::Impl::visit(const Parser::GCodeCommand &cmd) {
    this->module->appendInstruction(GCodeIROpcode::Prologue);
    GCodeSyscallType syscallType = static_cast<GCodeSyscallType>(cmd.getCommand().getField());
    std::vector<std::reference_wrapper<Parser::GCodeWord>> paramList;
    cmd.getParameters(paramList);
    for (auto param : paramList) {
      param.get().getValue().visit(*this);
      this->module->appendInstruction(GCodeIROpcode::SetArg, GCodeRuntimeValue(static_cast<int64_t>(param.get().getField())));
    }
    cmd.getCommand().getValue().visit(*this);
    this->module->appendInstruction(GCodeIROpcode::Syscall, GCodeRuntimeValue(static_cast<int64_t>(syscallType)));
  }

  void GCodeIRTranslator::Impl::visit(const Parser::GCodeUnaryOperation &node) {
    node.getArgument().visit(*this);
    switch (node.getOperation()) {
      case Parser::GCodeUnaryOperation::Operation::Negate:
        this->module->appendInstruction(GCodeIROpcode::Negate);
        break;
    }
  }

  void GCodeIRTranslator::Impl::visit(const Parser::GCodeBinaryOperation &node) {
    node.getLeftArgument().visit(*this);
    node.getRightArgument().visit(*this);
    switch (node.getOperation()) {
      case Parser::GCodeBinaryOperation::Operation::Add:
        this->module->appendInstruction(GCodeIROpcode::Add);
        break;
      case Parser::GCodeBinaryOperation::Operation::Subtract:
        this->module->appendInstruction(GCodeIROpcode::Subtract);
        break;
      case Parser::GCodeBinaryOperation::Operation::Multiply:
        this->module->appendInstruction(GCodeIROpcode::Multiply);
        break;
      case Parser::GCodeBinaryOperation::Operation::Divide:
        this->module->appendInstruction(GCodeIROpcode::Divide);
        break;
      case Parser::GCodeBinaryOperation::Operation::Power:
        this->module->appendInstruction(GCodeIROpcode::Power);
        break;
      case Parser::GCodeBinaryOperation::Operation::Modulo:
        this->module->appendInstruction(GCodeIROpcode::Modulo);
        break;
      case Parser::GCodeBinaryOperation::Operation::Equals:
        this->module->appendInstruction(GCodeIROpcode::Compare);
        this->module->appendInstruction(GCodeIROpcode::Test, static_cast<int64_t>(GCodeCompare::Equals));
        break;
      case Parser::GCodeBinaryOperation::Operation::NotEquals:
        this->module->appendInstruction(GCodeIROpcode::Compare);
        this->module->appendInstruction(GCodeIROpcode::Test, static_cast<int64_t>(GCodeCompare::NotEquals));
        break;
      case Parser::GCodeBinaryOperation::Operation::Greater:
        this->module->appendInstruction(GCodeIROpcode::Compare);
        this->module->appendInstruction(GCodeIROpcode::Test, static_cast<int64_t>(GCodeCompare::Greater));
        break;
      case Parser::GCodeBinaryOperation::Operation::GreaterOrEquals:
        this->module->appendInstruction(GCodeIROpcode::Compare);
        this->module->appendInstruction(GCodeIROpcode::Test, static_cast<int64_t>(GCodeCompare::Equals) | static_cast<int64_t>(GCodeCompare::Greater));
        break;
      case Parser::GCodeBinaryOperation::Operation::Lesser:
        this->module->appendInstruction(GCodeIROpcode::Compare);
        this->module->appendInstruction(GCodeIROpcode::Test, static_cast<int64_t>(GCodeCompare::Lesser));
        break;
      case Parser::GCodeBinaryOperation::Operation::LesserOrEquals:
        this->module->appendInstruction(GCodeIROpcode::Compare);
        this->module->appendInstruction(GCodeIROpcode::Test, static_cast<int64_t>(GCodeCompare::Equals) | static_cast<int64_t>(GCodeCompare::Lesser));
        break;
      case Parser::GCodeBinaryOperation::Operation::And:
        this->module->appendInstruction(GCodeIROpcode::And);
        break;
      case Parser::GCodeBinaryOperation::Operation::Or:
        this->module->appendInstruction(GCodeIROpcode::Or);
        break;
      case Parser::GCodeBinaryOperation::Operation::Xor:
        this->module->appendInstruction(GCodeIROpcode::Xor);
        break;
    }
  }

  void GCodeIRTranslator::Impl::visit(const Parser::GCodeFunctionCall &call) {
    std::vector<std::reference_wrapper<Parser::GCodeNode>> args;
    call.getArguments(args);
    std::reverse(args.begin(), args.end());
    for (auto arg : args) {
      arg.get().visit(*this);
    }
    this->module->appendInstruction(GCodeIROpcode::Push, static_cast<int64_t>(args.size()));
    std::size_t symbol = this->module->getSymbolId(call.getFunctionIdentifier());
    this->module->appendInstruction(GCodeIROpcode::Invoke, static_cast<int64_t>(symbol));
  }

  void GCodeIRTranslator::Impl::visit(const Parser::GCodeProcedureDefinition &definition) {
    auto label = this->module->newLabel();
    label->jump();
    std::string procedureName = "procedure" + std::to_string(definition.getIdentifier());
    auto &proc = this->module->getNamedLabel(procedureName);
    this->module->registerProcedure(definition.getIdentifier(), procedureName);
    proc.bind();
    definition.getBody().visit(*this);
    std::vector<std::reference_wrapper<Parser::GCodeNode>> rets;
    definition.getReturnValues(rets);
    for (auto ret : rets) {
      ret.get().visit(*this);
    }
    this->module->appendInstruction(GCodeIROpcode::Ret, static_cast<int64_t>(rets.size()));
    label->bind();
  }

  void GCodeIRTranslator::Impl::visit(const Parser::GCodeProcedureCall &call) {
    std::vector<std::reference_wrapper<Parser::GCodeNode>> args;
    call.getArguments(args);
    for (auto arg : args) {
      arg.get().visit(*this);
    }
    call.getProcedureId().visit(*this);
    this->module->appendInstruction(GCodeIROpcode::Call, static_cast<int64_t>(args.size()));
  }

  void GCodeIRTranslator::Impl::visit(const Parser::GCodeConditional &conditional) {
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

  void GCodeIRTranslator::Impl::visit(const Parser::GCodeWhileLoop &loop) {
    auto loopStart = this->module->newLabel();
    if (!loop.isDoWhile()) {
      auto loopEnd = this->module->newLabel();
      loopEnd->jump();
      loopStart->bind();
      loop.getBody().visit(*this);
      loopEnd->bind();
    } else {
      loopStart->bind();
      loop.getBody().visit(*this);
    }
    loop.getCondition().visit(*this);
    loopStart->jumpIf();
  }

  void GCodeIRTranslator::Impl::visit(const Parser::GCodeRepeatLoop &loop) {
    auto loopStart = this->module->newLabel();
    auto loopEnd = this->module->newLabel();
    loop.getCounter().visit(*this);
    loopEnd->jump();
    loopStart->bind();
    this->module->appendInstruction(GCodeIROpcode::Push, 1L);
    this->module->appendInstruction(GCodeIROpcode::Subtract);
    loop.getBody().visit(*this);
    loopEnd->bind();
    this->module->appendInstruction(GCodeIROpcode::Dup);
    this->module->appendInstruction(GCodeIROpcode::Push, 0L);
    this->module->appendInstruction(GCodeIROpcode::Compare);
    this->module->appendInstruction(GCodeIROpcode::Test, static_cast<int64_t>(GCodeCompare::Greater));
    loopStart->jumpIf();
  }

  void GCodeIRTranslator::Impl::visit(const Parser::GCodeConstantValue &value) {
    if (value.is(Parser::GCodeNode::Type::IntegerContant)) {
      this->module->appendInstruction(GCodeIROpcode::Push, GCodeRuntimeValue(value.asInteger()));
    } else if (value.is(Parser::GCodeNode::Type::FloatContant)) {
      this->module->appendInstruction(GCodeIROpcode::Push, GCodeRuntimeValue(value.asFloat()));
    }
  }

  void GCodeIRTranslator::Impl::visit(const Parser::GCodeLabel &label) {
    label.getStatement().visit(*this);
  }

  void GCodeIRTranslator::Impl::visit(const Parser::GCodeNumberedVariable &variable) {
    this->module->appendInstruction(GCodeIROpcode::LoadNumbered, variable.getIdentifier());
  }

  void GCodeIRTranslator::Impl::visit(const Parser::GCodeNamedVariable &variable) {
    int64_t symbol = this->module->getSymbolId(variable.getIdentifier());
    this->module->appendInstruction(GCodeIROpcode::LoadNamed, symbol);
  }

  void GCodeIRTranslator::Impl::visit(const Parser::GCodeNumberedVariableAssignment &assignment) {
    assignment.getValue().visit(*this);
    this->module->appendInstruction(GCodeIROpcode::StoreNumbered, assignment.getIdentifier());
  }

  void GCodeIRTranslator::Impl::visit(const Parser::GCodeNamedVariableAssignment &assignment) {
    assignment.getValue().visit(*this);
    int64_t symbol = this->module->getSymbolId(assignment.getIdentifier());
    this->module->appendInstruction(GCodeIROpcode::StoreNamed, symbol);
  }

  void GCodeIRTranslator::Impl::visit(const Parser::GCodeProcedureReturn &ret) {
    std::vector<std::reference_wrapper<Parser::GCodeNode>> rets;
    ret.getReturnValues(rets);
    for (auto value : rets) {
      value.get().visit(*this);
    }
    this->module->appendInstruction(GCodeIROpcode::Ret, static_cast<int64_t>(rets.size()));
  }

  void GCodeIRTranslator::Impl::visit(const Parser::GCodeNamedStatement &stmt) {
    auto &start = this->module->getNamedLabel(stmt.getIdentifier() + "_start");
    auto &end = this->module->getNamedLabel(stmt.getIdentifier() + "_end");
    start.bind();
    stmt.getStatement().visit(*this);
    end.bind();
  }

  void GCodeIRTranslator::Impl::visit(const Parser::GCodeLoopControl &ctrl) {
    std::string label = ctrl.getLoopIdentifier();
    switch (ctrl.getControlType()) {
      case Parser::GCodeLoopControl::ControlType::Break:
        label += "_end";
        break;
      case Parser::GCodeLoopControl::ControlType::Continue:
        label += "_start";
        break;
    }
    auto &dest = this->module->getNamedLabel(label);
    dest.jump();
  }
}