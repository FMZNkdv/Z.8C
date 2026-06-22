![Logo](Logo.png)
This is the 8chip emulator, written in Zig.

## Requirements

- Zig 0.17.0 (dev.947+36069a2a7)
- Linux (tested only on linux x86_64)
- Brain..? 🧑🏿‍🦯

## Clone repo

```bash
git clone https://github.com/FMZNkdv/Z.8C.git
cd Z.8C
```
## Building

```bash
zig build -Doptimize=ReleaseFast
```

## Running

```bash
.zig-out/bin/Z.8C ROMs/tank.ch8
```