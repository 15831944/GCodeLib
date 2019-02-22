#ifndef GCODELIB_PARSER_AST_H_
#define GCODELIB_PARSER_AST_H_

#include "gcodelib/Base.h"
#include <memory>
#include <functional>
#include <vector>
#include <iostream>

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

    GCodeNode(Type);
    virtual ~GCodeNode() = default;

    Type getType() const;
    bool is(Type) const;

    friend std::ostream &operator<<(std::ostream &, const GCodeNode &);

   protected:
    virtual void dump(std::ostream &) const = 0;

   private:
    Type type;
  };

  template <GCodeNode::Type NodeType, typename DataType>
  class GCodeConstant : public GCodeNode {
   public:
    GCodeConstant(DataType value)
     : GCodeNode::GCodeNode(NodeType), value(value) {}
    DataType getValue() const {
      return this->value;
    }
   protected:
    void dump(std::ostream &os) const override {
      os << this->value;
    }
   private:
    DataType value;
  };

  using GCodeIntegerContant = GCodeConstant<GCodeNode::Type::IntegerContant, int64_t>;
  using GCodeFloatContant = GCodeConstant<GCodeNode::Type::FloatContant, double>;

  class GCodeWord : public GCodeNode {
   public:
    GCodeWord(unsigned char, std::unique_ptr<GCodeNode>);
    unsigned char getField() const;
    GCodeNode &getValue() const;
   protected:
    void dump(std::ostream &) const override;
   private:
    unsigned char field;
    std::unique_ptr<GCodeNode> value;
  };

  class GCodeCommand : public GCodeNode {
   public:
    GCodeCommand(std::unique_ptr<GCodeNode>, std::vector<std::unique_ptr<GCodeNode>>);
    GCodeNode &getCommand() const;
    void getParameters(std::vector<std::reference_wrapper<GCodeNode>> &) const;
   protected:
    void dump(std::ostream &) const override;
   private:
    std::unique_ptr<GCodeNode> command;
    std::vector<std::unique_ptr<GCodeNode>> parameters;
  };

  class GCodeBlock : public GCodeNode {
   public:
    GCodeBlock(std::vector<std::unique_ptr<GCodeNode>>);
    void getContent(std::vector<std::reference_wrapper<GCodeNode>> &) const;
   protected:
    void dump(std::ostream &) const override;
   private:
    std::vector<std::unique_ptr<GCodeNode>> content;
  };
}

#endif