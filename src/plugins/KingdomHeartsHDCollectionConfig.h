//
// Created by vitor on 1/21/26.
//

#ifndef MELONDS_KINGDOMHEARTSHDCOLLECTIONCONFIG_H
#define MELONDS_KINGDOMHEARTSHDCOLLECTIONCONFIG_H

#include "../types.h"

namespace Plugins
{
using namespace melonDS;

struct HDCollectionConfig
{
    u8 unk0;
    u8 unk1;
    u8 unk2;
    u8 unk3;
    u8 unk4;
    u8 unk5;
    u8 unk6;
    u8 unk7;
    u8 unk8;
    u8 unk9;
    u8 unkA;
    u8 unkB;
    u8 unkC;
    u8 unkD;
    u8 unkE;
    u8 unkF;
    u8 unk10;
    u8 unk11;
    u8 unk12;
    u8 unk13;
    u8 unk14;
    u8 unk15;
    u8 unk16;
    u8 unk17;
    u8 windowMode; // 0x00 => fullscreen; 0x02 => windowed
    u8 unk19;
    u8 unk1A;
    u8 unk1B;
    u8 unk1C;
    u8 unk1D;
    u8 unk1E;
    u8 unk1F;
    u8 unk20;
    u8 unk21;
    u8 unk22;
    u8 unk23;
    u8 unk24;
    u8 unk25;
    u8 unk26;
    u16 masterVolume; // values goes from 1 to 10
    u16 bgmVolume; // values goes from 1 to 10
    u16 sfxVolume; // values goes from 1 to 10
    u16 voicesVolume; // values goes from 1 to 10
    u8 joystickLayout; // 0 => auto; 1 => xbox; 2 => playstation; 3 => generic
    u8 unk31;
    u8 unk32;
    u8 unk33;
    u8 unk34;
    u8 unk35;
    u8 unk36;
    u8 unk37;
    u8 unk38;
    u8 unk39;
    u8 unk3A;
    u8 unk3B;
    u8 unk3C;
    u8 unk3D;
    u8 unk3E;
    u8 unk3F;
    u8 unk4x[16];
    u8 unk5x[16];
    u8 unk6x[16];
    u8 unk7x[16];
    u8 unk8x[16];
    u8 unk9x[16];
    u8 unkAx[16];
    u8 unkBx[16];
    u8 unkC0;
    u8 unkC1;
    u8 unkC2;
    u8 unkC3;

    // The controls below follow IBM PC keyboard scancode Set 1: https://www.vetra.com/scancodes.html
    u8 controls_forward;
    // TODO: KH there is more after that
};
}

#endif //MELONDS_KINGDOMHEARTSHDCOLLECTIONCONFIG_H