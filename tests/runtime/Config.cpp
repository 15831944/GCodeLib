#include "gcodelib/runtime/Config.h"
#include "catch.hpp"

using namespace GCodeLib::Runtime;

TEST_CASE("Configuration") {
  const double newTolerance = 1e-6;
  GCodeRuntimeConfig config;
  REQUIRE(config.getComparisonTolerance() == GCodeRuntimeConfig::DefaultComparisonTolerance);
  REQUIRE(config.hasComparisonTolerance());
  REQUIRE_FALSE(config.setComparisonTolerance(-newTolerance));
  REQUIRE(config.getComparisonTolerance() == GCodeRuntimeConfig::DefaultComparisonTolerance);
  REQUIRE(config.setComparisonTolerance(newTolerance));
  REQUIRE(config.getComparisonTolerance() == newTolerance);
  REQUIRE(config.hasComparisonTolerance());
  REQUIRE(config.setComparisonTolerance(0.0));
  REQUIRE(config.getComparisonTolerance() == 0.0);
  REQUIRE_FALSE(config.hasComparisonTolerance());
  REQUIRE(GCodeRuntimeConfig::Default.getComparisonTolerance() == GCodeRuntimeConfig::DefaultComparisonTolerance);
  REQUIRE(GCodeRuntimeConfig::Default.hasComparisonTolerance());
}