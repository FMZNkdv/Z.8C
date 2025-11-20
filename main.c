#include "arch/chip8.h"
#include "graphics/graphics.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
    char* rom_path = NULL;
    
    if(argc == 2) {
        rom_path = malloc(strlen(argv[1]) + 1);
        if (rom_path) {
            strcpy(rom_path, argv[1]);
        }
    }
    
    Graphics graphics;
    graphics_init(&graphics);
    
    if(!rom_path) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Open8B", 
                                "Drag and drop a .ch8 file onto this window to start!\n\n"
                                "Or run from terminal: ./Open8B roms/tank.ch8", NULL);
    }
    
    int running = 1;
    Uint32 last_timer_time = SDL_GetTicks();
    
    while(running) {
        if(rom_path) {
            CHIP8 chip8;
            chip8_init(&chip8);
            chip8_load_rom(&chip8, rom_path);
            
            int emulation_running = 1;
            while(emulation_running && running) {
                graphics_handle_input(&chip8, &running, &rom_path);
                
                for(int i = 0; i < 8; i++) {
                    chip8_emulate_cycle(&chip8);
                }
                
                Uint32 current_time = SDL_GetTicks();
                if(current_time - last_timer_time > 16) {
                    if(chip8.delay_timer > 0) chip8.delay_timer--;
                    if(chip8.sound_timer > 0) chip8.sound_timer--;
                    last_timer_time = current_time;
                }
                
                if(chip8.draw_flag) {
                    graphics_render(&graphics, &chip8);
                }
                
                SDL_Delay(2);
            }
        } else {
            graphics_handle_input(NULL, &running, &rom_path);
            SDL_Delay(16);
        }
    }
    
    if(rom_path) {
        free(rom_path);
    }
    
    graphics_cleanup(&graphics);
    return 0;
}