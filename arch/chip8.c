#include "chip8.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const uint8_t fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, 0x20, 0x60, 0x20, 0x20, 0x70,
    0xF0, 0x10, 0xF0, 0x80, 0xF0, 0xF0, 0x10, 0xF0, 0x10, 0xF0,
    0x90, 0x90, 0xF0, 0x10, 0x10, 0xF0, 0x80, 0xF0, 0x10, 0xF0,
    0xF0, 0x80, 0xF0, 0x90, 0xF0, 0xF0, 0x10, 0x20, 0x40, 0x40,
    0xF0, 0x90, 0xF0, 0x90, 0xF0, 0xF0, 0x90, 0xF0, 0x10, 0xF0,
    0xF0, 0x90, 0xF0, 0x90, 0x90, 0xE0, 0x90, 0xE0, 0x90, 0xE0,
    0xF0, 0x80, 0x80, 0x80, 0xF0, 0xE0, 0x90, 0x90, 0x90, 0xE0,
    0xF0, 0x80, 0xF0, 0x80, 0xF0, 0xF0, 0x80, 0xF0, 0x80, 0x80
};

void chip8_init(CHIP8* chip8) {
    memset(chip8, 0, sizeof(CHIP8));
    chip8->pc = 0x200;
    
    for(int i = 0; i < 80; i++) {
        chip8->memory[i] = fontset[i];
    }
    
    srand(time(NULL));
}

void chip8_load_rom(CHIP8* chip8, const char* filename) {
    FILE* file = fopen(filename, "rb");
    if(!file) {
        printf("Error: Could not open file %s\n", filename);
        exit(1);
    }
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);
    
    if(file_size > 1024 * 1024) {
        printf("Warning: ROM file is very large (%ld bytes), loading first 1MB\n", file_size);
        file_size = 1024 * 1024;
    }
    
    size_t bytes_to_load = (size_t)file_size;
    size_t max_chip8_memory = MEM_SIZE - 0x200;
    
    if(bytes_to_load > max_chip8_memory) {
        printf("Warning: ROM too large for CHIP-8 memory, loading first %zu bytes\n", max_chip8_memory);
        bytes_to_load = max_chip8_memory;
    }
    
    size_t bytes_read = fread(&chip8->memory[0x200], 1, bytes_to_load, file);
    if(bytes_read != bytes_to_load) {
        printf("Warning: Only read %zu bytes out of %zu requested\n", bytes_read, bytes_to_load);
    }
    
    fclose(file);
    printf("Successfully loaded %zu bytes from ROM\n", bytes_read);
}

