#pragma once

#include <raylib.h>
#include <cstdlib>
#include <Chip8.h>  


namespace App
{
	struct Context
	{
		Color offColor, onColor;
		unsigned width, height;
		Chip8_VM cpu;
		uint8_t ipf;
	};

	static Context context;

	void init();
	void run();
	void shutdown();
}