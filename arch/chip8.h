#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>

#define MEM_SIZE 4096
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define STACK_SIZE 16
#define NUM_REGISTERS 16
#define NUM_KEYS 16

typedef struct {
    uint8_t memory[MEM_SIZE];
    uint8_t V[NUM_REGISTERS];
    uint16_t I;
    uint16_t pc;
    uint16_t stack[STACK_SIZE];
    uint8_t sp;
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint8_t display[DISPLAY_WIDTH * DISPLAY_HEIGHT];
    uint8_t key[NUM_KEYS];
    int draw_flag;
} CHIP8;

void chip8_init(CHIP8* chip8);
void chip8_load_rom(CHIP8* chip8, const char* filename);
void chip8_emulate_cycle(CHIP8* chip8);

#endif