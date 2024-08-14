@echo off
set game=recoded
if exist roms\%game%.nds (
    start MelonMix.exe -f roms/%game%.nds
) else (
    echo "No file %game%.nds found inside roms"
    pause
)