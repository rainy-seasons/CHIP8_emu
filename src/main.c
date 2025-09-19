#include "../include/SDL3/SDL.h"
#include <stdio.h>
#include "cpu.h"

#define CHIP8_HEIGHT 32
#define CHIP8_WIDTH 64
#define PIXEL_SIZE 10

int main(int argc, char const* argv[])
{
	const char* rom = argv[1];
	if (!rom)
	{
		printf("Usage: chip8 <name_of_rom>");
		return -1;
	}

	chip8_t cpu;

	init_cpu(&cpu);
	load_rom(&cpu, rom);

	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		SDL_Log("SDL_Init Failed!");
		return -1;
	}

	SDL_Window *window = SDL_CreateWindow("CHIP8", 800, 600, 0); // SDL_WINDOW_RESIZABLE | SDL_WINDOW_BORDERLESS

	if (!window)
	{
		printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
		SDL_Quit();
	}

	SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
	if (!renderer)
	{
		printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
		SDL_DestroyWindow(window);
		SDL_Quit();
	}

	SDL_Event event;

	int running = 1;

	while (running)
	{ 
		emulate_cycle(&cpu); 

#ifdef DEBUG
		print_debug_info(&cpu);
#endif

		if (cpu.draw_flag)
		{
			SDL_Delay(2);
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderClear(renderer);
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			for (int y = 0; y < CHIP8_HEIGHT; y++)
			{
				for (int x = 0; x < CHIP8_WIDTH; x++)
				{
					if (cpu.display[y * CHIP8_WIDTH + x])
					{
						SDL_FRect rect = { x * PIXEL_SIZE, y * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE };
						SDL_RenderFillRect(renderer, &rect);
					}
				}
			}

			SDL_RenderPresent(renderer);
			cpu.draw_flag = 0; 
		}

		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_EVENT_QUIT)
			{
				running = 0;
			}
		}

		//SDL_SetRenderDrawColor(renderer, 0, 128, 255, 255);
		//SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		//SDL_RenderClear(renderer);
		//SDL_RenderPresent(renderer); // present the frame
	}

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 1;
}
