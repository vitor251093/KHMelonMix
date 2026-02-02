<div align="center">
  <img src="https://raw.githubusercontent.com/vitor251093/KHMelonMix/master/logo.png" alt="KH Melon Mix" width="70%" />
</div>
<h4 align="center"><i>Logo by JackSilverson</i></h4>

<p align="center">
<a href="https://www.gnu.org/licenses/gpl-3.0" alt="License: GPLv3"><img src="https://img.shields.io/badge/License-GPL%20v3-%23ff554d.svg"></a>
<a href="https://discord.gg/AhRscvkqB9" alt="Discord"><img src="https://img.shields.io/badge/Discord-KHMelonMix-7289da?logo=discord&logoColor=white"></a>
<a href="https://github.com/vitor251093/KHMelonMix/actions/workflows/build-windows.yml?query=event%3Apush"><img src="https://github.com/vitor251093/KHMelonMix/actions/workflows/build-windows.yml/badge.svg" /></a>
<a href="https://github.com/vitor251093/KHMelonMix/actions/workflows/build-ubuntu.yml?query=event%3Apush"><img src="https://github.com/vitor251093/KHMelonMix/actions/workflows/build-ubuntu.yml/badge.svg" /></a>
<a href="https://github.com/vitor251093/KHMelonMix/actions/workflows/build-macos.yml?query=event%3Apush"><img src="https://github.com/vitor251093/KHMelonMix/actions/workflows/build-macos.yml/badge.svg" /></a>
<a href="https://github.com/vitor251093/KHMelonMix/actions/workflows/build-bsd.yml?query=event%3Apush"><img src="https://github.com/vitor251093/KHMelonMix/actions/workflows/build-bsd.yml/badge.svg" /></a>
</p>
melonDS, sorta

This project aims to turn the KH DS games into playable PC games with a single screen and controls suited for a regular controller.

This is a version of [melonDS](https://github.com/melonDS-emu/melonDS) with some modifications made specifically to improve the experience of those specific games. Any issues you have with KH Melon Mix should be reported in this same repository, and not in melonDS repository.

The project has an internal plugin system, so different enhancements can be applied to different games. So if you also want to improve the experience of a different game, fell free to fork this project and introduce a new plugin to the list.

Join our [Discord](https://discord.gg/AhRscvkqB9) to keep in touch with the latest updates and/or to help with the project.

<hr>

<h2 align="center"><a href="https://github.com/vitor251093/KHMelonMix/blob/master/HOW_TO_PLAY.md">HOW TO PLAY</a></h2>

<h2 align="center"><a href="https://github.com/vitor251093/KHMelonMix/blob/master/TROUBLESHOOTING.md">TROUBLESHOOTING</a></h2>

<h2 align="center"><a href="https://github.com/vitor251093/KHMelonMix/blob/master/BUILD.md">HOW TO BUILD FROM SCRATCH</a></h2>

<hr>

## What exactly does the Melon Mix do for the KH games?
- Support to any aspect ratio
- Support to any resolution
- Floating dialog boxes (Days only)
- In-game HUD adapted to a single screen
- Command menu controls
- Camera controls (analog stick)
- Lock On and Switch target as separate controls
- DS cutscene to HD cutscene replacement (Days only, for now)
- Remastered BGM replacement

## Which other features are being worked on?
- Texture replacement
- Camera Mode
- In-engine cutscene with HD cutscene replacement
- Menus adapted to a single screen
- Camera controls (mouse/touchpad)
- Pause functionality to HD cutscenes

## Which other features are being considered?
- Lua support
- 3D models replacement (do not expect modern games quality on that one, it still needs to run on the DS CPU)
- Rumble to joystick controllers
- Support to RetroAchievements
- Command Menu should reflect the mission world (toggleable feature)

<hr>

<p align="center"><img src="https://raw.githubusercontent.com/vitor251093/KHMelonMix/master/screenshot.png"></p>
<p align="center"><i>From melonDS with a widescreen cheat to KH Melon Mix v0.7.1</i></p>

## Team
* **Programmers:** @vitor251093, @justedni and @Kite2810
* **Designers:** @sandwichwater and @DaniKH1
* **Moderators:** BlazeFaia

## Credits

 * All people that supported and developed [melonDS](https://github.com/melonDS-emu/melonDS) (seriously, support them if possible; this project wouldn't exist without melonDS)
 * Logo of the project by JackSilverson
 * Logo of Days Melon Mix and ReCoded Melon Mix by DiabeticMedic
 * @NPO-197 for introducing Lua support on the project
 * All the members from our Discord server that helped testing and developing the Melon Mix!
 * Michael Lipinski, for the [documentation](https://pdfs.semanticscholar.org/657d/adf4888f6302701095055b0d7a066e42b36f.pdf) regarding the way a NDS works
 * Datel Design & Development Ltd, for the [documentation](https://uk.codejunkies.com/support_downloads/Trainer-Toolkit-for-Nintendo-DS-User-Manual.pdf) regarding how to write AR Codes

## Licenses

[![GNU GPLv3 Image](https://www.gnu.org/graphics/gplv3-127x51.png)](http://www.gnu.org/licenses/gpl-3.0.en.html)

KH Melon Mix is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

### External
* Images used in the Input Config Dialog - see `src/frontend/qt_sdl/InputConfig/resources/LICENSE.md`

