#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"

/* INFO
 *
 * 60 hz. Can handle this in different ways like redrawing the screen only when an 
 * 		instruction that modifies the display data is run
 *
 * 		DXYN -- the drawing instruction
 * 		used to draw a sprite on the screen
 * 		each sprite is a byte where each bit is a horizontal pixel
 * 		sprites are between 1 and 15 bytes tall
 * 		they're drawn to the screen by treating all 0 bits as transparent
 * 		and all 1 bits will flip the pixels in the locations of the screen that it's drawn to (XOR)
 * 		
 * 		This will cause some flickering because a sprite is erased by drawing it again (flipping the bits)
 * 		and then re-drawn in the new position. Which means it will disappear for a while before being redrawn
 *
 * 700 instructions per second
 *
 */

// 			First nibble - what kind of instruction it is
// 			X: The second nibble, used to look up one of the registers (VX) from V0-VF
// 			Y: Third nibble. Also used to look up a register (VY)
// 			N: Fourth nibble, a 4-bit number
// 			NN: Second byte (third and fourth nibbles). 8-bit immediate number.
// 			NNN: The second, third, and fourth nibbles. A 12-bit immediate memory address
// 			EXTRACT THESE BEFORE DECODING instead of doing it inside each instruction
// 			A #define or other macro directive would work well here
//
//			(Also X and Y are always used to look up the values in registers). Don't use the actual value X in the instruction. It's only for the N operands. X and Y should always look up a value in the corresponding register

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
	// get next opcode (two bytes)
	cpu->opcode = (cpu->memory[cpu->pc] << 8) | (cpu->memory[cpu->pc + 1]);
	//cpu->pc += 2; // pre-increment program counter for next opcode

	switch (cpu->opcode & 0xF000)
	{
		case 0x0000: // 0NNN
			switch(cpu->opcode)
			{
				case 0x00E0: // CLS - clear screen
					clear_screen(cpu);
					cpu->pc += 2;
					break;
				case 0x00EE:  // RET - return from a subrouting, sets PC = stack[sp] then sp--
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
		case 0x6000: // 6XNN:  Vx = NN
			(cpu->V[cpu->opcode & 0x0F00]) = (cpu->opcode & 0x00FF);
			cpu->pc += 2;
			break;
		case 0x7000: // 7XNN: Vx += NN
			(cpu->V[cpu->opcode & 0x0F00]) += (cpu->opcode & 0x00FF);
			cpu->pc += 2;
			break;
		case 0xA000: // ANNN: LD I, addr
			cpu->ir = cpu->opcode & 0x0FFF;
			cpu->pc += 2;
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
