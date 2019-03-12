#include "catch.hpp"
#include "gcodelib/runtime/Runtime.h"
#include <type_traits>
#include <cmath>

using namespace GCodeLib::Runtime;

TEST_CASE("Runtime basic operations") {
  GCodeCascadeVariableScope scope;
  GCodeRuntimeState state(scope);
  SECTION("PC operations") {
    REQUIRE(state.getPC() == 0);
    REQUIRE(state.nextPC() == 0);
    REQUIRE(state.getPC() == 1);
    REQUIRE_NOTHROW(state.jump(100));
    REQUIRE(state.nextPC() == 100);
    REQUIRE(state.getPC() == 101);
  }
  SECTION("Stack manipulations") {
    REQUIRE_THROWS(state.peek());
    REQUIRE_THROWS(state.pop());
    REQUIRE_NOTHROW(state.push(GCodeRuntimeValue(100L)));
    REQUIRE_NOTHROW(state.push(GCodeRuntimeValue(3.14)));
    REQUIRE_NOTHROW(state.push(GCodeRuntimeValue("Hello")));
    REQUIRE(state.peek().getString().compare("Hello") == 0);
    REQUIRE(state.pop().getString().compare("Hello") == 0);
    REQUIRE(state.peek().getFloat() == Approx(3.14));
    REQUIRE(state.pop().getFloat() == Approx(3.14));
    REQUIRE(state.peek().getInteger() == 100);
    REQUIRE(state.pop().getInteger() == 100);
    REQUIRE_THROWS(state.peek());
    REQUIRE_THROWS(state.pop());
    REQUIRE_THROWS(state.dup());
    REQUIRE_NOTHROW(state.push(GCodeRuntimeValue(200L)));
    REQUIRE_NOTHROW(state.dup());
    REQUIRE(state.pop().getInteger() == 200);
    REQUIRE(state.pop().getInteger() == 200);
    REQUIRE_THROWS(state.pop());
  }
}

template <typename T>
struct value_type_cast {};

template <>
struct value_type_cast<int64_t> {
  static int64_t get(const GCodeRuntimeValue &value) {
    return value.getInteger();
  }
};

template <>
struct value_type_cast<double> {
  static double get(const GCodeRuntimeValue &value) {
    return value.getFloat();
  }
};

template <typename A, typename B, typename E = void>
struct value_common_type {};

template <typename A, typename B>
struct value_common_type<A, B, typename std::enable_if<std::is_same<A, B>::value>::type> {
  using Type = A;
};

template <>
struct value_common_type<int64_t, double> {
  using Type = double;
};

template <>
struct value_common_type<double, int64_t> {
  using Type = double;
};

template <typename T, typename E = void>
struct value_comparison {
  static T get(T value) {
    return value;
  }
};

template <typename T>
struct value_comparison<T, typename std::enable_if<std::is_floating_point<T>::value>::type> {
  static Approx get(T value) {
    return Approx(value);
  }
};

