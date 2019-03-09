#include "catch.hpp"
#include "gcodelib/runtime/Interpreter.h"
#include "gcodelib/runtime/Error.h"
#include <functional>
#include <cmath>

using namespace GCodeLib::Runtime;

class GCodeTestInterpreter : public GCodeInterpreter {
 public:
  using InterpretHookFn = std::function<void(GCodeRuntimeState &)>;
  using SyscallHookFn = std::function<void(GCodeSyscallType, const GCodeRuntimeValue &, const GCodeScopedDictionary<unsigned char> &)>;
  GCodeTestInterpreter(GCodeIRModule &module,
    InterpretHookFn hook,
    SyscallHookFn syscall = [](GCodeSyscallType, const GCodeRuntimeValue &, const GCodeScopedDictionary<unsigned char> &) {})
    : GCodeInterpreter(module), interpretHook(hook), syscallHook(syscall) {}
  
  virtual void execute() {
    GCodeCascadeVariableScope sessionScope(&this->getSystemScope());
    this->state = GCodeRuntimeState(sessionScope);
    this->interpret();
    this->interpretHook(this->state.value());
    this->state.reset();
  }

  GCodeVariableScope &getSystemScope() override {
    return scope;
  }
 protected:

  void syscall(GCodeSyscallType syscall, const GCodeRuntimeValue &function, const GCodeScopedDictionary<unsigned char> &args) override {
    this->syscallHook(syscall, function, args);
  }
 private:
  GCodeCascadeVariableScope scope;
  InterpretHookFn interpretHook;
  SyscallHookFn syscallHook;
};

static std::unique_ptr<GCodeIRModule> make_ir(std::initializer_list<GCodeIRInstruction> code) {
  std::unique_ptr<GCodeIRModule> ir = std::make_unique<GCodeIRModule>();
  for (const auto &instr : code) {
    ir->appendInstruction(instr.getOpcode(), instr.getValue());
  }
  return ir;
}

static void make_ir(GCodeIRModule &module, std::initializer_list<GCodeIRInstruction> code) {
  for (const auto &instr : code) {
    module.appendInstruction(instr.getOpcode(), instr.getValue());
  }
}

static void expect(GCodeIRModule &module,
  std::function<void(GCodeRuntimeState &)> hook,
  std::function<void(GCodeTestInterpreter &)> before = [](GCodeTestInterpreter &) {},
  GCodeTestInterpreter::SyscallHookFn syscallHook = [](GCodeSyscallType, const GCodeRuntimeValue &, const GCodeScopedDictionary<unsigned char> &) {}) {
  bool result = false;
  GCodeTestInterpreter interp(module, [&](GCodeRuntimeState &state) {
    hook(state);
    result = true;
  }, syscallHook);
  before(interp);
  interp.execute();
  REQUIRE(result);
}

static void expectSyscall(GCodeIRModule &module, GCodeTestInterpreter::SyscallHookFn hook, const int32_t count = 1) {
  int32_t realCount = 0;
  GCodeTestInterpreter interp(module, [](GCodeRuntimeState &) {}, [&](GCodeSyscallType type, const GCodeRuntimeValue &function, const GCodeScopedDictionary<unsigned char> &args) {
    hook(type, function, args);
    realCount++;
  });
  interp.execute();
  REQUIRE((count < 0 || realCount == count));
}

static constexpr int64_t IntConstants[] = {
  100,
  200,
  300
};

static constexpr double FloatConstants[] = {
  3.14,
  6.28
};

static constexpr const char *StringConstants[] = {
  "Hello"
};