void chip8_emulate_cycle(CHIP8* chip8) {
    uint16_t opcode = (chip8->memory[chip8->pc] << 8) | chip8->memory[chip8->pc + 1];
    chip8->pc += 2;
    
    uint16_t nnn = opcode & 0x0FFF;
    uint8_t n = opcode & 0x000F;
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    uint8_t kk = opcode & 0x00FF;
    
    switch(opcode & 0xF000) {
        case 0x0000:
            switch(opcode) {
                case 0x00E0:
                    memset(chip8->display, 0, sizeof(chip8->display));
                    chip8->draw_flag = 1;
                    break;
                case 0x00EE:
                    chip8->sp--;
                    chip8->pc = chip8->stack[chip8->sp];
                    break;
            }
            break;
            
        case 0x1000:
            chip8->pc = nnn;
            break;
            
        case 0x2000:
            chip8->stack[chip8->sp] = chip8->pc;
            chip8->sp++;
            chip8->pc = nnn;
            break;
            
        case 0x3000:
            if(chip8->V[x] == kk) chip8->pc += 2;
            break;
            
        case 0x4000:
            if(chip8->V[x] != kk) chip8->pc += 2;
            break;
            
        case 0x5000:
            if(chip8->V[x] == chip8->V[y]) chip8->pc += 2;
            break;
            
        case 0x6000:
            chip8->V[x] = kk;
            break;
            
        case 0x7000:
            chip8->V[x] += kk;
            break;
            
        case 0x8000:
            switch(n) {
                case 0x0: chip8->V[x] = chip8->V[y]; break;
                case 0x1: chip8->V[x] |= chip8->V[y]; break;
                case 0x2: chip8->V[x] &= chip8->V[y]; break;
                case 0x3: chip8->V[x] ^= chip8->V[y]; break;
                case 0x4:
                    chip8->V[0xF] = (chip8->V[x] + chip8->V[y] > 255) ? 1 : 0;
                    chip8->V[x] += chip8->V[y];
                    break;
                case 0x5:
                    chip8->V[0xF] = (chip8->V[x] > chip8->V[y]) ? 1 : 0;
                    chip8->V[x] -= chip8->V[y];
                    break;
                case 0x6:
                    chip8->V[0xF] = chip8->V[x] & 0x1;
                    chip8->V[x] >>= 1;
                    break;
                case 0x7:
                    chip8->V[0xF] = (chip8->V[y] > chip8->V[x]) ? 1 : 0;
                    chip8->V[x] = chip8->V[y] - chip8->V[x];
                    break;
                case 0xE:
                    chip8->V[0xF] = (chip8->V[x] & 0x80) >> 7;
                    chip8->V[x] <<= 1;
                    break;
            }
            break;
            
        case 0x9000:
            if(chip8->V[x] != chip8->V[y]) chip8->pc += 2;
            break;
            
        case 0xA000:
            chip8->I = nnn;
            break;
            
        case 0xB000:
            chip8->pc = nnn + chip8->V[0];
            break;
            
        case 0xC000:
            chip8->V[x] = (rand() % 256) & kk;
            break;
            
        case 0xD000: {
            uint8_t x_pos = chip8->V[x] % DISPLAY_WIDTH;
            uint8_t y_pos = chip8->V[y] % DISPLAY_HEIGHT;
            chip8->V[0xF] = 0;
            
            for(int row = 0; row < n; row++) {
                uint8_t sprite_byte = chip8->memory[chip8->I + row];
                for(int col = 0; col < 8; col++) {
                    if((sprite_byte & (0x80 >> col)) != 0) {
                        int display_index = (y_pos + row) * DISPLAY_WIDTH + (x_pos + col);
                        if(display_index < DISPLAY_WIDTH * DISPLAY_HEIGHT) {
                            if(chip8->display[display_index] == 1) {
                                chip8->V[0xF] = 1;
                            }
                            chip8->display[display_index] ^= 1;
                        }
                    }
                }
            }
            chip8->draw_flag = 1;
            break;
        }
            
        case 0xE000:
            switch(kk) {
                case 0x9E:
                    if(chip8->key[chip8->V[x]]) chip8->pc += 2;
                    break;
                case 0xA1:
                    if(!chip8->key[chip8->V[x]]) chip8->pc += 2;
                    break;
            }
            break;
            
        case 0xF000:
            switch(kk) {
                case 0x07: chip8->V[x] = chip8->delay_timer; break;
                case 0x0A: {
                    int key_pressed = 0;
                    for(int i = 0; i < NUM_KEYS; i++) {
                        if(chip8->key[i]) {
                            chip8->V[x] = i;
                            key_pressed = 1;
                            break;
                        }
                    }
                    if(!key_pressed) chip8->pc -= 2;
                    break;
                }
                case 0x15: chip8->delay_timer = chip8->V[x]; break;
                case 0x18: chip8->sound_timer = chip8->V[x]; break;
                case 0x1E: chip8->I += chip8->V[x]; break;
                case 0x29: chip8->I = chip8->V[x] * 5; break;
                case 0x33:
                    chip8->memory[chip8->I] = chip8->V[x] / 100;
                    chip8->memory[chip8->I + 1] = (chip8->V[x] / 10) % 10;
                    chip8->memory[chip8->I + 2] = chip8->V[x] % 10;
                    break;
                case 0x55:
                    for(int i = 0; i <= x; i++) {
                        chip8->memory[chip8->I + i] = chip8->V[i];
                    }
                    break;
                case 0x65:
                    for(int i = 0; i <= x; i++) {
                        chip8->V[i] = chip8->memory[chip8->I + i];
                    }
                    break;
            }
            break;
            
        default:
            printf("Unknown opcode: 0x%04X\n", opcode);
    }
    
    if(chip8->delay_timer > 0) chip8->delay_timer--;
    if(chip8->sound_timer > 0) chip8->sound_timer--;
}    long file_size = ftell(file);
    rewind(file);
    
    if(file_size > MEM_SIZE - 0x200) {
        printf("Error: ROM too large\n");
        fclose(file);
        exit(1);
    }
    
    fread(&chip8->memory[0x200], 1, file_size, file);
    fclose(file);
}