template <typename T1, typename T2>
void arithmetic_test(GCodeRuntimeState &state, T1 V1, T2 V2) {
  using CT = typename value_common_type<T1, T2>::Type;
  auto cmp = &value_comparison<CT>::get;
  REQUIRE_NOTHROW(state.push(GCodeRuntimeValue(V1)));
  REQUIRE_NOTHROW(state.push(GCodeRuntimeValue(V2)));
  SECTION("Negation") {
    REQUIRE_NOTHROW(state.negate());
    REQUIRE(value_type_cast<T2>::get(state.pop()) == -V2);
    REQUIRE_NOTHROW(state.negate());
    REQUIRE(value_type_cast<T1>::get(state.pop()) == -V1);
    REQUIRE_THROWS(state.pop());
  }
  SECTION("Add") {
    REQUIRE_NOTHROW(state.add());
    REQUIRE(value_type_cast<CT>::get(state.pop()) == cmp(V1 + V2));
    REQUIRE_THROWS(state.pop());
  }
  SECTION("Subtract") {
    REQUIRE_NOTHROW(state.subtract());
    REQUIRE(value_type_cast<CT>::get(state.pop()) == cmp(V1 - V2));
    REQUIRE_THROWS(state.pop());
  }
  SECTION("Multiply") {
    REQUIRE_NOTHROW(state.multiply());
    REQUIRE(value_type_cast<CT>::get(state.pop()) == cmp(V1 * V2));
    REQUIRE_THROWS(state.pop());
  }
  SECTION("Divide") {
    REQUIRE_NOTHROW(state.divide());
    REQUIRE(value_type_cast<CT>::get(state.pop()) == cmp(V1 / V2));
    REQUIRE_THROWS(state.pop());
  }
  SECTION("Power") {
    REQUIRE_NOTHROW(state.power());
    REQUIRE(value_type_cast<double>::get(state.pop()) == value_comparison<double>::get(pow(V1, V2)));
    REQUIRE_THROWS(state.pop());
  }
  SECTION("Modulo") {
    REQUIRE_NOTHROW(state.modulo());
    REQUIRE(value_type_cast<CT>::get(state.pop()) == cmp(fmod(V1, V2)));
    REQUIRE_THROWS(state.pop());
  }
  SECTION("Bitwise and") {
    REQUIRE_NOTHROW(state.iand());
    REQUIRE(value_type_cast<int64_t>::get(state.pop()) == (static_cast<int64_t>(V1) & static_cast<int64_t>(V2)));
    REQUIRE_THROWS(state.pop());
  }
  SECTION("Bitwise or") {
    REQUIRE_NOTHROW(state.ior());
    REQUIRE(value_type_cast<int64_t>::get(state.pop()) == (static_cast<int64_t>(V1) | static_cast<int64_t>(V2)));
    REQUIRE_THROWS(state.pop());
  }
  SECTION("Bitwise xor") {
    REQUIRE_NOTHROW(state.ixor());
    REQUIRE(value_type_cast<int64_t>::get(state.pop()) == (static_cast<int64_t>(V1) ^ static_cast<int64_t>(V2)));
    REQUIRE_THROWS(state.pop());
  }
}

