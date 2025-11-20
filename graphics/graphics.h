#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "../arch/chip8.h"
#include <SDL2/SDL.h>

#define SCALE 10
#define SCREEN_WIDTH (DISPLAY_WIDTH * SCALE)
#define SCREEN_HEIGHT (DISPLAY_HEIGHT * SCALE)

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
} Graphics;

void graphics_init(Graphics* graphics);
void graphics_cleanup(Graphics* graphics);
void graphics_render(Graphics* graphics, CHIP8* chip8);
void graphics_handle_input(CHIP8* chip8, int* running, char** rom_path);

#endif