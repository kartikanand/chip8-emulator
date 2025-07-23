# CHIP-8 Emulator

A C++17 CHIP-8 emulator built with SFML for graphics and input handling. This emulator implements the classic CHIP-8 instruction set with a 64×32 monochrome display.

## Features

- Full CHIP-8 instruction set implementation
- 64×32 pixel display with XOR-based sprite drawing
- Hexadecimal keypad input (0-F)
- Built-in font support
- Sound and delay timers
- Cross-platform graphics via SFML

## Requirements

- C++17 compatible compiler (clang++)
- SFML 2.x
- macOS with Homebrew (tested configuration)

## Setup

1. Install SFML via Homebrew:
```bash
brew install sfml@2
```

2. Build the emulator:
```bash
make
```

## Usage

Run a CHIP-8 ROM file:
```bash
./chip8 <rom_file.ch8>
```

## Technical Details

- **Memory**: 4KB RAM with font data at 0x000-0x04F, programs loaded at 0x200
- **Display**: 64×32 monochrome, scaled 20x for visibility (1280×640 window)
- **Timers**: 60Hz delay and sound timers
- **Architecture**: Fetch-decode-execute cycle with 16-bit instructions
