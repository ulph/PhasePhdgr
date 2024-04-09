#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

enum Opcode {
    OP_BLOCK_SET_OUTPUT_TO_INPUT,
    OP_BLOCK_ADD_OUTPUT_TO_INPUT,
    OP_BLOCK_PROCESS,
    OP_SAMPLE_SET_OUTPUT_TO_INPUT,
    OP_SAMPLE_ADD_OUTPUT_TO_INPUT,
    OP_SAMPLE_SET_PREV_OUTPUT_TO_INPUT,
    OP_SAMPLE_ADD_PREV_OUTPUT_TO_INPUT,
    OP_SAMPLE_PROCESS,
    OP_LOOP,
};

struct Instruction {
    Opcode opcode;
    int param0;
    int param1;
    int param2;
    int param3;

    Instruction(Opcode opcode, int param0 = 0, int param1 = 0, int param2 = 0, int param3 = 0)
        : opcode(opcode), param0(param0), param1(param1), param2(param2), param3(param3) {}
    bool operator==(const Instruction& i) {
        return opcode == i.opcode && param0 == i.param0 && param1 == i.param1 && param2 == i.param2 &&
               param3 == i.param3;
    }
};

#endif
