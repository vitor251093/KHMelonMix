#!/bin/bash

# #498 - Temporary workaround while Wayland support is broken
export QT_QPA_PLATFORM=xcb

./MelonMix.AppImage -f roms/recoded.nds
