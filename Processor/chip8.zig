const std = @import("std");

const fontset = [80]u8{
    0xF0, 0x90, 0x90, 0x90, 0xF0, 0x20, 0x60, 0x20, 0x20, 0x70,
    0xF0, 0x10, 0xF0, 0x80, 0xF0, 0xF0, 0x10, 0xF0, 0x10, 0xF0,
    0x90, 0x90, 0xF0, 0x10, 0x10, 0xF0, 0x80, 0xF0, 0x10, 0xF0,
    0xF0, 0x80, 0xF0, 0x90, 0xF0, 0xF0, 0x10, 0x20, 0x40, 0x40,
    0xF0, 0x90, 0xF0, 0x90, 0xF0, 0xF0, 0x90, 0xF0, 0x10, 0xF0,
    0xF0, 0x90, 0xF0, 0x90, 0x90, 0xE0, 0x90, 0xE0, 0x90, 0xE0,
    0xF0, 0x80, 0x80, 0x80, 0xF0, 0xE0, 0x90, 0x90, 0x90, 0xE0,
    0xF0, 0x80, 0xF0, 0x80, 0xF0, 0xF0, 0x80, 0xF0, 0x80, 0x80,
};

pub const Chip8 = struct {
    memory: [4096]u8,
    v: [16]u8,
    i: u16,
    pc: u16,
    stack: [16]u16,
    sp: u8,
    delay_timer: u8,
    sound_timer: u8,
    display: [64 * 32]u8,
    key: [16]u8,
    draw_flag: bool,

    pub fn init() Chip8 {
        var c8 = std.mem.zeroes(Chip8);
        c8.pc = 0x200;
        @memcpy(c8.memory[0..fontset.len], &fontset);
        return c8;
    }

    pub fn loadRom(self: *Chip8, io: std.Io, gpa: std.mem.Allocator, path: []const u8) !void {
        const data = std.Io.Dir.cwd().readFileAlloc(io, path, gpa, .limited(1048576)) catch |err| switch (err) {
            error.StreamTooLong => {
                return err;
            },
            else => return err,
        };
        defer gpa.free(data);

        const max_chip8_memory = 4096 - 0x200;
        var bytes_to_load = data.len;
        if (bytes_to_load > max_chip8_memory) {
            bytes_to_load = max_chip8_memory;
        }

        @memcpy(self.memory[0x200..][0..bytes_to_load], data[0..bytes_to_load]);
    }

    pub fn tickTimers(self: *Chip8) void {
        if (self.delay_timer > 0) self.delay_timer -= 1;
        if (self.sound_timer > 0) self.sound_timer -= 1;
    }

    pub fn emulateCycle(self: *Chip8, io: std.Io) void {
        const opcode: u16 = (@as(u16, self.memory[self.pc]) << 8) | self.memory[self.pc + 1];
        self.pc +%= 2;

        const nnn: u16 = opcode & 0x0FFF;
        const n: u8 = @truncate(opcode & 0x000F);
        const x: u8 = @truncate((opcode & 0x0F00) >> 8);
        const y: u8 = @truncate((opcode & 0x00F0) >> 4);
        const kk: u8 = @truncate(opcode & 0x00FF);

        switch (opcode & 0xF000) {
            0x0000 => switch (opcode) {
                0x00E0 => {
                    @memset(&self.display, 0);
                    self.draw_flag = true;
                },
                0x00EE => {
                    self.sp -= 1;
                    self.pc = self.stack[self.sp];
                },
                else => {},
            },
            0x1000 => self.pc = nnn,
            0x2000 => {
                self.stack[self.sp] = self.pc;
                self.sp += 1;
                self.pc = nnn;
            },
            0x3000 => if (self.v[x] == kk) {
                self.pc +%= 2;
            },
            0x4000 => if (self.v[x] != kk) {
                self.pc +%= 2;
            },
            0x5000 => if (self.v[x] == self.v[y]) {
                self.pc +%= 2;
            },
            0x6000 => self.v[x] = kk,
            0x7000 => self.v[x] +%= kk,
            0x8000 => switch (n) {
                0x0 => self.v[x] = self.v[y],
                0x1 => self.v[x] |= self.v[y],
                0x2 => self.v[x] &= self.v[y],
                0x3 => self.v[x] ^= self.v[y],
                0x4 => {
                    const sum: u16 = @as(u16, self.v[x]) + @as(u16, self.v[y]);
                    self.v[0xF] = if (sum > 255) 1 else 0;
                    self.v[x] = @truncate(sum);
                },
                0x5 => {
                    self.v[0xF] = if (self.v[x] > self.v[y]) 1 else 0;
                    self.v[x] -%= self.v[y];
                },
                0x6 => {
                    self.v[0xF] = self.v[x] & 0x1;
                    self.v[x] >>= 1;
                },
                0x7 => {
                    self.v[0xF] = if (self.v[y] > self.v[x]) 1 else 0;
                    self.v[x] = self.v[y] -% self.v[x];
                },
                0xE => {
                    self.v[0xF] = (self.v[x] & 0x80) >> 7;
                    self.v[x] <<= 1;
                },
                else => {},
            },
            0x9000 => if (self.v[x] != self.v[y]) {
                self.pc +%= 2;
            },
            0xA000 => self.i = nnn,
            0xB000 => self.pc = nnn +% self.v[0],
            0xC000 => {
                var buf: [1]u8 = undefined;
                std.Io.random(io, &buf);
                self.v[x] = buf[0] & kk;
            },
            0xD000 => {
                const x_pos: usize = self.v[x] % 64;
                const y_pos: usize = self.v[y] % 32;
                self.v[0xF] = 0;
                var row: usize = 0;
                while (row < n) : (row += 1) {
                    const sprite_byte = self.memory[@as(usize, self.i) + row];
                    var col: usize = 0;
                    while (col < 8) : (col += 1) {
                        const mask: u8 = @as(u8, 0x80) >> @as(u3, @intCast(col));
                        if ((sprite_byte & mask) != 0) {
                            const idx = (y_pos + row) * 64 + (x_pos + col);
                            if (idx < self.display.len) {
                                if (self.display[idx] == 1) self.v[0xF] = 1;
                                self.display[idx] ^= 1;
                            }
                        }
                    }
                }
                self.draw_flag = true;
            },
            0xE000 => switch (kk) {
                0x9E => if (self.key[self.v[x]] != 0) {
                    self.pc +%= 2;
                },
                0xA1 => if (self.key[self.v[x]] == 0) {
                    self.pc +%= 2;
                },
                else => {},
            },
            0xF000 => switch (kk) {
                0x07 => self.v[x] = self.delay_timer,
                0x0A => {
                    var key_pressed = false;
                    for (self.key, 0..) |k, idx| {
                        if (k != 0) {
                            self.v[x] = @intCast(idx);
                            key_pressed = true;
                            break;
                        }
                    }
                    if (!key_pressed) self.pc -%= 2;
                },
                0x15 => self.delay_timer = self.v[x],
                0x18 => self.sound_timer = self.v[x],
                0x1E => self.i +%= self.v[x],
                0x29 => self.i = @as(u16, self.v[x]) * 5,
                0x33 => {
                    self.memory[self.i] = self.v[x] / 100;
                    self.memory[self.i + 1] = (self.v[x] / 10) % 10;
                    self.memory[self.i + 2] = self.v[x] % 10;
                },
                0x55 => {
                    var idx: u8 = 0;
                    while (idx <= x) : (idx += 1) self.memory[self.i + idx] = self.v[idx];
                },
                0x65 => {
                    var idx: u8 = 0;
                    while (idx <= x) : (idx += 1) self.v[idx] = self.memory[self.i + idx];
                },
                else => {},
            },
            else => {},
        }
    }
};