TEST_CASE("Opcode execution") {
  SECTION("Push") {
    auto ir = make_ir({
      { GCodeIROpcode::Push, IntConstants[0] },
      { GCodeIROpcode::Push, FloatConstants[0] },
      { GCodeIROpcode::Push, std::string(StringConstants[0]) }
    });
    expect(*ir, [](GCodeRuntimeState &state) {
      REQUIRE(state.pop().getString().compare(StringConstants[0]) == 0);
      REQUIRE(state.pop().getFloat() == FloatConstants[0]);
      REQUIRE(state.pop().getInteger() == IntConstants[0]);
      REQUIRE_THROWS(state.pop());
    });
  }
  SECTION("Jump") {
    auto ir = make_ir({
      { GCodeIROpcode::Push, IntConstants[0] },
      { GCodeIROpcode::Jump, 3L },
      { GCodeIROpcode::Push, IntConstants[1] }
    });
    expect(*ir, [](GCodeRuntimeState &state) {
      REQUIRE(state.getPC() == 3);
      REQUIRE(state.pop().getInteger() == IntConstants[0]);
      REQUIRE_THROWS(state.pop());
    });
  }
  SECTION("JumpIf") {
    auto ir = make_ir({
      { GCodeIROpcode::Push, 0L },
      { GCodeIROpcode::JumpIf, 5L },
      { GCodeIROpcode::Push, IntConstants[0] },
      { GCodeIROpcode::Push, 1L },
      { GCodeIROpcode::JumpIf, 6L },
      { GCodeIROpcode::Push, IntConstants[1] }
    });
    expect(*ir, [](GCodeRuntimeState &state) {
      REQUIRE(state.getPC() == 6);
      REQUIRE(state.pop().getInteger() == IntConstants[0]);
      REQUIRE_THROWS(state.pop());
    });
  }
  SECTION("Dup") {
    auto ir = make_ir({
      { GCodeIROpcode::Push, IntConstants[0] },
      { GCodeIROpcode::Dup }
    });
    expect(*ir, [](GCodeRuntimeState &state) {
      REQUIRE(state.pop().getInteger() == IntConstants[0]);
      REQUIRE(state.pop().getInteger() == IntConstants[0]);
      REQUIRE_THROWS(state.pop());
    });
  }
  SECTION("Negate") {
    auto ir = make_ir({
      { GCodeIROpcode::Push, IntConstants[0] },
      { GCodeIROpcode::Negate }
    });
    expect(*ir, [](GCodeRuntimeState &state) {
      REQUIRE(state.pop().getInteger() == -IntConstants[0]);
      REQUIRE_THROWS(state.pop());
    });
  }
  SECTION("Add") {
    auto ir = make_ir({
      { GCodeIROpcode::Push, IntConstants[0] },
      { GCodeIROpcode::Push, IntConstants[1] },
      { GCodeIROpcode::Add }
    });
    expect(*ir, [](GCodeRuntimeState &state) {
      REQUIRE(state.pop().getInteger() == IntConstants[0] + IntConstants[1]);
      REQUIRE_THROWS(state.pop());
    });
  }
  SECTION("Subtract") {
    auto ir = make_ir({
      { GCodeIROpcode::Push, IntConstants[0] },
      { GCodeIROpcode::Push, IntConstants[1] },
      { GCodeIROpcode::Subtract }
    });
    expect(*ir, [](GCodeRuntimeState &state) {
      REQUIRE(state.pop().getInteger() == IntConstants[0] - IntConstants[1]);
      REQUIRE_THROWS(state.pop());
    });
  }
  SECTION("Multiply") {
    auto ir = make_ir({
      { GCodeIROpcode::Push, IntConstants[0] },
      { GCodeIROpcode::Push, IntConstants[1] },
      { GCodeIROpcode::Multiply }
    });
    expect(*ir, [](GCodeRuntimeState &state) {
      REQUIRE(state.pop().getInteger() == IntConstants[0] * IntConstants[1]);
      REQUIRE_THROWS(state.pop());
    });
  }
  SECTION("Divide") {
    auto ir = make_ir({
      { GCodeIROpcode::Push, IntConstants[1] },
      { GCodeIROpcode::Push, IntConstants[0] },
      { GCodeIROpcode::Divide }
    });
    expect(*ir, [](GCodeRuntimeState &state) {
      REQUIRE(state.pop().getFloat() == IntConstants[1] / IntConstants[0]);
      REQUIRE_THROWS(state.pop());
    });
  }
  SECTION("Power") {
    auto ir = make_ir({
      { GCodeIROpcode::Push, FloatConstants[0] },
      { GCodeIROpcode::Push, FloatConstants[1] },
      { GCodeIROpcode::Power }
    });
    expect(*ir, [](GCodeRuntimeState &state) {
      REQUIRE(state.pop().getFloat() == pow(FloatConstants[0], FloatConstants[1]));
      REQUIRE_THROWS(state.pop());
    });
  }
  SECTION("Modulo") {
    auto ir = make_ir({
      { GCodeIROpcode::Push, IntConstants[0] },
      { GCodeIROpcode::Push, IntConstants[1] },
      { GCodeIROpcode::Modulo }
    });
    expect(*ir, [](GCodeRuntimeState &state) {
      REQUIRE(state.pop().getInteger() == IntConstants[0] % IntConstants[1]);
      REQUIRE_THROWS(state.pop());
    });
  }
  SECTION("And") {
    auto ir = make_ir({
      { GCodeIROpcode::Push, IntConstants[0] },
      { GCodeIROpcode::Push, IntConstants[1] },
      { GCodeIROpcode::And }
    });
    expect(*ir, [](GCodeRuntimeState &state) {
      REQUIRE(state.pop().getInteger() == (IntConstants[0] & IntConstants[1]));
      REQUIRE_THROWS(state.pop());
    });
  }
  SECTION("Or") {
    auto ir = make_ir({
      { GCodeIROpcode::Push, IntConstants[0] },
      { GCodeIROpcode::Push, IntConstants[1] },
      { GCodeIROpcode::Or }
    });
    expect(*ir, [](GCodeRuntimeState &state) {
      REQUIRE(state.pop().getInteger() == (IntConstants[0] | IntConstants[1]));
      REQUIRE_THROWS(state.pop());
    });
  }
  SECTION("Xor") {
    auto ir = make_ir({
      { GCodeIROpcode::Push, IntConstants[0] },
      { GCodeIROpcode::Push, IntConstants[1] },
      { GCodeIROpcode::Xor }
    });
    expect(*ir, [](GCodeRuntimeState &state) {
      REQUIRE(state.pop().getInteger() == (IntConstants[0] ^ IntConstants[1]));
      REQUIRE_THROWS(state.pop());
    });
  }
  SECTION("Not") {
    auto ir = make_ir({
      { GCodeIROpcode::Push, IntConstants[0] },
      { GCodeIROpcode::Not }
    });
    expect(*ir, [](GCodeRuntimeState &state) {
      REQUIRE(state.pop().getInteger() == !IntConstants[0]);
      REQUIRE_THROWS(state.pop());
    });
  }
  SECTION("Compare & Test") {
    auto ir = make_ir({
      { GCodeIROpcode::Push, IntConstants[0] },
      { GCodeIROpcode::Push, IntConstants[1] },
      { GCodeIROpcode::Compare },
      { GCodeIROpcode::Test, static_cast<int64_t>(GCodeCompare::Lesser) | static_cast<int64_t>(GCodeCompare::Equals) }
    });
    expect(*ir, [](GCodeRuntimeState &state) {
      REQUIRE((state.pop().getInteger() && IntConstants[0] <= IntConstants[1]));
      REQUIRE_THROWS(state.pop());
    });
  }
  SECTION("Load named") {
    GCodeIRModule ir;
    make_ir(ir, {
      { GCodeIROpcode::LoadNamed, static_cast<int64_t>(ir.getSymbolId(StringConstants[0])) }
    });
    expect(ir, [](GCodeRuntimeState &state) {
      REQUIRE(state.pop().getInteger() == IntConstants[0]);
      REQUIRE_THROWS(state.pop());
    }, [](GCodeTestInterpreter &interp) {
      interp.getSystemScope().getNamed().put(StringConstants[0], IntConstants[0]);
    });
  }
  SECTION("Load numbered") {
    GCodeIRModule ir;
    make_ir(ir, {
      { GCodeIROpcode::LoadNumbered, IntConstants[1] }
    });
    expect(ir, [](GCodeRuntimeState &state) {
      REQUIRE(state.pop().getInteger() == IntConstants[0]);
      REQUIRE_THROWS(state.pop());
    }, [](GCodeTestInterpreter &interp) {
      interp.getSystemScope().getNumbered().put(IntConstants[1], IntConstants[0]);
    });
  }
  SECTION("Store named") {
    GCodeIRModule ir;
    make_ir(ir, {
      { GCodeIROpcode::Push, IntConstants[0] },
      { GCodeIROpcode::StoreNamed, static_cast<int64_t>(ir.getSymbolId(StringConstants[0])) },
      { GCodeIROpcode::LoadNamed, static_cast<int64_t>(ir.getSymbolId(StringConstants[0])) }
    });
    expect(ir, [](GCodeRuntimeState &state) {
      REQUIRE(state.pop().getInteger() == IntConstants[0]);
      REQUIRE_THROWS(state.pop());
    });
  }
  SECTION("Store numbered") {
    GCodeIRModule ir;
    make_ir(ir, {
      { GCodeIROpcode::Push, IntConstants[0] },
      { GCodeIROpcode::StoreNumbered, IntConstants[1] },
      { GCodeIROpcode::LoadNumbered, IntConstants[1] }
    });
    expect(ir, [](GCodeRuntimeState &state) {
      REQUIRE(state.pop().getInteger() == IntConstants[0]);
      REQUIRE_THROWS(state.pop());
    });
  }
}