template <typename T1, typename T2>
void comparison_test(GCodeRuntimeState &state, T1 V1, T2 V2) {
  assert(V1 < V2);
  SECTION("Comparison") {
    REQUIRE_NOTHROW(state.push(GCodeRuntimeValue(V1)));
    REQUIRE_NOTHROW(state.push(GCodeRuntimeValue(V2)));
    REQUIRE_NOTHROW(state.compare());
    REQUIRE_NOTHROW(state.push(GCodeRuntimeValue(V2)));
    REQUIRE_NOTHROW(state.push(GCodeRuntimeValue(V1)));
    REQUIRE_NOTHROW(state.compare());
    REQUIRE_NOTHROW(state.push(GCodeRuntimeValue(V1)));
    REQUIRE_NOTHROW(state.push(GCodeRuntimeValue(V1)));
    REQUIRE_NOTHROW(state.compare());
    int64_t results[] = {
      state.pop().getInteger(),
      state.pop().getInteger(),
      state.pop().getInteger()
    };
    REQUIRE((results[0] & static_cast<int64_t>(GCodeCompare::Equals)) != 0);
    REQUIRE((results[1] & static_cast<int64_t>(GCodeCompare::Equals)) == 0);
    REQUIRE((results[2] & static_cast<int64_t>(GCodeCompare::Equals)) == 0);
    REQUIRE((results[0] & static_cast<int64_t>(GCodeCompare::NotEquals)) == 0);
    REQUIRE((results[1] & static_cast<int64_t>(GCodeCompare::NotEquals)) != 0);
    REQUIRE((results[2] & static_cast<int64_t>(GCodeCompare::NotEquals)) != 0);
    REQUIRE((results[0] & static_cast<int64_t>(GCodeCompare::Greater)) == 0);
    REQUIRE((results[1] & static_cast<int64_t>(GCodeCompare::Greater)) != 0);
    REQUIRE((results[2] & static_cast<int64_t>(GCodeCompare::Greater)) == 0);
    REQUIRE((results[0] & static_cast<int64_t>(GCodeCompare::Lesser)) == 0);
    REQUIRE((results[1] & static_cast<int64_t>(GCodeCompare::Lesser)) == 0);
    REQUIRE((results[2] & static_cast<int64_t>(GCodeCompare::Lesser)) != 0);
  }
  SECTION("Test") {
    REQUIRE_NOTHROW(state.push(GCodeRuntimeValue(V1)));
    REQUIRE_NOTHROW(state.push(GCodeRuntimeValue(V2)));
    REQUIRE_NOTHROW(state.compare());
    REQUIRE_NOTHROW(state.dup());
    REQUIRE_NOTHROW(state.dup());
    REQUIRE_NOTHROW(state.dup());
    REQUIRE_NOTHROW(state.dup());
    REQUIRE_NOTHROW(state.dup());
    REQUIRE_NOTHROW(state.test(static_cast<int64_t>(GCodeCompare::Equals)));
    REQUIRE(state.pop().getInteger() == 0);
    REQUIRE_NOTHROW(state.test(static_cast<int64_t>(GCodeCompare::NotEquals)));
    REQUIRE(state.pop().getInteger() != 0);
    REQUIRE_NOTHROW(state.test(static_cast<int64_t>(GCodeCompare::Lesser)));
    REQUIRE(state.pop().getInteger() != 0);
    REQUIRE_NOTHROW(state.test(static_cast<int64_t>(GCodeCompare::Greater)));
    REQUIRE(state.pop().getInteger() == 0);
    REQUIRE_NOTHROW(state.test(static_cast<int64_t>(GCodeCompare::Lesser) | static_cast<int64_t>(GCodeCompare::Equals)));
    REQUIRE(state.pop().getInteger() != 0);
    REQUIRE_NOTHROW(state.test(static_cast<int64_t>(GCodeCompare::Greater) | static_cast<int64_t>(GCodeCompare::Equals)));
    REQUIRE(state.pop().getInteger() == 0);
    
    REQUIRE_NOTHROW(state.push(GCodeRuntimeValue(V2)));
    REQUIRE_NOTHROW(state.push(GCodeRuntimeValue(V1)));
    REQUIRE_NOTHROW(state.compare());
    REQUIRE_NOTHROW(state.dup());
    REQUIRE_NOTHROW(state.dup());
    REQUIRE_NOTHROW(state.dup());
    REQUIRE_NOTHROW(state.dup());
    REQUIRE_NOTHROW(state.dup());
    REQUIRE_NOTHROW(state.test(static_cast<int64_t>(GCodeCompare::Equals)));
    REQUIRE(state.pop().getInteger() == 0);
    REQUIRE_NOTHROW(state.test(static_cast<int64_t>(GCodeCompare::NotEquals)));
    REQUIRE(state.pop().getInteger() != 0);
    REQUIRE_NOTHROW(state.test(static_cast<int64_t>(GCodeCompare::Lesser)));
    REQUIRE(state.pop().getInteger() == 0);
    REQUIRE_NOTHROW(state.test(static_cast<int64_t>(GCodeCompare::Greater)));
    REQUIRE(state.pop().getInteger() != 0);
    REQUIRE_NOTHROW(state.test(static_cast<int64_t>(GCodeCompare::Lesser) | static_cast<int64_t>(GCodeCompare::Equals)));
    REQUIRE(state.pop().getInteger() == 0);
    REQUIRE_NOTHROW(state.test(static_cast<int64_t>(GCodeCompare::Greater) | static_cast<int64_t>(GCodeCompare::Equals)));
    REQUIRE(state.pop().getInteger() != 0);
    
    REQUIRE_NOTHROW(state.push(GCodeRuntimeValue(V1)));
    REQUIRE_NOTHROW(state.push(GCodeRuntimeValue(V1)));
    REQUIRE_NOTHROW(state.compare());
    REQUIRE_NOTHROW(state.dup());
    REQUIRE_NOTHROW(state.dup());
    REQUIRE_NOTHROW(state.dup());
    REQUIRE_NOTHROW(state.dup());
    REQUIRE_NOTHROW(state.dup());
    REQUIRE_NOTHROW(state.test(static_cast<int64_t>(GCodeCompare::Equals)));
    REQUIRE(state.pop().getInteger() != 0);
    REQUIRE_NOTHROW(state.test(static_cast<int64_t>(GCodeCompare::NotEquals)));
    REQUIRE(state.pop().getInteger() == 0);
    REQUIRE_NOTHROW(state.test(static_cast<int64_t>(GCodeCompare::Lesser)));
    REQUIRE(state.pop().getInteger() == 0);
    REQUIRE_NOTHROW(state.test(static_cast<int64_t>(GCodeCompare::Greater)));
    REQUIRE(state.pop().getInteger() == 0);
    REQUIRE_NOTHROW(state.test(static_cast<int64_t>(GCodeCompare::Lesser) | static_cast<int64_t>(GCodeCompare::Equals)));
    REQUIRE(state.pop().getInteger() != 0);
    REQUIRE_NOTHROW(state.test(static_cast<int64_t>(GCodeCompare::Greater) | static_cast<int64_t>(GCodeCompare::Equals)));
    REQUIRE(state.pop().getInteger() != 0);
  }
  SECTION("Logical negation") {
    REQUIRE_NOTHROW(state.push(static_cast<T1>(1)));
    REQUIRE_NOTHROW(state.push(static_cast<T1>(10)));
    REQUIRE_NOTHROW(state.push(static_cast<T1>(-10)));
    REQUIRE_NOTHROW(state.push(static_cast<T1>(0)));
    REQUIRE_NOTHROW(state.inot());
    REQUIRE(state.pop().getInteger() == 1);
    REQUIRE_NOTHROW(state.inot());
    REQUIRE(state.pop().getInteger() == 0);
    REQUIRE_NOTHROW(state.inot());
    REQUIRE(state.pop().getInteger() == 0);
    REQUIRE_NOTHROW(state.inot());
    REQUIRE(state.pop().getInteger() == 0);
  }
}

