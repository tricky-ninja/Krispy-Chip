#include "app.h"
#include "imguiThemes.h"
#include <chrono>
#include <platform.h>
#include "imgui.h"

#include "rlImGui.h"

void App::init()
{
	context.offColor = Color(3, 1, 18, 255);
	context.onColor = Color(235, 54, 120, 255);

	context.cpu.keyMapping[0x0] = KEY_X;
	context.cpu.keyMapping[0x1] = KEY_ONE;
	context.cpu.keyMapping[0x2] = KEY_TWO;
	context.cpu.keyMapping[0x3] = KEY_THREE;
	context.cpu.keyMapping[0x4] = KEY_Q;
	context.cpu.keyMapping[0x5] = KEY_W;
	context.cpu.keyMapping[0x6] = KEY_E;
	context.cpu.keyMapping[0x7] = KEY_A;
	context.cpu.keyMapping[0x8] = KEY_S;
	context.cpu.keyMapping[0x9] = KEY_D;
	context.cpu.keyMapping[0xA] = KEY_Z;
	context.cpu.keyMapping[0xB] = KEY_C;
	context.cpu.keyMapping[0xC] = KEY_FOUR;
	context.cpu.keyMapping[0xD] = KEY_R;
	context.cpu.keyMapping[0xE] = KEY_F;
	context.cpu.keyMapping[0xF] = KEY_V;

	context.cpu.quirks[QUIRK_VF_RESET] = true;
	context.cpu.quirks[QUIRK_MEM] = true;
	context.cpu.quirks[QUIRK_MEM_PLUS_ONE] = true;
	context.cpu.quirks[QUIRK_DISP_WAIT] = true;
	context.cpu.quirks[QUIRK_CLIP] = true;
	context.cpu.quirks[QUIRK_SHIFT] = false;
	context.cpu.quirks[QUIRK_JUMP] = false;

	context.ipf = 11;

	context.width = 1200;
	context.height = 600;

	init_vm(&context.cpu);
	load_rom(&context.cpu, ASSETS_PATH "5-quirks.ch8");

	SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI);
	InitWindow(context.width, context.height, "Chip-8 Emulator");
	srand(time(0));
}

void App::run()
{
	double delta_time = 0.0;
	double timerAccumulator = 0.0;
	const double timerThershold = 1.0 / 60.0;

	auto start = std::chrono::high_resolution_clock::now();
	auto end = start;

	rlImGuiSetup(true);
	imguiThemes::retroDark();
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground;
	windowFlags = 0;


	RenderTexture2D emulatorTexture = LoadRenderTexture(context.width, context.height);
	while (!WindowShouldClose())
	{
		end = std::chrono::high_resolution_clock::now();
		delta_time = std::chrono::duration<double>(end - start).count();
		start = std::chrono::high_resolution_clock::now();

		timerAccumulator += delta_time;

		if (IsWindowResized()) {
			context.width = GetScreenWidth();
			context.height = GetScreenHeight();
		}

		while (timerAccumulator >= timerThershold)
		{
			for (uint8_t i = 0; i < context.ipf; i++)
			{
				cpu_cycle(&context.cpu);
				if (context.cpu.breakCycle) {
					context.cpu.breakCycle = false;
					break;
				}
			}
			update_timers(&context.cpu);
			timerAccumulator -= timerThershold;
		}


#pragma region Rendering

		BeginDrawing();
		ClearBackground(context.offColor);
		rlImGuiBegin();
		BeginTextureMode(emulatorTexture);
		ClearBackground(context.offColor);

		for (unsigned y = 0; y < 32; y++)
		{
			for (unsigned x = 0; x < 64; x++)
			{
				bool value = context.cpu.framebuffer[y * 64 + x];
				if (value) DrawRectangle(x * context.width / 2 / 64, y * context.width / 4 / 32, 64, 32, context.onColor);
				else DrawRectangle(x * context.width / 2 / 64, y * context.width / 4 / 32, 64, 32, context.offColor);
			}
		}
		EndTextureMode();

#pragma region imgui


		//ImGui::DockSpaceOverViewport(nullptr, ImGuiDockNodeFlags_NoSplit | ImGuiDockNodeFlags_NoResize);
		ImGui::DockSpaceOverViewport();


		if (ImGui::Begin("Display",0, windowFlags))
		{
			ImGui::Image((ImTextureID)(intptr_t)&emulatorTexture.texture, ImVec2(context.width/2, context.width/4), ImVec2(0.0f, 1.0f), // Bottom-left
				ImVec2(1.0f, 0.0f));

			ImGui::End();
		}

		if (ImGui::Begin("Registers", 0, windowFlags))
		{
			for (uint8_t i = 0; i <= 0xF; i++)
			{
				ImGui::Text("V[%0X]: %0X", i, context.cpu.registers[i]);
			}

			ImGui::Text("I: %0X", context.cpu.indexRegister);

			ImGui::NewLine();

			ImGui::Text("Delay Timer: %d", context.cpu.delayTimer);
			ImGui::Text("Sound Timer: %d", context.cpu.soundTimer);

			ImGui::End();
		}

		if (ImGui::Begin("Quirks", 0, windowFlags))
		{
			const char* quirk_ui[QUIRK_MAX] = {"Reset VF", "Increment I", "Increment I by x+1", "Display wait", "Clipping", "Shifting", "Jump"};
			for (unsigned i = 0; i < QUIRK_MAX; i++)
			{
				ImGui::Checkbox(quirk_ui[i], &context.cpu.quirks[i]);
			}
			ImGui::End();
		}


		rlImGuiEnd();
		EndDrawing();
#pragma endregion
		

#pragma endregion
	}
}

void App::shutdown()
{
	rlImGuiShutdown();
	CloseWindow();
}
