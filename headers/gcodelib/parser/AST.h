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
      Word,
      Command,
      Block
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

  class GCodeWord : public GCodeNode {
   public:
    GCodeWord(unsigned char, std::unique_ptr<GCodeConstantValue>, const SourcePosition &);
    unsigned char getField() const;
    GCodeConstantValue &getValue() const;
    void visit(Visitor &) override;
   protected:
    void dump(std::ostream &) const override;
   private:
    unsigned char field;
    std::unique_ptr<GCodeConstantValue> value;
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

  class GCodeNode::Visitor {
   public:
    virtual ~Visitor() = default;
    virtual void visit(const GCodeConstantValue &) {}
    virtual void visit(const GCodeWord &) {}
    virtual void visit(const GCodeCommand &) {}
    virtual void visit(const GCodeBlock &) {}
  };
}

#endif