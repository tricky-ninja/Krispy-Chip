#include <Chip8.h>
#include <iostream>

static unsigned char chip8_fontset[80] =
{
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};


void init_vm(Chip8_VM* cpu)
{
	std::cout << "[+] Initialising virtual machine\n";
	for (unsigned char i = 0; i < 16; i++)
	{
		cpu->registers[i] = 0;
		cpu->stack[i] = 0;
	}

	for (unsigned int i = 0; i < 64 * 32; i++)
	{
		cpu->framebuffer[i] = 0;
	}

	cpu->indexRegister = 0;
	cpu->delayTimer = 0;
	cpu->soundTimer = 0;
	cpu->stackPointer = 0;

	cpu->programCounter = 0x200;

	for (unsigned i = 0; i < 80; i++)
	{
		cpu->memory[i] = chip8_fontset[i];
	}

}

void load_rom(Chip8_VM* cpu, const char* filepath)
{
	std::cout << "[+] Loading rom " << filepath << "\n";

	FILE* file = fopen(filepath, "rb");
	if (!file) {
		std::cerr << "[-] Failed to open rom\n";
		return;
	}

	uint8_t buffer[1024];
	size_t bytesRead;
	unsigned i = 0x200;

	while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
		for (size_t j = 0; j < bytesRead; ++j) {
			cpu->memory[i++] = buffer[j];
			if (i >= sizeof(cpu->memory)) {
				std::cerr << "[-] ROM is too large to fit in memory\n";
				fclose(file);
				return;
			}
		}
	}

	if (bytesRead == 0 && i == 0x200) {
		std::cerr << "[-] Error reading the ROM file\n";
	}

	fclose(file);
}


void update_timers(Chip8_VM* cpu)
{
	if (cpu->delayTimer > 0) cpu->delayTimer -= 1;
	if (cpu->soundTimer > 0) cpu->soundTimer -= 1;
}