TEST_CASE("Invocation instructions") {
  SECTION("Syscalls") {
    auto ir = make_ir({
      { GCodeIROpcode::Prologue },
      { GCodeIROpcode::Push, IntConstants[0] },
      { GCodeIROpcode::SetArg, static_cast<int64_t>('A') },
      { GCodeIROpcode::Push, IntConstants[1] },
      { GCodeIROpcode::SetArg, static_cast<int64_t>('B') },
      { GCodeIROpcode::Prologue },
      { GCodeIROpcode::Push, IntConstants[1] },
      { GCodeIROpcode::SetArg, static_cast<int64_t>('A') },
      { GCodeIROpcode::Push, IntConstants[0] },
      { GCodeIROpcode::SetArg, static_cast<int64_t>('X') },
      { GCodeIROpcode::Push, IntConstants[2] },
      { GCodeIROpcode::Syscall, static_cast<int64_t>(GCodeSyscallType::General) }
    });
    expectSyscall(*ir, [](GCodeSyscallType type, const GCodeRuntimeValue &function, const GCodeScopedDictionary<unsigned char> &args) {
      REQUIRE(type == GCodeSyscallType::General);
      REQUIRE(function.getInteger() == IntConstants[2]);
      REQUIRE(args.has('A'));
      REQUIRE(args.has('X'));
      REQUIRE_FALSE(args.has('B'));
      REQUIRE(args.get('A').getInteger() == IntConstants[1]);
      REQUIRE(args.get('X').getInteger() == IntConstants[0]);
    });
  }
  SECTION("Zero-argument calls") {
    GCodeIRModule module;
    auto &label = module.getNamedLabel("label");
    module.registerProcedure(0, "label");
    make_ir(module, {
      { GCodeIROpcode::Push, IntConstants[0] },
      { GCodeIROpcode::Push, 0 },
      { GCodeIROpcode::Call, 0 },
      { GCodeIROpcode::Push, IntConstants[1] }
    });
    label.bind();
    expect(module, [](GCodeRuntimeState &state) {
      REQUIRE(state.pop().getInteger() == IntConstants[0]);
      REQUIRE_THROWS(state.pop());
    });
  }
  SECTION("Calls with arguments") {
    GCodeIRModule module;
    auto &label = module.getNamedLabel("label");
    module.registerProcedure(0, "label");
    make_ir(module, {
      { GCodeIROpcode::Push, IntConstants[0] },
      { GCodeIROpcode::Push, IntConstants[2] },
      { GCodeIROpcode::Push, IntConstants[1] },
      { GCodeIROpcode::Push, IntConstants[0] },
      { GCodeIROpcode::Push, 0 },
      { GCodeIROpcode::Call, 3 },
      { GCodeIROpcode::Push, IntConstants[1] }
    });
    label.bind();
    expect(module, [](GCodeRuntimeState &state) {
      REQUIRE(state.pop().getInteger() == IntConstants[0]);
      REQUIRE_THROWS(state.pop());
      REQUIRE(state.getScope().getNumbered().get(0).getInteger() == IntConstants[2]);
      REQUIRE(state.getScope().getNumbered().get(1).getInteger() == IntConstants[1]);
      REQUIRE(state.getScope().getNumbered().get(2).getInteger() == IntConstants[0]);
    });
  }
  SECTION("Return without arguments") {
    GCodeIRModule module;
    auto &label = module.getNamedLabel("label");
    module.registerProcedure(0, "label");
    make_ir(module, {
      { GCodeIROpcode::Push, IntConstants[0] },
      { GCodeIROpcode::Push, 0 },
      { GCodeIROpcode::Call, 0 },
      { GCodeIROpcode::Push, IntConstants[1] },
      { GCodeIROpcode::Jump, 6 }
    });
    label.bind();
    module.appendInstruction(GCodeIROpcode::Ret, 0);
    expect(module, [](GCodeRuntimeState &state) {
      REQUIRE(state.pop().getInteger() == IntConstants[1]);
      REQUIRE(state.pop().getInteger() == IntConstants[0]);
      REQUIRE_THROWS(state.pop());
    });
  }
  SECTION("Return with arguments") {
    GCodeIRModule module;
    auto &label = module.getNamedLabel("label");
    module.registerProcedure(0, "label");
    make_ir(module, {
      { GCodeIROpcode::Push, IntConstants[0] },
      { GCodeIROpcode::Push, 0 },
      { GCodeIROpcode::Call, 0 },
      { GCodeIROpcode::Push, IntConstants[1] },
      { GCodeIROpcode::Jump, 9 }
    });
    label.bind();
    module.appendInstruction(GCodeIROpcode::Push, IntConstants[2]);
    module.appendInstruction(GCodeIROpcode::Push, IntConstants[1]);
    module.appendInstruction(GCodeIROpcode::Push, IntConstants[0]);
    module.appendInstruction(GCodeIROpcode::Ret, 3);
    expect(module, [](GCodeRuntimeState &state) {
      REQUIRE(state.pop().getInteger() == IntConstants[1]);
      REQUIRE(state.pop().getInteger() == IntConstants[0]);
      REQUIRE_THROWS(state.pop());
      REQUIRE(state.getScope().getNumbered().get(0).getInteger() == IntConstants[2]);
      REQUIRE(state.getScope().getNumbered().get(1).getInteger() == IntConstants[1]);
      REQUIRE(state.getScope().getNumbered().get(2).getInteger() == IntConstants[0]);
    });
  }
  SECTION("Function invocation") {
    GCodeIRModule ir;
    make_ir(ir, {
      { GCodeIROpcode::Push, FloatConstants[0] },
      { GCodeIROpcode::Push, 1 },
      { GCodeIROpcode::Invoke, ir.getSymbolId("SIN") },
      { GCodeIROpcode::Push, FloatConstants[0] },
      { GCodeIROpcode::Push, FloatConstants[1] },
      { GCodeIROpcode::Push, 2 },
      { GCodeIROpcode::Invoke, ir.getSymbolId("ATAN") },
      { GCodeIROpcode::Push, 0 },
      { GCodeIROpcode::Push, 1 },
      { GCodeIROpcode::Invoke, ir.getSymbolId("EXISTS") },
      { GCodeIROpcode::Push },
      { GCodeIROpcode::Push, 1 },
      { GCodeIROpcode::Invoke, ir.getSymbolId("EXISTS") },
      { GCodeIROpcode::Push, 0 },
      { GCodeIROpcode::Push, 1 },
      { GCodeIROpcode::Invoke, ir.getSymbolId("ABCD") }
    });
    expect(ir, [](GCodeRuntimeState &state) {
      REQUIRE(state.pop().getInteger() == 0);
      REQUIRE(state.pop().getInteger() == 0);
      REQUIRE(state.pop().getInteger() == 1);
      REQUIRE(state.pop().getFloat() == atan2(FloatConstants[1], FloatConstants[0]));
      REQUIRE(state.pop().getFloat() == sin(FloatConstants[0]));
      REQUIRE_THROWS(state.pop());
    });
  }
}

