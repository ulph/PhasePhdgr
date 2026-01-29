#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include "modules/blitosc.hpp"
#include "connectiongraph/moduleaccessor.hpp"
#include <catch2/generators/catch_generators.hpp>
#include <random>
#include <algorithm>
#include <vector>
#include <map>
#include <set>

#include "module_fuzzer.hpp"

TEST_CASE( "blitosc", "[module]" ) {
    auto ma = ModuleAccessor();
    auto module = BlitOsc();
    SECTION("pad names") {
        REQUIRE(ma.getInputs(module).size() == 8);
        REQUIRE(ma.getInputs(module)[0].name == "freq");
        REQUIRE(ma.getInputs(module)[1].name == "shape");
        REQUIRE(ma.getInputs(module)[2].name == "pwm");
        REQUIRE(ma.getInputs(module)[3].name == "syncFreq");
        REQUIRE(ma.getInputs(module)[4].name == "sync");
        REQUIRE(ma.getInputs(module)[5].name == "reset");
        REQUIRE(ma.getInputs(module)[6].name == "softReset");
        REQUIRE(ma.getInputs(module)[7].name == "dcRemoval");

        REQUIRE(ma.getOutputs(module).size() == 3);
        REQUIRE(ma.getOutputs(module)[0].name == "derivative");
        REQUIRE(ma.getOutputs(module)[1].name == "out");
        REQUIRE(ma.getOutputs(module)[2].name == "integral");
    }
}