CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic
LIBS = -lSDL2
ARCH_DIR = arch
GRAPHICS_DIR = graphics
SOURCES = main.c $(ARCH_DIR)/chip8.c $(GRAPHICS_DIR)/graphics.c
OBJECTS = main.o chip8.o graphics.o
TARGET = Open8B
BINDIR = bin
ROMSDIR = roms

$(TARGET): $(OBJECTS) | $(BINDIR)
	$(CC) $(OBJECTS) -o $(TARGET) $(LIBS)
	mv $(TARGET) $(BINDIR)/

$(BINDIR):
	mkdir -p $(BINDIR)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.o

chip8.o: $(ARCH_DIR)/chip8.c
	$(CC) $(CFLAGS) -c $(ARCH_DIR)/chip8.c -o chip8.o

graphics.o: $(GRAPHICS_DIR)/graphics.c
	$(CC) $(CFLAGS) -c $(GRAPHICS_DIR)/graphics.c -o graphics.o

clean:
	rm -f $(OBJECTS)
	rm -f $(BINDIR)/$(TARGET)

distclean: clean
	rm -rf $(BINDIR)

install-deps:
	# Your Distro

roms:
	mkdir -p $(ROMSDIR)
	@echo "Place your .ch8 ROM files in the $(ROMSDIR) directory"

all: $(TARGET)

.PHONY: clean distclean install-deps roms all