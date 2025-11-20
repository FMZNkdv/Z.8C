#include "graphics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void graphics_init(Graphics* graphics) {
    SDL_Init(SDL_INIT_VIDEO);
    graphics->window = SDL_CreateWindow("Open8B", 
                                       SDL_WINDOWPOS_CENTERED, 
                                       SDL_WINDOWPOS_CENTERED, 
                                       SCREEN_WIDTH, 
                                       SCREEN_HEIGHT, 
                                       SDL_WINDOW_ALLOW_HIGHDPI);
    graphics->renderer = SDL_CreateRenderer(graphics->window, -1, SDL_RENDERER_ACCELERATED);
}

void graphics_cleanup(Graphics* graphics) {
    SDL_DestroyRenderer(graphics->renderer);
    SDL_DestroyWindow(graphics->window);
    SDL_Quit();
}

void graphics_render(Graphics* graphics, CHIP8* chip8) {
    SDL_SetRenderDrawColor(graphics->renderer, 0, 0, 0, 255);
    SDL_RenderClear(graphics->renderer);
    
    SDL_SetRenderDrawColor(graphics->renderer, 255, 255, 255, 255);
    for(int y = 0; y < DISPLAY_HEIGHT; y++) {
        for(int x = 0; x < DISPLAY_WIDTH; x++) {
            if(chip8->display[y * DISPLAY_WIDTH + x]) {
                SDL_Rect pixel = {x * SCALE, y * SCALE, SCALE, SCALE};
                SDL_RenderFillRect(graphics->renderer, &pixel);
            }
        }
    }
    
    SDL_RenderPresent(graphics->renderer);
    chip8->draw_flag = 0;
}

void graphics_handle_input(CHIP8* chip8, int* running, char** rom_path) {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        switch(event.type) {
            case SDL_QUIT:
                *running = 0;
                break;
                
            case SDL_DROPFILE:
                if (*rom_path) {
                    free(*rom_path);
                }
                *rom_path = malloc(strlen(event.drop.file) + 1);
                if (*rom_path) {
                    strcpy(*rom_path, event.drop.file);
                }
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "File Dropped", 
                                        "ROM loaded successfully! Restarting emulator...", NULL);
                break;
                
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                if (chip8) {
                    uint8_t key_state = (event.type == SDL_KEYDOWN) ? 1 : 0;
                    switch(event.key.keysym.sym) {
                        case SDLK_1: chip8->key[0x1] = key_state; break;
                        case SDLK_2: chip8->key[0x2] = key_state; break;
                        case SDLK_3: chip8->key[0x3] = key_state; break;
                        case SDLK_4: chip8->key[0xC] = key_state; break;
                        case SDLK_q: chip8->key[0x4] = key_state; break;
                        case SDLK_w: chip8->key[0x5] = key_state; break;
                        case SDLK_e: chip8->key[0x6] = key_state; break;
                        case SDLK_r: chip8->key[0xD] = key_state; break;
                        case SDLK_a: chip8->key[0x7] = key_state; break;
                        case SDLK_s: chip8->key[0x8] = key_state; break;
                        case SDLK_d: chip8->key[0x9] = key_state; break;
                        case SDLK_f: chip8->key[0xE] = key_state; break;
                        case SDLK_z: chip8->key[0xA] = key_state; break;
                        case SDLK_x: chip8->key[0x0] = key_state; break;
                        case SDLK_c: chip8->key[0xB] = key_state; break;
                        case SDLK_v: chip8->key[0xF] = key_state; break;
                        case SDLK_ESCAPE: *running = 0; break;
                    }
                }
                break;
        }
    }
}