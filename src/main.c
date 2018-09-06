#include "main.h"

typedef struct cpuState {
    unsigned short opcode;
    unsigned char memory[4096];
    unsigned char v[16]; //v0 -> vF
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
    unsigned char op1;                        //other stuff
    unsigned char op2;
    if(opcode == 0) //null opcode
        cpu->nullCount += 1;
    else
        cpu->nullCount = 0;
    if(cpu->nullCount >= 6) //exit if 4 null bytes in a row
        exit(0);
    switch(ident){
        case 0x0: 
            switch(*next){
                case 0xE0:          //00E0 - CLS
                    memset(cpu->gfx, 0, sizeof(cpu->gfx)); 
                    break;
                case 0xEE:          //00EE - RET
                    cpu->pc = (cpu->stack[cpu->sp]) - 2; //accounting for increment of pc at end of instruction
                    cpu->sp = 0;
                    break;
                case 0x00:          //0000 - NOP        (technically undefined in instruction set)
                    break;
                case 0xFF: printf("Debug\n"); break;
            } break;
        case 0x1:                   //1nnn - JP nnn
            cpu->pc = (opcode & 0x0FFF) - 2;
            break;
        case 0x2:                   //2nnn - CALL nnn
            cpu->sp += 1;
            cpu->stack[cpu->sp] = cpu->pc;
            cpu->pc = (opcode & 0x0FFF) - 2;
            break;
        case 0x3:                   //3xkk - SE Vx kk
            op1 = *first & 0x0F;
            op2 = *next;
            if(cpu->v[op1] == op2)
                cpu->pc += 2;
            break;
        default:
            printf("Skipping op %04x\n", opcode);
    }
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