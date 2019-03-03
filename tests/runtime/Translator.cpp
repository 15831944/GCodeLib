#include "catch.hpp"
#include "gcodelib/runtime/Translator.h"

using namespace GCodeLib::Parser;
using namespace GCodeLib::Runtime;
using NodeList = std::vector<std::unique_ptr<GCodeNode>>;

class GCodeTestMangler : public GCodeNameMangler {
 public:
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

template <typename ... T>
static std::vector<std::unique_ptr<GCodeNode>> make_node_list(T &&... args) {
  std::vector<std::unique_ptr<GCodeNode>> vec;
  MakeAstImpl<T...>::push(vec, std::forward<T>(args)...);
  return vec;
}

template <typename ... T>
static std::unique_ptr<GCodeBlock> make_ast(T &&... args) {
  std::vector<std::unique_ptr<GCodeNode>> vec;
  MakeAstImpl<T...>::push(vec, std::forward<T>(args)...);
  return std::make_unique<GCodeBlock>(std::move(vec), position);
}

template <typename T>
static std::unique_ptr<T> make_node(T &&arg) {
  return std::make_unique<T>(std::move(arg));
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

static constexpr int64_t IntegerConstants[] = {
  100,
  200,
  300
};

static constexpr double FloatConstants[] = {
  3.14
};

static constexpr const char *StringConstants[] = {
  "Hello"
};

TEST_CASE("Constants") {
  GCodeIRTranslator translator(mangler);
  auto ast = make_ast(GCodeConstantValue(IntegerConstants[0], position),
    GCodeConstantValue(FloatConstants[0], position),
    GCodeConstantValue(StringConstants[0], position));
  auto ir = translator.translate(*ast);
  REQUIRE(verify_ir(*ir, {
    { GCodeIROpcode::Push, IntegerConstants[0] },
    { GCodeIROpcode::Push, FloatConstants[0] },
    { GCodeIROpcode::Push, std::string(StringConstants[0]) }
  }));
}

TEST_CASE("Numbered variables") {
  GCodeIRTranslator translator(mangler);
  const int ID = 1;
  auto ast = make_ast(GCodeNumberedVariable(ID, position));
  auto ir = translator.translate(*ast);
  REQUIRE(verify_ir(*ir, {
    { GCodeIROpcode::LoadNumbered, static_cast<int64_t>(ID) }
  }));
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
  auto ast = make_ast(GCodeNumberedVariableAssignment(ID, std::make_unique<GCodeConstantValue>(IntegerConstants[0], position), position));
  auto ir = translator.translate(*ast);
  REQUIRE(verify_ir(*ir, {
    { GCodeIROpcode::Push, IntegerConstants[0] },
    { GCodeIROpcode::StoreNumbered, ID }
  }));
}

TEST_CASE("Named variable assignment") {
  GCodeIRTranslator translator(mangler);
  const std::string &ID = "abc";
  auto ast = make_ast(GCodeNamedVariableAssignment(ID, std::make_unique<GCodeConstantValue>(IntegerConstants[0], position), position));
  auto ir = translator.translate(*ast);
  REQUIRE(verify_ir(*ir, {
    { GCodeIROpcode::Push, IntegerConstants[0] },
    { GCodeIROpcode::StoreNamed, static_cast<int64_t>(ir->getSymbolId(ID)) }
  }));
}

TEST_CASE("Unary operations") {
  GCodeIRTranslator translator(mangler);
  GCodeConstantValue arg(IntegerConstants[0], position);
  std::map<GCodeUnaryOperation::Operation, GCodeIROpcode> ops = {
    { GCodeUnaryOperation::Operation::Negate, GCodeIROpcode::Negate }
  };
  for (const auto &kv : ops) {
    auto ast = make_ast(GCodeUnaryOperation(kv.first, std::make_unique<GCodeConstantValue>(arg), position));
    auto ir = translator.translate(*ast);
    REQUIRE(verify_ir(*ir, {
      { GCodeIROpcode::Push, IntegerConstants[0] },
      { kv.second }
    }));
  }
}

TEST_CASE("Binary operations") {
  GCodeIRTranslator translator(mangler);
  GCodeConstantValue left(IntegerConstants[0], position);
  GCodeConstantValue right(IntegerConstants[2], position);
  SECTION("Arithmetic") {
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
        { GCodeIROpcode::Push, IntegerConstants[0] },
        { GCodeIROpcode::Push, IntegerConstants[2] },
        { kv.second }
      }));
    }
  }
  SECTION("Comparison") {
    std::map<GCodeBinaryOperation::Operation, int64_t> ops = {
      { GCodeBinaryOperation::Operation::Equals, static_cast<int64_t>(GCodeCompare::Equals) },
      { GCodeBinaryOperation::Operation::NotEquals, static_cast<int64_t>(GCodeCompare::NotEquals) },
      { GCodeBinaryOperation::Operation::Greater, static_cast<int64_t>(GCodeCompare::Greater) },
      { GCodeBinaryOperation::Operation::GreaterOrEquals, static_cast<int64_t>(GCodeCompare::Greater) | static_cast<int64_t>(GCodeCompare::Equals) },
      { GCodeBinaryOperation::Operation::Lesser, static_cast<int64_t>(GCodeCompare::Lesser) },
      { GCodeBinaryOperation::Operation::LesserOrEquals, static_cast<int64_t>(GCodeCompare::Lesser) | static_cast<int64_t>(GCodeCompare::Equals) }
    };
    for (const auto &kv : ops) {
      auto ast = make_ast(GCodeBinaryOperation(kv.first, std::make_unique<GCodeConstantValue>(left), std::make_unique<GCodeConstantValue>(right), position));
      auto ir = translator.translate(*ast);
      REQUIRE(verify_ir(*ir, {
        { GCodeIROpcode::Push, IntegerConstants[0] },
        { GCodeIROpcode::Push, IntegerConstants[2] },
        { GCodeIROpcode::Compare },
        { GCodeIROpcode::Test, kv.second }
      }));
    }
  }
}

