#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

enum Opcode { 
    OP_NONE,

    OP_SET_OUTPUT_TO_INPUT,
    OP_ADD_OUTPUT_TO_INPUT,
    OP_PROCESS,

    OP_X_UNBUFFER_INPUT,
    OP_X_BUFFER_OUTPUT,

    OP_B_SET_OUTPUT_TO_INPUT,
    OP_B_ADD_OUTPUT_TO_INPUT,
    OP_B_PROCESS,
};

struct Instruction {
    Opcode opcode;
    int param0;
    int param1;
    int param2;
    int param3;

    Instruction() : opcode(OP_NONE), param0(0), param1(0), param2(0), param3(0) {}
    Instruction(Opcode opcode, int param0, int param1, int param2, int param3) : opcode(opcode), param0(param0), param1(param1), param2(param2), param3(param3) {}
    Instruction(Opcode opcode, int param0, int param1, int param2) : opcode(opcode), param0(param0), param1(param1), param2(param2), param3(0) {}
    Instruction(Opcode opcode, int param0, int param1) : opcode(opcode), param0(param0), param1(param1), param2(0), param3(0) {}
    Instruction(Opcode opcode, int param0) : opcode(opcode), param0(param0), param1(0), param2(0), param3(0) {}
};

#endif
