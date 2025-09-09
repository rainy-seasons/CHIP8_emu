#include <string.h>
#include "chip8.h"

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

// Fetch -- Read the instruction that the PC is poitning at from memory
// 			Each instruction is two bytes, so read two successive bytes and combine them into a 16-bit instruction
// 			
// 			Immediately increment the PC by two, to be ready to fetch the next opcode.
//
// Decode -- Instructions are divided into broad categories by the first nibble (half-byte)
// 				which is the first hexadecimal number. 
// 			So basically, do a if/else (or switch) based on the first number.
// 			Mask off the first number with a binary AND and have one case per number
// 			Some cases will need separate switch statements inside them to further decode the instruction
//
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
//
// Execute -- This won't really be a separate stage if using a switch statement. Just do the instruction inside the case. 
// 				
//
// Some starting instructions:
// 0x00E0 (clear screen)
// 0x1NNN (jump)
// 0x6XNN (set register VX)
// 0x7XNN (add value to register VX)
// 0xANNN (set index register I)
// 0xDXYN (display/draw)
// These are easily testable with the IBM logo program. It just displays the IBM logo. And it only uses these instructions.



// 0x000 - 0x1FF reserved for interpreter so the program is loaded at 0x200
uint32_t start = 0x200;

// Built in font with sprite data
// Each char is 4px wide and 5px tall
// Store in memory because games draw these like regular sprites
// They set the index register to the character memory location and then draw it
//
// likely going to put this at 0x050-0x9F
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

void init_cpu(chip8* cpu)
{
	memset(cpu->memory, 0, 4096);
	memset(cpu->V, 0, 16);
	memset(cpu->stack, 0, 16);
	memset(cpu->keys, 0, 16);

	cpu->ir = 0;
	cpu->pc = 0x200;
	cpu->sp = 0;
	cpu->delay_timer = 0;
	cpu->sound_timer = 0;
	cpu->opcode = 0;

	// load fonts
	for (int i = 0; i < 80; i++)
	{
		cpu->memory[i] = font[i];
	}

}
