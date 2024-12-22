#include "app.h"
#include "imguiThemes.h"
#include <chrono>
#include <platform.h>
#include "ImGuiFileDialog.h"
#include "imgui.h"
#include "iostream"

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
	context.running = true;
	context.ipf = 11;

	context.width = 1400;
	context.height = 700;
	context.logs = "";

	init_vm(&context.cpu);
	load_rom(&context.cpu, ASSETS_PATH "glitchGhost.ch8");

	SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
	InitWindow(context.width, context.height, "Chip-8 Emulator");
	srand(time(0));

}

void App::run()
{
	int fps = 60;

	double delta_time = 0.0;
	double timerAccumulator = 0.0;
	const double timerThershold = 1.0 / fps;

	auto start = std::chrono::high_resolution_clock::now();
	auto end = start;

	rlImGuiSetup(true);
	//imguiThemes::retroDark();
	//imguiThemes::Microsoft();
	imguiThemes::FutureDark();
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;



	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground;
	windowFlags = 0 | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration;

	bool showRegisters, showStack, showMemory, showLogs, showDisplay, showQuirks;

	showRegisters = showMemory = showStack = showLogs = showDisplay = showQuirks = true;

	RenderTexture2D emulatorTexture = LoadRenderTexture(context.width, context.height);
	while (!WindowShouldClose() && context.running)
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
				context.logs = context.cpu.logs;
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
				if (value) DrawRectangle(x * context.width / 64, y * context.height / 32, 64, 32, context.onColor);
				else DrawRectangle(x * context.width / 64, y * context.height / 32, 64, 32, context.offColor);
			}
		}
		EndTextureMode();

#pragma region imgui

#pragma region titleBar
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Open ROM")) {
					ImGuiFileDialog::Instance()->OpenDialog("ChooseROM", "Choose ROM File", ".ch8,.c8,.*", {ASSETS_PATH});
				}

				if (ImGui::MenuItem("Exit")) {
					context.running = false;
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View")) {
				ImGui::MenuItem("Show Registers", NULL, &showRegisters);
				ImGui::MenuItem("Show Stack", NULL, &showStack);
				ImGui::MenuItem("Show Memory", NULL, &showMemory);
				ImGui::MenuItem("Show Logs", NULL, &showLogs);
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		if (ImGuiFileDialog::Instance()->Display("ChooseROM")) {
			if (ImGuiFileDialog::Instance()->IsOk()) {
				std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
				std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

				init_vm(&context.cpu);
				load_rom(&context.cpu,filePathName.c_str());
			}
			ImGuiFileDialog::Instance()->Close();
		}
#pragma endregion
		

		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_AutoHideTabBar);

		if (showDisplay) {
			ImGui::Begin("Display", 0 ,windowFlags & 0);
			ImGui::Text("Screen Output:");
			ImGui::Separator();
			ImVec2 size = ImVec2(
				ImGui::GetWindowSize().x,
				std::min(ImGui::GetWindowSize().y, ImGui::GetWindowSize().x / 2.f)
			);
			ImGui::Image((ImTextureID)(intptr_t)&emulatorTexture.texture,
				size,
				ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
			ImGui::End();
		}

		if (showRegisters) {
			ImGui::Begin("Registers", 0, windowFlags);
			ImGui::Text("CPU Registers:");
			ImGui::Separator();
			for (uint8_t i = 0; i <= 0xF; i++) {
				ImGui::Text("V%X: 0x%02X", i, context.cpu.registers[i]);
			}
			ImGui::Separator();
			ImGui::Text("Special Registers:");
			ImGui::BulletText("I: 0x%X", context.cpu.indexRegister);
			ImGui::BulletText("PC: 0x%X", context.cpu.programCounter);
			ImGui::Separator();
			ImGui::Text("Timers:");
			ImGui::BulletText("Delay Timer: %d", context.cpu.delayTimer);
			ImGui::BulletText("Sound Timer: %d", context.cpu.soundTimer);
			ImGui::End();
		}

		if (showQuirks) {
			ImGui::Begin("Quirks", 0, windowFlags);
			ImGui::Text("CPU Quirks:");
			ImGui::Separator();
			const char* quirk_ui[QUIRK_MAX] = {
				"Reset VF", "Increment I", "Increment I by x+1",
				"Display wait", "Clipping", "Shifting", "Jump"
			};
			for (unsigned i = 0; i < QUIRK_MAX; i++) {
				ImGui::Checkbox(quirk_ui[i], &context.cpu.quirks[i]);
			}
			ImGui::End();
		}

		if (showStack) {
			ImGui::Begin("Stack");
			ImGui::Text("CPU Stack:");
			ImGui::Separator();
			for (uint8_t i = 0; i <= 0xF; i++) {
				ImGui::BulletText("%X: 0x%02X", i, context.cpu.stack[i]);
			}
			ImGui::End();
		}

		if (showMemory) {
			ImGui::Begin("Memory");
			ImGui::Text("Memory Viewer:");
			ImGui::Separator();
			ImGui::BeginChild("MemoryView");
			for (unsigned i = 0; i < 4096; i += 8) {
				ImGui::Text("%04X: ", i);
				ImGui::SameLine();
				for (unsigned j = 0; j < 8; j++) {
					ImGui::Text("%02X ", context.cpu.memory[i + j]);
					ImGui::SameLine();
				}
				ImGui::NewLine();
			}
			ImGui::EndChild();
			ImGui::End();
		}

		if (showLogs) {
			ImGui::Begin("Logs");
			ImGui::Text("Log Output:");
			ImGui::Separator();
			ImGui::BeginChild("LogView", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
			ImGui::TextUnformatted(context.logs.c_str());
			ImGui::EndChild();
			ImGui::End();
		}

		if (true) {
			ImGui::Begin("Debug");
			ImGui::Text("Debug Controls:");
			ImGui::Separator();
			ImGui::BeginChild("DebugView");
			ImGui::SliderInt("FPS", &fps, 1, 200);
			ImGui::SliderInt("Instructions per frame", (int*)&context.ipf, 1, 100);
			ImGui::EndChild();
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
