# Playing KH Melon Mix

## Windows (without the 1.5+2.5 collection)

1. Download the [latest version](https://github.com/vitor251093/KHMelonMix/releases/latest/download/MelonMix-windows-x86_64.zip) for Windows;
2. Open the downloaded ZIP file and copy the contents to an empty folder;
3. Create a folder named "roms" next to the Melon Mix executable that you just copied;
4. In order to play "358/2 Days":
   1. Dump an original copy of "358/2 Days" and copy the NDS file to your computer (instructions at "How to dump DS cart into an NDS file" down below);
   2. Rename that NDS file to "days.nds", place it inside the "roms" folder;
5. In order to play "Re:Coded":
   1. Dump an original copy of "Re:Coded" and copy the NDS file to your computer (instructions at "How to dump DS cart into an NDS file" down below);
   2. Rename that NDS file to "recoded.nds", place it inside the "roms" folder;
6. Place [this assets folder](https://drive.google.com/drive/folders/1vQL7QZ0cQhjJ2TRFt_7u76yG9MGj6bkr?usp=sharing) next to the Melon Mix executable file (optional; only if you want to upgrade the audio / cutscenes / textures / sprites);
7. In the future, launch the Melon Mix using "MelonMix_KHDays.bat" or "MelonMix_KHReCoded.bat", but, right now, launch "MelonMix.exe";
8. Open the Days/ReCoded NDS file with the Melon Mix, and then press **Config** -> **Input and hotkeys** to configure the controls (recommended mappings at the "Recommended controller binds" guide below). You **MUST** open the NDS file **BEFORE** opening the Input and hotkeys window, otherwise you will just see the melonDS Add-ons;
9. Press **Config -> Plugin settings** to configure the Melon Mix according to your personal preferences.

## Linux / Steam Deck (without the 1.5+2.5 collection)

1. Download the [latest version](https://github.com/vitor251093/KHMelonMix/releases/latest/download/MelonMix-appimage-x86_64.zip) for Linux;
2. Open the downloaded ZIP file and copy the contents to an empty folder;
3. Create a folder named "roms" next to the Melon Mix executable that you just copied;
4. In order to play "358/2 Days":
    1. Dump an original copy of "358/2 Days" and copy the NDS file to your computer (instructions at "How to dump DS cart into an NDS file" down below);
    2. Rename that NDS file to "days.nds", place it inside the "roms" folder;
5. In order to play "Re:Coded":
    1. Dump an original copy of "Re:Coded" and copy the NDS file to your computer (instructions at "How to dump DS cart into an NDS file" down below);
    2. Rename that NDS file to "recoded.nds", place it inside the "roms" folder;
6. Place [this assets folder](https://drive.google.com/drive/folders/1vQL7QZ0cQhjJ2TRFt_7u76yG9MGj6bkr?usp=sharing) next to the Melon Mix executable file (optional; only if you want to upgrade the audio / cutscenes / textures / sprites);
7. Right-click the binary/AppImage, go to "Properties", then go to the "Permissions" tab and check the "Allow executing file as program" checkbox. Repeat those steps for MelonMix_KHDays.sh and MelonMix_KHReCoded.sh as well;
8. In the future, launch the Melon Mix using "MelonMix_KHDays.sh" or "MelonMix_KHReCoded.sh", but, right now, launch the binary/AppImage;
8. Open the Days/ReCoded NDS file with the Melon Mix, and then press **Config** -> **Input and hotkeys** to configure the controls (recommended mappings at the "Recommended controller binds" guide below). You **MUST** open the NDS file **BEFORE** opening the Input and hotkeys window, otherwise you will just see the melonDS Add-ons;
9. Press **Config -> Plugin settings** to configure the Melon Mix according to your personal preferences.

## macOS (without the 1.5+2.5 collection)

1. Download the latest version for macOS ([Intel]((https://github.com/vitor251093/KHMelonMix/releases/latest/download/MelonMix-macOS-x86_64.zip)) / [Apple Silicon](https://github.com/vitor251093/KHMelonMix/releases/latest/download/MelonMix-macOS-arm64.zip));
2. Open the downloaded ZIP file and copy the contents to an empty folder;
3. Create a folder named "roms" next to the Melon Mix executable that you just copied;
4. In order to play "358/2 Days":
    1. Dump an original copy of "358/2 Days" and copy the NDS file to your computer (instructions at "How to dump DS cart into an NDS file" down below);
    2. Rename that NDS file to "days.nds", place it inside the "roms" folder;
5. In order to play "Re:Coded":
    1. Dump an original copy of "Re:Coded" and copy the NDS file to your computer (instructions at "How to dump DS cart into an NDS file" down below);
    2. Rename that NDS file to "recoded.nds", place it inside the "roms" folder;
6. Place [this assets folder](https://drive.google.com/drive/folders/1vQL7QZ0cQhjJ2TRFt_7u76yG9MGj6bkr?usp=sharing) inside the app's Contents folder (right click the Melon Mix app and press "Show Package Contents" to find it) (optional; only if you want to upgrade the audio / cutscenes / textures / sprites);
7. Move the Melon Mix app to the Applications folder, or run the following command in the terminal: `xattr -drs com.apple.quarantine <path>`, replacing `<path>` with the full path of the Melon Mix app;
8. Now you just need to launch the Melon Mix app, and open the desired NDS file with it;
9. Open the Days/ReCoded NDS file with the Melon Mix, and then press **Config** -> **Input and hotkeys** to configure the controls (recommended mappings at the "Recommended controller binds" guide below). You **MUST** open the NDS file **BEFORE** opening the Input and hotkeys window, otherwise you will just see the melonDS Add-ons;
10. Press **Config -> Plugin settings** to configure the Melon Mix according to your personal preferences.

## Windows (from the 1.5+2.5 collection) (Steam only)

1. Download the [latest version](https://github.com/vitor251093/KHMelonMix/releases/latest/download/MelonMix-windows-x86_64.zip) for Windows;
2. Extract the downloaded ZIP file to a new folder (let's refer to it in the next steps as the "extracted folder");
3. Open Steam, right-click your installation of the 1.5+2.5 collection, and press `Manage` -> `Browse local files`, which will open a folder (let's refer to it in the next steps as the "collection folder");
4. Inside the collection folder, rename `KINGDOM HEARTS HD 1.5+2.5 Launcher.exe` to `KINGDOM HEARTS HD 1.5+2.5 Launcher backup.exe`;
5. Copy `KINGDOM HEARTS HD 1.5+2.5 Launcher.exe` folder from the extracted folder to the collection folder;
6. Inside the collection folder, open the `Image` folder, and create a `melon` folder inside of it;
7. Copy `MelonMix.exe` and the `roms` folder from the extracted folder to the `melon` folder;
8. In order to play "358/2 Days":
    1. Dump an original copy of "358/2 Days" and copy the NDS file to your computer (instructions at "How to dump DS cart into an NDS file" down below);
    2. Rename that NDS file to "days.nds", place it inside the `roms` folder;
9. In order to play "Re:Coded":
    1. Dump an original copy of "Re:Coded" and copy the NDS file to your computer (instructions at "How to dump DS cart into an NDS file" down below);
    2. Rename that NDS file to "recoded.nds", place it inside the `roms` folder;
10. Place [this assets folder](https://drive.google.com/drive/folders/1vQL7QZ0cQhjJ2TRFt_7u76yG9MGj6bkr?usp=sharing) next to the Melon Mix executable file (optional; only if you want to upgrade the audio / cutscenes / textures / sprites);
11. Open the Melon Mix directly through `MelonMix.exe`;
12. Open the Days/ReCoded NDS file with the Melon Mix, and then press **Config** -> **Input and hotkeys** to configure the controls (recommended mappings at the "Recommended controller binds" down below);
13. Press **Config -> Plugin settings** to configure the Melon Mix according to your personal preferences.

## Linux / Steam Deck (from the 1.5+2.5 collection) (Steam only)

1. Download the [latest version](https://github.com/vitor251093/KHMelonMix/releases/latest/download/MelonMix-appimage-x86_64.zip) for Linux;
2. Extract the downloaded ZIP file to a new folder (let's refer to it in the next steps as the "extracted folder");
3. Open Steam, right-click your installation of the 1.5+2.5 collection, and press `Manage` -> `Browse local files`, which will open a folder (let's refer to it in the next steps as the "collection folder");
4. Inside the collection folder, rename `KINGDOM HEARTS HD 1.5+2.5 Launcher.exe` to `KINGDOM HEARTS HD 1.5+2.5 Launcher backup.exe`;
5. Copy `KINGDOM HEARTS HD 1.5+2.5 Launcher.exe` folder from the extracted folder to the collection folder;
6. Inside the collection folder, open the `Image` folder, and create a `melon` folder inside of it;
7. Copy `MelonMix.exe`, `MelonMix.AppImage` and the `roms` folder from the extracted folder to the `melon` folder;
8. In order to play "358/2 Days":
    1. Dump an original copy of "358/2 Days" and copy the NDS file to your computer (instructions at "How to dump DS cart into an NDS file" down below);
    2. Rename that NDS file to "days.nds", place it inside the `roms` folder;
9. In order to play "Re:Coded":
    1. Dump an original copy of "Re:Coded" and copy the NDS file to your computer (instructions at "How to dump DS cart into an NDS file" down below);
    2. Rename that NDS file to "recoded.nds", place it inside the `roms` folder;
10. Place [this assets folder](https://drive.google.com/drive/folders/1vQL7QZ0cQhjJ2TRFt_7u76yG9MGj6bkr?usp=sharing) next to the Melon Mix executable file (optional; only if you want to upgrade the audio / cutscenes / textures / sprites);
11. Right-click the AppImage, go to "Properties", then go to the "Permissions" tab and check the "Allow executing file as program" checkbox;
12. Open the Melon Mix directly through `MelonMix.AppImage`;
13. Open the Days/ReCoded NDS file with the Melon Mix, and then press **Config** -> **Input and hotkeys** to configure the controls (recommended mappings at the "Recommended controller binds" down below);
14. Press **Config -> Plugin settings** to configure the Melon Mix according to your personal preferences.

# Recommended Controller Binds
* (DS Keypad tab) Map the DS D-Pad to your controller's left analog stick
* (DS Keypad tab) Map the Start button to the equivalent key in your controller
* (DS Keypad tab) Map the A/B/X/Y buttons to the equivalent keys in your controller
* (DS Keypad tab) Map the L button to the L1/LB button in your controller
* (Touch Screen tab) Map the DS Touch Screen to your controller's right analog stick
* (Add-ons tab) Map the command menu to your controller's D-Pad
* (Add-ons tab) Map "Switch Target - Left" to the L2/LT button in your controller
* (Add-ons tab) Map "Lock On" to the R1/RB button in your controller
* (Add-ons tab) Map "Switch Target - Right" to the R2/RL button in your controller
* (Add-ons tab) Map "Fullscreen Map Toggle" to the Select/Share/View button in your controller
* (Add-ons tab) Map "HUD Toggle" to the R3/R button in your controller
* (General hotkeys) Map "Toggle fullscreen" to an available button of your preference

# How to dump DS cart into an NDS file
There are multiple ways to do so. Those are some of them:
- Using a DSi: https://dsi.cfw.guide/dumping-game-cards.html
- Using a 3DS: https://3ds.hacks.guide/dumping-titles-and-game-cartridges.html
