#ifndef GCODELIB_PARSER_AST_H_
#define GCODELIB_PARSER_AST_H_

#include "gcodelib/parser/Source.h"
#include <memory>
#include <functional>
#include <vector>
#include <variant>
#include <iosfwd>
#include <set>

namespace GCodeLib {

  class GCodeNode {
   public:
    enum class Type {
      IntegerContant,
      FloatContant,
      UnaryOperation,
      BinaryOperation,
      FunctionCall,
      Word,
      Command,
      Block,
      ProcedureDefinition,
      ProcedureCall,
      Conditional,
      WhileLoop
    };

    class Visitor;

    GCodeNode(Type, const SourcePosition &);
    virtual ~GCodeNode() = default;
    Type getType() const;
    bool is(Type) const;
    const SourcePosition &getPosition() const;
    const std::set<uint32_t> &getLabels() const;

    void addLabel(uint32_t);

    virtual void visit(Visitor &) = 0;
    friend std::ostream &operator<<(std::ostream &, const GCodeNode &);
   protected:
    virtual void dump(std::ostream &) const = 0;
   private:
    Type type;
    SourcePosition position;
    std::set<uint32_t> labels;
  };

  class GCodeConstantValue : public GCodeNode {
   public:
    GCodeConstantValue(int64_t, const SourcePosition &);
    GCodeConstantValue(double, const SourcePosition &);

    int64_t asInteger(int64_t = 0) const;
    double asFloat(double = 0.0) const;
    void visit(Visitor &) override;
   protected:
    void dump(std::ostream &) const override;
   private:
    std::variant<int64_t, double> value;
  };

  class GCodeUnaryOperation : public GCodeNode {
   public:
    enum class Operation {
      Negate = '-'
    };

    GCodeUnaryOperation(Operation, std::unique_ptr<GCodeNode>, const SourcePosition &);
    Operation getOperation() const;
    GCodeNode &getArgument() const;
    void visit(Visitor &) override;
   protected:
    void dump(std::ostream &) const override;
   private:
    Operation operation;
    std::unique_ptr<GCodeNode> argument;
  };

  class GCodeBinaryOperation : public GCodeNode {
   public:
    enum class Operation {
      Add = '+',
      Subtract = '-',
      Multiply = '*',
      Divide = '/',
      Power = '^',
      Modulo = '%',
      Equals = 'E',
      NotEquals = 'N',
      Greater = 'G',
      GreaterOrEquals = 'g',
      Lesser = 'L',
      LesserOrEquals = 'l',
      And = '&',
      Or = '|',
      Xor = 'X'
    };

    GCodeBinaryOperation(Operation, std::unique_ptr<GCodeNode>, std::unique_ptr<GCodeNode>, const SourcePosition &);
    Operation getOperation() const;
    GCodeNode &getLeftArgument() const;
    GCodeNode &getRightArgument() const;
    void visit(Visitor &) override;
   protected:
    void dump(std::ostream &) const override;
   private:
    Operation operation;
    std::unique_ptr<GCodeNode> leftArgument;
    std::unique_ptr<GCodeNode> rightArgument;
  };

  class GCodeFunctionCall : public GCodeNode {
   public:
    GCodeFunctionCall(const std::string &, std::vector<std::unique_ptr<GCodeNode>>, const SourcePosition &);
    const std::string &getFunctionIdentifier() const;
    void getArguments(std::vector<std::reference_wrapper<GCodeNode>> &) const;
    void visit(Visitor &) override;
   protected:
    void dump(std::ostream &) const override;
   private:
    std::string functionIdentifier;
    std::vector<std::unique_ptr<GCodeNode>> arguments;
  };

  class GCodeWord : public GCodeNode {
   public:
    GCodeWord(unsigned char, std::unique_ptr<GCodeNode>, const SourcePosition &);
    unsigned char getField() const;
    GCodeNode &getValue() const;
    void visit(Visitor &) override;
   protected:
    void dump(std::ostream &) const override;
   private:
    unsigned char field;
    std::unique_ptr<GCodeNode> value;
  };

