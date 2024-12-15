#include <raylib.h>
#include <cstdlib>
#include <Chip8.h>  

#define WIDTH 1200
#define HEIGHT WIDTH/2

Color offColor = Color(3, 1, 18, 255);
Color onColor = Color(235, 54, 120, 255);

int main()
{
    
    InitWindow(WIDTH, HEIGHT, "Chip-8 Emulator");

    Chip8_VM cpu;

    init_vm(&cpu);
    load_rom(&cpu, ASSETS_PATH "2-ibm-logo.ch8");

    int i = 0;

    while (!WindowShouldClose())
    {
       cpu_cycle(&cpu);
        i++;
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

