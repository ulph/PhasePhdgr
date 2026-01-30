#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include "connectiongraph/moduleaccessor.hpp"
#include <catch2/generators/catch_generators.hpp>
#include <random>
#include <algorithm>
#include <vector>
#include <map>
#include <set>

#include "module_fuzzer.hpp"
#include "moduleregister.hpp"

TEST_CASE("module fuzz test", "[module]") {
    const auto builtinModules = getAllBuiltinModules();
    for(auto [moduleName, moduleFactory]: builtinModules) {
        SECTION(moduleName) {
            moduleFuzzerTest(moduleFactory);
        }
    }
}