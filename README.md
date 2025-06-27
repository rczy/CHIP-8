# Yet Another CHIP-8 Emulator

This is the source code for my BSc thesis project, written mostly in C.

The [SDL2](https://www.libsdl.org/) library is used for the I/O modules (input, display, beeper, device) and timing.

More about CHIP-8: [Wikipedia](https://en.wikipedia.org/wiki/CHIP-8).

## Live Demo
Visit [https://rczy.github.io/chip-8/](https://rczy.github.io/chip-8/) for the live WebAssembly version.

## Build
Use the following Makefile targets to build this project in various forms:
- **linux:** GCC (13.3.0)
- **windows:** mingw-w64 GCC (13-win32)
- **web:** Wasm build with Emscripten (3.1.70)
- **all:** builds all of the above

Use the `msvc.ps1` PowerShell script to create a Windows debug build with the MSVC compiler.
Make sure to place the correct SDL2 dependencies in the `lib` directory (these can be overwritten).

## Run
To run the emulator, invoke it in the following form:
```
./chip-8 <ROM file> [options]
```

### Options
- `--ipf`: number of instructions per frame (IPF) [default: 9]
- `--tone`: frequency of the beeper's sound [default: 440]
- `--bg-color`: color of the background in hexadecimal RGB format [default: 000000]
- `--fg-color`: color of the pixels in the aforementioned form [default: 00FF00]

### Control Keys
- **Esc:** exits the emulator
- **Backspace:** restarts the emulator
- **Numpad -:** decreases IPF
- **Numpad +:** increases IPF

### Keyboard Mapping
| Emulator | COSMAC VIP |
| :------: | :--------: |
| 1 2 3 4  | 1 2 3 C    |
| Q W E R  | 4 5 6 D    |
| A S D F  | 7 8 9 E    |
| Y X C V  | A 0 B F    |

## Test
This project uses the [Acutest](https://github.com/mity/acutest) (Another C/C++ Unit Test facility) library for testing.

Use the following Makefile targets to run the tests:
- **test-all:** runs all tests
- **test-<file>:** builds and runs a specific test from the 'test' folder

## Attached ROMs
- animal-race (Brian Astle, 1977)
- blitz (David Winter)
- bowling (Gooitzen van der Wal, 1977)
- ibm
- kaleidoscope (Joseph Weisbecker, 1978)
- lunar-lander (Udo Pernisz, 1979)
- merlin (David Winter)
- outlaw (John Earnest, 2014)
- slipperyslope (John Earnest, 2018)

Test suite ROMs by [Timendus](https://github.com/Timendus/chip8-test-suite):
- beep
- chip8-logo
- corax+
- flags
- keypad
- quirks

## Useful Resources
- Tobias V. Langhoff: [Guide to making a CHIP-8 emulator](https://tobiasvl.github.io/blog/write-a-chip-8-emulator/)
- [Cowgod's Chip-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#1.0)
- Laurence Scotford: [Chip-8 on the COSMAC VIP](https://www.laurencescotford.net/2020/07/25/chip-8-on-the-cosmac-vip-index/)
- [VIPER Volume 1 Issue 1](https://github.com/chip-8/chip-8_documentation/blob/master/VIPER/VIPER_-_Volume_1_-_Issue_01.pdf)
- [CHIP-8 Instruction Set](https://johnearnest.github.io/Octo/docs/chip8ref.pdf)
- [CHIP-8 Variant Opcode Table](https://chip8.gulrak.net/)
- [CHIP-8 extensions and compatibility](https://chip-8.github.io/extensions/)

This project was really fun to achieve! :mortar_board:
