# Building KH Melon Mix

* [Linux](#linux)
* [Windows](#windows)
* [macOS](#macos)

## Linux
1. Install dependencies:
   * Ubuntu:
     * All versions: `sudo apt install cmake extra-cmake-modules libcurl4-gnutls-dev libpcap0.8-dev libsdl2-dev libarchive-dev libenet-dev libzstd-dev libfaad-dev libflac-dev liblua5.4-dev`
     * 24.04: `sudo apt install qt6-{base,base-private,multimedia,svg}-dev`
     * 22.04: `sudo apt install qtbase6-dev qtbase6-private-dev qtmultimedia6-dev libqt6svg6-dev`
     * Older versions: `sudo apt install qtbase5-dev qtbase5-private-dev qtmultimedia5-dev libqt5svg5-dev`  
       Also add `-DUSE_QT6=OFF` to the first CMake command below.
   * Fedora: `sudo dnf install gcc-c++ cmake extra-cmake-modules SDL2-devel libarchive-devel enet-devel libzstd-devel faad2-devel qt6-{qtbase,qtbase-private,qtmultimedia,qtsvg}-devel wayland-devel flac-devel`
   * Arch Linux: `sudo pacman -S base-devel cmake extra-cmake-modules git libpcap sdl2 qt6-{base,multimedia,svg} libarchive enet zstd faad2 flac`
2. Download the KH Melon Mix repository and prepare:
   ```bash
   git clone https://github.com/vitor251093/KHMelonMix
   cd KHMelonMix
   ```
3. Compile:
   ```bash
   cmake -B build
   cmake --build build -j$(nproc --all)
   ```

## Windows
1. Install [MSYS2](https://www.msys2.org/)
2. Open the MSYS2 terminal from the Start menu:
   * For x64 systems (most common), use **MSYS2 UCRT64**
   * For ARM64 systems, use **MSYS2 CLANGARM64**
3. Update the packages using `pacman -Syu` and reopen the same terminal if it asks you to
4. Install git and clone the repository
   ```bash
   pacman -S git
   git clone https://github.com/vitor251093/KHMelonMix
   cd KHMelonMix
   ```
5. Install dependencies:  
   Replace `<prefix>` below with `mingw-w64-ucrt-x86_64` on x64 systems, or `mingw-w64-clang-aarch64` on ARM64 systems.
   ```bash
   pacman -S <prefix>-{toolchain,cmake,ninja,SDL2,libarchive,enet,zstd,faad2,flac,lua}
   ```
6. Install Qt 6:
   The frontend requires Qt 6 (it uses the Qt 6 multimedia API); Qt 5 is no longer supported.
   ```bash
   pacman -S <prefix>-{qt6-base,qt6-svg,qt6-multimedia,qt6-tools}
   ```
7. Configure and compile:
   ```bash
   cmake -B build
   cmake --build build
   ```

If everything went well, `melonDS.exe` should now be in the `build` folder. This is a dynamically-linked build, so run it from the MSYS2 UCRT64 terminal (or copy the required DLLs next to the executable) so it can find the Qt/SDL DLLs.

### Standalone (static) builds
The distributable Windows builds produced by CI are statically linked so they run without any MSYS2 DLLs. These builds use vcpkg to compile the dependencies from source through a CMake preset - the same path as [`.github/workflows/build-windows.yml`](.github/workflows/build-windows.yml).
```bash
pacman -S <prefix>-{toolchain,cmake,ninja,git}
cmake --preset=release-mingw-x86_64
cmake --build --preset=release-mingw-x86_64
```
The first configure is slow because vcpkg builds every dependency, including Qt 6, from source. The resulting standalone `melonDS.exe` is placed in `build/release-mingw-x86_64`.

## macOS
1. Install the [Homebrew Package Manager](https://brew.sh)
2. Install dependencies: `brew install git pkg-config cmake sdl2 qt@6 libarchive enet zstd faad2 flac lua`
3. Download the KH Melon Mix repository and prepare:
   ```zsh
   git clone https://github.com/vitor251093/KHMelonMix
   cd KHMelonMix
   ```
4. Compile:
   ```zsh
   cmake -B build -DCMAKE_PREFIX_PATH="$(brew --prefix qt@6);$(brew --prefix libarchive)"
   cmake --build build -j$(sysctl -n hw.logicalcpu)
   ```
If everything went well, MelonMix.app should now be in the `build` directory.

### Self-contained app bundle
If you want an app bundle that can be distributed to other computers without needing to install dependencies through Homebrew, you can additionally run `
../tools/mac-libs.rb .` after the build is completed, or add `-DMACOS_BUNDLE_LIBS=ON` to the first CMake command.