TEST_CASE("Runtime arithmetic operations") {
  GCodeCascadeVariableScope scope;
  GCodeRuntimeState state(scope);
  SECTION("Integer-Integer") {
    arithmetic_test<int64_t, int64_t>(state, 100, 200);
  }
  SECTION("Integer-Float") {
    arithmetic_test<int64_t, double>(state, 100, 200.0);
  }
  SECTION("Float-Integer") {
    arithmetic_test<double, int64_t>(state, 100.0, 200);
  }
  SECTION("Float-Float") {
    arithmetic_test<double, double>(state, 100.0, 200.0);
  }
}

TEST_CASE("Runtime comparison operations") {
  GCodeCascadeVariableScope scope;
  GCodeRuntimeState state(scope);
  GCodeRuntimeConfig config;
  config.setComparisonTolerance(0.0);
  GCodeRuntimeState strict(scope, config);
  SECTION("Integer-Integer") {
    comparison_test<int64_t, int64_t>(state, 100, 200);
  }
  SECTION("Integer-Float") {
    comparison_test<int64_t, double>(state, 100, 200.0);
  }
  SECTION("Integer-Float strict") {
    comparison_test<int64_t, double>(strict, 100, 200.0);
  }
  SECTION("Float-Integer") {
    comparison_test<double, int64_t>(state, 100.0, 200);
  }
  SECTION("Float-Integer strict") {
    comparison_test<double, int64_t>(strict, 100.0, 200);
  }
  SECTION("Float-Float") {
    comparison_test<double, double>(state, 100.0, 200.0);
  }
  SECTION("Float-Float strict") {
    comparison_test<double, double>(strict, 100.0, 200.0);
  }
  SECTION("Near zero comparisons") {
    const double V1 = 100.0;
    const double V2 = V1 + config.getComparisonTolerance() / 10;
    REQUIRE_NOTHROW(state.push(GCodeRuntimeValue(V1)));
    REQUIRE_NOTHROW(state.push(GCodeRuntimeValue(V2)));
    REQUIRE_NOTHROW(state.compare());
    REQUIRE_NOTHROW(state.push(GCodeRuntimeValue(V2)));
    REQUIRE_NOTHROW(state.push(GCodeRuntimeValue(V1)));
    REQUIRE_NOTHROW(state.compare());
    REQUIRE_NOTHROW(state.push(GCodeRuntimeValue(V1)));
    REQUIRE_NOTHROW(state.push(GCodeRuntimeValue(V1)));
    REQUIRE_NOTHROW(state.compare());
    int64_t results[] = {
      state.pop().getInteger(),
      state.pop().getInteger(),
      state.pop().getInteger()
    };
    REQUIRE((results[0] & static_cast<int64_t>(GCodeCompare::Equals)) != 0);
    REQUIRE((results[1] & static_cast<int64_t>(GCodeCompare::Equals)) != 0);
    REQUIRE((results[2] & static_cast<int64_t>(GCodeCompare::Equals)) != 0);
    REQUIRE((results[0] & static_cast<int64_t>(GCodeCompare::NotEquals)) == 0);
    REQUIRE((results[1] & static_cast<int64_t>(GCodeCompare::NotEquals)) == 0);
    REQUIRE((results[2] & static_cast<int64_t>(GCodeCompare::NotEquals)) == 0);
    REQUIRE((results[0] & static_cast<int64_t>(GCodeCompare::Greater)) == 0);
    REQUIRE((results[1] & static_cast<int64_t>(GCodeCompare::Greater)) == 0);
    REQUIRE((results[2] & static_cast<int64_t>(GCodeCompare::Greater)) == 0);
    REQUIRE((results[0] & static_cast<int64_t>(GCodeCompare::Lesser)) == 0);
    REQUIRE((results[1] & static_cast<int64_t>(GCodeCompare::Lesser)) == 0);
    REQUIRE((results[2] & static_cast<int64_t>(GCodeCompare::Lesser)) == 0);
  }
}

