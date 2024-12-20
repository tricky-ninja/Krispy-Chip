#ifdef _WIN32
#define _WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <platform.h>

void platform_beep(unsigned long freq, double timeInSeconds)
{
	Beep(freq, (DWORD)(timeInSeconds * 1000));
}

#endif