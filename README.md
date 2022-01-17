# Use the Gameboy Printer to create Banners

![Screenshot](https://github.com/reini1305/gb_bannerprinter/raw/main/image/screenshot.png)
![Setup1](https://github.com/reini1305/gb_bannerprinter/raw/main/image/setup2.jpg)
![Setup2](https://github.com/reini1305/gb_bannerprinter/raw/main/image/setup.jpg)
# Usage
You can load the ROM onto a flash cart or in an emulator. The key mapping is as follows:
- D-Pad to choose a character from the map
- A selects the current character
- B deletes the last character
- SELECT sets the color scheme (black on white or white on black) and rotation
- START prints the current string

If your printer is properly connected, the printer icon will show a checkmark symbol.

# Compilation
The ROM can be created using [GBDK 2020](https://github.com/gbdk-2020/gbdk-2020). The repository contains a Makefile for Linux and Windows with WSL that will work if you clone it underneath the local gbdk folder.