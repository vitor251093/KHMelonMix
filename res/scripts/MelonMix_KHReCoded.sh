#!/bin/bash

# #498 - Temporary workaround while Wayland support is broken
export QT_QPA_PLATFORM=xcb

./MelonMix -f roms/recoded.nds
