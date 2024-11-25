#ifndef ROMGBASLOTCONFIG_H
#define ROMGBASLOTCONFIG_H

#include <string>

namespace MelonDSAndroid
{
    enum RomGbaSlotConfigType {
        NONE = 0,
        GBA_ROM = 1,
        MEMORY_EXPANSION = 2
    } ;

    struct RomGbaSlotConfig {
        RomGbaSlotConfigType type;
    };

    struct RomGbaSlotConfigNone {
        RomGbaSlotConfig _base = { .type = RomGbaSlotConfigType::NONE };
    };

    struct RomGbaSlotConfigGbaRom {
        RomGbaSlotConfig _base = { .type = RomGbaSlotConfigType::GBA_ROM };
        std::string romPath;
        std::string savePath;
    };

    struct RomGbaSlotConfigMemoryExpansion {
        RomGbaSlotConfig _base = { .type = RomGbaSlotConfigType::MEMORY_EXPANSION };
    };
}

#endif //ROMGBASLOTCONFIG_H
