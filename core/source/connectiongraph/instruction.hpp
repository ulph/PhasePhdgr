#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

enum Opcode { 
    OP_PROCESS,
    OP_RESET_INPUT,
    OP_ADD_OUTPUT_TO_INPUT,

    MARKER_SAMPLEWISE,
    MARKER_BLOCKWISE,
};

struct Instruction {
    Opcode opcode;
    int param0;
    int param1;
    int param2;
    int param3;

    Instruction(Opcode opcode, int param0, int param1, int param2, int param3) : opcode(opcode), param0(param0), param1(param1), param2(param2), param3(param3) {}
    Instruction(Opcode opcode, int param0, int param1) : opcode(opcode), param0(param0), param1(param1), param2(0), param3(0) {}
    Instruction(Opcode opcode, int param0) : opcode(opcode), param0(param0), param1(0), param2(0), param3(0) {}
};

#endif
