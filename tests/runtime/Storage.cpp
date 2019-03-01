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
  REQUIRE_NOTHROW(parent.remove('A'));
  REQUIRE_FALSE(parent.has('A'));
  REQUIRE_FALSE(child.has('A'));
  REQUIRE(child.has('B'));
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

TEST_CASE("Custom variable scope") {
  GCodeScopedDictionary<int64_t> numbered;
  GCodeScopedDictionary<std::string> named;
  GCodeCustomVariableScope scope(numbered, named);
  REQUIRE(&scope.getNamed() == &named);
  REQUIRE(&scope.getNumbered() == &numbered);
}
