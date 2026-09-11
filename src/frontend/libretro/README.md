# KH Melon Mix libretro core

A [libretro](https://www.libretro.com/) frontend for KH Melon Mix, so the
emulator can run inside RetroArch and the other libretro frontends.

It links the same `core` target the Qt/SDL frontend links, so the Kingdom
Hearts plugins (`src/plugins`) are compiled in and behave the same way.

## Building

```bash
cmake -B build-libretro -DBUILD_LIBRETRO=ON -DBUILD_QT_SDL=OFF -DENABLE_OGLRENDERER=OFF
cmake --build build-libretro
```

`ENABLE_OGLRENDERER=OFF` is required. The OpenGL sources in `core` need the
`MELONDS_GL_HEADER` definition, which only the Qt/SDL frontend supplies; the
CMake configure step fails with an explanation if the option is left on.

The build produces `khmelonmix_libretro.dll` / `.so` / `.dylib` plus the
matching `khmelonmix_libretro.info`.

## Installing

Two files, two directories:

| File | Goes to |
|---|---|
| `khmelonmix_libretro.<suffix>` | RetroArch's `cores/` directory |
| `khmelonmix_libretro.info` | RetroArch's `info/` directory |

The info file is not optional decoration. Copying only the library gets you a
core you can load by hand, but RetroArch will not offer it for `.nds` files and
will not associate it with a scanned Nintendo DS playlist, because both of
those come from the info file:

- `supported_extensions` is what RetroArch filters by when it lists the cores
  that can open the file you picked in **Load Content**. It has to agree with
  the `valid_extensions` the core reports from `retro_get_system_info`.
- `database` is what ties the core to a scanned playlist. RetroArch builds
  `Nintendo - Nintendo DS.lpl` from its own database, then offers the cores
  whose `database` field names it.
- The info file's name has to match the library's, minus the suffix, or
  RetroArch will not pair them.

For an existing playlist you can also set the association by hand: open the
playlist, press the menu button on it, and choose **Set Core Association**.

Note that the exact directories depend on the RetroArch install; check
**Settings > Directory > Cores** and **Core Info** for the paths your build
uses. Some portable RetroArch layouts point both at the same folder, which is
why the build copies the info file next to the library as well.

## What works

- Retail NDS ROMs, loaded from the buffer the frontend provides.
- The Kingdom Hearts plugins' per-frame behaviour: game-scene detection, the
  single-screen presentation, and the widescreen rendering (the plugin patches
  the game's own aspect-ratio setting, so the game itself draws a wider field
  of view into the DS framebuffer).
- RetroPad input, plus the plugin's extra logical buttons on port 2.
- Touchscreen through the pointer or mouse device.
- Cartridge saves through `RETRO_MEMORY_SAVE_RAM`, so RetroArch owns the
  `.srm` file.
- Savestates.
- Core options for the screen layout, screen gap, screen swap, touch mode,
  direct boot, external BIOS and the plugin's widescreen rendering.

## What does not work yet

- **Shader-composited HUD.** The plugins draw their replacement HUD and crop
  UI polygons from inside melonDS' OpenGL compositor
  (`Plugin::gpuOpenGL_FS()`, `Plugin::gpuOpenGL_applyChangesToPolygon()`).
  Those paths only run under the OpenGL and OpenGL-Compute 3D renderers. This
  core uses the software renderer, which is what makes it portable to every
  platform RetroArch targets. Adding a hardware path means requesting a GL
  context with `RETRO_ENVIRONMENT_SET_HW_RENDER` and presenting
  `GLCompositor`'s output texture; the video code is isolated in `Screen.cpp`
  so that can be added without touching anything else.
- **Replacement cutscenes and background music.** The plugins signal these
  through callbacks that the Qt frontend answers by playing external video and
  audio files out of band. libretro has no second audio or video channel, so
  the core leaves those callbacks unset; the affected cutscenes play in their
  original in-game form.
- DSi mode, local multiplayer, Wi-Fi, camera, microphone and Lua scripting.
- Cheats.

## Files

| File | Role |
|---|---|
| `Core.cpp` / `Core.h` | The `retro_*` entry points, NDS construction, ROM loading, the frame loop, savestates. `Core.h` is the shared contract for the whole directory. |
| `Platform.cpp` | The `melonDS::Platform` implementation, with no Qt and no SDL. |
| `Screen.cpp` / `Screen.h` | Screen layout, framebuffer composition, geometry, pointer-to-touch mapping. |
| `Input.cpp` / `Input.h` | RetroPad and pointer handling, plugin input hooks. |
| `CoreOptions.cpp` / `CoreOptions.h` | Core option declarations and reading. |
| `PluginJoystick.cpp` | No-op implementation of the one plugin hook that belongs to the frontend rather than to `core`. RetroArch owns the controller bindings. |
| `libretro.h` | Vendored upstream header, unmodified. |
| `link.T` | ld version script; hides every symbol that is not `retro_*`. |
| `khmelonmix_libretro.info` | RetroArch core info file. |
