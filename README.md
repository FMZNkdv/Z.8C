![lo8p](Logo-Open8B.png)

**Open8B** is a high-accuracy CHIP-8 emulator written in C using the SDL2 library for graphics and input. CHIP-8 is an interpreted programming language created in the mid-1970s for COSMAC VIP and Telmac 1800 microcomputers. While technically CHIP-8 is not a hardware processor, its virtual machine emulates the behavior of 8-bit systems.

## Table of Contents
- [Features](#features)
- [Installation and Building](#installation-and-building)
- [Usage](#usage)
- [CHIP-8 Architecture](#chip-8-architecture)
- [8-bit Processors](#8-bit-processors)

## Features

- Full compatibility with original CHIP-8 (35 instructions)
- Accurate timing (500Hz CPU, 60Hz timers)
- Drag & Drop ROM loading
- Automatic display scaling (64x32 to 640x320)
- Original keyboard layout
- Modular architecture for easy expansion
- Cross-platform (Linux, Windows, macOS)
- Fully open source under MIT license

## Installation and Building

### Dependencies

**Arch Linux:**
```bash
sudo pacman -S sdl2 gcc make
```

**Ubuntu/Debian:**
```bash
sudo apt-get install libsdl2-dev gcc make
```

**Fedora:**
```bash
sudo dnf install SDL2-devel gcc make
```

**macOS:**
```bash
brew install sdl2
```

### Building from Source

```bash
git clone https://github.com/FMZNkdv/Open8B.git
cd Open8B

make

make clean
```

## Usage

### Running with ROM File

```bash
./bin/Open8B roms/tank.ch8
```

### Running in Drag & Drop Mode

```bash
./bin/Open8B
```

Then simply drag a .ch8 file into the emulator window.

### Controls

**CHIP-8 Keyboard Layout:**

```
Original Layout      ->   Keyboard
+---+---+---+---+         +---+---+---+---+
| 1 | 2 | 3 | C |         | 1 | 2 | 3 | 4 |
+---+---+---+---+         +---+---+---+---+
| 4 | 5 | 6 | D |         | Q | W | E | R |
+---+---+---+---+   ->    +---+---+---+---+
| 7 | 8 | 9 | E |         | A | S | D | F |
+---+---+---+---+         +---+---+---+---+
| A | 0 | B | F |         | Z | X | C | V |
+---+---+---+---+         +---+---+---+---+
```

**Emulator Controls:**
- ESC - Exit emulator
- Drag & Drop - Load new ROM

## CHIP-8 Architecture

### Memory
- **Size**: 4 KB (4096 bytes)
- **Layout**:
  - 0x000-0x1FF: Reserved for interpreter
  - 0x050-0x0A0: Built-in font (16 characters 5x4 pixels)
  - 0x200-0xFFF: Program and data

### Registers
- 16 general-purpose registers (V0-VF, 8-bit)
- I register (16-bit) - memory address
- Program counter (PC, 16-bit)
- Stack pointer (SP, 8-bit)

### Timers
- Delay timer - Decrements at 60Hz, used for synchronization
- Sound timer - When non-zero, produces beeping sound

### Display
- Resolution: 64 x 32 pixels
- Monochrome: 1 bit per pixel (on/off)
- Sprites: Drawn via XOR operation

### Instruction Decoding

Each CHIP-8 instruction is 2 bytes long. The emulator decodes them using masks:

```c
uint16_t opcode = (memory[pc] << 8) | memory[pc + 1];
uint16_t nnn    = opcode & 0x0FFF;  // Address
uint8_t  kk     = opcode & 0x00FF;  // Byte
uint8_t  x      = (opcode & 0x0F00) >> 8;  // Register X
uint8_t  y      = (opcode & 0x00F0) >> 4;  // Register Y
uint8_t  n      = opcode & 0x000F;  // 4-bit value
```

### Instruction Examples

| Instruction | Description | Code |
|-------------|-------------|------|
| 00E0     | Clear screen | memset(display, 0, sizeof(display)) |
| 1NNN     | Jump to address NNN | pc = nnn |
| 6XKK     | Load constant KK into VX | V[x] = kk |
| 7XKK     | Add KK to VX | V[x] += kk |
| ANNN     | Set I to NNN | I = nnn |
| DXYN     | Draw sprite | XOR sprite at coordinates (VX, VY) |

## 8-bit Processors

### What is an 8-bit Processor?

An 8-bit processor processes data in 8-bit chunks per cycle. Key characteristics:

- Bit width: 8-bit registers and ALU
- Addressable memory: Typically 16-bit addressing (up to 64 KB)
- Clock speed: 1-20 MHz in real processors
- Architecture: Often based on accumulator or register files

### Historical 8-bit Processors

1. Intel 8080 (1974) - used in Altair 8800
2. MOS 6502 (1975) - Commodore 64, Apple II, NES
3. Zilog Z80 (1976) - ZX Spectrum, MSX, Game Boy
4. Motorola 6800 (1974) - various microcomputers

### CHIP-8 as an 8-bit System

While CHIP-8 is a virtual machine, it demonstrates typical characteristics of 8-bit systems:

- 8-bit operations: Most instructions work with 8-bit data
- Limited memory: 4 KB typical for early 8-bit systems
- Simple graphics: Low resolution, limited palette
- Basic operations: Arithmetic, logic, branching

### Emulation vs Simulation

- Emulation (Open8B): Reproducing functionality on different hardware
- Simulation: Modeling the internal state of a processor

Open8B is an emulator - it executes the same code as the original system but on modern hardware.

## Contributing

### Our Goals

1. Stability - reliable operation on all supported platforms
2. Accuracy - cycle-accurate emulation of original CHIP-8
3. Performance - efficient resource usage
4. Extensibility - easy foundation for adding new features

### Contribution Guidelines

1. Fork the repository
2. Create a feature branch (git checkout -b feature/AmazingFeature)
3. Commit your changes (git commit -m 'Add some AmazingFeature')
4. Push to the branch (git push origin feature/AmazingFeature)
5. Open a Pull Request
