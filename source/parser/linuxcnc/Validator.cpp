#include "gcodelib/parser/linuxcnc/Validator.h"
#include "gcodelib/parser/Error.h"
#include <algorithm>
#include <optional>

namespace GCodeLib::Parser::LinuxCNC {

  class GCodeLCNCValidator::Impl : public GCodeNode::Visitor {
    static constexpr unsigned int MaxReturns = 30;
   public:
    void visit(const GCodeBlock &block) override {
      std::vector<std::reference_wrapper<const GCodeNode>> content;
      block.getContent(content);
      std::for_each(content.begin(), content.end(), [this](auto ref) { ref.get().visit(*this); });
    }

    void visit(const GCodeNamedStatement &stmt) override {
      stmt.getStatement().visit(*this);
    }

    void visit(const GCodeProcedureDefinition &proc) override {
      if (this->currentProcedure.has_value()) {
        throw GCodeParseException("Nested procedure definitions are not allowed", proc.getPosition());
      }
      this->currentProcedure = proc.getIdentifier();
      proc.getBody().visit(*this);
      this->currentProcedure.reset();
      std::vector<std::reference_wrapper<const GCodeNode>> returnValues;
      proc.getReturnValues(returnValues);
      if (returnValues.size() > MaxReturns) {
        throw GCodeParseException("Returned value count should not exceed " + std::to_string(MaxReturns), proc.getPosition());
      }
    }

    void visit(const GCodeProcedureReturn &retStmt) override {
      if (!this->currentProcedure.has_value()) {
        throw GCodeParseException("Return statement must be located within procedure definition", retStmt.getPosition());
      }
      std::vector<std::reference_wrapper<const GCodeNode>> returnValues;
      retStmt.getReturnValues(returnValues);
      if (returnValues.size() > MaxReturns) {
        throw GCodeParseException("Returned value count should not exceed " + std::to_string(MaxReturns), retStmt.getPosition());
      }
    }

    void visit(const GCodeConditional &cond) override {
      cond.getThenBody().visit(*this);
      if (cond.getElseBody()) {
        cond.getElseBody()->visit(*this);
      }
    }

    void visit(const GCodeWhileLoop &loop) override {
      this->loops.push_back(loop.getLabel());
      loop.getBody().visit(*this);
      this->loops.pop_back();
    }

    void visit(const GCodeRepeatLoop &loop) override {
      this->loops.push_back(loop.getLabel());
      loop.getBody().visit(*this);
      this->loops.pop_back();
    }

    void visit(const GCodeLoopControl &ctrl) override {
      if (std::find(this->loops.begin(), this->loops.end(), ctrl.getLoopIdentifier()) == this->loops.end()) {
        throw GCodeParseException("Continue/Break label must correspont to surrounding loop", ctrl.getPosition());
      }
    }

    void visit(const GCodeProcedureCall &call) {
      std::vector<std::reference_wrapper<const GCodeNode>> args;
      call.getArguments(args);
      if (args.size() > MaxReturns) {
        throw GCodeParseException("Procedure argument count should not exceed " + std::to_string(MaxReturns), call.getPosition());
      }
    }
   private:
    std::optional<int64_t> currentProcedure;
    std::vector<int64_t> loops;
  };

  GCodeLCNCValidator::GCodeLCNCValidator()
    : impl(std::make_unique<Impl>()) {}
  
  GCodeLCNCValidator::~GCodeLCNCValidator() = default;
  
  void GCodeLCNCValidator::validate(const GCodeNode &root) {
    root.visit(*this->impl);
  }
}