static void test_predefined_functions(GCodeFunctionScope &scope) {
  double arg = -0.7;
  double arg2 = 6.28;
  std::vector<GCodeRuntimeValue> args = { arg, arg2 };
  std::vector<GCodeRuntimeValue> args2 = { arg2, arg };
  REQUIRE(scope.hasFunction("ATAN"));
  REQUIRE(scope.invoke("ATAN", args).getFloat() == atan2(arg, arg2));
  REQUIRE(scope.hasFunction("ABS"));
  REQUIRE(scope.invoke("ABS", args).getFloat() == fabs(arg));
  REQUIRE(scope.hasFunction("ACOS"));
  REQUIRE(scope.invoke("ACOS", args).getFloat() == acos(arg));
  REQUIRE(scope.hasFunction("ASIN"));
  REQUIRE(scope.invoke("ASIN", args).getFloat() == asin(arg));
  REQUIRE(scope.hasFunction("COS"));
  REQUIRE(scope.invoke("COS", args).getFloat() == cos(arg));
  REQUIRE(scope.hasFunction("EXP"));
  REQUIRE(scope.invoke("EXP", args).getFloat() == exp(arg));
  REQUIRE(scope.hasFunction("FIX"));
  REQUIRE(scope.invoke("FIX", args).getFloat() == floor(arg));
  REQUIRE(scope.hasFunction("FUP"));
  REQUIRE(scope.invoke("FUP", args).getFloat() == ceil(arg));
  REQUIRE(scope.hasFunction("ROUND"));
  REQUIRE(scope.invoke("ROUND", args).getFloat() == round(arg));
  REQUIRE(scope.hasFunction("LN"));
  REQUIRE(scope.invoke("LN", args2).getFloat() == log(arg2));
  REQUIRE(scope.hasFunction("SIN"));
  REQUIRE(scope.invoke("SIN", args).getFloat() == sin(arg));
  REQUIRE(scope.hasFunction("SQRT"));
  REQUIRE(scope.invoke("SQRT", args2).getFloat() == sqrt(arg2));
  REQUIRE(scope.hasFunction("TAN"));
  REQUIRE(scope.invoke("TAN", args).getFloat() == tan(arg));
  REQUIRE(scope.hasFunction("EXISTS"));
  REQUIRE(scope.invoke("EXISTS", args).getInteger() == 1);
}

