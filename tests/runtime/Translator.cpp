#include "catch.hpp"
#include "gcodelib/runtime/Translator.h"

using namespace GCodeLib::Parser;
using namespace GCodeLib::Runtime;
using NodeList = std::vector<std::unique_ptr<GCodeNode>>;

class GCodeTestMangler : public GCodeNameMangler {

  std::string getProcedureName(const std::string &proc) const override {
    return "procedure_" + proc;
  }

  std::string getStatementStart(const std::string &stmt) const override {
    return "stmt_" + stmt + "_start";
  }

  std::string getStatementEnd(const std::string &stmt) const override {
    return "stmt_" + stmt + "_end";
  }

  std::string getLoop(const std::string &loop) const override {
    return "loop_" + loop;
  }
};

static GCodeTestMangler mangler;
static SourcePosition position("", 1, 2, 3);

template <typename ... T>
struct MakeAstImpl {};

template <>
struct MakeAstImpl<> {
  static void push(NodeList &) {}
};

template <typename A, typename ... B>
struct MakeAstImpl<A, B...>  {
  static void push(NodeList &list, A &&arg, B &&... args) {
    list.push_back(std::make_unique<A>(std::move(arg)));
    MakeAstImpl<B...>::push(list, std::forward<B>(args)...);
  };
};

template <class NodeType, typename ... T>
static std::unique_ptr<GCodeBlock> make_node_list(T &&... args) {
  std::vector<std::unique_ptr<GCodeNode>> vec;
  MakeAstImpl<T...>::push(vec, std::forward<T>(args)...);
  return std::make_unique<NodeType>(std::move(vec), position);
}

template <typename ... T>
static auto make_ast(T &&... args) {
  return make_node_list<GCodeBlock, T...>(std::forward<T>(args)...);
}

static bool operator!=(const GCodeRuntimeValue &v1, const GCodeRuntimeValue &v2) {
  if (v1.getType() != v2.getType()) {
    return true;
  }
  switch (v1.getType()) {
    case GCodeRuntimeValue::Type::Integer:
      return v1.getInteger() != v2.getInteger();
    case GCodeRuntimeValue::Type::Float:
      return v1.getFloat() != v2.getFloat();
    case GCodeRuntimeValue::Type::String:
      return v1.getString().compare(v2.getString()) != 0;
    default:
      return false;
  }
}

static bool verify_ir(GCodeIRModule &module, std::initializer_list<GCodeIRInstruction> code) {
  if (module.length() != code.size()) {
    return false;
  }
  std::size_t i = 0;
  for (auto &ci : code) {
    const GCodeIRInstruction &instr = module.at(i++);
    if (instr.getOpcode() != ci.getOpcode() ||
      instr.getValue() != ci.getValue()) {
      return false;
    }
  }
  return true;
}

TEST_CASE("Constants") {
  GCodeIRTranslator translator(mangler);
  auto ast = make_ast(GCodeConstantValue(100L, position), GCodeConstantValue(3.14, position), GCodeConstantValue("Hello", position));
  auto ir = translator.translate(*ast);
  REQUIRE(ir->length() == 3);
  REQUIRE(ir->at(0).getOpcode() == GCodeIROpcode::Push);
  REQUIRE(ir->at(0).getValue().getInteger() == 100);
  REQUIRE(ir->at(1).getOpcode() == GCodeIROpcode::Push);
  REQUIRE(ir->at(1).getValue().getFloat() == 3.14);
  REQUIRE(ir->at(2).getOpcode() == GCodeIROpcode::Push);
  REQUIRE(ir->at(2).getValue().getString().compare("Hello") == 0);
}

TEST_CASE("Numbered variables") {
  GCodeIRTranslator translator(mangler);
  const int ID = 1;
  auto ast = make_ast(GCodeNumberedVariable(ID, position));
  auto ir = translator.translate(*ast);
  REQUIRE(ir->length() == 1);
  REQUIRE(ir->at(0).getOpcode() == GCodeIROpcode::LoadNumbered);
  REQUIRE(ir->at(0).getValue().getInteger() == ID);
}

