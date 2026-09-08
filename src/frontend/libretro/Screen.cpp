/*
    Copyright 2016-2025 melonDS team

    This file is part of melonDS.

    melonDS is free software: you can redistribute it and/or modify it under
    the terms of the GNU General Public License as published by the Free
    Software Foundation, either version 3 of the License, or (at your option)
    any later version.

    melonDS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with melonDS. If not, see http://www.gnu.org/licenses/.
*/

#include <string.h>

#include "Screen.h"
#include "Core.h"

namespace Libretro
{
namespace Screen
{

using namespace melonDS;

// How the GPU indexes its two framebuffers. The bottom screen is the only
// touchable one, and that stays true whatever the layout and SwapScreens do
// with it, so the touch mapping keys off this index rather than off a position.
enum
{
    screenIndex_Top = 0,
    screenIndex_Bottom = 1,
};

// RETRO_DEVICE_ID_POINTER_X/Y are reported in this signed range over the full
// video frame, independently of the window size or of any scaling.
constexpr int PointerRange = 0x7FFF;

struct LayoutState
{
    u32 BaseWidth;
    u32 BaseHeight;
    float Aspect;

    // All indexed by DS screen, not by slot in the frame: SwapScreens is
    // resolved while the layout is computed, so nothing downstream has to
    // look at it again.
    bool Visible[2];
    u32 PosX[2];
    u32 PosY[2];

