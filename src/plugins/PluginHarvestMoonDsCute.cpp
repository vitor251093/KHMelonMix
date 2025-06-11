#include "PluginHarvestMoonDsCute.h"

#define ASPECT_RATIO_ADDRESS_US      0
#define ASPECT_RATIO_ADDRESS_EU      0
#define ASPECT_RATIO_ADDRESS_JP      0

namespace Plugins
{

u32 PluginHarvestMoonDsCute::usGamecode = 1161052737;
u32 PluginHarvestMoonDsCute::euGamecode = 0;
u32 PluginHarvestMoonDsCute::jpGamecode = 0;

#define getAnyByCart(usAddress,euAddress,jpAddress) (isUsaCart() ? (usAddress) : (isEuropeCart() ? (euAddress) : (jpAddress)))

PluginHarvestMoonDsCute::PluginHarvestMoonDsCute(u32 gameCode)
{
    GameCode = gameCode;

    Plugin::hudToggle();
}

void PluginHarvestMoonDsCute::onLoadROM() {
    Plugin::onLoadROM();

    loadLocalization();

    u8* rom = (u8*)nds->GetNDSCart()->GetROM();
}

void PluginHarvestMoonDsCute::onLoadState() {
    Plugin::onLoadState();

    loadLocalization();
}

void PluginHarvestMoonDsCute::loadLocalization() {
    u8* rom = (u8*)nds->GetNDSCart()->GetROM();
    u32 romLength = nds->GetNDSCart()->GetROMLength();

    std::string language = "en-US";

    std::string LocalizationFilePath = localizationFilePath(language);
    Platform::FileHandle* f = Platform::OpenLocalFile(LocalizationFilePath.c_str(), Platform::FileMode::ReadText);
    if (f) {
        char linebuf[1024];
        char entryname[32];
        char entryval[1024];
        while (!Platform::IsEndOfFile(f))
        {
            if (!Platform::FileReadLine(linebuf, 1024, f))
                break;

            int ret = sscanf(linebuf, "%31[A-Za-z_0-9]=%[^\t\r\n]", entryname, entryval);
            entryname[31] = '\0';
            if (ret < 2) continue;

            std::string entrynameStr = std::string(entryname);
            if (entrynameStr.compare(0, 2, "0x") == 0) {
                int addrGap = 0;
                unsigned int addr = std::stoul(entrynameStr.substr(2), nullptr, 16);

                bool ended = false;
                for (int i = 0; i < 1023; i++) {
                    if (*((u8*)&rom[addr + i]) == 0x00) {
                        break;
                    }

                    if (entryval[i + addrGap] == '\\' && entryval[i + addrGap + 1] == 'n') {
                        *((u8*)&rom[addr + i]) = 0x0A;
                        addrGap++;
                        continue;
                    }

                    if (entryval[i + addrGap] == 0 || entryval[i + addrGap] == '\0') {
                        ended = true;
                    }

                    if (!ended) {
                        *((u8*)&rom[addr + i]) = entryval[i + addrGap];
                    }
                }
            }
        }

        CloseFile(f);
    }
    else if (false) { // Method that exports the strings from the game ROM
        int firstAddr = 0;
        int lastAddr = 0;
        bool validCharFound = false;
        bool forbCharFound = false;
        for (int addr = 0; addr < romLength; addr++) {
            bool usual = rom[addr] >= 0x41 && rom[addr] <= 0x7E;

            bool accents = rom[addr] == 0x21 || rom[addr] == 0x3F || (rom[addr] >= 0xBD && rom[addr] <= 0xD0) ||
                           rom[addr] == 0xD3 || rom[addr] == 0xD4;
            bool quotes = rom[addr] == 0xE2 || rom[addr] == 0x80 || rom[addr] == 0x9C || rom[addr] == 0x9D;
            bool unusual = (rom[addr] >= 0x20 && rom[addr] <= 0x40) || accents || quotes || rom[addr] == 0x0A ||
                            rom[addr] == 0x81 || rom[addr] == 0xF4 || rom[addr] == 0x63 || rom[addr] == 0x99;

            bool forb = rom[addr] == 0x93 || rom[addr] == 0x5F || rom[addr] == 0x2F;
            if (usual || unusual) {
                if (firstAddr == 0) {
                    firstAddr = addr;
                    lastAddr = addr;
                }
                else {
                    lastAddr = addr;
                }
            }
            if (usual) {
                validCharFound = true;
            }
            if (forb) {
                forbCharFound = true;
            }
            if (!usual && !unusual) {
                if (firstAddr != 0) {
                    if (!forbCharFound && validCharFound && lastAddr - firstAddr > 2) {
                        printf("0x%08X=", firstAddr);
                        for (int pAddr = firstAddr; pAddr <= lastAddr; pAddr++) {
                            if ((char)rom[pAddr] == 0x0A) {
                                printf("\\n");
                            }
                            else {
                                printf("%c", (char)rom[pAddr]);
                            }
                        }
                        printf("\n");
                    }

                    firstAddr = 0;
                    lastAddr = 0;
                    validCharFound = false;
                    forbCharFound = false;
                }
            }
        }
    }
};

std::string PluginHarvestMoonDsCute::localizationFilePath(std::string language) {
    std::string filename = language + ".ini";
    std::string assetsRegionSubfolderName = assetsRegionSubfolder();
    std::filesystem::path _assetsFolderPath = assetsFolderPath();
    std::filesystem::path fullPath = _assetsFolderPath / "localization" / assetsRegionSubfolderName / filename;
    if (std::filesystem::exists(fullPath)) {
        return fullPath.string();
    }

    return "";
}

std::string PluginHarvestMoonDsCute::assetsFolder() {
    return "hmdscute";
}

std::string PluginHarvestMoonDsCute::assetsRegionSubfolder() {
    return getAnyByCart("us", "eu", "jp");
}

int PluginHarvestMoonDsCute::detectGameScene()
{
    if (nds == nullptr)
    {
        return GameScene;
    }

    return 0;
}

std::vector<ShapeData2D> PluginHarvestMoonDsCute::renderer_2DShapes(int gameScene, int gameSceneState) {
    auto shapes = std::vector<ShapeData2D>();
    return shapes;
}

int PluginHarvestMoonDsCute::renderer_screenLayout() {
    return screenLayout_Top;
};

int PluginHarvestMoonDsCute::renderer_brightnessMode() {
    return brightnessMode_TopScreen;
}

bool PluginHarvestMoonDsCute::renderer_showOriginalUI() {
    return true;
}

void PluginHarvestMoonDsCute::applyAddonKeysToInputMaskOrTouchControls(u32* InputMask, u16* touchX, u16* touchY, bool* isTouching, u32* HotkeyMask, u32* HotkeyPress) {
    
}
void PluginHarvestMoonDsCute::applyTouchKeyMaskToTouchControls(u16* touchX, u16* touchY, bool* isTouching, u32 TouchKeyMask) {
    _superApplyTouchKeyMaskToTouchControls(touchX, touchY, isTouching, TouchKeyMask, CameraSensitivity, true);
}

u32 PluginHarvestMoonDsCute::getAspectRatioAddress() {
    return getAnyByCart(ASPECT_RATIO_ADDRESS_US, ASPECT_RATIO_ADDRESS_EU, ASPECT_RATIO_ADDRESS_JP);
}

}