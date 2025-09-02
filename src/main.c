#include <stdio.h>
#include "chip8.h"
#include "../include/SDL3/SDL.h"

int main()
{
	
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
		return 1;
	}

	SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
	if (!renderer)
	{
		printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	int running = 1;

	SDL_Event event;

	while (running)
	{
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_EVENT_QUIT)
			{
				running = 0;
			}
		}

		SDL_SetRenderDrawColor(renderer, 0, 128, 255, 255);
		SDL_RenderClear(renderer);
		SDL_RenderPresent(renderer); // present the frame
	}

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
