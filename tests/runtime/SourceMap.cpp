#include "catch.hpp"
#include "gcodelib/runtime/SourceMap.h"

using namespace GCodeLib::Parser;
using namespace GCodeLib::Runtime;

bool operator==(const SourcePosition &p1, const SourcePosition &p2) {
  return p1.getTag().compare(p2.getTag()) == 0 &&
    p1.getLine() == p2.getLine() &&
    p1.getColumn() == p2.getColumn() &&
    p1.getChecksum() == p2.getChecksum();
}

TEST_CASE("Source block") {
  SourcePosition position("test", 1, 2, 3);
  const std::size_t START = 10;
  const std::size_t LENGTH = 100;
  IRSourceBlock block(position, START, LENGTH);
  REQUIRE(block.getSourcePosition() == position);
  REQUIRE(block.getStartAddress() == START);
  REQUIRE(block.getLength() == LENGTH);
  REQUIRE(block.includes(START));
  REQUIRE(block.includes(START + LENGTH / 2));
  REQUIRE(block.includes(START + LENGTH - 1));
  REQUIRE_FALSE(block.includes(-START));
  REQUIRE_FALSE(block.includes(START - 1));
  REQUIRE_FALSE(block.includes(START + LENGTH));
}

TEST_CASE("Source map") {
  SourcePosition pos[] = {
    SourcePosition("test", 1, 2, 3),
    SourcePosition("test", 2, 3, 4),
    SourcePosition("test", 3, 4, 5)
  };
  IRSourceMap map;
  REQUIRE(!map.locate(0).has_value());
  map.addBlock(pos[0], 0, 10);
  map.addBlock(pos[1], 0, 5);
  map.addBlock(pos[2], 1, 3);

  auto l1 = map.locate(2);
  auto l2 = map.locate(4);
  auto l3 = map.locate(6);
  auto l4 = map.locate(10);
  REQUIRE(l1.has_value());
  REQUIRE(l1.value() == pos[2]);
  REQUIRE(l2.has_value());
  REQUIRE(l2.value() == pos[1]);
  REQUIRE(l3.has_value());
  REQUIRE(l3.value() == pos[0]);
  REQUIRE_FALSE(l4.has_value());
}