TEST_CASE("Procedure return") {
  GCodeIRTranslator translator(mangler);
  auto ast = make_ast(GCodeProcedureReturn(make_node_list(
    GCodeConstantValue(IntegerConstants[0], position),
    GCodeConstantValue(2.718, position)
  ), position));
  auto ir = translator.translate(*ast);
  REQUIRE(verify_ir(*ir, {
    { GCodeIROpcode::Push, IntegerConstants[0] },
    { GCodeIROpcode::Push, 2.718 },
    { GCodeIROpcode::Ret, 2L }
  }));
}

TEST_CASE("Procedure definition") {
  GCodeIRTranslator translator(mangler);
  const int ID = 0;
  auto ast = make_ast(GCodeProcedureDefinition(ID,
    make_ast(GCodeConstantValue(IntegerConstants[0], position)),
    make_node_list(GCodeConstantValue(IntegerConstants[2], position)),
    position));
  auto ir = translator.translate(*ast);
  REQUIRE(verify_ir(*ir, {
    { GCodeIROpcode::Jump, 4L },
    { GCodeIROpcode::Push, IntegerConstants[0] },
    { GCodeIROpcode::Push, IntegerConstants[2] },
    { GCodeIROpcode::Ret, 1L }
  }));
  REQUIRE(ir->getProcedure(ID).getAddress() == 1);
}

TEST_CASE("Procedure call") {
  GCodeIRTranslator translator(mangler);
  const int64_t ID = 0;
  auto ast = make_ast(GCodeProcedureCall(
    make_ast(GCodeConstantValue(ID, position)),
    make_node_list(GCodeConstantValue(IntegerConstants[0], position), GCodeConstantValue(IntegerConstants[1], position)),
    position));
  auto ir = translator.translate(*ast);
  REQUIRE(verify_ir(*ir, {
    { GCodeIROpcode::Push, IntegerConstants[0] },
    { GCodeIROpcode::Push, IntegerConstants[1] },
    { GCodeIROpcode::Push, ID },
    { GCodeIROpcode::Call, 2L }
  }));
}