TEST_CASE("Interpreter operations") {
  auto ir = make_ir({
    { GCodeIROpcode::Push, 0 },
    { GCodeIROpcode::Syscall, static_cast<int64_t>(GCodeSyscallType::General) },
    { GCodeIROpcode::Push, 0 },
    { GCodeIROpcode::Syscall, static_cast<int64_t>(GCodeSyscallType::General) }
  });
  class GCodeLocalInterpreter : public GCodeInterpreter {
    public:
    GCodeLocalInterpreter(GCodeIRModule &module, bool &exec)
      : GCodeInterpreter(module), exec(exec) {}
    GCodeVariableScope &getSystemScope() override {
      return scope;
    }
  protected:
    void syscall(GCodeSyscallType syscall, const GCodeRuntimeValue &function, const GCodeScopedDictionary<unsigned char> &args) override {
      REQUIRE_FALSE(this->exec);
      this->exec = true;
      REQUIRE_NOTHROW(this->getState());
      REQUIRE_NOTHROW(this->stop());
      REQUIRE_THROWS(this->getState());
      test_predefined_functions(this->getFunctions());
    }
  private:
    GCodeCascadeVariableScope scope;
    bool &exec;
  };
  bool exec = false;
  GCodeLocalInterpreter interp(*ir, exec);
  interp.execute();
  REQUIRE(exec);
}