void chip8_emulate_cycle(CHIP8* chip8) {
    uint16_t opcode = (chip8->memory[chip8->pc] << 8) | chip8->memory[chip8->pc + 1];
    chip8->pc += 2;
    
    uint16_t nnn = opcode & 0x0FFF;
    uint8_t n = opcode & 0x000F;
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    uint8_t kk = opcode & 0x00FF;
    
    switch(opcode & 0xF000) {
        case 0x0000:
            switch(opcode) {
                case 0x00E0:
                    memset(chip8->display, 0, sizeof(chip8->display));
                    chip8->draw_flag = 1;
                    break;
                case 0x00EE:
                    chip8->sp--;
                    chip8->pc = chip8->stack[chip8->sp];
                    break;
            }
            break;
            
        case 0x1000:
            chip8->pc = nnn;
            break;
            
        case 0x2000:
            chip8->stack[chip8->sp] = chip8->pc;
            chip8->sp++;
            chip8->pc = nnn;
            break;
            
        case 0x3000:
            if(chip8->V[x] == kk) chip8->pc += 2;
            break;
            
        case 0x4000:
            if(chip8->V[x] != kk) chip8->pc += 2;
            break;
            
        case 0x5000:
            if(chip8->V[x] == chip8->V[y]) chip8->pc += 2;
            break;
            
        case 0x6000:
            chip8->V[x] = kk;
            break;
            
        case 0x7000:
            chip8->V[x] += kk;
            break;
            
        case 0x8000:
            switch(n) {
                case 0x0: chip8->V[x] = chip8->V[y]; break;
                case 0x1: chip8->V[x] |= chip8->V[y]; break;
                case 0x2: chip8->V[x] &= chip8->V[y]; break;
                case 0x3: chip8->V[x] ^= chip8->V[y]; break;
                case 0x4:
                    chip8->V[0xF] = (chip8->V[x] + chip8->V[y] > 255) ? 1 : 0;
                    chip8->V[x] += chip8->V[y];
                    break;
                case 0x5:
                    chip8->V[0xF] = (chip8->V[x] > chip8->V[y]) ? 1 : 0;
                    chip8->V[x] -= chip8->V[y];
                    break;
                case 0x6:
                    chip8->V[0xF] = chip8->V[x] & 0x1;
                    chip8->V[x] >>= 1;
                    break;
                case 0x7:
                    chip8->V[0xF] = (chip8->V[y] > chip8->V[x]) ? 1 : 0;
                    chip8->V[x] = chip8->V[y] - chip8->V[x];
                    break;
                case 0xE:
                    chip8->V[0xF] = (chip8->V[x] & 0x80) >> 7;
                    chip8->V[x] <<= 1;
                    break;
            }
            break;
            
        case 0x9000:
            if(chip8->V[x] != chip8->V[y]) chip8->pc += 2;
            break;
            
        case 0xA000:
            chip8->I = nnn;
            break;
            
        case 0xB000:
            chip8->pc = nnn + chip8->V[0];
            break;
            
        case 0xC000:
            chip8->V[x] = (rand() % 256) & kk;
            break;
            
        case 0xD000: {
            uint8_t x_pos = chip8->V[x] % DISPLAY_WIDTH;
            uint8_t y_pos = chip8->V[y] % DISPLAY_HEIGHT;
            chip8->V[0xF] = 0;
            
            for(int row = 0; row < n; row++) {
                uint8_t sprite_byte = chip8->memory[chip8->I + row];
                for(int col = 0; col < 8; col++) {
                    if((sprite_byte & (0x80 >> col)) != 0) {
                        int display_index = (y_pos + row) * DISPLAY_WIDTH + (x_pos + col);
                        if(display_index < DISPLAY_WIDTH * DISPLAY_HEIGHT) {
                            if(chip8->display[display_index] == 1) {
                                chip8->V[0xF] = 1;
                            }
                            chip8->display[display_index] ^= 1;
                        }
                    }
                }
            }
            chip8->draw_flag = 1;
            break;
        }
            
        case 0xE000:
            switch(kk) {
                case 0x9E:
                    if(chip8->key[chip8->V[x]]) chip8->pc += 2;
                    break;
                case 0xA1:
                    if(!chip8->key[chip8->V[x]]) chip8->pc += 2;
                    break;
            }
            break;
            
        case 0xF000:
            switch(kk) {
                case 0x07: chip8->V[x] = chip8->delay_timer; break;
                case 0x0A: {
                    int key_pressed = 0;
                    for(int i = 0; i < NUM_KEYS; i++) {
                        if(chip8->key[i]) {
                            chip8->V[x] = i;
                            key_pressed = 1;
                            break;
                        }
                    }
                    if(!key_pressed) chip8->pc -= 2;
                    break;
                }
                case 0x15: chip8->delay_timer = chip8->V[x]; break;
                case 0x18: chip8->sound_timer = chip8->V[x]; break;
                case 0x1E: chip8->I += chip8->V[x]; break;
                case 0x29: chip8->I = chip8->V[x] * 5; break;
                case 0x33:
                    chip8->memory[chip8->I] = chip8->V[x] / 100;
                    chip8->memory[chip8->I + 1] = (chip8->V[x] / 10) % 10;
                    chip8->memory[chip8->I + 2] = chip8->V[x] % 10;
                    break;
                case 0x55:
                    for(int i = 0; i <= x; i++) {
                        chip8->memory[chip8->I + i] = chip8->V[i];
                    }
                    break;
                case 0x65:
                    for(int i = 0; i <= x; i++) {
                        chip8->V[i] = chip8->memory[chip8->I + i];
                    }
                    break;
            }
            break;
            
        default:
            printf("Unknown opcode: 0x%04X\n", opcode);
    }
    
    if(chip8->delay_timer > 0) chip8->delay_timer--;
    if(chip8->sound_timer > 0) chip8->sound_timer--;
}
