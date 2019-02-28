#include "gcodelib/runtime/Translator.h"
#include <algorithm>

namespace GCodeLib::Runtime {

  class GCodeIRTranslator::Impl : public Parser::GCodeNode::Visitor {
   public:
    Impl(Parser::GCodeNameMangler &);
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
    void visit(const Parser::GCodeNumberedVariable &) override;
    void visit(const Parser::GCodeNamedVariable &) override;
    void visit(const Parser::GCodeNumberedVariableAssignment &) override;
    void visit(const Parser::GCodeNamedVariableAssignment &) override;
    void visit(const Parser::GCodeProcedureReturn &) override;
    void visit(const Parser::GCodeNamedStatement &) override;
    void visit(const Parser::GCodeLoopControl &) override;
   private:
    std::unique_ptr<GCodeIRModule> module;
    Parser::GCodeNameMangler &mangler;
  };

  GCodeIRTranslator::GCodeIRTranslator(Parser::GCodeNameMangler &mangler)
    : impl(std::make_unique<Impl>(mangler)) {}

  GCodeIRTranslator::~GCodeIRTranslator() = default;

  std::unique_ptr<GCodeIRModule> GCodeIRTranslator::translate(const Parser::GCodeBlock &ast) {
    return this->impl->translate(ast);
  }

  GCodeIRTranslator::Impl::Impl(Parser::GCodeNameMangler &mangler)
    : mangler(mangler) {}

  std::unique_ptr<GCodeIRModule> GCodeIRTranslator::Impl::translate(const Parser::GCodeBlock &ast) {
    this->module = std::make_unique<GCodeIRModule>();
    this->visit(ast);
    return std::move(this->module);
  }

  void GCodeIRTranslator::Impl::visit(const Parser::GCodeBlock &block) {
    auto sourceMap = this->module->newPositionRegister(block.getPosition());
    std::vector<std::reference_wrapper<const Parser::GCodeNode>> content;
    block.getContent(content);
    for (auto node : content) {
      node.get().visit(*this);
    }
  }

  void GCodeIRTranslator::Impl::visit(const Parser::GCodeCommand &cmd) {
    auto sourceMap = this->module->newPositionRegister(cmd.getPosition());
    this->module->appendInstruction(GCodeIROpcode::Prologue);
    GCodeSyscallType syscallType = static_cast<GCodeSyscallType>(cmd.getCommand().getField());
    std::vector<std::reference_wrapper<const Parser::GCodeWord>> paramList;
    cmd.getParameters(paramList);
    for (auto param : paramList) {
      param.get().getValue().visit(*this);
      this->module->appendInstruction(GCodeIROpcode::SetArg, GCodeRuntimeValue(static_cast<int64_t>(param.get().getField())));
    }
    cmd.getCommand().getValue().visit(*this);
    this->module->appendInstruction(GCodeIROpcode::Syscall, GCodeRuntimeValue(static_cast<int64_t>(syscallType)));
  }

  void GCodeIRTranslator::Impl::visit(const Parser::GCodeUnaryOperation &node) {
    auto sourceMap = this->module->newPositionRegister(node.getPosition());
    node.getArgument().visit(*this);
    switch (node.getOperation()) {
      case Parser::GCodeUnaryOperation::Operation::Negate:
        this->module->appendInstruction(GCodeIROpcode::Negate);
        break;
    }
  }

  void GCodeIRTranslator::Impl::visit(const Parser::GCodeBinaryOperation &node) {
    auto sourceMap = this->module->newPositionRegister(node.getPosition());
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
    auto sourceMap = this->module->newPositionRegister(call.getPosition());
    std::vector<std::reference_wrapper<const Parser::GCodeNode>> args;
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
    auto sourceMap = this->module->newPositionRegister(definition.getPosition());
    auto label = this->module->newLabel();
    label->jump();
    std::string procedureName = this->mangler.getProcedureName(definition.getIdentifier());
    auto &proc = this->module->getNamedLabel(procedureName);
    this->module->registerProcedure(definition.getIdentifier(), procedureName);
    proc.bind();
    definition.getBody().visit(*this);
    std::vector<std::reference_wrapper<const Parser::GCodeNode>> rets;
    definition.getReturnValues(rets);
    for (auto ret : rets) {
      ret.get().visit(*this);
    }
    this->module->appendInstruction(GCodeIROpcode::Ret, static_cast<int64_t>(rets.size()));
    label->bind();
  }

  void GCodeIRTranslator::Impl::visit(const Parser::GCodeProcedureCall &call) {
    auto sourceMap = this->module->newPositionRegister(call.getPosition());
    std::vector<std::reference_wrapper<const Parser::GCodeNode>> args;
    call.getArguments(args);
    for (auto arg : args) {
      arg.get().visit(*this);
    }
    call.getProcedureId().visit(*this);
    this->module->appendInstruction(GCodeIROpcode::Call, static_cast<int64_t>(args.size()));
  }

  void GCodeIRTranslator::Impl::visit(const Parser::GCodeConditional &conditional) {
    auto sourceMap = this->module->newPositionRegister(conditional.getPosition());
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
    auto sourceMap = this->module->newPositionRegister(loop.getPosition());
    std::string loopName = this->mangler.getLoop(loop.getLabel());
    auto &continueLoop = this->module->getNamedLabel(this->mangler.getStatementStart(loopName));
    auto &breakLoop = this->module->getNamedLabel(this->mangler.getStatementEnd(loopName));
    auto loopStart = this->module->newLabel();
    if (!loop.isDoWhile()) {
      continueLoop.jump();
      loopStart->bind();
      loop.getBody().visit(*this);
    } else {
      loopStart->bind();
      loop.getBody().visit(*this);
    }
    continueLoop.bind();
    loop.getCondition().visit(*this);
    loopStart->jumpIf();
    breakLoop.bind();
  }

