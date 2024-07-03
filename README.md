<p align="center"><img src="https://raw.githubusercontent.com/vitor251093/KHMelonMix/master/res/icon/khDaysMM_128x128.png"></p>
<h2 align="center"><b>Kingdom Hearts - Melon Mix</b></h2>
<p align="center">
<a href="https://www.gnu.org/licenses/gpl-3.0" alt="License: GPLv3"><img src="https://img.shields.io/badge/License-GPL%20v3-%23ff554d.svg"></a>
<a href="https://github.com/vitor251093/KHMelonMix/actions/workflows/build-windows.yml?query=event%3Apush"><img src="https://github.com/vitor251093/KHMelonMix/actions/workflows/build-windows.yml/badge.svg" /></a>
<a href="https://github.com/vitor251093/KHMelonMix/actions/workflows/build-ubuntu.yml?query=event%3Apush"><img src="https://github.com/vitor251093/KHMelonMix/actions/workflows/build-ubuntu.yml/badge.svg" /></a>
<a href="https://github.com/vitor251093/KHMelonMix/actions/workflows/build-macos.yml?query=event%3Apush"><img src="https://github.com/vitor251093/KHMelonMix/actions/workflows/build-macos.yml/badge.svg" /></a>
</p>
melonDS, sorta

This project aims to turn the Kingdom Hearts DS games into playable PC games with a single screen and controls suited for a regular controller.

This is a version of melonDS with some modifications made specifically to improve those specific games experience. Any issues you have with KH Melon Mix should be reported in this same repository, and not in melonDS repository.

