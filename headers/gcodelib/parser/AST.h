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

#ifndef GCODELIB_PARSER_AST_H_
#define GCODELIB_PARSER_AST_H_

#include "gcodelib/parser/Source.h"
#include <memory>
#include <functional>
#include <vector>
#include <variant>
#include <iosfwd>

namespace GCodeLib::Parser {

  class GCodeNode {
   public:
    enum class Type {
      NoOperaation,
      IntegerContant,
      FloatContant,
      StringConstant,
      NumberedVariable,
      NamedVariable,
      UnaryOperation,
      BinaryOperation,
      FunctionCall,
      Word,
      Command,
      Block,
      NamedStatement,
      ProcedureDefinition,
      ProcedureReturn,
      ProcedureCall,
      NumberedAssignment,
      NamedAssignment,
      Conditional,
      WhileLoop,
      RepeatLoop,
      LoopControl
    };

    class Visitor;

    GCodeNode(Type, const SourcePosition &);
    virtual ~GCodeNode() = default;
    Type getType() const;
    bool is(Type) const;
    const SourcePosition &getPosition() const;

    virtual void visit(Visitor &) const = 0;
    friend std::ostream &operator<<(std::ostream &, const GCodeNode &);
   protected:
    virtual void dump(std::ostream &) const = 0;
   private:
    Type type;
    SourcePosition position;
  };

  template <typename T>
  class GCodeVisitableNode : public GCodeNode {
   public:
    using GCodeNode::GCodeNode;
    void visit(Visitor &) const override;
  };

  class GCodeNoOperation;
  class GCodeNoOperation : public GCodeVisitableNode<GCodeNoOperation> {
   public:
    GCodeNoOperation(const SourcePosition &);
   protected:
    void dump(std::ostream &) const override;
  };

  class GCodeConstantValue;
  class GCodeConstantValue : public GCodeVisitableNode<GCodeConstantValue> {
   public:
    GCodeConstantValue(int64_t, const SourcePosition &);
    GCodeConstantValue(double, const SourcePosition &);
    GCodeConstantValue(const std::string &, const SourcePosition &);

    int64_t asInteger(int64_t = 0) const;
    double asFloat(double = 0.0) const;
    const std::string &asString(const std::string & = "") const;
   protected:
    void dump(std::ostream &) const override;
   private:
    std::variant<int64_t, double, std::string> value;
  };

  template <class Self, GCodeNode::Type NodeType, typename ValueType>
  class GCodeVariable : public GCodeVisitableNode<Self> {
   public:
    GCodeVariable(const ValueType &identifier, const SourcePosition &position)
      : GCodeVisitableNode<Self>::GCodeVisitableNode(NodeType, position), identifier(identifier) {}

    const ValueType &getIdentifier() const {
      return this->identifier;
    }
   private:
    ValueType identifier;
  };

  class GCodeNamedVariable;
  class GCodeNumberedVariable;

  class GCodeNamedVariable : public GCodeVariable<GCodeNamedVariable, GCodeNode::Type::NamedVariable, std::string> {
   public:
    using GCodeVariable<GCodeNamedVariable, GCodeNode::Type::NamedVariable, std::string>::GCodeVariable;
   protected:
    void dump(std::ostream &) const override;
  };

  class GCodeNumberedVariable : public GCodeVariable<GCodeNumberedVariable, GCodeNode::Type::NumberedVariable, int64_t> {
   public:
    using GCodeVariable<GCodeNumberedVariable, GCodeNode::Type::NumberedVariable, int64_t>::GCodeVariable;
   protected:
    void dump(std::ostream &) const override;
  };

  template <class Self, GCodeNode::Type NodeType, typename KeyType>
  class GCodeVariableAssignment : public GCodeVisitableNode<Self> {
   public:
    GCodeVariableAssignment(const KeyType &key, std::unique_ptr<GCodeNode> value, const SourcePosition &position)
      : GCodeVisitableNode<Self>::GCodeVisitableNode(NodeType, position), key(key), value(std::move(value)) {}
    
    const KeyType &getIdentifier() const {
      return this->key;
    }

    const GCodeNode &getValue() const {
      return *this->value;
    }
   private:
    KeyType key;
    std::unique_ptr<GCodeNode> value;
  };

  class GCodeNamedVariableAssignment;
  class GCodeNumberedVariableAssignment;

  class GCodeNamedVariableAssignment : public GCodeVariableAssignment<GCodeNamedVariableAssignment, GCodeNode::Type::NamedAssignment, std::string> {
   public:
    using GCodeVariableAssignment<GCodeNamedVariableAssignment, GCodeNode::Type::NamedAssignment, std::string>::GCodeVariableAssignment;
   protected:
    void dump(std::ostream &) const override;
  };

  class GCodeNumberedVariableAssignment : public GCodeVariableAssignment<GCodeNumberedVariableAssignment, GCodeNode::Type::NumberedAssignment, int64_t> {
   public:
    using GCodeVariableAssignment<GCodeNumberedVariableAssignment, GCodeNode::Type::NumberedAssignment, int64_t>::GCodeVariableAssignment;
   protected:
    void dump(std::ostream &) const override;
  };