TEST_CASE("Call-Return") {
  GCodeCascadeVariableScope scope;
  GCodeRuntimeState state(scope);
  GCodeVariableScope &base = state.getScope();
  REQUIRE_NOTHROW(state.jump(10));
  REQUIRE_NOTHROW(state.call(100));
  REQUIRE(state.getPC() == 100);
  REQUIRE(&state.getScope() != &base);
  REQUIRE_NOTHROW(state.ret());
  REQUIRE(state.getPC() == 10);
  REQUIRE(&state.getScope() == &base);
  REQUIRE_THROWS(state.ret());
}

TEST_CASE("Function scope") {
  GCodeFunctionScope scope;
  REQUIRE_FALSE(scope.hasFunction("fn1"));
  REQUIRE_FALSE(scope.hasFunction("fn2"));
  REQUIRE_NOTHROW(scope.bindFunction("fn1", [](const std::vector<GCodeRuntimeValue> &args) {
    int64_t sum = 0;
    for (const auto &val : args) {
      sum += val.asInteger();
    }
    return GCodeRuntimeValue(sum);
  }));
  REQUIRE_NOTHROW(scope.bindFunction("fn2", [](const std::vector<GCodeRuntimeValue> &args) {
    int64_t prod = 1;
    for (const auto &val : args) {
      prod *= val.asInteger();
    }
    return GCodeRuntimeValue(prod);
  }));
  REQUIRE_NOTHROW(scope.unbindFunction("fn3"));
  REQUIRE(scope.hasFunction("fn1"));
  REQUIRE(scope.hasFunction("fn2"));
  REQUIRE(scope.invoke("fn1", std::vector<GCodeRuntimeValue>{1L, 2L, 3L, 4L}).asInteger() == 10);
  REQUIRE(scope.invoke("fn2", std::vector<GCodeRuntimeValue>{1L, 2L, 3L, 4L}).asInteger() == 24);
  REQUIRE(scope.invoke("fn3", std::vector<GCodeRuntimeValue>{1L, 2L, 3L, 4L}).is(GCodeRuntimeValue::Type::None));
  REQUIRE_NOTHROW(scope.unbindFunction("fn2"));
  REQUIRE(scope.hasFunction("fn1"));
  REQUIRE_FALSE(scope.hasFunction("fn2"));
  REQUIRE_NOTHROW(scope.unbindAll());
  REQUIRE_FALSE(scope.hasFunction("fn1"));
  REQUIRE_FALSE(scope.hasFunction("fn2"));
}