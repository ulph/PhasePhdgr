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

auto generateDoubleSequence(auto randomGen, size_t period, size_t offset, size_t length) {
    std::vector<double> seq(length);
    auto val = randomGen();
    for(auto n=0u; n<seq.size(); ++n) {
        if(((n+offset)%period) == 0) {
            val = randomGen(); 
        }
        seq[n] = val;
    }
    return seq;
}

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

    SECTION("process fuzzed") {
        auto seed = GENERATE(2, 99);
        auto period = GENERATE(1, 128);

        const auto length = 1024;

        std::mt19937 gen(seed);
        std::uniform_real_distribution<double> dist(-1.0, 1.0);
        std::map<int, std::vector<double>> inputValues;
        std::map<int, std::vector<double>> outputValues;

        for(auto i=0u; i<ma.getInputs(module).size(); ++i){
            inputValues[i] = generateDoubleSequence([&gen, &dist](){ return dist(gen); }, period, i, length);
        }

        for(auto o=0u; o<ma.getOutputs(module).size(); ++o){
            outputValues[o] = std::vector<double>(length);
        }

        for (auto n=0u; n<length; ++n) {
            for(auto i=0u; i<ma.getInputs(module).size(); ++i){
                ma.setInput(module, i, inputValues[i][n]);
            }
            module.process();
            for(auto o=0u; o<ma.getOutputs(module).size(); ++o){
                outputValues[o][n] = ma.getOutput(module, o);
            }
        }

    }
}