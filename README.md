<p align="center"><img src="https://raw.githubusercontent.com/vitor251093/KHDays_FM/master/res/icon/khDaysMM_128x128.png"></p>
<h2 align="center"><b>Kingdom Hearts 358/2 Days - Melon Mix</b></h2>
melonDS, sorta

The goal of this project is to turn "Kingdom Hearts 358/2 Days" into a playable PC game, with a single screen and controls compatible with a regular controller.

This is basically melonDS, but with some modifications made specifically to improve this specific game experience. Any issues you have with KHDays Melon Mix should be reported in this same repository, and not in melonDS repository.

Join our [Discord](https://discord.gg/dQZx65QUnE) to keep in touch with the latest updates and/or to help with the project.
<hr>

## How to play

First, download the [latest version](https://github.com/vitor251093/KHDays_FM/releases/latest) for your system (not the source code zip). Then, you will need an original copy of "Kingdom Hearts 358/2 Days" in order to dump it into a NDS file. Place that NDS file inside a "rom" folder, next to the downloaded Melon Mix executable (which was inside the zip file that you downloaded), and name it game.nds. Now you just need to launch the Melon Mix executable.

### Steam Deck

The AppImage build is compatible with the Steam Deck. In order to use it, download the AppImage build, extract the zip, right click the AppImage and go to "Properties". Then go to the "Permissions" tab and check the "Allow executing file as program" checkbox. Don't forget the NDS file, like mentioned above.

<p align="center"><img src="https://raw.githubusercontent.com/vitor251093/KHDays_FM/master/screenshot.png"></p>
<p align="center"><i>Kingdom Hearts 358/2 Days - Melon Mix v0.2.2 (upcoming)</i></p>

## How to build from scratch

### Linux
1. Install dependencies:
   * Ubuntu 22.04: `sudo apt install cmake extra-cmake-modules libcurl4-gnutls-dev libpcap0.8-dev libsdl2-dev qtbase5-dev qtbase5-private-dev qtmultimedia5-dev libarchive-dev libzstd-dev`
   * Older Ubuntu: `sudo apt install cmake extra-cmake-modules libcurl4-gnutls-dev libpcap0.8-dev libsdl2-dev qt5-default qtbase5-private-dev qtmultimedia5-dev libarchive-dev libzstd-dev`
   * Arch Linux: `sudo pacman -S base-devel cmake extra-cmake-modules git libpcap sdl2 qt5-base qt5-multimedia libarchive zstd`
3. Download the KHDays MM repository and prepare:
   ```bash
   git clone https://github.com/vitor251093/KHDays_FM
   cd KHDays_FM
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
5. Download the KHDays MM repository and prepare:
   ```bash
   git clone https://github.com/vitor251093/KHDays_FM
   cd KHDays_FM
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
If everything went well, KHDays MM and the libraries it needs should now be in the `dist` folder.

#### Static builds (without DLLs, standalone executable)
5. Install dependencies: `pacman -S mingw-w64-x86_64-{cmake,SDL2,toolchain,qt5-static,libarchive,zstd}`
6. Compile:
   ```bash
   cmake -B build -DBUILD_STATIC=ON -DCMAKE_PREFIX_PATH=/mingw64/qt5-static
   cmake --build build
   ```
If everything went well, khDaysMM should now be in the `build` folder.

### macOS
1. Install the [Homebrew Package Manager](https://brew.sh)
2. Install dependencies: `brew install git pkg-config cmake sdl2 qt@6 libarchive zstd`
3. Download the KHDays MM repository and prepare:
   ```zsh
   git clone https://github.com/vitor251093/KHDays_FM
   cd KHDays_FM
   ```
4. Compile:
   ```zsh
   cmake -B build -DCMAKE_PREFIX_PATH="$(brew --prefix qt@6);$(brew --prefix libarchive)" -DUSE_QT6=ON
   cmake --build build -j$(sysctl -n hw.logicalcpu)
   ```
If everything went well, khDaysMM.app should now be in the `build` directory.

#### Self-contained app bundle
If you want an app bundle that can be distributed to other computers without needing to install dependencies through Homebrew, you can additionally run `
../tools/mac-bundle.rb khDaysMM.app` after the build is completed, or add `-DMACOS_BUNDLE_LIBS=ON` to the first CMake command.

## Credits

 * All people that supported and developed melonDS
 * sandwichwater, for the innumerous amount of tests and for the hi-res textures
 * Michael Lipinski, for the [documentation](https://pdfs.semanticscholar.org/657d/adf4888f6302701095055b0d7a066e42b36f.pdf) regarding the way a NDS works

## Licenses

[![GNU GPLv3 Image](https://www.gnu.org/graphics/gplv3-127x51.png)](http://www.gnu.org/licenses/gpl-3.0.en.html)

KHDays MM is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

### External
* Images used in the Input Config Dialog - see `src/frontend/qt_sdl/InputConfig/resources/LICENSE.md`
