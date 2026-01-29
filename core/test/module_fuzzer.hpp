#pragma once

#include "connectiongraph/moduleaccessor.hpp"
#include "units.hpp"
#include <algorithm>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <map>
#include <random>
#include <set>
#include <vector>

inline auto generateDoubleSequence(auto randomGen, size_t period, size_t offset,
                                   size_t length) {
  std::vector<double> seq(length);
  auto val = 0;
  for (auto n = 0u; n < seq.size(); ++n) {
    if (((n + offset) % (period+1)) == 0) {
      val = randomGen();
    }
    seq[n] = val;
  }
  return seq;
}

inline auto createDistributionFromUnit(const auto unit, const auto nyquist) {
    if(unit == UNIT_HZ) {
        return std::normal_distribution<float>{nyquist/2.f, nyquist/2.f}; // some negative values as well as some values above nyquist
    }
    if(unit == UNIT_SAMPLES) {
        return std::normal_distribution<float>{5, 5};
    }
    if(unit == UNIT_SECONDS) {
        return std::normal_distribution<float>{1, 1};
    }
    return std::normal_distribution<float>(0, 0.1);
}

inline auto generateInputValues(ModuleAccessor &ma, Module &module, auto seed,
                                auto period, auto length, auto nyquist) {
  std::map<int, std::vector<double>> inputValues;
  for (auto i = 0u; i < ma.getInputs(module).size(); ++i) {
    std::mt19937 gen(seed);
    auto dist = createDistributionFromUnit(ma.getInputs(module).at(i).unit, nyquist);
    inputValues[i] = generateDoubleSequence([&gen, &dist]() { return dist(gen); }, period, i, length);
  }
  return inputValues;
}

inline auto runAndCollect(ModuleAccessor &ma, Module &module,
                          const std::map<int, std::vector<double>> &inputValues,
                          size_t length) {
  std::map<int, std::vector<double>> outputValues;
  for (auto o = 0u; o < ma.getOutputs(module).size(); ++o) {
    outputValues[o] = std::vector<double>(length);
  }

  for (auto n = 0u; n < length; ++n) {
    for (auto i = 0u; i < ma.getInputs(module).size(); ++i) {
      ma.setInput(module, i, inputValues.at(i).at(n));
    }
    ma.process(module);
    for (auto o = 0u; o < ma.getOutputs(module).size(); ++o) {
      outputValues[o][n] = ma.getOutput(module, o);
    }
  }

  return outputValues;
}

inline void ensureValidNumbers(const std::map<int, std::vector<double>> &values) {
    for(auto i=0u; i<values.size(); ++i) {
        INFO("pad " << i);
        for(auto n=0u; n<values.at(i).size(); ++n) {
            INFO("sample position " << n);
            const auto sample = values.at(i).at(n);
            REQUIRE(std::isfinite(sample)); // ie, not NaN nor Inf
        }
    }
}

inline void moduleFuzzerTest(Module* (* moduleFactory)()) {

  SECTION("process fuzzed") {

    // given
    ModuleAccessor ma;
    std::unique_ptr<Module> modulePtr(moduleFactory());
    auto& module = *modulePtr;

    auto seed = GENERATE(2, 99);
    auto period = GENERATE(1, 128);
    auto fs = GENERATE(100, 11025, 22050, 44100, 48000, 96000, 192000);
    auto length = 1024;

    INFO("seed: " << seed);
    INFO("period: " << period);
    INFO("fs: " << fs);
    INFO("length: " << length);

    ma.setFs(module, fs);

    auto inputValues = generateInputValues(ma, module, seed, period, length, fs/2);

    // when
    auto outputValues = runAndCollect(ma, module, inputValues, length);

    // then
    ensureValidNumbers(outputValues);
  }
}