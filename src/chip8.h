#include <stdint.h>
#include <stdbool.h>

typedef struct {
	uint8_t memory[4096];
	uint8_t V[16]; // 16 8-bit Registers. V0 - VF
	uint8_t stack[16];

	uint16_t ir; // index register
	uint16_t pc; // program counter
			 
	uint8_t  sp; // stack pointer
	uint8_t delay_reg; // delay timer
	uint8_t sound_reg; // sound timer

	// top left to bottom right
	int framebuffer[64 * 32];

} chip8;
