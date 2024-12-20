#pragma once
#include <stdint.h>
#include <thread>

enum Quirks
{
    QUIRK_VF_RESET,
    QUIRK_MEM,
    QUIRK_MEM_PLUS_ONE,
    QUIRK_DISP_WAIT,
    QUIRK_CLIP,
    QUIRK_SHIFT,
    QUIRK_JUMP,
    QUIRK_MAX
};

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
    unsigned keyMapping[16];
    std::thread audioThread;
    bool breakCycle = false;
    bool quirks[QUIRK_MAX];

};

void init_vm(Chip8_VM* cpu);
void load_rom(Chip8_VM* cpu, const char* filepath);
void update_timers(Chip8_VM* cpu);
void cpu_cycle(Chip8_VM* cpu);