TEST_CASE("Function call") {
  GCodeIRTranslator translator(mangler);
  auto ast = make_ast(GCodeFunctionCall(
    StringConstants[0],
    make_node_list(GCodeConstantValue(IntegerConstants[0], position), GCodeConstantValue(IntegerConstants[1], position)),
    position
  ));
  auto ir = translator.translate(*ast);
  REQUIRE(verify_ir(*ir, {
    { GCodeIROpcode::Push, IntegerConstants[1] },
    { GCodeIROpcode::Push, IntegerConstants[0] },
    { GCodeIROpcode::Push, 2L },
    { GCodeIROpcode::Invoke, static_cast<int64_t>(ir->getSymbolId(StringConstants[0])) }
  }));
}

TEST_CASE("Conditionals") {
  GCodeIRTranslator translator(mangler);
  SECTION("If-Else") {
    auto ast = make_ast(GCodeConditional(
      make_node(GCodeConstantValue(IntegerConstants[0], position)),
      make_node(GCodeConstantValue(IntegerConstants[1], position)),
      make_node(GCodeConstantValue(IntegerConstants[2], position)),
      position));
    auto ir = translator.translate(*ast);
    REQUIRE(verify_ir(*ir, {
      { GCodeIROpcode::Push, IntegerConstants[0] },
      { GCodeIROpcode::Not },
      { GCodeIROpcode::JumpIf, 5L },
      { GCodeIROpcode::Push, IntegerConstants[1] },
      { GCodeIROpcode::Jump, 6L },
      { GCodeIROpcode::Push, IntegerConstants[2] }
    }));
  }
  SECTION("If") {
    auto ast = make_ast(GCodeConditional(
      make_node(GCodeConstantValue(IntegerConstants[0], position)),
      make_node(GCodeConstantValue(IntegerConstants[1], position)),
      nullptr,
      position));
    auto ir = translator.translate(*ast);
    REQUIRE(verify_ir(*ir, {
      { GCodeIROpcode::Push, IntegerConstants[0] },
      { GCodeIROpcode::Not },
      { GCodeIROpcode::JumpIf, 4L },
      { GCodeIROpcode::Push, IntegerConstants[1] },
    }));
  }
}

TEST_CASE("While loop") {
  const int64_t ID = 0;
  std::string loopName = mangler.GCodeNameMangler::getLoop(ID);
  GCodeIRTranslator translator(mangler);
  SECTION("While loop") {
    auto ast = make_ast(GCodeWhileLoop(
      ID,
      make_node(GCodeConstantValue(IntegerConstants[0], position)),
      make_node(GCodeConstantValue(IntegerConstants[1], position)),
      false,
      position));
    auto ir = translator.translate(*ast);
    REQUIRE(verify_ir(*ir, {
      { GCodeIROpcode::Jump, 2L },
      { GCodeIROpcode::Push, IntegerConstants[1] },
      { GCodeIROpcode::Push, IntegerConstants[0] },
      { GCodeIROpcode::JumpIf, 1L }
    }));
    REQUIRE(ir->getNamedLabel(mangler.getStatementStart(loopName)).getAddress() == 2);
    REQUIRE(ir->getNamedLabel(mangler.getStatementEnd(loopName)).getAddress() == 4); 
  }
  SECTION("Do-While loop") {
    auto ast = make_ast(GCodeWhileLoop(
      0,
      make_node(GCodeConstantValue(IntegerConstants[0], position)),
      make_node(GCodeConstantValue(IntegerConstants[1], position)),
      true,
      position));
    auto ir = translator.translate(*ast);
    REQUIRE(verify_ir(*ir, {
      { GCodeIROpcode::Push, IntegerConstants[1] },
      { GCodeIROpcode::Push, IntegerConstants[0] },
      { GCodeIROpcode::JumpIf, 0L }
    }));
    REQUIRE(ir->getNamedLabel(mangler.getStatementStart(loopName)).getAddress() == 1);
    REQUIRE(ir->getNamedLabel(mangler.getStatementEnd(loopName)).getAddress() == 3); 
  }
}

