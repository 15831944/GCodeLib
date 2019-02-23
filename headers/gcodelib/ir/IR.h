#ifndef GCODELIB_IR_IR_H_
#define GCODELIB_IR_IR_H_

#include "gcodelib/Base.h"
#include <variant>
#include <map>
#include <memory>
#include <vector>
#include <functional>
#include <optional>
#include <iosfwd>

namespace GCodeLib {

  class GCodeIRConstant {
   public:
    enum class Type {
      Integer,
      Float
    };

    GCodeIRConstant();
    GCodeIRConstant(int64_t);
    GCodeIRConstant(double);

    Type getType() const;
    bool is(Type) const;

    int64_t getInteger(int64_t = 0) const;
    double getFloat(double = 0.0) const;
    int64_t asInteger() const;
    double asFloat() const;

    friend std::ostream &operator<<(std::ostream &, const GCodeIRConstant &);
   private:
    Type type;
    std::variant<int64_t, double> value;
  };

  class GCodeSystemCommand {
   public:
    enum class FunctionType {
      General = 'G',
      Misc = 'M',
      FeedRate = 'F',
      SpindleSpeed = 'S',
      ToolSelection = 'T'
    };

    GCodeSystemCommand(FunctionType, GCodeIRConstant, const std::map<unsigned char, GCodeIRConstant> &);
    FunctionType getFunctionType() const;
    const GCodeIRConstant &getFunction() const;
    bool hasParameter(unsigned char) const;
    GCodeIRConstant getParameter(unsigned char) const;

    friend std::ostream &operator<<(std::ostream &, const GCodeSystemCommand &);
   private:
    FunctionType type;
    GCodeIRConstant function;
    std::map<unsigned char, GCodeIRConstant> parameters;
  };

  class GCodeIRInstruction {
   public:
    enum class Type {
      SystemCommand
    };

    GCodeIRInstruction(const GCodeSystemCommand &);
    Type getType() const;
    bool is(Type) const;

    const GCodeSystemCommand &getSystemCommand() const; 

    friend std::ostream &operator<<(std::ostream &, const GCodeIRInstruction &);
   private:
    Type type;
    std::variant<GCodeSystemCommand> value;
  };

  class GCodeIRModule {
   public:
    std::size_t size() const;
    const GCodeIRInstruction &at(std::size_t) const;

    std::size_t appendInstruction(std::unique_ptr<GCodeIRInstruction>);

    friend std::ostream &operator<<(std::ostream &, const GCodeIRModule &);
   private:
    std::vector<std::unique_ptr<GCodeIRInstruction>> code;
  };
}

#endif