  class GCodeCommand : public GCodeNode {
   public:
    GCodeCommand(std::unique_ptr<GCodeWord>, std::vector<std::unique_ptr<GCodeWord>>, const SourcePosition &);
    GCodeWord &getCommand() const;
    void getParameters(std::vector<std::reference_wrapper<GCodeWord>> &) const;
    void visit(Visitor &) override;
   protected:
    void dump(std::ostream &) const override;
   private:
    std::unique_ptr<GCodeWord> command;
    std::vector<std::unique_ptr<GCodeWord>> parameters;
  };

  class GCodeBlock : public GCodeNode {
   public:
    GCodeBlock(std::vector<std::unique_ptr<GCodeNode>>, const SourcePosition &);
    void getContent(std::vector<std::reference_wrapper<GCodeNode>> &) const;
    void visit(Visitor &) override;
   protected:
    void dump(std::ostream &) const override;
   private:
    std::vector<std::unique_ptr<GCodeNode>> content;
  };

  class GCodeProcedureDefinition : public GCodeNode {
   public:
    GCodeProcedureDefinition(int64_t, std::unique_ptr<GCodeNode>, std::vector<std::unique_ptr<GCodeNode>>, const SourcePosition &);
    int64_t getIdentifier() const;
    GCodeNode &getBody() const;
    void getReturnValues(std::vector<std::reference_wrapper<GCodeNode>> &) const;
    void visit(Visitor &) override;
   protected:
    void dump(std::ostream &) const override;
   private:
    int64_t identifier;
    std::unique_ptr<GCodeNode> body;
    std::vector<std::unique_ptr<GCodeNode>> retValues;
  };

  class GCodeProcedureCall : public GCodeNode {
   public:
    GCodeProcedureCall(std::unique_ptr<GCodeNode>, std::vector<std::unique_ptr<GCodeNode>>, const SourcePosition &);
    GCodeNode &getProcedureId() const;
    void getArguments(std::vector<std::reference_wrapper<GCodeNode>> &) const;
    void visit(Visitor &) override;
   protected:
    void dump(std::ostream &) const override;
   private:
    std::unique_ptr<GCodeNode> procedureId;
    std::vector<std::unique_ptr<GCodeNode>> args;
  };

  class GCodeConditional : public GCodeNode {
   public:
    GCodeConditional(std::unique_ptr<GCodeNode>, std::unique_ptr<GCodeNode>, std::unique_ptr<GCodeNode>, const SourcePosition &);
    GCodeNode &getCondition() const;
    GCodeNode &getThenBody() const;
    GCodeNode *getElseBody() const;
    void visit(Visitor &) override;
   protected:
    void dump(std::ostream &) const override;
   private:
    std::unique_ptr<GCodeNode> condition;
    std::unique_ptr<GCodeNode> thenBody;
    std::unique_ptr<GCodeNode> elseBody;
  };

  class GCodeWhileLoop : public GCodeNode {
   public:
    GCodeWhileLoop(std::unique_ptr<GCodeNode>, std::unique_ptr<GCodeNode>, const SourcePosition &);
    GCodeNode &getCondition() const;
    GCodeNode &getBody() const;
    void visit(Visitor &) override;
   protected:
    void dump(std::ostream &) const override;
   private:
    std::unique_ptr<GCodeNode> condition;
    std::unique_ptr<GCodeNode> body;
  };

  class GCodeNode::Visitor {
   public:
    virtual ~Visitor() = default;
    virtual void visit(const GCodeConstantValue &) {}
    virtual void visit(const GCodeUnaryOperation &) {}
    virtual void visit(const GCodeBinaryOperation &) {}
    virtual void visit(const GCodeFunctionCall &) {}
    virtual void visit(const GCodeWord &) {}
    virtual void visit(const GCodeCommand &) {}
    virtual void visit(const GCodeBlock &) {}
    virtual void visit(const GCodeProcedureDefinition &) {}
    virtual void visit(const GCodeProcedureCall &) {}
    virtual void visit(const GCodeConditional &) {}
    virtual void visit(const GCodeWhileLoop &) {}
  };
}

#endif