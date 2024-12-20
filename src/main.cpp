#include <raylib.h>
#include <cstdlib>
#include <Chip8.h>  
#include <chrono>
#include <platform.h>


#define WIDTH 1200
#define HEIGHT WIDTH/2

Color offColor = Color(3, 1, 18, 255);
Color onColor = Color(235, 54, 120, 255);

int main()
{
    
    InitWindow(WIDTH, HEIGHT, "Chip-8 Emulator");
    srand(time(0));

    Chip8_VM cpu;
    cpu.keyMapping[0x0] = KEY_X;
    cpu.keyMapping[0x1] = KEY_ONE;
    cpu.keyMapping[0x2] = KEY_TWO;
    cpu.keyMapping[0x3] = KEY_THREE;
    cpu.keyMapping[0x4] = KEY_Q;
    cpu.keyMapping[0x5] = KEY_W;
    cpu.keyMapping[0x6] = KEY_E;
    cpu.keyMapping[0x7] = KEY_A;
    cpu.keyMapping[0x8] = KEY_S;
    cpu.keyMapping[0x9] = KEY_D;
    cpu.keyMapping[0xA] = KEY_Z;
    cpu.keyMapping[0xB] = KEY_C;
    cpu.keyMapping[0xC] = KEY_FOUR;
    cpu.keyMapping[0xD] = KEY_R;
    cpu.keyMapping[0xE] = KEY_F;
    cpu.keyMapping[0xF] = KEY_V;

    cpu.quirks[QUIRK_VF_RESET] = true;
    cpu.quirks[QUIRK_MEM] = true;
    cpu.quirks[QUIRK_MEM_PLUS_ONE] = true;
    cpu.quirks[QUIRK_DISP_WAIT] = true;
    cpu.quirks[QUIRK_CLIP] = true;
    cpu.quirks[QUIRK_SHIFT] = false;
    cpu.quirks[QUIRK_JUMP] = false;


    
    init_vm(&cpu);
    load_rom(&cpu, ASSETS_PATH "5-quirks.ch8");


    double delta_time = 0.0;
    double timerAccumulator = 0.0;
    const double timerThershold = 1.0 / 60.0;

    auto start = std::chrono::high_resolution_clock::now();
    auto end = start;
    uint8_t ipf = 11;   // instructions per frame

    while (!WindowShouldClose())
    {
        end = std::chrono::high_resolution_clock::now();
        delta_time = std::chrono::duration<double>(end - start).count();
        start = std::chrono::high_resolution_clock::now();

        timerAccumulator += delta_time;

        while (timerAccumulator >= timerThershold)
        {
            for (uint8_t i = 0; i < ipf; i++) 
            {
                cpu_cycle(&cpu);
                if (cpu.breakCycle) { 
                    cpu.breakCycle = false;
                    break; 
                }
            }
            update_timers(&cpu);
            timerAccumulator -= timerThershold;
        }

        BeginDrawing();
        ClearBackground(offColor);

        for (unsigned y = 0; y < 32; y++)
        {
            for (unsigned x = 0; x < 64; x++)
            {
                bool value = cpu.framebuffer[y * 64 + x];
                if (value) DrawRectangle(x* WIDTH / 64, y* HEIGHT / 32, 64, 32, onColor);
                else DrawRectangle(x * WIDTH/64, y * HEIGHT/32, 64,32, offColor);
            }
        }

        EndDrawing();
    }
    CloseWindow();

    return 0;
}

