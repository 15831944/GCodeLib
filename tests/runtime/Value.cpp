#include "catch.hpp"
#include "gcodelib/runtime/Value.h"
#include <sstream>

using namespace GCodeLib::Runtime;
using Type = GCodeRuntimeValue::Type;

TEST_CASE("Runtime values") {
  const int64_t INT = 42;
  const double REAL = 3.14;
  const std::string &STR = "Hello, world!";
  GCodeRuntimeValue none;
  GCodeRuntimeValue integer(INT);
  GCodeRuntimeValue real(REAL);
  GCodeRuntimeValue string(STR);
  SECTION("Value types") {
    REQUIRE(none.getType() == Type::None);
    REQUIRE(none.is(Type::None));
    REQUIRE(integer.getType() == Type::Integer);
    REQUIRE(integer.is(Type::Integer));
    REQUIRE(real.getType() == Type::Float);
    REQUIRE(real.is(Type::Float));
    REQUIRE(string.getType() == Type::String);
    REQUIRE(string.is(Type::String));
  }
  SECTION("Values") {
    REQUIRE(integer.getInteger() == INT);
    REQUIRE(real.getInteger(INT) == INT);
    REQUIRE(real.getFloat() == Approx(REAL));
    REQUIRE(integer.getFloat(REAL) == Approx(REAL));
    REQUIRE(string.getString().compare(STR) == 0);
    REQUIRE(integer.getString(STR).compare(STR) == 0);
  }
  SECTION("Value constructor") {
    REQUIRE(GCodeRuntimeValue::Empty.getType() == Type::None);
    REQUIRE(GCodeRuntimeValue::Empty.is(Type::None));
    REQUIRE_NOTHROW(GCodeRuntimeValue::Empty.assertType(Type::None));
    GCodeRuntimeValue copy(integer);
    REQUIRE(copy.getType() == Type::Integer);
    REQUIRE(copy.getInteger() == INT);
    copy = real;
    REQUIRE(copy.getType() == Type::Float);
    REQUIRE(copy.getFloat() == Approx(REAL));
  }
  SECTION("Value conversions") {
    REQUIRE(integer.asInteger() == INT);
    REQUIRE(real.asInteger() == 3);
    REQUIRE(string.asInteger() == 0);
    REQUIRE(integer.asFloat() == Approx(static_cast<double>(INT)));
    REQUIRE(real.asFloat() == Approx(REAL));
    REQUIRE(string.asFloat() == Approx(0.0));
    REQUIRE(integer.asString().compare(std::to_string(INT)) == 0);
    REQUIRE(real.asString().compare(std::to_string(REAL)) == 0);
    REQUIRE(string.asString().compare(STR) == 0);
  }
  SECTION("Value type asserts") {
    REQUIRE((integer.isNumeric() && real.isNumeric()));
    REQUIRE_FALSE((none.isNumeric() || string.isNumeric()));
    REQUIRE_NOTHROW(integer.assertType(Type::Integer).getInteger() == INT);
    REQUIRE_NOTHROW(real.assertType(Type::Float).getFloat() == Approx(REAL));
    REQUIRE_NOTHROW(none.assertType(Type::None));
    REQUIRE_NOTHROW(string.assertType(Type::String).getString().compare(STR) == 0);
    REQUIRE_THROWS(integer.assertType(Type::Float));
    REQUIRE_THROWS(real.assertType(Type::Integer));
    REQUIRE_THROWS(none.assertType(Type::String));
    REQUIRE_THROWS(string.assertType(Type::None));
    REQUIRE_NOTHROW(integer.assertNumeric().getInteger() == INT);
    REQUIRE_NOTHROW(real.assertNumeric().asFloat() == Approx(REAL));
    REQUIRE_THROWS(none.assertNumeric());
    REQUIRE_THROWS(string.assertNumeric());
  }
  SECTION("Value output") {
    std::stringstream ss;
    ss << none << integer << real << string;
    REQUIRE(ss.str().compare("423.14" + STR) == 0);
  }
}