#include "gcodelib/runtime/Interpreter.h"
#include "gcodelib/runtime/Error.h"
#include <cmath>

namespace GCodeLib::Runtime {

  static GCodeFunctionScope::FnType wrap_math_function(double(*fn)(double)) {
    return [=](const std::vector<GCodeRuntimeValue> &args) {
      return fn(args.at(0).asFloat());
    };
  }

  static GCodeFunctionScope::FnType wrap_math_function(double(*fn)(double, double)) {
    return [=](const std::vector<GCodeRuntimeValue> &args) {
      return fn(args.at(0).asFloat(), args.at(1).asFloat());
    };
  }

  static void bind_default_functions(GCodeFunctionScope &scope) {
    scope.bindFunction("ATAN", wrap_math_function(atan2));
    scope.bindFunction("ABS", wrap_math_function(fabs));
    scope.bindFunction("ACOS", wrap_math_function(acos));
    scope.bindFunction("ASIN", wrap_math_function(asin));
    scope.bindFunction("COS", wrap_math_function(cos));
    scope.bindFunction("EXP", wrap_math_function(exp));
    scope.bindFunction("FIX", wrap_math_function(floor));
    scope.bindFunction("FUP", wrap_math_function(ceil));
    scope.bindFunction("ROUND", wrap_math_function(round));
    scope.bindFunction("LN", wrap_math_function(log));
    scope.bindFunction("SIN", wrap_math_function(sin));
    scope.bindFunction("SQRT", wrap_math_function(sqrt));
    scope.bindFunction("TAN", wrap_math_function(tan));
    scope.bindFunction("EXISTS", [](const std::vector<GCodeRuntimeValue> &args) {
      return GCodeRuntimeValue(args.at(0).is(GCodeRuntimeValue::Type::None) ? 0L : 1L);
    });
  }

  GCodeInterpreter::GCodeInterpreter(GCodeIRModule &module)
    : module(module) {
    bind_default_functions(this->functions);
  }
  
  void GCodeInterpreter::execute() {
    GCodeCascadeVariableScope sessionScope(&this->getSystemScope());
    this->state = GCodeRuntimeState(sessionScope);
    GCodeRuntimeState &frame = this->getState();
    GCodeScopedDictionary<unsigned char> args;
    while (this->state.has_value() && frame.getPC() < this->module.length()) {
      const GCodeIRInstruction &instr = this->module.at(frame.nextPC());
      switch (instr.getOpcode()) {
        case GCodeIROpcode::Push:
          frame.push(instr.getValue());
          break;
        case GCodeIROpcode::Prologue:
          args.clear();
          break;
        case GCodeIROpcode::SetArg: {
          unsigned char key = static_cast<unsigned char>(instr.getValue().assertNumeric().asInteger());
          args.put(key, frame.pop());
        } break;
        case GCodeIROpcode::Syscall: {
          GCodeSyscallType type = static_cast<GCodeSyscallType>(instr.getValue().assertNumeric().asInteger());
          GCodeRuntimeValue function = frame.pop();
          this->syscall(type, function, args);
        } break;
        case GCodeIROpcode::Jump: {
          std::size_t pc = static_cast<std::size_t>(instr.getValue().assertNumeric().asInteger());
          frame.jump(pc);
        } break;
        case GCodeIROpcode::JumpIf: {
          std::size_t pc = static_cast<std::size_t>(instr.getValue().assertNumeric().asInteger());
          bool cond = frame.pop().assertNumeric().asInteger() != 0;
          if (cond) {
            frame.jump(pc);
          }
        } break;
        case GCodeIROpcode::Call: {
          int64_t pid = frame.pop().assertNumeric().asInteger();
          frame.call(this->module.getProcedure(pid).getAddress());
          std::size_t argc = static_cast<std::size_t>(instr.getValue().assertNumeric().getInteger());
          while (argc-- > 0) {
            frame.getScope().getNumbered().put(argc, frame.pop());
          }
        } break;
        case GCodeIROpcode::Ret: {
          frame.ret();
          std::size_t argc = static_cast<std::size_t>(instr.getValue().assertNumeric().getInteger());
          while (argc-- > 0) {
            frame.getScope().getNumbered().put(argc, frame.pop());
          }
        } break;
        case GCodeIROpcode::Dup:
          frame.dup();
          break;
        case GCodeIROpcode::Negate:
          frame.negate();
          break;
        case GCodeIROpcode::Add:
          frame.add();
          break;
        case GCodeIROpcode::Subtract:
          frame.subtract();
          break;
        case GCodeIROpcode::Multiply:
          frame.multiply();
          break;
        case GCodeIROpcode::Divide:
          frame.divide();
          break;
        case GCodeIROpcode::Power:
          frame.power();
          break;
        case GCodeIROpcode::Modulo:
          frame.modulo();
          break;
        case GCodeIROpcode::Compare:
          frame.compare();
          break;
        case GCodeIROpcode::Test: {
          int64_t mask = instr.getValue().assertNumeric().asInteger();
          frame.test(mask);
        } break;
        case GCodeIROpcode::And:
          frame.iand();
          break;
        case GCodeIROpcode::Or:
          frame.ior();
          break;
        case GCodeIROpcode::Xor:
          frame.ixor();
          break;
        case GCodeIROpcode::Not:
          frame.inot();
          break;
        case GCodeIROpcode::Invoke: {
          const std::string &functionId = this->module.getSymbol(instr.getValue().assertNumeric().getInteger());
          std::size_t argc = frame.pop().assertNumeric().asInteger();
          std::vector<GCodeRuntimeValue> args;
          while (argc--) {
            args.push_back(frame.pop());
          }
          frame.push(this->functions.invoke(functionId, args));
        } break;
        case GCodeIROpcode::LoadNumbered: {
          const GCodeRuntimeValue &value = frame.getScope().getNumbered().get(instr.getValue().assertNumeric().getInteger());
          frame.push(value);
        } break;
        case GCodeIROpcode::LoadNamed: {
          const std::string &symbol = this->module.getSymbol(static_cast<std::size_t>(instr.getValue().assertNumeric().getInteger()));
          const GCodeRuntimeValue &value = frame.getScope().getNamed().get(symbol);
          frame.push(value);
        } break;
        case GCodeIROpcode::StoreNumbered: {
          GCodeRuntimeValue value = frame.pop();
          frame.getScope().getNumbered().put(instr.getValue().assertNumeric().getInteger(), value);
        } break;
        case GCodeIROpcode::StoreNamed: {
          GCodeRuntimeValue value = frame.pop();
          const std::string &symbol = this->module.getSymbol(static_cast<std::size_t>(instr.getValue().assertNumeric().getInteger()));
          frame.getScope().getNamed().put(symbol, value);
        } break;
      }
    }
    this->state.reset();
  }

  GCodeFunctionScope &GCodeInterpreter::getFunctions() {
    return this->functions;
  }
  
  GCodeRuntimeState &GCodeInterpreter::getState() {
    if (this->state.has_value()) {
      return this->state.value();
    } else {
      throw GCodeRuntimeError("Execution state is not defined");
    }
  }

  void GCodeInterpreter::stop() {
    this->state.reset();
  }
}