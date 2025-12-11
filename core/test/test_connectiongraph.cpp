#include "connectiongraph.hpp"

#define TEST(condition, msg_on_fail)                                                                        \
    if (!(condition)) {                                                                                     \
        std::cerr << "TEST failed: " << __FILE__ << ":" << __LINE__ << " " << #condition " " << msg_on_fail \
                  << std::endl;                                                                             \
        exit(-1);                                                                                           \
    } else {                                                                                                \
        std::cout << "TEST passed: " << __FILE__ << ":" << __LINE__ << " " << #condition << std::endl;      \
    }

class ConnectionGraphTest : public ConnectionGraph {
   public:
    std::set<int> getRecursiveModules(int module) { return findRecursiveModules(module); }
    std::vector<Instruction> getProgram() { return program; }
};

class TestModule : public Module {
   public:
    TestModule() {
        inputs.push_back(Pad("in1"));
        inputs.push_back(Pad("in2"));
        outputs.push_back(Pad("out"));
    }
    Module* clone() const override { return nullptr; }
    void processSample(int sample) override {
        outputs[0].values[sample] = inputs[0].values[sample] + inputs[1].values[sample];
    }
    static Module* factory() { return new TestModule(); }
};

void recursionSimple() {
    ConnectionGraphTest cg;
    cg.registerModule("test", TestModule::factory);
    // 0-->1-->2-->3-->4
    //     ^---5---|
    int m0 = cg.addModule("test");
    int m1 = cg.addModule("test");
    int m2 = cg.addModule("test");
    int m3 = cg.addModule("test");
    int m4 = cg.addModule("test");
    int m5 = cg.addModule("test");
    cg.connect(m0, m1);
    cg.connect(m1, m2);
    cg.connect(m2, m3);
    cg.connect(m3, m4);
    cg.connect(m3, m5);
    cg.connect(m5, m1, 1);
    std::set<int> recursive = cg.getRecursiveModules(m4);
    TEST(recursive.size() == 4, "");
    TEST(recursive.count(m1) == 1, "");
    TEST(recursive.count(m2) == 1, "");
    TEST(recursive.count(m3) == 1, "");
    TEST(recursive.count(m5) == 1, "");
}

void recursionComplex() {
    ConnectionGraphTest cg;
    cg.registerModule("test", TestModule::factory);
    // 0-->1-->2-->3-->4-->6-->9-->10-->11-->12
    //     ^---5---|       ^        ^---|
    // 7-->8---------------|
    //     ^|
    int m0 = cg.addModule("test");
    int m1 = cg.addModule("test");
    int m2 = cg.addModule("test");
    int m3 = cg.addModule("test");
    int m4 = cg.addModule("test");
    int m5 = cg.addModule("test");
    int m6 = cg.addModule("test");
    int m7 = cg.addModule("test");
    int m8 = cg.addModule("test");
    int m9 = cg.addModule("test");
    int m10 = cg.addModule("test");
    int m11 = cg.addModule("test");
    int m12 = cg.addModule("test");
    cg.connect(m0, m1);
    cg.connect(m1, m2);
    cg.connect(m2, m3);
    cg.connect(m3, m4);
    cg.connect(m3, m5);
    cg.connect(m5, m1, 1);
    cg.connect(m4, m6);
    cg.connect(m7, m8);
    cg.connect(m8, m8, 1);
    cg.connect(m8, m6);
    cg.connect(m6, m9);
    cg.connect(m9, m10);
    cg.connect(m10, m11);
    cg.connect(m11, m12);
    cg.connect(m11, m10, 1);
    std::set<int> recursive = cg.getRecursiveModules(m12);
    TEST(recursive.size() == 7, "");
    TEST(recursive.count(m1) == 1, "");
    TEST(recursive.count(m2) == 1, "");
    TEST(recursive.count(m3) == 1, "");
    TEST(recursive.count(m5) == 1, "");
    TEST(recursive.count(m8) == 1, "");
    TEST(recursive.count(m10) == 1, "");
    TEST(recursive.count(m11) == 1, "");
}

void compilation() {
    ConnectionGraphTest cg;
    cg.registerModule("test", TestModule::factory);
    // 0-->1-->2-->3-->4
    //     ^---|   ^
    // 5-----------|
    int m0 = cg.addModule("test");
    int m1 = cg.addModule("test");
    int m2 = cg.addModule("test");
    int m3 = cg.addModule("test");
    int m4 = cg.addModule("test");
    int m5 = cg.addModule("test");
    cg.connect(m0, m1);
    cg.connect(m1, m2);
    cg.connect(m2, m3);
    cg.connect(m3, m4);
    cg.connect(m2, m1);
    cg.connect(m5, m3, 1);
    cg.compileProgram(m4);
    auto program = cg.getProgram();
    TEST(program[0] == Instruction(OP_BLOCK_PROCESS, m5), "")
    TEST(program[1] == Instruction(OP_BLOCK_SET_OUTPUT_TO_INPUT, m5, 0, m3, 1), "")
    TEST(program[2] == Instruction(OP_BLOCK_PROCESS, m0), "")
    TEST(program[3] == Instruction(OP_BLOCK_SET_OUTPUT_TO_INPUT, m0, 0, m1, 0), "")
    TEST(program[4] == Instruction(OP_SAMPLE_ADD_PREV_OUTPUT_TO_INPUT, m2, 0, m1, 0), "")
    TEST(program[5] == Instruction(OP_SAMPLE_PROCESS, m1), "")
    TEST(program[6] == Instruction(OP_SAMPLE_SET_OUTPUT_TO_INPUT, m1, 0, m2, 0), "")
    TEST(program[7] == Instruction(OP_SAMPLE_PROCESS, m2), "")
    TEST(program[8] == Instruction(OP_LOOP, 4), "")
    TEST(program[9] == Instruction(OP_BLOCK_SET_OUTPUT_TO_INPUT, m2, 0, m3, 0), "")
    TEST(program[10] == Instruction(OP_BLOCK_PROCESS, m3), "")
    TEST(program[11] == Instruction(OP_BLOCK_SET_OUTPUT_TO_INPUT, m3, 0, m4, 0), "")
}

void process() {
    ConnectionGraph cg;
    cg.registerModule("test", TestModule::factory);
    // 0-->1-->2-->3-->4
    //     ^---|   ^
    // 5-----------|
    int m0 = cg.addModule("test");
    int m1 = cg.addModule("test");
    int m2 = cg.addModule("test");
    int m3 = cg.addModule("test");
    int m4 = cg.addModule("test");
    int m5 = cg.addModule("test");
    cg.connect(m0, m1);
    cg.connect(m1, m2);
    cg.connect(m2, m3);
    cg.connect(m3, m4);
    cg.connect(m2, m1);
    cg.connect(m5, m3, 1);
    cg.setInput(m0, 0, 1);
    cg.setInput(m5, 0, 99);
    cg.compileProgram(m4);
    cg.processBlock(m4, 48000);
    float buf[ConnectionGraph::k_blockSize];
    cg.getOutputBlock(m4, 0, buf);
    for (int i = 0; i < ConnectionGraph::k_blockSize; ++i) {
        TEST(buf[i] == 100 + i, "");
    }
}

int main() {
    recursionSimple();
    recursionComplex();
    compilation();
    process();
    std::cout << "All tests passed." << std::endl;
    return 0;
}
