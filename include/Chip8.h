#pragma once
#include <stdint.h>

struct Chip8_VM
{
    uint8_t memory[4096];
    uint8_t registers[16];
    uint8_t soundTimer;
    uint8_t delayTimer;
    uint8_t stackPointer;
    uint8_t framebuffer[64 * 32];

    uint16_t stack[16];
    uint16_t indexRegister;
    uint16_t programCounter;
};

void init_vm(Chip8_VM* cpu);
void load_rom(Chip8_VM* cpu, const char* filepath);
void update_timers(Chip8_VM* cpu);
void cpu_cycle(Chip8_VM* cpu);