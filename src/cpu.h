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
	uint8_t display[64*32];
	uint8_t keypad[16];
	unsigned char key;
	uint8_t draw_flag; // bool
} chip8_t;

void init_cpu(chip8_t* cpu);
int load_rom(chip8_t* cpu, const char* filename);
void emulate_cycle(chip8_t* cpu);
void clear_screen(chip8_t* cpu);
void update_timers(chip8_t* cpu);
#endif
