const std = @import("std");
const sdl2 = @import("sdl2.zig");
const chip8 = @import("../Processor/chip8.zig");

pub const scale: c_int = 10;
pub const screen_width: c_int = @as(c_int, @intCast(64)) * scale;
pub const screen_height: c_int = @as(c_int, @intCast(32)) * scale;

const keymap = [_]struct { sym: i32, key: u8 }{
    .{ .sym = '1', .key = 0x1 },
    .{ .sym = '2', .key = 0x2 },
    .{ .sym = '3', .key = 0x3 },
    .{ .sym = '4', .key = 0xC },
    .{ .sym = 'q', .key = 0x4 },
    .{ .sym = 'w', .key = 0x5 },
    .{ .sym = 'e', .key = 0x6 },
    .{ .sym = 'r', .key = 0xD },
    .{ .sym = 'a', .key = 0x7 },
    .{ .sym = 's', .key = 0x8 },
    .{ .sym = 'd', .key = 0x9 },
    .{ .sym = 'f', .key = 0xE },
    .{ .sym = 'z', .key = 0xA },
    .{ .sym = 'x', .key = 0x0 },
    .{ .sym = 'c', .key = 0xB },
    .{ .sym = 'c', .key = 0xF },
};

pub const InitError = error{
    SdlInitFailed,
    WindowCreationFailed,
    RendererCreationFailed,
};

pub const Graphics = struct {
    window: *sdl2.Window,
    renderer: *sdl2.Renderer,

    pub fn init() InitError!Graphics {
        if (sdl2.SDL_Init(0x00000020) != 0) {
            return InitError.SdlInitFailed;
        }
        errdefer sdl2.SDL_Quit();

        const window = sdl2.SDL_CreateWindow(
            "Z.8C",
            0x2FFF0000,
            0x2FFF0000,
            screen_width,
            screen_height,
            0x00002000,
        ) orelse {
            return InitError.WindowCreationFailed;
        };
        errdefer sdl2.SDL_DestroyWindow(window);

        const renderer = sdl2.SDL_CreateRenderer(
            window,
            -1,
            0x00000002,
        ) orelse {
            return InitError.RendererCreationFailed;
        };

        return Graphics{ .window = window, .renderer = renderer };
    }

    pub fn deinit(self: *Graphics) void {
        sdl2.SDL_DestroyRenderer(self.renderer);
        sdl2.SDL_DestroyWindow(self.window);
        sdl2.SDL_Quit();
    }

    pub fn render(self: *Graphics, chip: *chip8.Chip8) void {
        _ = sdl2.SDL_SetRenderDrawColor(self.renderer, 0, 0, 0, 255);
        _ = sdl2.SDL_RenderClear(self.renderer);
        _ = sdl2.SDL_SetRenderDrawColor(self.renderer, 255, 255, 255, 255);

        for (0..32) |y| {
            for (0..64) |x| {
                if (chip.display[y * 64 + x] != 0) {
                    var pixel = sdl2.Rect{
                        .x = @as(c_int, @intCast(x)) * scale,
                        .y = @as(c_int, @intCast(y)) * scale,
                        .w = scale,
                        .h = scale,
                    };
                    _ = sdl2.SDL_RenderFillRect(self.renderer, &pixel);
                }
            }
        }

        sdl2.SDL_RenderPresent(self.renderer);
        chip.draw_flag = false;
    }

    pub fn handleInput(
        self: *Graphics,
        gpa: std.mem.Allocator,
        chip: ?*chip8.Chip8,
        running: *bool,
    ) !?[]u8 {
        _ = self;
        var dropped_path: ?[]u8 = null;

        var event: sdl2.Event = undefined;
        while (sdl2.SDL_PollEvent(&event) != 0) {
            switch (event.type) {
                0x100 => running.* = false,

                0x1000 => {
                    const file_ptr = event.asDrop().getFile();
                    defer sdl2.SDL_free(file_ptr);
                    dropped_path = try gpa.dupe(u8, std.mem.span(file_ptr));
                    _ = sdl2.SDL_ShowSimpleMessageBox(
                        0x00000040,
                        "File Dropped!",
                        "ROM loaded!",
                        null,
                    );
                },

                0x300, 0x301 => {
                    if (chip) |ch| {
                        const key_state: u8 = if (event.type == 0x300) 1 else 0;
                        const sym = event.asKeyboard().keysym.sym;
                        if (sym == 27) {
                            running.* = false;
                        } else {
                            for (keymap) |entry| {
                                if (entry.sym == sym) {
                                    ch.key[entry.key] = key_state;
                                    break;
                                }
                            }
                        }
                    }
                },

                else => {},
            }
        }

        return dropped_path;
    }
};

pub fn showInfoBox(title: [:0]const u8, message: [:0]const u8) void {
    _ = sdl2.SDL_ShowSimpleMessageBox(0x00000040, title, message, null);
}