TEST_CASE("Named variables") {
  GCodeIRTranslator translator(mangler);
  const std::string &ID = "abc";
  auto ast = make_ast(GCodeNamedVariable(ID, position));
  auto ir = translator.translate(*ast);
  REQUIRE(verify_ir(*ir, {
    { GCodeIROpcode::LoadNamed, static_cast<int64_t>(ir->getSymbolId(ID)) }
  }));
}

TEST_CASE("Numbered variable assignment") {
  GCodeIRTranslator translator(mangler);
  const int64_t ID = 1;
  auto ast = make_ast(GCodeNumberedVariableAssignment(ID, std::make_unique<GCodeConstantValue>(100L, position), position));
  auto ir = translator.translate(*ast);
  REQUIRE(verify_ir(*ir, {
    { GCodeIROpcode::Push, 100L },
    { GCodeIROpcode::StoreNumbered, ID }
  }));
}

TEST_CASE("Named variable assignment") {
  GCodeIRTranslator translator(mangler);
  const std::string &ID = "abc";
  auto ast = make_ast(GCodeNamedVariableAssignment(ID, std::make_unique<GCodeConstantValue>(100L, position), position));
  auto ir = translator.translate(*ast);
  REQUIRE(verify_ir(*ir, {
    { GCodeIROpcode::Push, 100L },
    { GCodeIROpcode::StoreNamed, static_cast<int64_t>(ir->getSymbolId(ID)) }
  }));
}

TEST_CASE("Unary operations") {
  GCodeIRTranslator translator(mangler);
  GCodeConstantValue arg(100L, position);
  std::map<GCodeUnaryOperation::Operation, GCodeIROpcode> ops = {
    { GCodeUnaryOperation::Operation::Negate, GCodeIROpcode::Negate }
  };
  for (const auto &kv : ops) {
    auto ast = make_ast(GCodeUnaryOperation(kv.first, std::make_unique<GCodeConstantValue>(arg), position));
    auto ir = translator.translate(*ast);
    REQUIRE(verify_ir(*ir, {
      { GCodeIROpcode::Push, 100L },
      { kv.second }
    }));
  }
}

TEST_CASE("Binary operations") {
  GCodeIRTranslator translator(mangler);
  GCodeConstantValue left(100L, position);
  GCodeConstantValue right(200L, position);
  std::map<GCodeBinaryOperation::Operation, GCodeIROpcode> ops = {
    { GCodeBinaryOperation::Operation::Add, GCodeIROpcode::Add },
    { GCodeBinaryOperation::Operation::Subtract, GCodeIROpcode::Subtract },
    { GCodeBinaryOperation::Operation::Multiply, GCodeIROpcode::Multiply },
    { GCodeBinaryOperation::Operation::Divide, GCodeIROpcode::Divide },
    { GCodeBinaryOperation::Operation::Power, GCodeIROpcode::Power },
    { GCodeBinaryOperation::Operation::Modulo, GCodeIROpcode::Modulo },
    { GCodeBinaryOperation::Operation::And, GCodeIROpcode::And },
    { GCodeBinaryOperation::Operation::Or, GCodeIROpcode::Or },
    { GCodeBinaryOperation::Operation::Xor, GCodeIROpcode::Xor },
  };
  for (const auto &kv : ops) {
    auto ast = make_ast(GCodeBinaryOperation(kv.first, std::make_unique<GCodeConstantValue>(left), std::make_unique<GCodeConstantValue>(right), position));
    auto ir = translator.translate(*ast);
    REQUIRE(verify_ir(*ir, {
      { GCodeIROpcode::Push, 100L },
      { GCodeIROpcode::Push, 200L },
      { kv.second }
    }));
  }
}
