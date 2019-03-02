#include "catch.hpp"
#include "gcodelib/runtime/Storage.h"

using namespace GCodeLib::Runtime;
using Type = GCodeRuntimeValue::Type;

TEST_CASE("Scoped dictionary") {
  GCodeScopedDictionary<char> parent;
  GCodeScopedDictionary<char> child(&parent);
  REQUIRE(parent.getParent() == nullptr);
  REQUIRE(child.getParent() == &parent);
  REQUIRE_FALSE(parent.has('A'));
  REQUIRE_FALSE(parent.hasOwn('A'));
  REQUIRE_FALSE(child.has('A'));
  REQUIRE_FALSE(child.hasOwn('A'));
  REQUIRE(parent.get('A').is(Type::None));
  REQUIRE(child.get('A').is(Type::None));
  REQUIRE_NOTHROW(parent.put('A', 100L));
  REQUIRE(parent.has('A'));
  REQUIRE(parent.hasOwn('A'));
  REQUIRE(child.has('A'));
  REQUIRE_FALSE(child.hasOwn('A'));
  REQUIRE(parent.get('A').is(Type::Integer));
  REQUIRE(parent.get('A').getInteger() == 100L);
  REQUIRE(child.get('A').is(Type::Integer));
  REQUIRE(child.get('A').getInteger() == 100L);
  REQUIRE_NOTHROW(child.put('A', 200L));
  REQUIRE_NOTHROW(child.put('B', 300L));
  REQUIRE(parent.get('A').getInteger() == 200L);
  REQUIRE_FALSE(parent.has('B'));
  REQUIRE(child.get('A').getInteger() == 200L);
  REQUIRE(child.get('B').getInteger() == 300L);
  REQUIRE(child.hasOwn('B'));
  REQUIRE_NOTHROW(child.remove('A'));
  REQUIRE_FALSE(parent.has('A'));
  REQUIRE_FALSE(child.has('A'));
  REQUIRE(child.has('B'));
  REQUIRE_NOTHROW(child.remove('B'));
  REQUIRE_FALSE(child.has('B'));
  REQUIRE_FALSE(child.has('C'));
  REQUIRE_NOTHROW(child.remove('C'));
  REQUIRE_FALSE(child.has('C'));
  REQUIRE_NOTHROW(parent.put('A', 100L));
  REQUIRE_NOTHROW(child.clear());
  REQUIRE_FALSE(child.has('B'));
  REQUIRE(parent.has('A'));
  REQUIRE(child.has('A'));
}

TEST_CASE("Virtual dictionary") {
  GCodeVirtualDictionary<char> dict;
  REQUIRE_FALSE(dict.has('A'));
  REQUIRE_FALSE(dict.hasSupplier('A'));
  REQUIRE(dict.get('A').is(Type::None));
  REQUIRE_NOTHROW(dict.put('A', 100L));
  REQUIRE_FALSE(dict.has('A'));
  REQUIRE_FALSE(dict.hasSupplier('A'));

  int i = 0;
  REQUIRE_NOTHROW(dict.putSupplier('A', [&]() {
    return GCodeRuntimeValue(static_cast<int64_t>(++i));
  }));
  REQUIRE(dict.has('A'));
  REQUIRE(dict.hasSupplier('A'));
  REQUIRE(dict.get('A').is(Type::Integer));
  REQUIRE(dict.get('A').getInteger() == i);
  REQUIRE(dict.get('A').getInteger() == i);
  REQUIRE(dict.get('A').getInteger() == i);
  REQUIRE_NOTHROW(dict.remove('A'));
  REQUIRE(dict.get('A').getInteger() == i);
  REQUIRE_NOTHROW(dict.removeSupplier('A'));
  REQUIRE_FALSE(dict.has('A'));
  REQUIRE_NOTHROW(dict.putSupplier('A', []()->GCodeRuntimeValue { return 0L; }));
  REQUIRE(dict.get('A').getInteger() == 0);
  REQUIRE_NOTHROW(dict.clear());
  REQUIRE(dict.has('A'));
  REQUIRE_NOTHROW(dict.clearSuppliers());
  REQUIRE_FALSE(dict.has('A'));
  REQUIRE_FALSE(dict.hasSupplier('A'));
}