TEST_CASE("Error handling") {
  class GCodeLocalInterpreter : public GCodeInterpreter {
    public:
    GCodeLocalInterpreter(GCodeIRModule &module)
      : GCodeInterpreter(module) {}
    GCodeVariableScope &getSystemScope() override {
      return scope;
    }
  protected:
    void syscall(GCodeSyscallType syscall, const GCodeRuntimeValue &function, const GCodeScopedDictionary<unsigned char> &args) override {}
  private:
    GCodeCascadeVariableScope scope;
  };
  SECTION("With source position") {
    GCodeLib::Parser::SourcePosition src("test", 1, 2, 3);
    GCodeIRModule module;
    auto pos = module.newPositionRegister(src);
    module.appendInstruction(GCodeIROpcode::Dup);
    pos.reset();
    GCodeLocalInterpreter interp(module);
    try {
      interp.execute();
      REQUIRE(false);
    } catch (GCodeRuntimeError &err) {
      REQUIRE(err.getLocation().has_value());
      REQUIRE(err.getLocation().value().getLine() == src.getLine());
      REQUIRE(err.getLocation().value().getColumn() == src.getColumn());
    }
  }
  SECTION("Without source position") {
    GCodeIRModule module;
    module.appendInstruction(GCodeIROpcode::Dup);
    GCodeLocalInterpreter interp(module);
    try {
      interp.execute();
      REQUIRE(false);
    } catch (GCodeRuntimeError &err) {
      REQUIRE_FALSE(err.getLocation().has_value());
    }
  }
}