void cpu_cycle(Chip8_VM* cpu)
{
	uint16_t opcode = cpu->memory[cpu->programCounter] << 8 | cpu->memory[cpu->programCounter + 1];
	std::cout << std::hex << std::uppercase << opcode << "\n";
	switch (opcode & 0xF000)
	{

	case 0x0000:
		if (opcode == 0x00E0)	// Clear screen
		{
			for (unsigned int i = 0; i < 64 * 32; i++)
			{
				cpu->framebuffer[i] = 0;
			}
			cpu->programCounter += 2;
		}
		else if (opcode == 0x00EE)
		{
			cpu->stackPointer -= 1;
			cpu->programCounter = cpu->stack[cpu->stackPointer];
			cpu->programCounter += 2;
		}
		else
		{
			std::cout << "[-] Invalid instruction: " << std::hex << std::uppercase << opcode << "\n";
			cpu->programCounter += 2;
		}
		break;

	case 0x1000:	// 0x1NNN Jumps to address NNN
		cpu->programCounter = opcode & 0x0FFF;
		break;

	case 0x2000:	// 0x2NNN Calls subroutine at NNN
		cpu->stack[cpu->stackPointer] = cpu->programCounter;
		cpu->stackPointer += 1;
		cpu->programCounter = opcode & 0x0FFF;
		break;

	case 0x3000:
	{
		uint8_t x = (opcode & 0x0F00) >> 8;
		uint8_t value = (opcode & 0x00FF);
		if (cpu->registers[x] == value) cpu->programCounter += 2;
		cpu->programCounter += 2;
	}
	break;

	case 0x4000:
	{
		uint8_t x = (opcode & 0x0F00) >> 8;
		uint8_t value = (opcode & 0x00FF);
		if (cpu->registers[x] != value) cpu->programCounter += 2;
		cpu->programCounter += 2;
	}
	break;

	case 0x5000:
	{
		uint8_t x = (opcode & 0x0F00) >> 8;
		uint8_t y = (opcode & 0x00F0) >> 4;
		if (cpu->registers[x] == cpu->registers[y]) cpu->programCounter += 2;
		cpu->programCounter += 2;
	}
	break;

	case 0x6000:	// 0x6XNN Loads the value NN into register X
	{
		uint8_t x = (opcode & 0x0F00) >> 8;
		uint8_t value = opcode & 0x00FF;
		cpu->registers[x] = value;
		cpu->programCounter += 2;
	}
	break;

	case 0x7000:	// 0x7XNN Adds the value NN to register X
	{
		uint8_t x = (opcode & 0x0F00) >> 8;
		uint8_t value = opcode & 0x00FF;
		cpu->registers[x] += value;
		cpu->programCounter += 2;
	}
	break;

	case 0x8000:	// 0x8XY0 - 0x8XY7, 0x8XYE
	{
		uint8_t x = (opcode & 0x0F00) >> 8;
		uint8_t y = (opcode & 0x00F0) >> 4;

		switch (opcode & 0x000F)
		{
		case 0x0:	// Assign
			cpu->registers[x] = cpu->registers[y];
			cpu->programCounter += 2;
			break;
		case 0x1:	// Bitwise or
			cpu->registers[x] |= cpu->registers[y];
			cpu->programCounter += 2;
			break;
		case 0x2:	// Bitwsie and
			cpu->registers[x] &= cpu->registers[y];
			cpu->programCounter += 2;
			break;
		case 0x3:	// Bitwise xor
			cpu->registers[x] ^= cpu->registers[y];
			cpu->programCounter += 2;
			break;
		case 0x4:	// Vx += Vy (sets carry flag VF to 1, if there is carry)
		{
			uint16_t result = cpu->registers[x] + cpu->registers[y];
			cpu->registers[x] = result & 0x00FF;
			if (result > 255) cpu->registers[0xF] = 1;
			else cpu->registers[0xF] = 0;
			cpu->programCounter += 2;
		}
		break;
		case 0x5:	// Vx -= Vy (sets VF to 1 if Vx >= Vy)
		{
			uint8_t x_value = cpu->registers[x];
			cpu->registers[x] -= cpu->registers[y];
			if (x_value >= cpu->registers[y]) cpu->registers[0xF] = 1;
			else cpu->registers[0xF] = 0;
			cpu->programCounter += 2;
		}
			break;
		case 0x6:	// Vx >>= 1 (Stores lsb in VF)
		{
			uint8_t lsb = cpu->registers[x] & 0x1;
			cpu->registers[x] >>= 1;
			cpu->registers[0xF] = lsb;
			cpu->programCounter += 2;
		}
		break;
		case 0x7:	// Vx -= Vy (sets VF to 1 if Vy >= Vx)
		{
			uint8_t x_value = cpu->registers[x];
			cpu->registers[x] = cpu->registers[y] - cpu->registers[x];
			if (cpu->registers[y] >= x_value) cpu->registers[0xF] = 1;
			else cpu->registers[0xF] = 0;
			cpu->programCounter += 2;
		}
			break;
		case 0xE:
		{
			uint8_t msb = (cpu->registers[x] & 0x80) >> 7;
			cpu->registers[x] <<= 1;
			cpu->registers[0xF] = msb;
			cpu->programCounter += 2;
		}
		break;
		default:
			break;
		}
	}
	break;

	case 0x9000:
	{
		uint8_t x = (opcode & 0x0F00) >> 8;
		uint8_t y = (opcode & 0x00F0) >> 4;
		if (cpu->registers[x] != cpu->registers[y]) cpu->programCounter += 2;
		cpu->programCounter += 2;
	}
	break;

	case 0xA000:	// 0xANNN Loads the value NNN into index register
		cpu->indexRegister = opcode & 0x0FFF;
		cpu->programCounter += 2;
		break;

	case 0xB000:
		__debugbreak();
		break;

	case 0xC000:
		__debugbreak();
		break;


		/*
		DXYN - Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.
		Each row of 8 pixels is read as bit-coded starting from memory location I; I value does not change after the execution of this instruction.
		As described above, VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that does not happen
		(https://en.wikipedia.org/wiki/CHIP-8)
		*/
	case 0xD000:
	{
		uint8_t x = (opcode & 0x0F00) >> 8;
		uint8_t y = (opcode & 0x00F0) >> 4;
		uint8_t n = opcode & 0x000F;

		uint8_t startX = cpu->registers[x];
		uint8_t startY = cpu->registers[y];

		cpu->registers[0xF] = 0;

		for (unsigned y = 0; y < n; y++)
		{
			uint8_t row = cpu->memory[cpu->indexRegister + y];

			for (unsigned x = 0; x < 8; x++)
			{
				// Extracts the xth bit from the row and checks if it is 1
				if ((row & (0x80 >> x)) != 0)
				{
					uint8_t xCoord = (startX + x) % 64;
					uint8_t yCoord = (startY + y) % 64;

					unsigned index = xCoord + yCoord * 64;

					cpu->framebuffer[index] ^= 1;

					if (cpu->framebuffer[index] == 0) cpu->registers[0xF] = 1;	// If the pixel is 0 after xor it must have been 1 before since we only xor with 1
				}
			}
		}
		cpu->programCounter += 2;
	}
	break;

	case 0xE000:
		__debugbreak();
		break;

	case 0xF000:
	{

		uint8_t x = (opcode & 0x0F00) >> 8;
		switch (opcode & 0x00FF)
		{

		case 0x29:
			cpu->indexRegister = cpu->registers[x] * 5;
			cpu->programCounter += 2;
			break;

		case 0x33:
			cpu->programCounter += 2;
			break;

		case 0x1E:
			cpu->indexRegister += cpu->registers[x];
			cpu->programCounter += 2;
			break;

		case 0x65:
			for (uint8_t i = 0; i <= x; i++)
			{
				cpu->registers[i] = cpu->memory[cpu->indexRegister + i];
			}
			cpu->programCounter += 2;
			break;

		case 0x55:
			for (uint8_t i = 0; i <= x; i++)
			{
				cpu->memory[cpu->indexRegister + i] = cpu->registers[i];
			}
			cpu->programCounter += 2;
			break;

		case 0x0A:
			std::cin.get();
			break;

		default:
			__debugbreak();

		}
		break;

	}
	default:
		std::cout << "[-] Invalid instruction: " << std::hex << std::uppercase << opcode << "\n";
		cpu->programCounter += 2;
		break;
	}

}
