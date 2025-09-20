#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"

uint8_t font[80] = {
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

void init_cpu(chip8_t* cpu)
{
	memset(cpu->memory, 0, 4096);
	memset(cpu->V, 0, 16);
	memset(cpu->stack, 0, sizeof(cpu->stack));
	memset(cpu->keypad, 0, 16);

	cpu->ir = 0;
	cpu->pc = 0x200;
	cpu->sp = 0;
	cpu->delay_timer = 0;
	cpu->sound_timer = 0;
	cpu->opcode = 0;

	memcpy(&cpu->memory[0], font, sizeof(font));
}

int load_rom(chip8_t* cpu, const char* filename)
{
	FILE* fp = fopen(filename, "rb");
	if (!fp)
	{
		printf("File not found: %s", filename);
		return -1;
	}

	fseek(fp, 0, SEEK_END);
	unsigned long buf_size = ftell(fp);
	rewind(fp);

	printf("Read %lu bytes from %s\n", buf_size, filename);
	fread(&cpu->memory[0x200], buf_size, 1, fp); // Read the rom into chip8_t memory

	fclose(fp);
	return 1;
}

void emulate_cycle(chip8_t* cpu)
{
	// Get next two byte opcode
	cpu->opcode = (cpu->memory[cpu->pc] << 8) | (cpu->memory[cpu->pc + 1]);

	switch (cpu->opcode & 0xF000)
	{
		case 0x0000: // 0NNN
			switch(cpu->opcode)
			{
				case 0x00E0: // CLS - clear screen
					clear_screen(cpu);
					cpu->pc += 2;
					break;
				case 0x00EE:  // RET - return from a subroutine, sets PC = stack[sp] then sp--
					cpu->sp -= 1;
					cpu->pc = cpu->stack[cpu->sp];
					cpu->pc += 2;
					break;
				default:
					printf("Error: unknown opcode: %x", cpu->opcode);
					cpu->pc += 2;
					break;
			}
			break;
		case 0x1000: // 1NNN: JP addr - jump to NNN
			cpu->pc = cpu->opcode & 0x0FFF;
			break;
		case 0x2000: // 2NNN: calls subroutine at NNN
			cpu->stack[cpu->sp] = cpu->pc;
			cpu->sp++;
			cpu->pc = cpu->opcode & 0x0FFF;
			break;
		case 0x3000: // 3XNN: Skips next instruction if Vx == NN
			if (cpu->V[(cpu->opcode & 0x0F00)] == (cpu->opcode & 0x00FF))
			{
				cpu->pc += 4;
				break;
			}
			cpu->pc += 2;
			break;
		case 0x4000: // 4XNN: Skips next instruction if Vx != NN
			if (cpu->V[(cpu->opcode & 0x0F00)] != (cpu->opcode & 0x00FF))
			{
				cpu->pc += 4;
				break;
			}
			cpu->pc += 2;
			break;
		case 0x5000: // 5XY0: Skips if the values in VX and VY are equal
			if (cpu->V[(cpu->opcode & 0x0F00)] == cpu->V[(cpu->opcode & 0x00F0)])
			{
				cpu->pc += 4;
				break;
			}
			cpu->pc += 2;
			break;
		case 0x6000: // 6XNN:  Vx = NN
			(cpu->V[cpu->opcode & 0x0F00]) = (cpu->opcode & 0x00FF);
			cpu->pc += 2;
			break;
		case 0x7000: // 7XNN: Vx += NN
			(cpu->V[cpu->opcode & 0x0F00]) += (cpu->opcode & 0x00FF);
			cpu->pc += 2;
			break;
		case 0x8000:
			uint8_t x = (cpu->opcode & 0x0F00) >> 8;
			uint8_t y = (cpu->opcode & 0x00F0) >> 4;
			switch(cpu->opcode & 0x000F)
			{
				case 0x0000: // 8XY0: Set VX to value of VY
					cpu->V[x] = cpu->V[y];
					break;
				case 0x0001: // 8XY1: Set VX to VX OR VY  (VX |= VY)
					cpu->V[x] |= cpu->V[y];
					break;
				case 0x0002: // 8XY2: Set VX to VX AND VY (VX &= VY)
					cpu->V[x] &= cpu->V[y];
					break;
				case 0x0003: // 8XY3: Set VX to VX XOR VY (VX ^= VY)
					cpu->V[x] ^= cpu->V[y];
					break;
				case 0x0004: // 8XY4: Add VY to VX. VF is set to 1 when there's an overflow (greater than 255), and 0 if not.
					cpu->V[x] += cpu->V[y];
					cpu->V[0xF] = (cpu->V[x] > 0xFF) ? 1 : 0;
					break;
				case 0x0005: // 8XY5: VY subtracted from VX. VF = 0 when there's underflow, and 1 when there's not. 
					if (cpu->V[x] - cpu->V[y] < 0)
						cpu->V[0xF] = 0;
					else
						cpu->V[0xF] = 1;

					cpu->V[x] -= cpu->V[y];
					break;
				case 0x0006: // 8XY6: Shifts VX to the right by 1, then stores the LSB of VX prior to the shift into VF
					cpu->V[0xF] = cpu->V[x] & 0x01;
					cpu->V[x] >>= 1;
					break;
				case 0x0007: // 8XY7: VX = VY - VX. VF = 0 if underflow, otherwise VF = 1.
					cpu->V[0xF] = cpu->V[x] > cpu->V[y] ? 0 : 1;
					cpu->V[x] = cpu->V[y] - cpu->V[x];
					break;
				case 0x000E: // 8XYE: Shift VX to the left by 1. Set VF to 1 if the MSB of VX prior to shift was set, or 0 if it was unset
					cpu->V[0xF] = (cpu->V[x] & 0x80) >> 7;
					cpu->V[x] <<= 1;
					break;
			}
			cpu->pc += 2;
			break;

		case 0x9000: // 9XY0: Skips the next instruction if VX != VY.
			if ((cpu->V[cpu->opcode & 0x0F00]) != cpu->V[cpu->opcode & 0x00F0])
				cpu->pc += 4;
			else
				cpu->pc += 2;
			break;

		case 0xA000: // ANNN: LD I, addr
			cpu->ir = cpu->opcode & 0x0FFF;
			cpu->pc += 2;
			break;
		case 0xB000: // BNNN: Jumps to address NNN + V0. PC = V0 + NNN
			cpu->pc = (cpu->opcode & 0x0FFF) + cpu->V[0];
			break;
		case 0xC000: // CXNN: VX = (NN & randomNumber)
			int r = rand() % RAND_MAX;
			break;
		case 0xD000: // DXYN: draw(Vx, Vy, N)
			uint8_t x_coord = cpu->V[(cpu->opcode & 0x0F00) >> 8];
			uint8_t y_coord = cpu->V[(cpu->opcode & 0x00F0) >> 4];
			uint8_t height  = cpu->opcode & 0x000F;

			cpu->V[0xF] = 0; // reset collision flag

			for (int row = 0; row < height; row++)
			{
				uint8_t sprite_byte = cpu->memory[cpu->ir + row];

				for (int col = 0; col < 8; col++)
				{
					if ((sprite_byte & (0x80 >> col)) != 0) // check if pixel in sprite is set
					{
						int x = (x_coord + col) % 64;
						int y = (y_coord + row) % 32;
						int index = (y * 64) + x;

						if (cpu->display[index] == 1)
						{
							cpu->V[0xF] = 1; 
						}
						cpu->display[index] ^= 1; // xor pixel
					}
				}
			}
			cpu->draw_flag = 1;
			cpu->pc += 2;
			break;
	}
}

void clear_screen(chip8_t* cpu)
{
	memset(cpu->display, 0, sizeof(cpu->display));
}

void update_timers(chip8_t* cpu)
{
	printf("update_timers() NOT IMPLEMENTED YET.\n");
}
