const std = @import("std");
const sdl2 = @import("Render/sdl2.zig");
const Chip8Mod = @import("Processor/chip8.zig");
const GraphicsMod = @import("Render/graphics.zig");

const Chip8 = Chip8Mod.Chip8;
const Graphics = GraphicsMod.Graphics;

pub fn main(init: std.process.Init) !void {
    const gpa = init.gpa;
    const io = init.io;

    const args = try init.minimal.args.toSlice(init.arena.allocator());

    var rom_path: ?[]u8 = null;
    if (args.len == 2) {
        rom_path = try gpa.dupe(u8, args[1]);
    }
    defer if (rom_path) |p| gpa.free(p);

    var gfx = try Graphics.init();
    defer gfx.deinit();

    if (rom_path == null) {
        GraphicsMod.showInfoBox(
            "Z.8C",
            "Run roms from terminal: ./Z.8C roms.ch8 or drop",
        );
    }

    var running = true;
    var last_timer_ms: u32 = sdl2.SDL_GetTicks();

    while (running) {
        if (rom_path) |path| {
            var chip8 = Chip8.init();
            chip8.loadRom(io, gpa, path) catch {
                gpa.free(path);
                rom_path = null;
                continue;
            };

            var emulation_running = true;
            while (emulation_running and running) {
                if (try gfx.handleInput(gpa, &chip8, &running)) |new_path| {
                    gpa.free(path);
                    rom_path = new_path;
                    emulation_running = false;
                    continue;
                }

                var cycle: u8 = 0;
                while (cycle < 8) : (cycle += 1) {
                    chip8.emulateCycle(io);
                }

                const now = sdl2.SDL_GetTicks();
                if (now -% last_timer_ms > 16) {
                    chip8.tickTimers();
                    last_timer_ms = now;
                }

                if (chip8.draw_flag) gfx.render(&chip8);

                sdl2.SDL_Delay(2);
            }
        } else {
            if (try gfx.handleInput(gpa, null, &running)) |new_path| {
                rom_path = new_path;
            }
            sdl2.SDL_Delay(16);
        }
    }
}
