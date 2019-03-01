#include "catch.hpp"
#include "gcodelib/Error.h"

using namespace GCodeLib;

TEST_CASE("GCodeLibException") {
  const std::string &MSG = "Hello, world!";
  SECTION("Error messages") {
    try {
      throw GCodeLibException(MSG);
    } catch (GCodeLibException &ex) {
      REQUIRE(ex.getMessage().compare(MSG) == 0);
      REQUIRE(MSG.compare(ex.what()) == 0);
      REQUIRE(!ex.getLocation().has_value());
    }
    try {
      throw GCodeLibException(MSG.c_str());
    } catch (GCodeLibException &ex) {
      REQUIRE(ex.getMessage().compare(MSG) == 0);
      REQUIRE(MSG.compare(ex.what()) == 0);
      REQUIRE(!ex.getLocation().has_value());
    }
  }
  SECTION("Error location") {
    Parser::SourcePosition pos(MSG, 1, 2, 3);
    GCodeLibException ex(MSG, pos);
    GCodeLibException ex2(MSG.c_str(), pos);
    REQUIRE(ex.getLocation().has_value());
    REQUIRE(ex2.getLocation().has_value());
    REQUIRE(ex.getMessage().compare(MSG) == 0);
    REQUIRE(ex2.getMessage().compare(MSG) == 0);
    REQUIRE(MSG.compare(ex.what()) == 0);
    REQUIRE(MSG.compare(ex2.what()) == 0);
    REQUIRE(ex.getLocation().value().getTag().compare(pos.getTag()) == 0);
    REQUIRE(ex2.getLocation().value().getTag().compare(pos.getTag()) == 0);
    REQUIRE(ex.getLocation().value().getLine() == pos.getLine());
    REQUIRE(ex2.getLocation().value().getLine() == pos.getLine());
    REQUIRE(ex.getLocation().value().getColumn() == pos.getColumn());
    REQUIRE(ex2.getLocation().value().getColumn() == pos.getColumn());
    REQUIRE(ex.getLocation().value().getChecksum() == pos.getChecksum());
    REQUIRE(ex2.getLocation().value().getChecksum() == pos.getChecksum());
  }
}