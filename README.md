# klive

Spectrum emulator/IDO, Work in progress (10/2020)

The emulator boot until the prompt is displayed
(c) 1982 Sinclair Reseach Ltd

The keyboard is now working.
Almost all is working, but the zx still crashes.

Still a few work to do before the emulator is 100% ok
------

Attempt to create a modern and powerful for
the old and excellent ZX Spectrum IDE
But the spectrum is booting.

![Alt text](doc/ide-state.png?raw=true "Ide Status")

Planned features : planned, * in progress, > done)

* Modern all in one IDE
- All in one tool (except the C compiler)
- Z80 Asm included with syntax colored editor
- C compiler (external)
- Resource editor (Sprites)
- Real time compilation / execution of project
> Debugger
> ROM RAM viewer
> ROM/RAM Disassembler Routines labels (custom)
* ZX 48 Emulator (mine, so it won't be so accurate but should play games)
  -> I hope with enhanced tools like stack overflow detection etc...
* Zx Screen emulation (missing borders)
* Some useless so essential gadgets (clock, lines -> for UI Architecture tests)

And if all is working as expected, I'll enhance the project
with common features
- Plugins
- Tap / Tzx loader
- 128K
- Printer emulation
- ZX Interface 1 emulation
- Internet direct games downloads with search tool

# Dependencies

Kubuntu
use apt-file search if needed

sudo apt install clangd
sudo apt install libqt6core5compat6-dev
sudo apt install libqt6core6
sudo apt install libqt6core7
sudo apt install libqt6gui6
sudo apt install qmake6
sudo apt install qt6-base-dev
sudo apt install qt6-base-private-dev
sudo apt install qt6-connectivity-dev
sudo apt install qt6-declarative-dev-tools
sudo apt install qt6-l10n-tools
sudo apt install qt6-positioning-dev
sudo apt install qt6-tools-dev
sudo apt install qt6-tools-dev-tools
sudo apt-get install apt-file