  class GCodeUnaryOperation;
  class GCodeUnaryOperation : public GCodeVisitableNode<GCodeUnaryOperation> {
   public:
    enum class Operation {
      Negate
    };

    GCodeUnaryOperation(Operation, std::unique_ptr<GCodeNode>, const SourcePosition &);
    Operation getOperation() const;
    const GCodeNode &getArgument() const;
   protected:
    void dump(std::ostream &) const override;
   private:
    Operation operation;
    std::unique_ptr<GCodeNode> argument;
  };

  class GCodeBinaryOperation;
  class GCodeBinaryOperation : public GCodeVisitableNode<GCodeBinaryOperation> {
   public:
    enum class Operation {
      Add,
      Subtract,
      Multiply,
      Divide,
      Power,
      Modulo,
      Equals,
      NotEquals,
      Greater,
      GreaterOrEquals,
      Lesser,
      LesserOrEquals,
      And,
      Or,
      Xor
    };

    GCodeBinaryOperation(Operation, std::unique_ptr<GCodeNode>, std::unique_ptr<GCodeNode>, const SourcePosition &);
    Operation getOperation() const;
    const GCodeNode &getLeftArgument() const;
    const GCodeNode &getRightArgument() const;
   protected:
    void dump(std::ostream &) const override;
   private:
    Operation operation;
    std::unique_ptr<GCodeNode> leftArgument;
    std::unique_ptr<GCodeNode> rightArgument;
  };

  class GCodeFunctionCall;
  class GCodeFunctionCall : public GCodeVisitableNode<GCodeFunctionCall> {
   public:
    GCodeFunctionCall(const std::string &, std::vector<std::unique_ptr<GCodeNode>>, const SourcePosition &);
    const std::string &getFunctionIdentifier() const;
    void getArguments(std::vector<std::reference_wrapper<const GCodeNode>> &) const;
   protected:
    void dump(std::ostream &) const override;
   private:
    std::string functionIdentifier;
    std::vector<std::unique_ptr<GCodeNode>> arguments;
  };

  class GCodeWord;
  class GCodeWord : public GCodeVisitableNode<GCodeWord> {
   public:
    GCodeWord(unsigned char, std::unique_ptr<GCodeNode>, const SourcePosition &);
    unsigned char getField() const;
    const GCodeNode &getValue() const;
   protected:
    void dump(std::ostream &) const override;
   private:
    unsigned char field;
    std::unique_ptr<GCodeNode> value;
  };

  class GCodeCommand;
  class GCodeCommand : public GCodeVisitableNode<GCodeCommand> {
   public:
    GCodeCommand(std::unique_ptr<GCodeWord>, std::vector<std::unique_ptr<GCodeWord>>, const SourcePosition &);
    const GCodeWord &getCommand() const;
    void getParameters(std::vector<std::reference_wrapper<const GCodeWord>> &) const;
   protected:
    void dump(std::ostream &) const override;
   private:
    std::unique_ptr<GCodeWord> command;
    std::vector<std::unique_ptr<GCodeWord>> parameters;
  };

  class GCodeBlock;
  class GCodeBlock : public GCodeVisitableNode<GCodeBlock> {
   public:
    GCodeBlock(std::vector<std::unique_ptr<GCodeNode>>, const SourcePosition &);
    void getContent(std::vector<std::reference_wrapper<const GCodeNode>> &) const;
   protected:
    void dump(std::ostream &) const override;
   private:
    std::vector<std::unique_ptr<GCodeNode>> content;
  };

  class GCodeNamedStatement;
  class GCodeNamedStatement : public GCodeVisitableNode<GCodeNamedStatement> {
   public:
    GCodeNamedStatement(const std::string &, std::unique_ptr<GCodeNode>, const SourcePosition &);
    const std::string &getIdentifier() const;
    const GCodeNode &getStatement() const;
   protected:
    void dump(std::ostream &) const override;
   private:
    std::string identifier;
    std::unique_ptr<GCodeNode> statement;
  };

  class GCodeProcedureDefinition;
  class GCodeProcedureDefinition : public GCodeVisitableNode<GCodeProcedureDefinition> {
   public:
    GCodeProcedureDefinition(int64_t, std::unique_ptr<GCodeNode>, std::vector<std::unique_ptr<GCodeNode>>, const SourcePosition &);
    int64_t getIdentifier() const;
    const GCodeNode &getBody() const;
    void getReturnValues(std::vector<std::reference_wrapper<const GCodeNode>> &) const;
   protected:
    void dump(std::ostream &) const override;
   private:
    int64_t identifier;
    std::unique_ptr<GCodeNode> body;
    std::vector<std::unique_ptr<GCodeNode>> retValues;
  };

  class GCodeProcedureReturn;
  class GCodeProcedureReturn : public GCodeVisitableNode<GCodeProcedureReturn> {
   public:
    GCodeProcedureReturn(std::vector<std::unique_ptr<GCodeNode>>, const SourcePosition &);
    void getReturnValues(std::vector<std::reference_wrapper<const GCodeNode>> &) const;
   protected:
    void dump(std::ostream &) const override;
   private:
    std::vector<std::unique_ptr<GCodeNode>> returnValues;
  };

