#include "catch.hpp"
#include "gcodelib/runtime/IR.h"
#include <sstream>
#include <iostream>

using namespace GCodeLib::Runtime;

TEST_CASE("IR instruction") {
  SECTION("Basic") {
    GCodeIRInstruction instr1(GCodeIROpcode::Push);
    GCodeIRInstruction instr2(GCodeIROpcode::Invoke, 100L);
    REQUIRE(instr1.getOpcode() == GCodeIROpcode::Push);
    REQUIRE(instr2.getOpcode() == GCodeIROpcode::Invoke);
    REQUIRE(instr1.getValue().is(GCodeRuntimeValue::Type::None));
    REQUIRE(instr2.getValue().getInteger() == 100);
    REQUIRE_NOTHROW(instr1.setValue(200L));
    REQUIRE_NOTHROW(instr2.setValue(300L));
    REQUIRE(instr1.getValue().getInteger() == 200);
    REQUIRE(instr2.getValue().getInteger() == 300);
  }
  SECTION("IO") {
    std::stringstream ss;
    GCodeIRModule module;
    std::size_t id = module.getSymbolId("Hello");
    SECTION("") {
      GCodeIRInstruction(GCodeIROpcode::SetArg, static_cast<int64_t>('A')).dump(ss, module);
      REQUIRE(ss.str().compare("SyscalArgument      A") == 0);
    }
    SECTION("") {
      GCodeIRInstruction(GCodeIROpcode::Syscall, static_cast<int64_t>(GCodeSyscallType::Misc)).dump(ss, module);
      REQUIRE(ss.str().compare("Syscall             M") == 0);
    }
    SECTION("") {
      GCodeIRInstruction(GCodeIROpcode::Invoke, static_cast<int64_t>(id)).dump(ss, module);
      REQUIRE(ss.str().compare("Invoke              Hello") == 0);
    }
    SECTION("") {
      GCodeIRInstruction(GCodeIROpcode::LoadNamed, static_cast<int64_t>(id)).dump(ss, module);
      REQUIRE(ss.str().compare("LoadNamed           Hello") == 0);
    }
    SECTION("") {
      GCodeIRInstruction(GCodeIROpcode::StoreNamed, static_cast<int64_t>(id)).dump(ss, module);
      REQUIRE(ss.str().compare("StoreNamed          Hello") == 0);
    }
    SECTION("") {
      GCodeIRInstruction(GCodeIROpcode::Test, static_cast<int64_t>(GCodeCompare::Equals)).dump(ss, module);
      REQUIRE(ss.str().compare("Test                00000001") == 0);
    }
    SECTION("") {
      GCodeIRInstruction(GCodeIROpcode::Push, 100L).dump(ss, module);
      REQUIRE(ss.str().compare("Push                100") == 0);
    }
    SECTION("") {
      GCodeIRInstruction(GCodeIROpcode::Push).dump(ss, module);
      REQUIRE(ss.str().compare("Push                ") == 0);
    }
  }
}

TEST_CASE("IR label") {
  GCodeIRModule module;
  REQUIRE(module.linked());
  GCodeIRLabel &label = module.getNamedLabel("test");
  REQUIRE_FALSE(module.linked());
  REQUIRE_FALSE(label.bound());
  REQUIRE_THROWS(label.getAddress());
  REQUIRE_NOTHROW(label.jump());
  REQUIRE_NOTHROW(label.jumpIf());
  REQUIRE_NOTHROW(label.bind());
  REQUIRE(label.bound());
  REQUIRE(module.linked());
  REQUIRE(label.getAddress() == module.length());
  REQUIRE_THROWS(label.bind());
  REQUIRE_NOTHROW(label.jump());
  REQUIRE_NOTHROW(label.jumpIf());

  REQUIRE(module.at(0).getOpcode() == GCodeIROpcode::Jump);
  REQUIRE(module.at(0).getValue().getInteger() == label.getAddress());
  REQUIRE(module.at(1).getOpcode() == GCodeIROpcode::JumpIf);
  REQUIRE(module.at(1).getValue().getInteger() == label.getAddress());
  REQUIRE(module.at(2).getOpcode() == GCodeIROpcode::Jump);
  REQUIRE(module.at(2).getValue().getInteger() == label.getAddress());
  REQUIRE(module.at(3).getOpcode() == GCodeIROpcode::JumpIf);
  REQUIRE(module.at(3).getValue().getInteger() == label.getAddress());
}

TEST_CASE("IR module") {
  GCodeIRModule module;
  SECTION("Instructions") {
    REQUIRE(module.length() == 0);
    REQUIRE_THROWS(module.at(0));
    REQUIRE_NOTHROW(module.appendInstruction(GCodeIROpcode::Power));
    REQUIRE_NOTHROW(module.appendInstruction(GCodeIROpcode::Push, 3.14));
    REQUIRE(module.length() == 2);
    REQUIRE(module.at(0).getOpcode() == GCodeIROpcode::Power);
    REQUIRE(module.at(1).getValue().getFloat() == 3.14);
  }
  SECTION("Symbols") {
    std::size_t hello = module.getSymbolId("hello");
    std::size_t world = module.getSymbolId("world");
    REQUIRE(hello == module.getSymbolId("hello"));
    REQUIRE(world == module.getSymbolId("world"));
    REQUIRE_FALSE(hello == world);
    REQUIRE(module.getSymbol(hello).compare("hello") == 0);
    REQUIRE(module.getSymbol(world).compare("world") == 0);
    REQUIRE(module.getSymbol(10000).compare("") == 0);
  }
  SECTION("Labels") {
    auto label1 = module.newLabel();
    REQUIRE_FALSE(label1 == nullptr);
    auto &label2 = module.getNamedLabel("test1");
    auto &label3 = module.getNamedLabel("test2");
    REQUIRE(&label2 == &module.getNamedLabel("test1"));
    REQUIRE(&label3 == &module.getNamedLabel("test2"));
    REQUIRE_FALSE(&label2 == &label3);
    REQUIRE_NOTHROW(module.registerProcedure(0, "test1"));
    REQUIRE_THROWS(module.registerProcedure(1, "test3"));
    REQUIRE(&module.getProcedure(0) == &label2);
    REQUIRE_THROWS(module.getProcedure(1));
  }
  SECTION("Source maps") {
    GCodeLib::Parser::SourcePosition pos("", 1, 2, 3);
    {
      auto reg = module.newPositionRegister(pos);
      module.appendInstruction(GCodeIROpcode::Push, 100L);
    }
    REQUIRE(module.getSourceMap().locate(0).has_value());
    REQUIRE(module.getSourceMap().locate(0).value().getLine() == pos.getLine());
    REQUIRE(module.getSourceMap().locate(0).value().getColumn() == pos.getColumn());
  }
}