#ifndef _CHIP8_H
#define _CHIP8_H
#include <stdint.h>

typedef struct {
	uint16_t opcode; 
	
	uint8_t memory[4096];

	uint8_t V[16]; // 16 8-bit Registers. V0 - VF; VF doubles as a carry flag

	// These can both only address 12 bits even though they're 16 bits long
	uint16_t ir; // index register
	uint16_t pc; // program counter
			 
	uint16_t stack[16];
	uint16_t sp; 
		
	uint8_t delay_timer; // decremented at 60hz until zero
	uint8_t sound_timer; // functions same as delay timer but beeps if not zero
						 // This is independent of the fetch/decode/execute loop

	// top left to bottom right
	// uint8_t display[64][32]
	uint8_t display[64 * 32];

	uint8_t keys[16]; // hex keypad (COSMAC VIP)
					  // 1 2 3 C
					  // 4 5 6 D
					  // 7 8 9 E
					  // A 0 B F
					  // -------
					  // Interpreted to be:
					  // ------
					  // 1 2 3 4
					  // Q W E R
					  // A S D F
					  // Z X C V

	unsigned char key;

	int draw_flag; // bool

} chip8;

void init_cpu(chip8* cpu);
int load_rom(chip8* cpu, const char* filename);
void emulate_cycle(chip8* cpu);
void clear_screen(chip8* cpu);
void update_timers(chip8* cpu);

#endif