  void GCodeIRTranslator::Impl::visit(const Parser::GCodeRepeatLoop &loop) {
    auto sourceMap = this->module->newPositionRegister(loop.getPosition());
    std::string loopName = this->mangler.getLoop(loop.getLabel());
    auto &continueLoop = this->module->getNamedLabel(this->mangler.getStatementStart(loopName));
    auto &breakLoop = this->module->getNamedLabel(this->mangler.getStatementEnd(loopName));
    auto loopStart = this->module->newLabel();
    loop.getCounter().visit(*this);
    continueLoop.jump();
    loopStart->bind();
    this->module->appendInstruction(GCodeIROpcode::Push, 1L);
    this->module->appendInstruction(GCodeIROpcode::Subtract);
    loop.getBody().visit(*this);
    continueLoop.bind();
    this->module->appendInstruction(GCodeIROpcode::Dup);
    this->module->appendInstruction(GCodeIROpcode::Push, 0L);
    this->module->appendInstruction(GCodeIROpcode::Compare);
    this->module->appendInstruction(GCodeIROpcode::Test, static_cast<int64_t>(GCodeCompare::Greater));
    loopStart->jumpIf();
    breakLoop.bind();
  }

  void GCodeIRTranslator::Impl::visit(const Parser::GCodeConstantValue &value) {
    auto sourceMap = this->module->newPositionRegister(value.getPosition());
    if (value.is(Parser::GCodeNode::Type::IntegerContant)) {
      this->module->appendInstruction(GCodeIROpcode::Push, GCodeRuntimeValue(value.asInteger()));
    } else if (value.is(Parser::GCodeNode::Type::FloatContant)) {
      this->module->appendInstruction(GCodeIROpcode::Push, GCodeRuntimeValue(value.asFloat()));
    } else if (value.is(Parser::GCodeNode::Type::StringConstant)) {
      this->module->appendInstruction(GCodeIROpcode::Push, GCodeRuntimeValue(value.asString()));
    }
  }

  void GCodeIRTranslator::Impl::visit(const Parser::GCodeNumberedVariable &variable) {
    auto sourceMap = this->module->newPositionRegister(variable.getPosition());
    this->module->appendInstruction(GCodeIROpcode::LoadNumbered, variable.getIdentifier());
  }

  void GCodeIRTranslator::Impl::visit(const Parser::GCodeNamedVariable &variable) {
    auto sourceMap = this->module->newPositionRegister(variable.getPosition());
    int64_t symbol = this->module->getSymbolId(variable.getIdentifier());
    this->module->appendInstruction(GCodeIROpcode::LoadNamed, symbol);
  }

  void GCodeIRTranslator::Impl::visit(const Parser::GCodeNumberedVariableAssignment &assignment) {
    auto sourceMap = this->module->newPositionRegister(assignment.getPosition());
    assignment.getValue().visit(*this);
    this->module->appendInstruction(GCodeIROpcode::StoreNumbered, assignment.getIdentifier());
  }

  void GCodeIRTranslator::Impl::visit(const Parser::GCodeNamedVariableAssignment &assignment) {
    auto sourceMap = this->module->newPositionRegister(assignment.getPosition());
    assignment.getValue().visit(*this);
    int64_t symbol = this->module->getSymbolId(assignment.getIdentifier());
    this->module->appendInstruction(GCodeIROpcode::StoreNamed, symbol);
  }

  void GCodeIRTranslator::Impl::visit(const Parser::GCodeProcedureReturn &ret) {
    auto sourceMap = this->module->newPositionRegister(ret.getPosition());
    std::vector<std::reference_wrapper<const Parser::GCodeNode>> rets;
    ret.getReturnValues(rets);
    for (auto value : rets) {
      value.get().visit(*this);
    }
    this->module->appendInstruction(GCodeIROpcode::Ret, static_cast<int64_t>(rets.size()));
  }

  void GCodeIRTranslator::Impl::visit(const Parser::GCodeNamedStatement &stmt) {
    auto sourceMap = this->module->newPositionRegister(stmt.getPosition());
    auto &start = this->module->getNamedLabel(this->mangler.getStatementStart(stmt.getIdentifier()));
    auto &end = this->module->getNamedLabel(this->mangler.getStatementEnd(stmt.getIdentifier()));
    start.bind();
    stmt.getStatement().visit(*this);
    end.bind();
  }

  void GCodeIRTranslator::Impl::visit(const Parser::GCodeLoopControl &ctrl) {
    auto sourceMap = this->module->newPositionRegister(ctrl.getPosition());
    std::string label = this->mangler.getLoop(ctrl.getLoopIdentifier());
    switch (ctrl.getControlType()) {
      case Parser::GCodeLoopControl::ControlType::Break:
        label = this->mangler.getStatementEnd(label);
        break;
      case Parser::GCodeLoopControl::ControlType::Continue:
        label = this->mangler.getStatementStart(label);
        break;
    }
    auto &dest = this->module->getNamedLabel(label);
    dest.jump();
  }
}