TEST_CASE("Dictionary multiplexer") {
  GCodeScopedDictionary<char> master;
  GCodeScopedDictionary<char> slave;
  GCodeDictionaryMultiplexer<char> dict1(master);
  GCodeDictionaryMultiplexer<char> dict2(master, &slave);
  REQUIRE_NOTHROW(slave.put('B', 1000L));
  REQUIRE_FALSE(dict1.has('A'));
  REQUIRE_FALSE(dict2.has('A'));
  REQUIRE(dict1.get('A').is(Type::None));
  REQUIRE(dict2.get('A').is(Type::None));
  REQUIRE_NOTHROW(dict1.put('A', 100L));
  REQUIRE(dict1.has('A'));
  REQUIRE(dict2.has('A'));
  REQUIRE(dict1.get('A').getInteger() == 100);
  REQUIRE(dict2.get('A').getInteger() == 100);
  REQUIRE_FALSE(dict1.has('B'));
  REQUIRE(dict2.has('B'));
  REQUIRE(dict1.get('B').is(Type::None));
  REQUIRE(dict2.get('B').getInteger() == 1000);
  REQUIRE_NOTHROW(dict2.put('B', 0L));
  REQUIRE(dict1.has('B'));
  REQUIRE(dict2.has('B'));
  REQUIRE(dict1.get('B').getInteger() == 0);
  REQUIRE(dict2.get('B').getInteger() == 0);
  REQUIRE_NOTHROW(dict1.remove('B'));
  REQUIRE(dict1.has('A'));
  REQUIRE_FALSE(dict1.has('B'));
  REQUIRE(dict2.has('A'));
  REQUIRE(dict2.has('B'));
  REQUIRE_NOTHROW(dict1.clear());
  REQUIRE_FALSE(dict1.has('A'));
  REQUIRE_FALSE(dict2.has('A'));
  REQUIRE(dict2.has('B'));
}

TEST_CASE("Custom variable scope") {
  GCodeScopedDictionary<int64_t> numbered;
  GCodeScopedDictionary<std::string> named;
  GCodeCustomVariableScope scope(numbered, named);
  REQUIRE(&scope.getNamed() == &named);
  REQUIRE(&scope.getNumbered() == &numbered);
}

TEST_CASE("Cascade variable scope") {
  GCodeScopedDictionary<int64_t> numbered;
  GCodeScopedDictionary<std::string> named;
  GCodeCustomVariableScope scope1(numbered, named);
  GCodeCascadeVariableScope scope2(&scope1);

  SECTION("Variable inheritance") {
    REQUIRE_FALSE(scope2.getNamed().has("abc"));
    REQUIRE_FALSE(scope2.getNumbered().has(1));
    scope1.getNamed().put("abc", 100L);
    scope1.getNumbered().put(1, 200L);
    REQUIRE(scope2.getNamed().has("abc"));
    REQUIRE(scope2.getNumbered().has(1));
    REQUIRE(scope2.getNamed().get("abc").getInteger() == 100L);
    REQUIRE(scope2.getNumbered().get(1).getInteger() == 200L);
  }
  SECTION("Own variables") {
    REQUIRE_FALSE(scope1.getNamed().has("abc"));
    REQUIRE_FALSE(scope1.getNumbered().has(1));
    scope2.getNamed().put("abc", 100L);
    scope2.getNumbered().put(1, 200L);
    REQUIRE_FALSE(scope1.getNamed().has("abc"));
    REQUIRE_FALSE(scope1.getNumbered().has(1));
    REQUIRE(scope2.getNamed().has("abc"));
    REQUIRE(scope2.getNumbered().has(1));
  }
}