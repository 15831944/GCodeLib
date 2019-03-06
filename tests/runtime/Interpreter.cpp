#include "catch.hpp"
#include "gcodelib/runtime/Interpreter.h"
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
  std::function<void(GCodeTestInterpreter &)> before = [](GCodeTestInterpreter &) {}) {
  bool result = false;
  GCodeTestInterpreter interp(module, [&](GCodeRuntimeState &state) {
    hook(state);
    result = true;
  });
  before(interp);
  interp.execute();
  REQUIRE(result);
}

static constexpr int64_t IntConstants[] = {
  100,
  200
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