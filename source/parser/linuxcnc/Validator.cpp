/*
  SPDX short identifier: MIT
  Copyright 2019 Jevgēnijs Protopopovs
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/

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