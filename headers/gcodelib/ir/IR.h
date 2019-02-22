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

    friend std::ostream &operator<<(std::ostream &, const GCodeIRConstant &);
   private:
    Type type;
    std::variant<int64_t, double> value;
  };

  class GCodeIRInstruction {
   public:
    enum class Type {
      Command
    };

    GCodeIRInstruction(Type);
    virtual ~GCodeIRInstruction() = default;
    Type getType() const;
    bool is(Type) const;

    friend std::ostream &operator<<(std::ostream &, const GCodeIRInstruction &);
   protected:
    virtual void dump(std::ostream &) const = 0;
   private:
    Type type;
  };

  class GCodeIRCommand : public GCodeIRInstruction {
   public:
    enum class FunctionType {
      General = 'G',
      Misc = 'M'
    };

    GCodeIRCommand(FunctionType, GCodeIRConstant, const std::map<unsigned char, GCodeIRConstant> &);
    FunctionType getFunctionType() const;
    const GCodeIRConstant &getFunction() const;
    bool hasParameter(unsigned char) const;
    GCodeIRConstant getParameter(unsigned char) const;
   protected:
    void dump(std::ostream &) const override;
   private:
    FunctionType type;
    GCodeIRConstant function;
    std::map<unsigned char, GCodeIRConstant> parameters;
  };

  class GCodeIRModule {
   public:
    GCodeIRModule(std::vector<std::unique_ptr<GCodeIRInstruction>>);
    std::size_t size();
    std::optional<std::reference_wrapper<GCodeIRInstruction>> at(std::size_t);

    friend std::ostream &operator<<(std::ostream &, const GCodeIRModule &);
   private:
    std::vector<std::unique_ptr<GCodeIRInstruction>> code;
  };
}

#endif