  class GCodeProcedureCall;
  class GCodeProcedureCall : public GCodeVisitableNode<GCodeProcedureCall> {
   public:
    GCodeProcedureCall(std::unique_ptr<GCodeNode>, std::vector<std::unique_ptr<GCodeNode>>, const SourcePosition &);
    const GCodeNode &getProcedureId() const;
    void getArguments(std::vector<std::reference_wrapper<const GCodeNode>> &) const;
   protected:
    void dump(std::ostream &) const override;
   private:
    std::unique_ptr<GCodeNode> procedureId;
    std::vector<std::unique_ptr<GCodeNode>> args;
  };

  class GCodeConditional;
  class GCodeConditional : public GCodeVisitableNode<GCodeConditional> {
   public:
    GCodeConditional(std::unique_ptr<GCodeNode>, std::unique_ptr<GCodeNode>, std::unique_ptr<GCodeNode>, const SourcePosition &);
    const GCodeNode &getCondition() const;
    const GCodeNode &getThenBody() const;
    const GCodeNode *getElseBody() const;
   protected:
    void dump(std::ostream &) const override;
   private:
    std::unique_ptr<GCodeNode> condition;
    std::unique_ptr<GCodeNode> thenBody;
    std::unique_ptr<GCodeNode> elseBody;
  };

  template <class Self, GCodeNode::Type NodeType, typename Label>
  class GCodeLoop : public GCodeVisitableNode<Self> {
   public:
    GCodeLoop(Label label, const SourcePosition &position)
      : GCodeVisitableNode<Self>::GCodeVisitableNode(NodeType, position), label(label) {}
    
    Label getLabel() const {
      return this->label;
    }
   private:
    Label label;
  };

  class GCodeWhileLoop;
  class GCodeWhileLoop : public GCodeLoop<GCodeWhileLoop, GCodeNode::Type::WhileLoop, int64_t> {
   public:
    GCodeWhileLoop(int64_t, std::unique_ptr<GCodeNode>, std::unique_ptr<GCodeNode>, bool, const SourcePosition &);
    const GCodeNode &getCondition() const;
    const GCodeNode &getBody() const;
    bool isDoWhile() const;
   protected:
    void dump(std::ostream &) const override;
   private:
    std::unique_ptr<GCodeNode> condition;
    std::unique_ptr<GCodeNode> body;
    bool doWhileLoop;
  };

  class GCodeRepeatLoop;
  class GCodeRepeatLoop : public GCodeLoop<GCodeRepeatLoop, GCodeNode::Type::RepeatLoop, int64_t> {
   public:
    GCodeRepeatLoop(int64_t, std::unique_ptr<GCodeNode>, std::unique_ptr<GCodeNode>, const SourcePosition &);
    const GCodeNode &getCounter() const;
    const GCodeNode &getBody() const;
   protected:
    void dump(std::ostream &) const override;
   private:
    std::unique_ptr<GCodeNode> counter;
    std::unique_ptr<GCodeNode> body;
  };

  class GCodeLoopControl;
  class GCodeLoopControl : public GCodeVisitableNode<GCodeLoopControl> {
   public:
    enum class ControlType {
      Break,
      Continue
    };

    GCodeLoopControl(int64_t, ControlType, const SourcePosition &);
    int64_t getLoopIdentifier() const;
    ControlType getControlType() const;
   protected:
    void dump(std::ostream &) const override;
   private:
    int64_t identifier;
    ControlType controlType;
  };

  class GCodeNode::Visitor {
   public:
    virtual ~Visitor() = default;
    virtual void visit(const GCodeNoOperation &) {}
    virtual void visit(const GCodeConstantValue &) {}
    virtual void visit(const GCodeNamedVariable &) {}
    virtual void visit(const GCodeNumberedVariable &) {}
    virtual void visit(const GCodeUnaryOperation &) {}
    virtual void visit(const GCodeBinaryOperation &) {}
    virtual void visit(const GCodeFunctionCall &) {}
    virtual void visit(const GCodeWord &) {}
    virtual void visit(const GCodeCommand &) {}
    virtual void visit(const GCodeBlock &) {}
    virtual void visit(const GCodeNamedStatement &) {}
    virtual void visit(const GCodeProcedureDefinition &) {}
    virtual void visit(const GCodeProcedureReturn &) {}
    virtual void visit(const GCodeProcedureCall &) {}
    virtual void visit(const GCodeConditional &) {}
    virtual void visit(const GCodeWhileLoop &) {}
    virtual void visit(const GCodeRepeatLoop &) {}
    virtual void visit(const GCodeLoopControl &) {}
    virtual void visit(const GCodeNumberedVariableAssignment &) {}
    virtual void visit(const GCodeNamedVariableAssignment &) {}
  };

  template <typename T>
  void GCodeVisitableNode<T>::visit(Visitor &visitor) const {
    visitor.visit(*static_cast<const T *>(this));
  }
}

#endif