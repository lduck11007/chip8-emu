#include "main.h"

typedef struct cpuState {
    unsigned short opcode;
    unsigned char memory[4096];
    unsigned char v[16];
    unsigned short index;
    unsigned short pc; //program counter
    unsigned char gfx[64 * 32]; //graphics buffer
    unsigned int delay_counter;
    unsigned int sound_counter;
    unsigned short stack[16];
    unsigned short sp; //stack pointer
    int nullCount; //count of null instructions in a row to exit if too many occur. A bit of a hack but it prevents some random errors
} cpuState;

void run(cpuState* cpu){
    unsigned char* first = &cpu->memory[cpu->pc];
    unsigned char* next = &cpu->memory[cpu->pc+1];
    unsigned short opcode = *first << 8 | *next; //combine both bytes into single instruction
    unsigned int ident = (*first & 0xF0) >> 4; //First half-byte of opcode, identifier.
    printf("%x\t%x\t%x\t%x\n", ident, opcode, *first, *next);
    if(opcode == 0) //null opcode
        cpu->nullCount += 1;
    else
        cpu->nullCount = 0;
    if(cpu->nullCount >= 5) //exit if more than 5 null bytes in a row
        exit(0);
    cpu->pc += 2; //advance 2 bytes (instructions are 2 bytes long) 

}

void initializeCpu(cpuState* cpu){
    cpu->pc = 0x200;
    cpu->opcode = 0;
    cpu->index = 0;
    cpu->sp = 0;
    cpu->nullCount = 0;
}

int main(int argc, char* argv[]) {
    cpuState cpu;
    cpuState* cpuPtr = &cpu;
    FILE* f;
    unsigned char buffer[4096];
    if (argc == 1) {
        printf("Useage: chip8 [file]\n");
        exit(1);
    }
    f = fopen(argv[1], "rb");
    if (f)
        fread(buffer+0x200, 4096, 1, f); //offset of 0x200 at buffer
    else {
        printf("Error: could not open file\n");
        exit(1);
    }
    for (int i = 0; i < 4096; i++)
        cpuPtr->memory[i] = buffer[i];
    initializeCpu(cpuPtr);
    for(;;)
        run(cpuPtr);

}