Join our [Discord](https://discord.gg/dQZx65QUnE) to keep in touch with the latest updates and/or to help with the project.

> **WARNING:** Kingdom Hearts Re:Coded support is still in development

<hr>

## How to play "Kingdom Hearts 358/2 Days"

1. Download the [latest version](https://github.com/vitor251093/KHMelonMix/releases/latest) for your system (not the source code zip);
2. Open the downloaded ZIP file and copy the contents to an empty folder;
3. Dump an original copy of "Kingdom Hearts 358/2 Days" and copy the NDS file to your computer;
4. Rename that NDS file to "days.nds", place it inside a folder named "roms", and then place that folder next to the Melon Mix executable that you just copied;
5. Now you just need to launch the Melon Mix using "MelonMix_KHDays.bat" (or MelonMix_KHDays.sh if you are not on Windows).

## How to play "Kingdom Hearts Re:Coded" (still a WIP; only for testing purposes)

1. Download the [latest version](https://github.com/vitor251093/KHMelonMix/releases/latest) for your system (not the source code zip);
2. Open the downloaded ZIP file and copy the contents to an empty folder;
3. Dump an original copy of "Kingdom Hearts Re:Coded" and copy the NDS file to your computer;
4. Rename that NDS file to "recoded.nds", place it inside a folder named "roms", and then place that folder next to the Melon Mix executable that you just copied;
5. Now you just need to launch the Melon Mix using "MelonMix_KHReCoded.bat" (or MelonMix_KHReCoded.sh if you are not on Windows).

### Recommended Controller Binds
* Map the DS D-Pad to your controller's left analog stick
* Map the DS Touch Screen to your controller's right analog stick
* Map the command menu to your controller's D-Pad

### Steam Deck
The AppImage build is compatible with the Steam Deck. In order to use it, download the AppImage build, extract the zip, right click the AppImage and go to "Properties". Then go to the "Permissions" tab and check the "Allow executing file as program" checkbox. Don't forget the NDS file, like mentioned above.

<p align="center"><img src="https://raw.githubusercontent.com/vitor251093/KHMelonMix/master/screenshot.png"></p>
<p align="center"><i>Kingdom Hearts 358/2 Days - Melon Mix v0.2.2</i></p>

## How to build from scratch

### Linux
1. Install dependencies:
   * Ubuntu 22.04: `sudo apt install cmake extra-cmake-modules libcurl4-gnutls-dev libpcap0.8-dev libsdl2-dev qtbase5-dev qtbase5-private-dev qtmultimedia5-dev libarchive-dev libzstd-dev`
   * Older Ubuntu: `sudo apt install cmake extra-cmake-modules libcurl4-gnutls-dev libpcap0.8-dev libsdl2-dev qt5-default qtbase5-private-dev qtmultimedia5-dev libarchive-dev libzstd-dev`
   * Arch Linux: `sudo pacman -S base-devel cmake extra-cmake-modules git libpcap sdl2 qt5-base qt5-multimedia libarchive zstd`
3. Download the KH Melon Mix repository and prepare:
   ```bash
   git clone https://github.com/vitor251093/KHMelonMix
   cd KHMelonMix
   ```

3. Compile:
   ```bash
   cmake -B build
   cmake --build build -j$(nproc --all)
   ```

### Windows
1. Install [MSYS2](https://www.msys2.org/)
2. Open the **MSYS2 MinGW 64-bit** terminal
3. Update the packages using `pacman -Syu` and reopen the terminal if it asks you to
4. Install git to clone the repository
   ```bash
   pacman -S git
   ```
5. Download the KH Melon Mix repository and prepare:
   ```bash
   git clone https://github.com/vitor251093/KHMelonMix
   cd KHMelonMix
   ```
#### Dynamic builds (with DLLs)
5. Install dependencies: `pacman -S mingw-w64-x86_64-{cmake,SDL2,toolchain,qt5-base,qt5-svg,qt5-multimedia,qt5-tools,libarchive,zstd}`
6. Compile:
   ```bash
   cmake -B build
   cmake --build build
   cd build
   ../tools/msys-dist.sh
   ```
If everything went well, KH Melon Mix and the libraries it needs should now be in the `dist` folder.

#### Static builds (without DLLs, standalone executable)
5. Install dependencies: `pacman -S mingw-w64-x86_64-{cmake,SDL2,toolchain,qt5-static,libarchive,zstd}`
6. Compile:
   ```bash
   cmake -B build -DBUILD_STATIC=ON -DCMAKE_PREFIX_PATH=/mingw64/qt5-static
   cmake --build build
   ```
If everything went well, KH Melon Mix should now be in the `build` folder.

### macOS
1. Install the [Homebrew Package Manager](https://brew.sh)
2. Install dependencies: `brew install git pkg-config cmake sdl2 qt@6 libarchive zstd`
3. Download the KH Melon Mix repository and prepare:
   ```zsh
   git clone https://github.com/vitor251093/KHMelonMix
   cd KHMelonMix
   ```
4. Compile:
   ```zsh
   cmake -B build -DCMAKE_PREFIX_PATH="$(brew --prefix qt@6);$(brew --prefix libarchive)" -DUSE_QT6=ON
   cmake --build build -j$(sysctl -n hw.logicalcpu)
   ```
If everything went well, MelonMix.app should now be in the `build` directory.

#### Self-contained app bundle
If you want an app bundle that can be distributed to other computers without needing to install dependencies through Homebrew, you can additionally run `
../tools/mac-bundle.rb MelonMix.app` after the build is completed, or add `-DMACOS_BUNDLE_LIBS=ON` to the first CMake command.

## Credits

 * All people that supported and developed melonDS
 * sandwichwater and DaniKH, for the innumerous amount of tests and for the hi-res textures
 * Michael Lipinski, for the [documentation](https://pdfs.semanticscholar.org/657d/adf4888f6302701095055b0d7a066e42b36f.pdf) regarding the way a NDS works

## Licenses

[![GNU GPLv3 Image](https://www.gnu.org/graphics/gplv3-127x51.png)](http://www.gnu.org/licenses/gpl-3.0.en.html)

KH Melon Mix is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

### External
* Images used in the Input Config Dialog - see `src/frontend/qt_sdl/InputConfig/resources/LICENSE.md`
