@echo off
:Start
TITLE MelonMix - Select a game
set game=
echo Which game would you like to play?
echo 1) Kingdom Hearts 358/2 Days
echo 2) Kingdom Hearts Re:Coded
echo 3) Quit
set /p game=
if /I "%game%"=="1" goto Days
if /I "%game%"=="Kingdom Hearts 358/2 Days" goto Days
if /I "%game%"=="358/2 Days" goto Days
if /I "%game%"=="Days" goto Days
if /I "%game%"=="2" goto Recoded
if /I "%game%"=="Kingdom Hearts Re:Coded" goto Recoded
if /I "%game%"=="Kingdom Hearts Recoded" goto Recoded
if /I "%game%"=="Re:Coded" goto Recoded
if /I "%game%"=="Recoded" goto Recoded
if /I "%game%"=="Coded" goto Recoded
if /I "%game%"=="3" goto End
if /I "%game%"=="N" goto End
if /I "%game%"=="No" goto End
if /I "%game%"=="Q" goto End
if /I "%game%"=="Quit" goto End
if /I "%game%"=="C" goto End
if /I "%game%"=="Close" goto End
if /I "%game%"=="Cancel" goto End
if /I "%game%"=="Exit" goto End
if /I "%game%"=="X" goto End
if /I "%game%"=="End" goto End
echo "%game%" is not a valid answer
goto Start

:Days
set gamename=Kingdom Hearts 358/2 Days
set game=days
goto Fullscreen

:Recoded
set gamename=Kingdom Hearts Re:Coded
set game=recoded
goto Fullscreen

:Fullscreen
TITLE MelonMix - %gamename%
echo.
set toggle=1
echo %gamename%:
echo 1) Fullscreen
echo 2) Windowed
echo 3) Back
set /p toggle=
if /I "%toggle%"=="1" set fullscreen=-f & goto Game
if /I "%toggle%"=="Fullscreen" set fullscreen=-f & goto Game
if /I "%toggle%"=="2" set "fullscreen=" & goto Game
if /I "%toggle%"=="Windowed" set "fullscreen=" & goto Game
if /I "%toggle%"=="3" echo. & goto Start
if /I "%toggle%"=="B" echo. & goto Start
if /I "%toggle%"=="Back" echo. & goto Start
if /I "%toggle%"=="N" echo. & goto Start
if /I "%toggle%"=="No" echo. & goto Start
if /I "%toggle%"=="Cancel" echo. & goto Start
if /I "%toggle%"=="Q" goto End
if /I "%toggle%"=="Quit" goto End
if /I "%toggle%"=="C" goto End
if /I "%toggle%"=="Close" goto End
if /I "%toggle%"=="Exit" goto End
if /I "%toggle%"=="X" goto End
if /I "%toggle%"=="End" goto End
echo "%toggle%" is not a valid answer
goto Fullscreen

:Game
echo.
if exist roms\%game%.nds (
	echo Starting %gamename%
    start MelonMix.exe %fullscreen% roms/%game%.nds
) else (
    echo Error: %game%.nds was not found in roms folder...
	echo.
    goto Start
)
goto End
:End