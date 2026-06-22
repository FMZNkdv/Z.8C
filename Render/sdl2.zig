pub const Rect = extern struct { x: c_int, y: c_int, w: c_int, h: c_int };
pub const Window = opaque {};
pub const Renderer = opaque {};

pub extern fn SDL_Init(flags: u32) c_int;
pub extern fn SDL_Quit() void;
pub extern fn SDL_GetError() [*c]const u8;
pub extern fn SDL_DestroyRenderer(renderer: *Renderer) void;
pub extern fn SDL_SetRenderDrawColor(renderer: *Renderer, r: u8, g: u8, b: u8, a: u8) c_int;
pub extern fn SDL_RenderClear(renderer: *Renderer) c_int;
pub extern fn SDL_RenderFillRect(renderer: *Renderer, rect: *const Rect) c_int;
pub extern fn SDL_RenderPresent(renderer: *Renderer) void;
pub extern fn SDL_free(mem: ?*anyopaque) void;
pub extern fn SDL_GetTicks() u32;
pub extern fn SDL_Delay(ms: u32) void;
pub extern fn SDL_PollEvent(event: *Event) c_int;
pub extern fn SDL_DestroyWindow(window: *Window) void;

pub const Keysym = extern struct {
    scancode: i32,
    sym: i32,
    mod: u16,
    _pad: u16,
    unused: u32,
};

pub const KeyboardEvent = extern struct {
    type: u32,
    timestamp: u32,
    windowID: u32,
    state: u8,
    repeat: u8,
    _pad: [2]u8,
    keysym: Keysym,
};

pub const DropEvent = extern struct {
    type: u32,
    timestamp: u32,
    _file_lo: u32,
    _file_hi: u32,
    windowID: u32,
    _pad: u32,

    pub fn getFile(self: *const DropEvent) [*c]u8 {
        const lo: u64 = self._file_lo;
        const hi: u64 = self._file_hi;
        const addr: u64 = lo | (hi << 32);
        return @ptrFromInt(addr);
    }
};

pub const Event = extern struct {
    type: u32,
    _rest: [52]u8,

    pub inline fn asKeyboard(self: *const Event) *const KeyboardEvent {
        return @ptrCast(self);
    }

    pub inline fn asDrop(self: *const Event) *const DropEvent {
        return @ptrCast(self);
    }
};

pub extern fn SDL_CreateWindow(
    title: [*c]const u8,
    x: c_int,
    y: c_int,
    w: c_int,
    h: c_int,
    flags: u32,
) ?*Window;

pub extern fn SDL_CreateRenderer(
    window: *Window,
    index: c_int,
    flags: u32,
) ?*Renderer;

pub extern fn SDL_ShowSimpleMessageBox(
    flags: u32,
    title: [*c]const u8,
    message: [*c]const u8,
    window: ?*Window,
) c_int;