TEST_CASE("Repeat loop") {
  const int64_t ID = 0;
  GCodeIRTranslator translator(mangler);
  auto ast = make_ast(GCodeRepeatLoop(
    ID,
    make_node(GCodeConstantValue(IntegerConstants[0], position)),
    make_node(GCodeConstantValue(IntegerConstants[1], position)),
    position));
  auto ir = translator.translate(*ast);
  REQUIRE(verify_ir(*ir, {
    { GCodeIROpcode::Push, IntegerConstants[0] },
    { GCodeIROpcode::Jump, 5L },
    { GCodeIROpcode::Push, 1L },
    { GCodeIROpcode::Subtract },
    { GCodeIROpcode::Push, IntegerConstants[1] },
    { GCodeIROpcode::Dup },
    { GCodeIROpcode::Push, 0L },
    { GCodeIROpcode::Compare },
    { GCodeIROpcode::Test, static_cast<int64_t>(GCodeCompare::Greater) },
    { GCodeIROpcode::JumpIf, 2L }
  }));
  std::string loopName = mangler.GCodeNameMangler::getLoop(ID);
  REQUIRE(ir->getNamedLabel(mangler.getStatementStart(loopName)).getAddress() == 5);
  REQUIRE(ir->getNamedLabel(mangler.getStatementEnd(loopName)).getAddress() == 10);
}

TEST_CASE("Named statement") {
  const std::string &NAME = "test";
  GCodeIRTranslator translator(mangler);
  auto ast = make_ast(GCodeNamedStatement(
    NAME,
    make_node(GCodeConstantValue(IntegerConstants[0], position)),
    position));
  auto ir = translator.translate(*ast);
  REQUIRE(verify_ir(*ir, {
    { GCodeIROpcode::Push, IntegerConstants[0] }
  }));
  REQUIRE(ir->getNamedLabel(mangler.getStatementStart(NAME)).getAddress() == 0);
  REQUIRE(ir->getNamedLabel(mangler.getStatementEnd(NAME)).getAddress() == 1);
}

TEST_CASE("Loop control") {
  const int64_t ID = 0;
  GCodeIRTranslator translator(mangler);
  auto ast = make_ast(GCodeWhileLoop(ID,
    make_node(GCodeConstantValue(IntegerConstants[0], position)),
    make_ast(
      GCodeLoopControl(ID, GCodeLoopControl::ControlType::Continue, position),
      GCodeLoopControl(ID, GCodeLoopControl::ControlType::Break, position)),
    true,
    position));
  auto ir = translator.translate(*ast);
  REQUIRE(verify_ir(*ir, {
    { GCodeIROpcode::Jump, 2L },
    { GCodeIROpcode::Jump, 4L },
    { GCodeIROpcode::Push, IntegerConstants[0] },
    { GCodeIROpcode::JumpIf, 0L }
  }));
}

TEST_CASE("Command") {
  GCodeIRTranslator translator(mangler);
  std::vector<std::unique_ptr<GCodeWord>> args;
  args.push_back(make_node(GCodeWord('A', make_node(GCodeConstantValue(IntegerConstants[0], position)), position)));
  args.push_back(make_node(GCodeWord('B', make_node(GCodeConstantValue(IntegerConstants[1], position)), position)));
  args.push_back(make_node(GCodeWord('C', make_node(GCodeConstantValue(IntegerConstants[2], position)), position)));
  auto ast = make_ast(GCodeCommand(
    make_node(GCodeWord('G', make_node(GCodeConstantValue(IntegerConstants[0], position)), position)),
    std::move(args),
    position));
  auto ir = translator.translate(*ast);
  REQUIRE(verify_ir(*ir, {
    { GCodeIROpcode::Prologue },
    { GCodeIROpcode::Push, IntegerConstants[0] },
    { GCodeIROpcode::SetArg, static_cast<int64_t>('A') },
    { GCodeIROpcode::Push, IntegerConstants[1] },
    { GCodeIROpcode::SetArg, static_cast<int64_t>('B') },
    { GCodeIROpcode::Push, IntegerConstants[2] },
    { GCodeIROpcode::SetArg, static_cast<int64_t>('C') },
    { GCodeIROpcode::Push, IntegerConstants[0] },
    { GCodeIROpcode::Syscall, static_cast<int64_t>('G') }
  }));
}