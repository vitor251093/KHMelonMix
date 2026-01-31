# Troubleshooting
Below, you can see common problems that may happen while using the Melon Mix.

Most of them involve multiple steps. Each step of each troubleshooting is going to give you a suggestion on how to solve the issue. If it works, then your problem is solved. If it doesn't, move on to the next one. If neither of them works, reach for us in our Discord or using the Github issues. 

## Where can I get a NDS file?
You need to dump your own DS cartridge. There are multiple ways to do so. Those are some of them:
- Using a DSi: https://dsi.cfw.guide/dumping-game-cards.html
- Using a 3DS: https://3ds.hacks.guide/dumping-titles-and-game-cartridges.html

## I can't see one or more cutscenes in 358/2 Days

1. We are currently only replacing the prerendered DS cutscenes, and not the cutscenes rendered with the game engine. If you were not aware of that, double check if you really have an issue.
2. Do you intend to download the HD cutscenes? If no, then you are probably experiencing a bug with the rendering of a DS cutscene.
3. Did you download [the assets folder](https://drive.google.com/drive/folders/1vQL7QZ0cQhjJ2TRFt_7u76yG9MGj6bkr?usp=sharing)? If no, download it, and then test again.
4. Open the folder where the Melon Mix application is. From there, are the MP4 files of the cutscenes inside the assets/days/cutscenes/cinematics folder? If no, organize the folders structure so that matches, and then test again.
5. Look for hd802.mp4 inside assets/days/cutscenes/cinematics. If you can't see it, that's because the assets folder wasn't completely downloaded. Open the Drive link from step 1, check which cutscene files are missing, download them manually, place then inside assets/days/cutscenes/cinematics, and then try again. 
6. Try opening hd802.mp4 with VLC. Is it playing? If yes, test again by launching the Melon Mix application directly, instead of using the bash file, the batch file, or Steam.
7. hd802.mp4 not playing may be a codec issue or a corrupted download issue. Try downloading that specific file again from the Drive, and see if that solves the issue.
8. Open `MelonMix_KHDays.bat` with notepad, add a new line after `@echo off` with `set QT_MEDIA_BACKEND=windows`, save the file, and then use it to launch the Melon Mix.

## I can't see the 358/2 Days cutscenes if I launch the Melon Mix using the sh/bat files or Steam 

1. First and foremost, be sure that you tested the suggestions at **I can't see the cutscenes in 358/2 Days**, right above.
2. If you are using the bat/sh file, be sure that you are running those from the folder where the Melon Mix application is. They won't work otherwise.
3. If you are running the Melon Mix from Steam, open the game properties through Steam, and be sure that you filled the "Start in" field with the folder where the Melon Mix application is. It won't work otherwise.

## I can't see one or more cutscenes in Re:Coded

We are currently only replacing the Days cutscenes, and not the Re:Coded ones. If you were not aware of that, double check if you really have an issue.

## My FPS won't reach 60

1. (Windows only) Using the Windows search function, open the system "Graphics Settings". Browse to the Melon Mix directory and select the executable (exe file). This will add it to the list. Edit its Options, and then switch it to "High performance".
2. Try reducing the internal resolution.
3. Try enabling the JIT Recompiler (Emu Settings -> CPU emulation).

## How can I play the game in ultrawide aspect ratio?
On the title bar menu, go to View -> "Aspect Ratio" and change the aspect ratio to "Top window". Then, resize the window to fill your monitor, and reset the game. Upon loading your save, the game is going to run in whichever aspect ratio the window is at this moment.