    // Offset in words of each screen's first pixel inside Buffer. Derived from
    // PosX/PosY/BaseWidth once here so RenderFrame never redoes layout math.
    u32 Offset[2];
};

static u32* Buffer = nullptr;
static LayoutState CurrentLayout = {};

static void ClearBuffer()
{
    if (Buffer != nullptr)
        memset(Buffer, 0, MaxScreenWidth * MaxScreenHeight * sizeof(u32));
}

// Translates the plugin's own layout enum (PluginShapes.h) into ours. The
// Kingdom Hearts plugins change this per game scene, which is why the auto
// layout has to be re-read every frame.
static ScreenLayoutType PluginLayout(Plugins::Plugin* plugin)
{
    if (plugin == nullptr)
        return screenLayout_Top;

    switch (plugin->renderer_screenLayout())
    {
    case Plugins::screenLayout_Bottom: return screenLayout_Bottom;
    case Plugins::screenLayout_BothVertical: return screenLayout_TopBottom;
    case Plugins::screenLayout_BothHorizontal: return screenLayout_LeftRight;
    }

    return screenLayout_Top;
}

static LayoutState ComputeLayout(Plugins::Plugin* plugin)
{
    LayoutState layout = {};

    ScreenLayoutType type = Config.ScreenLayout;
    if (type == screenLayout_Auto)
        type = PluginLayout(plugin);

    // slotA is the top (or left) half of the frame, slotB the bottom (or
    // right) one, and both hold a DS screen index. SwapScreens only exchanges
    // the contents of the two slots, never the geometry, and for the
    // single-screen layouts it is what picks the other screen.
    const int slotA = Config.SwapScreens ? screenIndex_Bottom : screenIndex_Top;
    const int slotB = Config.SwapScreens ? screenIndex_Top : screenIndex_Bottom;

    switch (type)
    {
    case screenLayout_TopBottom:
        {
            u32 gap = Config.ScreenGap;
            if (gap > MaxScreenGap)
                gap = MaxScreenGap;

            layout.BaseWidth = ScreenWidth;
            layout.BaseHeight = ScreenHeight*2 + gap;
            layout.Visible[slotA] = true;
            layout.Visible[slotB] = true;
            layout.PosY[slotB] = ScreenHeight + gap;
        }
        break;

    case screenLayout_LeftRight:
        // The gap is a vertical separator only; side by side screens touch.
        layout.BaseWidth = ScreenWidth*2;
        layout.BaseHeight = ScreenHeight;
        layout.Visible[slotA] = true;
        layout.Visible[slotB] = true;
        layout.PosX[slotB] = ScreenWidth;
        break;

    case screenLayout_Bottom:
        layout.BaseWidth = ScreenWidth;
        layout.BaseHeight = ScreenHeight;
        layout.Visible[slotB] = true;
        break;

    case screenLayout_Top:
    default:
        layout.BaseWidth = ScreenWidth;
        layout.BaseHeight = ScreenHeight;
        layout.Visible[slotA] = true;
        break;
    }

    layout.Aspect = (float)layout.BaseWidth / (float)layout.BaseHeight;

    // The Kingdom Hearts plugins make the game render a widescreen image inside
    // the same 256x192 buffer instead of producing more pixels, so the frame is
    // not physically wider and the intended shape can only reach the frontend
    // as a separate aspect ratio. It describes one DS screen, so it is
    // meaningless once both screens share the frame.
    if (plugin != nullptr && !(layout.Visible[screenIndex_Top] && layout.Visible[screenIndex_Bottom]))
    {
        const float forced = plugin->renderer_forcedAspectRatio();
        if (forced > 0.0f)
            layout.Aspect = forced;
    }

    for (int i = 0; i < 2; i++)
        layout.Offset[i] = layout.PosY[i]*layout.BaseWidth + layout.PosX[i];

    return layout;
}

void Init()
{
    if (Buffer == nullptr)
        Buffer = new u32[MaxScreenWidth * MaxScreenHeight];

    // A frame can be asked for before the first RefreshLayout, so start from a
    // valid layout instead of a zero-sized one.
    CurrentLayout = ComputeLayout(nullptr);
    ClearBuffer();
}

void DeInit()
{
    delete[] Buffer;
    Buffer = nullptr;
}

bool RefreshLayout(Plugins::Plugin* plugin)
{
    const LayoutState next = ComputeLayout(plugin);

    const bool geometryChanged = next.BaseWidth != CurrentLayout.BaseWidth ||
                                 next.BaseHeight != CurrentLayout.BaseHeight ||
                                 next.Aspect != CurrentLayout.Aspect;

    bool placementChanged = false;
    for (int i = 0; i < 2; i++)
    {
        if (next.Visible[i] != CurrentLayout.Visible[i] || next.Offset[i] != CurrentLayout.Offset[i])
            placementChanged = true;
    }

    if (geometryChanged || placementChanged)
    {
        CurrentLayout = next;

        // A screen that moved or went away leaves its old pixels behind, and
        // the gap rows are never written by anyone, so both would show garbage
        // from the previous layout.
        ClearBuffer();

        Log(RETRO_LOG_INFO, "[Screen] layout %ux%u, aspect ratio %.4f\n",
            CurrentLayout.BaseWidth, CurrentLayout.BaseHeight, CurrentLayout.Aspect);
    }

    return geometryChanged;
}

void GetGeometry(retro_game_geometry* geometry)
{
    geometry->base_width = CurrentLayout.BaseWidth;
    geometry->base_height = CurrentLayout.BaseHeight;
    geometry->max_width = MaxScreenWidth;
    geometry->max_height = MaxScreenHeight;
    geometry->aspect_ratio = CurrentLayout.Aspect;
}

void RenderFrame(melonDS::NDS& nds)
{
    if (Buffer == nullptr)
        return;

    // With an accelerated 3D renderer the GPU widens every framebuffer row into
    // three layers plus one padding word for the compositing shader, so the
    // source stride is not always the screen width.
    const u32 srcStride = nds.GPU.GPU3D.IsRendererAccelerated() ? (ScreenWidth*3 + 1) : ScreenWidth;
    const int frontBuffer = nds.GPU.FrontBuffer;

    for (int i = 0; i < 2; i++)
    {
        if (!CurrentLayout.Visible[i])
            continue;

        // Null until the GPU has a renderer, which the frontend may not have
        // set up yet when the first frames are pumped.
        const u32* src = nds.GPU.Framebuffer[frontBuffer][i].get();
        if (src == nullptr)
            continue;

        u32* dst = Buffer + CurrentLayout.Offset[i];
        for (u32 y = 0; y < ScreenHeight; y++)
            memcpy(dst + y*CurrentLayout.BaseWidth, src + y*srcStride, ScreenWidth * sizeof(u32));
    }

    VideoCallback(Buffer, CurrentLayout.BaseWidth, CurrentLayout.BaseHeight,
                  CurrentLayout.BaseWidth * sizeof(u32));
}

bool PointerToTouch(int pointerX, int pointerY, u16* touchX, u16* touchY)
{
    if (!CurrentLayout.Visible[screenIndex_Bottom])
        return false;

    if (pointerX < -PointerRange || pointerX > PointerRange ||
        pointerY < -PointerRange || pointerY > PointerRange)
        return false;

    // The far edge of the range lands exactly on base_width/base_height, one
    // past the last pixel, so it has to be clamped back into the frame.
    int frameX = ((pointerX + PointerRange) * (int)CurrentLayout.BaseWidth) / (PointerRange*2);
    if (frameX > (int)CurrentLayout.BaseWidth - 1)
        frameX = (int)CurrentLayout.BaseWidth - 1;

    int frameY = ((pointerY + PointerRange) * (int)CurrentLayout.BaseHeight) / (PointerRange*2);
    if (frameY > (int)CurrentLayout.BaseHeight - 1)
        frameY = (int)CurrentLayout.BaseHeight - 1;

    // Every screen is composed at its native size, so the position inside the
    // touchable screen's rectangle already is the touch coordinate.
    const int localX = frameX - (int)CurrentLayout.PosX[screenIndex_Bottom];
    const int localY = frameY - (int)CurrentLayout.PosY[screenIndex_Bottom];
    if (localX < 0 || localX >= (int)ScreenWidth || localY < 0 || localY >= (int)ScreenHeight)
        return false;

    *touchX = (u16)localX;
    *touchY = (u16)localY;
    return true;
}

}
}
