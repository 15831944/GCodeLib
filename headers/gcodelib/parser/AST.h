#ifndef GCODELIB_PARSER_AST_H_
#define GCODELIB_PARSER_AST_H_

#include "gcodelib/Base.h"
#include <memory>
#include <functional>
#include <vector>
#include <variant>
#include <iosfwd>

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


  class GCodeConstantValue : public GCodeNode {
   public:
    GCodeConstantValue(int64_t);
    GCodeConstantValue(double);

    int64_t asInteger(int64_t = 0) const;
    double asFloat(double = 0.0) const;
   protected:
    void dump(std::ostream &) const override;
   private:
    std::variant<int64_t, double> value;
  };

  class GCodeWord : public GCodeNode {
   public:
    GCodeWord(unsigned char, std::unique_ptr<GCodeConstantValue>);
    unsigned char getField() const;
    GCodeConstantValue &getValue() const;
   protected:
    void dump(std::ostream &) const override;
   private:
    unsigned char field;
    std::unique_ptr<GCodeConstantValue> value;
  };

  class GCodeCommand : public GCodeNode {
   public:
    GCodeCommand(std::unique_ptr<GCodeWord>, std::vector<std::unique_ptr<GCodeWord>>);
    GCodeWord &getCommand() const;
    void getParameters(std::vector<std::reference_wrapper<GCodeWord>> &) const;
   protected:
    void dump(std::ostream &) const override;
   private:
    std::unique_ptr<GCodeWord> command;
    std::vector<std::unique_ptr<GCodeWord>> parameters;
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