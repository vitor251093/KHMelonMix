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

#include "plugins/PluginManager.h"

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

#include <optional>
#include <vector>
#include <string>
#include <algorithm>

#include <SDL2/SDL.h>

#include "main.h"

#include "types.h"
#include "version.h"

#include "ScreenLayout.h"

#include "Args.h"
#include "NDS.h"
#include "NDSCart.h"
#include "GBACart.h"
#include "GPU.h"
#include "SPU.h"
#include "Wifi.h"
#include "Platform.h"
#include "LocalMP.h"
#include "Config.h"
#include "RTC.h"
#include "DSi.h"
#include "DSi_I2C.h"
#include "GPU3D_Soft.h"
#include "GPU3D_OpenGL.h"
#include "GPU3D_Compute.h"

#include "Savestate.h"

#include "EmuInstance.h"

using namespace melonDS;


EmuThread::EmuThread(EmuInstance* inst, QObject* parent) : QThread(parent)
{
    emuInstance = inst;

    emuStatus = emuStatus_Paused;
    emuPauseStack = emuPauseStackRunning;
    emuActive = false;
}

void EmuThread::attachWindow(MainWindow* window)
{
    mainWindow = window;

    connect(this, SIGNAL(windowTitleChange(QString)), window, SLOT(onTitleUpdate(QString)));
    connect(this, SIGNAL(windowEmuStart()), window, SLOT(onEmuStart()));
    connect(this, SIGNAL(windowEmuStop()), window, SLOT(onEmuStop()));
    connect(this, SIGNAL(windowEmuPause(bool)), window, SLOT(onEmuPause(bool)));
    connect(this, SIGNAL(windowEmuReset()), window, SLOT(onEmuReset()));
    connect(this, SIGNAL(autoScreenSizingChange(int)), window->panel, SLOT(onAutoScreenSizingChanged(int)));
    connect(this, SIGNAL(windowFullscreenToggle()), window, SLOT(onFullscreenToggled()));
    connect(this, SIGNAL(screenEmphasisToggle()), window, SLOT(onScreenEmphasisToggled()));

    if (window->winHasMenu())
    {
        connect(this, SIGNAL(windowLimitFPSChange()), window->actLimitFramerate, SLOT(trigger()));
        connect(this, SIGNAL(swapScreensToggle()), window->actScreenSwap, SLOT(trigger()));
    }

    connect(this, SIGNAL(windowStartBgmMusic(quint16, quint8, bool, quint32, QString)), window, SLOT(asyncStartBgmMusic(quint16, quint8, bool, quint32, QString)));
    connect(this, SIGNAL(windowStopBgmMusic(quint16, bool, quint32)), window, SLOT(asyncStopBgmMusic(quint16, bool, quint32)));
    connect(this, SIGNAL(windowPauseBgmMusic()), window, SLOT(asyncPauseBgmMusic()));
    connect(this, SIGNAL(windowUnpauseBgmMusic()), window, SLOT(asyncUnpauseBgmMusic()));
    connect(this, SIGNAL(windowUpdateBgmMusicVolume(quint8)), window, SLOT(asyncUpdateBgmMusicVolume(quint8)));
    connect(this, SIGNAL(windowStopAllBgm()), window, SLOT(asyncStopAllBgm()));

    connect(this, SIGNAL(windowStartVideo(QString)), window, SLOT(asyncStartVideo(QString)));
    connect(this, SIGNAL(windowStopVideo()), window, SLOT(asyncStopVideo()));
    connect(this, SIGNAL(windowPauseVideo()), window, SLOT(asyncPauseVideo()));
    connect(this, SIGNAL(windowUnpauseVideo()), window, SLOT(asyncUnpauseVideo()));
}

void EmuThread::detachWindow(MainWindow* window)
{
    disconnect(this, SIGNAL(windowTitleChange(QString)), window, SLOT(onTitleUpdate(QString)));
    disconnect(this, SIGNAL(windowEmuStart()), window, SLOT(onEmuStart()));
    disconnect(this, SIGNAL(windowEmuStop()), window, SLOT(onEmuStop()));
    disconnect(this, SIGNAL(windowEmuPause(bool)), window, SLOT(onEmuPause(bool)));
    disconnect(this, SIGNAL(windowEmuReset()), window, SLOT(onEmuReset()));
    disconnect(this, SIGNAL(autoScreenSizingChange(int)), window->panel, SLOT(onAutoScreenSizingChanged(int)));
    disconnect(this, SIGNAL(windowFullscreenToggle()), window, SLOT(onFullscreenToggled()));
    disconnect(this, SIGNAL(screenEmphasisToggle()), window, SLOT(onScreenEmphasisToggled()));

    if (window->winHasMenu())
    {
        disconnect(this, SIGNAL(windowLimitFPSChange()), window->actLimitFramerate, SLOT(trigger()));
        disconnect(this, SIGNAL(swapScreensToggle()), window->actScreenSwap, SLOT(trigger()));
    }

    disconnect(this, SIGNAL(windowStartBgmMusic(quint16, quint8, bool, quint32, QString)), window, SLOT(asyncStartBgmMusic(quint16, quint8, bool, quint32, QString)));
    disconnect(this, SIGNAL(windowStopBgmMusic(quint16, bool, quint32)), window, SLOT(asyncStopBgmMusic(quint16, bool, quint32)));
    disconnect(this, SIGNAL(windowPauseBgmMusic()), window, SLOT(asyncPauseBgmMusic()));
    disconnect(this, SIGNAL(windowUnpauseBgmMusic()), window, SLOT(asyncUnpauseBgmMusic()));
    disconnect(this, SIGNAL(windowUpdateBgmMusicVolume(quint8)), window, SLOT(asyncUpdateBgmMusicVolume(quint8)));
    disconnect(this, SIGNAL(windowStopAllBgm()), window, SLOT(asyncStopAllBgm()));

    disconnect(this, SIGNAL(windowStartVideo(QString)), window, SLOT(asyncStartVideo(QString)));
    disconnect(this, SIGNAL(windowStopVideo()), window, SLOT(asyncStopVideo()));
    disconnect(this, SIGNAL(windowPauseVideo()), window, SLOT(asyncPauseVideo()));
    disconnect(this, SIGNAL(windowUnpauseVideo()), window, SLOT(asyncUnpauseVideo()));
}

void EmuThread::run()
{
    emuInstance->plugin = nullptr;

    Config::Table& globalCfg = emuInstance->getGlobalConfig();
    u32 mainScreenPos[3];

    //emuInstance->updateConsole();
    // No carts are inserted when melonDS first boots

    mainScreenPos[0] = 0;
    mainScreenPos[1] = 0;
    mainScreenPos[2] = 0;
    autoScreenSizing = 0;

    //videoSettingsDirty = false;

    if (emuInstance->usesOpenGL())
    {
        emuInstance->initOpenGL(0);

        useOpenGL = true;
        videoRenderer = globalCfg.GetInt("3D.Renderer");
    }
    else
    {
        useOpenGL = false;
        videoRenderer = 0;
    }

    //updateRenderer();
    videoSettingsDirty = true;

    u32 nframes = 0;
    double perfCountsSec = 1.0 / SDL_GetPerformanceFrequency();
    double lastTime = SDL_GetPerformanceCounter() * perfCountsSec;
    double frameLimitError = 0.0;
    double lastMeasureTime = lastTime;

    u32 winUpdateCount = 0, winUpdateFreq = 1;
    u8 dsiVolumeLevel = 0x1F;

    char melontitle[100];

    bool fastforward = false;
    bool slowmo = false;
    emuInstance->fastForwardToggled = false;
    emuInstance->slowmoToggled = false;

    while (emuStatus != emuStatus_Exit)
    {
        MPInterface::Get().Process();
        emuInstance->inputProcess();

        auto nds = emuInstance->getNDS();
        auto rom = nds == nullptr ? nullptr : nds->NDSCartSlot.GetCart();
        if (rom != nullptr) {
            u32 gamecode = rom->GetHeader().GameCodeAsU32();
            bool shouldLoadPlugin = emuInstance->plugin == nullptr || emuInstance->plugin->getGameCode() != gamecode;
            if (shouldLoadPlugin)
            {
                lastVideoRenderer = -1;
                videoSettingsDirty = true;

                emit windowStopAllBgm();
                emuInstance->plugin = Plugins::PluginManager::load(gamecode);
                emuInstance->plugin->setNds(emuInstance->getNDS());
                emuInstance->plugin->onLoadROM();
                emuInstance->plugin->shouldInvalidateConfigs = true;
            }
            if (emuInstance->plugin->shouldInvalidateConfigs)
            {
                emuInstance->plugin->shouldInvalidateConfigs = false;
                emuInstance->plugin->loadConfigs([cfg = globalCfg](const std::string& path){
                    Config::Table& ref = const_cast <Config::Table&>(cfg);
                    return ref.GetBool(path);
                },
                [cfg = globalCfg](const std::string& path){
                    Config::Table& ref = const_cast <Config::Table&>(cfg);
                    return ref.GetInt(path);
                },
                [cfg = globalCfg](const std::string& path){
                    Config::Table& ref = const_cast <Config::Table&>(cfg);
                    return ref.GetString(path);
                });
            }
            if (shouldLoadPlugin)
            {
                if (emuInstance->plugin->shouldStartInFullscreen()) {
                    emit windowFullscreenToggle();
                }
                emuInstance->inputLoadConfig();

                printf("Loading plugin %s for game code %u\n", typeid(*emuInstance->plugin).name(), gamecode);
            }

            emuInstance->plugin->applyTouchKeyMaskToTouchControls(&emuInstance->touchX, &emuInstance->touchY, &emuInstance->isTouching, emuInstance->touchInputMask);
            emuInstance->plugin->applyHotkeyToInputMaskOrTouchControls(&emuInstance->inputMask, &emuInstance->touchX, &emuInstance->touchY, &emuInstance->isTouching, &emuInstance->hotkeyMask, &emuInstance->hotkeyPress);
            emuInstance->plugin->applyAddonKeysToInputMaskOrTouchControls(&emuInstance->inputMask, &emuInstance->touchX, &emuInstance->touchY, &emuInstance->isTouching, &emuInstance->pluginMask, &emuInstance->pluginPress);
            
            if (!emuInstance->isRumbling && emuInstance->plugin->shouldRumble()) {
                emuInstance->inputRumbleStart(1000); // duration in ms
            }
            if (emuInstance->isRumbling && !emuInstance->plugin->shouldRumble()) {
                emuInstance->inputRumbleStop();
            }
        }

        if (emuInstance->hotkeyPressed(HK_FrameLimitToggle)) emit windowLimitFPSChange();

        if (emuInstance->hotkeyPressed(HK_Pause)) emuTogglePause();
        if (emuInstance->hotkeyPressed(HK_Reset)) emuReset();
        if (emuInstance->hotkeyPressed(HK_FrameStep)) emuFrameStep();

        if (emuInstance->hotkeyPressed(HK_FullscreenToggle)) emit windowFullscreenToggle();

        if (emuInstance->hotkeyPressed(HK_SwapScreens)) emit swapScreensToggle();
        if (emuInstance->hotkeyPressed(HK_SwapScreenEmphasis)) emit screenEmphasisToggle();

        if (emuStatus == emuStatus_Running || emuStatus == emuStatus_FrameStep)
        {
            if (emuStatus == emuStatus_FrameStep) emuStatus = emuStatus_Paused;

            if (emuInstance->hotkeyPressed(HK_SolarSensorDecrease))
            {
                int level = emuInstance->nds->GBACartSlot.SetInput(GBACart::Input_SolarSensorDown, true);
                if (level != -1)
                {
                    emuInstance->osdAddMessage(0, "Solar sensor level: %d", level);
                }
            }
            if (emuInstance->hotkeyPressed(HK_SolarSensorIncrease))
            {
                int level = emuInstance->nds->GBACartSlot.SetInput(GBACart::Input_SolarSensorUp, true);
                if (level != -1)
                {
                    emuInstance->osdAddMessage(0, "Solar sensor level: %d", level);
                }
            }

            if (emuInstance->nds->ConsoleType == 1)
            {
                DSi* dsi = static_cast<DSi*>(emuInstance->nds);
                double currentTime = SDL_GetPerformanceCounter() * perfCountsSec;

                // Handle power button
                if (emuInstance->hotkeyDown(HK_PowerButton))
                {
                    dsi->I2C.GetBPTWL()->SetPowerButtonHeld(currentTime);
                }
                else if (emuInstance->hotkeyReleased(HK_PowerButton))
                {
                    dsi->I2C.GetBPTWL()->SetPowerButtonReleased(currentTime);
                }

                // Handle volume buttons
                if (emuInstance->hotkeyDown(HK_VolumeUp))
                {
                    dsi->I2C.GetBPTWL()->SetVolumeSwitchHeld(DSi_BPTWL::volumeKey_Up);
                }
                else if (emuInstance->hotkeyReleased(HK_VolumeUp))
                {
                    dsi->I2C.GetBPTWL()->SetVolumeSwitchReleased(DSi_BPTWL::volumeKey_Up);
                }

                if (emuInstance->hotkeyDown(HK_VolumeDown))
                {
                    dsi->I2C.GetBPTWL()->SetVolumeSwitchHeld(DSi_BPTWL::volumeKey_Down);
                }
                else if (emuInstance->hotkeyReleased(HK_VolumeDown))
                {
                    dsi->I2C.GetBPTWL()->SetVolumeSwitchReleased(DSi_BPTWL::volumeKey_Down);
                }

                dsi->I2C.GetBPTWL()->ProcessVolumeSwitchInput(currentTime);
            }

            if (useOpenGL)
                emuInstance->makeCurrentGL();

            // update render settings if needed
            if (videoSettingsDirty)
            {
                emuInstance->renderLock.lock();
                if (useOpenGL)
                {
                    emuInstance->setVSyncGL(true);
                    videoRenderer = globalCfg.GetInt("3D.Renderer");
                }
#ifdef OGLRENDERER_ENABLED
                else
#endif
                {
                    videoRenderer = 0;
                }

                updateRenderer();

                videoSettingsDirty = false;
                emuInstance->renderLock.unlock();
            }

            emuInstance->nds->SetKeyMask(emuInstance->inputMask);

            if (emuInstance->isTouching)
                emuInstance->nds->TouchScreen(emuInstance->touchX, emuInstance->touchY);
            else
                emuInstance->nds->ReleaseScreen();

            if (emuInstance->hotkeyPressed(HK_Lid))
            {
                bool lid = !emuInstance->nds->IsLidClosed();
                emuInstance->nds->SetLidClosed(lid);
                emuInstance->osdAddMessage(0, lid ? "Lid closed" : "Lid opened");
            }

            // microphone input
            emuInstance->micProcess();

            refreshPluginGameScene();
            bool shouldRenderFrame = emuInstance->plugin->shouldRenderFrame();

            // auto screen layout
            {
                mainScreenPos[2] = mainScreenPos[1];
                mainScreenPos[1] = mainScreenPos[0];
                mainScreenPos[0] = emuInstance->nds->PowerControl9 >> 15;

                int guess;
                if (mainScreenPos[0] == mainScreenPos[2] &&
                    mainScreenPos[0] != mainScreenPos[1])
                {
                    // constant flickering, likely displaying 3D on both screens
                    // TODO: when both screens are used for 2D only...???
                    guess = screenSizing_Even;
                }
                else
                {
                    if (mainScreenPos[0] == 1)
                        guess = screenSizing_EmphTop;
                    else
                        guess = screenSizing_EmphBot;
                }

                if (guess != autoScreenSizing)
                {
                    autoScreenSizing = guess;
                    emit autoScreenSizingChange(autoScreenSizing);
                }
            }


            // emulate
            u32 nlines;
            if (emuInstance->nds->GPU.GetRenderer3D().NeedsShaderCompile())
            {
                compileShaders();
                nlines = 1;
            }
            else
            {
                nlines = emuInstance->nds->RunFrame();
            }

            refreshPluginState();

            if (emuInstance->ndsSave)
                emuInstance->ndsSave->CheckFlush();

            if (emuInstance->gbaSave)
                emuInstance->gbaSave->CheckFlush();

            if (emuInstance->firmwareSave)
                emuInstance->firmwareSave->CheckFlush();

            if (!useOpenGL)
            {
                frontBufferLock.lock();
                frontBuffer = emuInstance->nds->GPU.FrontBuffer;
                frontBufferLock.unlock();
            }
            else
            {
                frontBuffer = emuInstance->nds->GPU.FrontBuffer;

                if (shouldRenderFrame)
                {
                    emuInstance->plugin->buildShapes();
                    emuInstance->drawScreenGL();
                }
            }

#ifdef MELONCAP
            MelonCap::Update();
#endif // MELONCAP

            winUpdateCount++;
            if (winUpdateCount >= winUpdateFreq && !useOpenGL)
            {
                emit windowUpdate();
                winUpdateCount = 0;
            }
            
            if (emuInstance->hotkeyPressed(HK_FastForwardToggle)) emuInstance->fastForwardToggled = !emuInstance->fastForwardToggled;
            if (emuInstance->hotkeyPressed(HK_SlowMoToggle)) emuInstance->slowmoToggled = !emuInstance->slowmoToggled;

            bool enablefastforward = (emuInstance->hotkeyDown(HK_FastForward) | emuInstance->fastForwardToggled) || pluginShouldFastForward();
            bool enableslowmo = emuInstance->hotkeyDown(HK_SlowMo) | emuInstance->slowmoToggled;

            if (useOpenGL)
            {
                // when using OpenGL: when toggling fast-forward or slowmo, change the vsync interval
                if ((enablefastforward || enableslowmo) && !(fastforward || slowmo))
                {
                    emuInstance->setVSyncGL(false);
                }
                else if (!(enablefastforward || enableslowmo) && (fastforward || slowmo))
                {
                    emuInstance->setVSyncGL(true);
                }
            }

            fastforward = enablefastforward;
            slowmo = enableslowmo;

            if (slowmo) emuInstance->curFPS = emuInstance->slowmoFPS;
            else if (fastforward) emuInstance->curFPS = emuInstance->fastForwardFPS;
            else if (!emuInstance->doLimitFPS && !emuInstance->doAudioSync) emuInstance->curFPS = 1000.0;
            else emuInstance->curFPS = emuInstance->targetFPS;

            if (emuInstance->audioDSiVolumeSync && emuInstance->nds->ConsoleType == 1)
            {
                DSi* dsi = static_cast<DSi*>(emuInstance->nds);
                u8 volumeLevel = dsi->I2C.GetBPTWL()->GetVolumeLevel();
                if (volumeLevel != dsiVolumeLevel)
                {
                    dsiVolumeLevel = volumeLevel;
                    emit syncVolumeLevel();
                }

                emuInstance->audioVolume = volumeLevel * (256.0 / 31.0);
            }

            if (emuInstance->doAudioSync && !(fastforward || slowmo))
                emuInstance->audioSync();

            double frametimeStep = nlines / (emuInstance->curFPS * 263.0);

            if (frametimeStep < 0.001) frametimeStep = 0.001;

            if (emuInstance->doLimitFPS)
            {
                double curtime = SDL_GetPerformanceCounter() * perfCountsSec;

                frameLimitError += frametimeStep - (curtime - lastTime);
                if (frameLimitError < -frametimeStep)
                    frameLimitError = -frametimeStep;
                if (frameLimitError > frametimeStep)
                    frameLimitError = frametimeStep;

                if (round(frameLimitError * 1000.0) > 0.0)
                {
                    SDL_Delay(round(frameLimitError * 1000.0));
                    double timeBeforeSleep = curtime;
                    curtime = SDL_GetPerformanceCounter() * perfCountsSec;
                    frameLimitError -= curtime - timeBeforeSleep;
                }

                lastTime = curtime;
            }

            nframes++;
            if (nframes >= 30)
            {
                double time = SDL_GetPerformanceCounter() * perfCountsSec;
                double dt = time - lastMeasureTime;
                lastMeasureTime = time;

                u32 fps = round(nframes / dt);
                nframes = 0;

                float fpstarget = 1.0/frametimeStep;

                winUpdateFreq = fps / (u32)round(fpstarget);
                if (winUpdateFreq < 1)
                    winUpdateFreq = 1;
                    
                double actualfps = (59.8261 * 263.0) / nlines;
                snprintf(melontitle, sizeof(melontitle), "[%d/%.0f] khDaysMM " MELONDS_VERSION, fps, actualfps);
                changeWindowTitle(melontitle);
            }
        }
        else
        {
            // paused
            nframes = 0;
            lastTime = SDL_GetPerformanceCounter() * perfCountsSec;
            lastMeasureTime = lastTime;

            emit windowUpdate();

            snprintf(melontitle, sizeof(melontitle), "khDaysMM " MELONDS_VERSION);
            changeWindowTitle(melontitle);

            SDL_Delay(75);

            if (useOpenGL)
            {
                emuInstance->drawScreenGL();
            }

            if (emuInstance->plugin != nullptr) {
                refreshPluginState();
            }
        }

        handleMessages();

        //Lua Script Stuff (-for now happens at the end of each frame regardless of emuStatus)
        emit signalLuaUpdate();
    }
}

void EmuThread::sendMessage(Message msg)
{
    msgMutex.lock();
    msgQueue.enqueue(msg);
    msgMutex.unlock();
}

void EmuThread::waitMessage(int num)
{
    if (QThread::currentThread() == this) return;
    msgSemaphore.acquire(num);
}

void EmuThread::waitAllMessages()
{
    if (QThread::currentThread() == this) return;
    while (!msgQueue.empty())
        msgSemaphore.acquire();
}

void EmuThread::handleMessages()
{
    bool glborrow = false;

    msgMutex.lock();
    while (!msgQueue.empty())
    {
        Message msg = msgQueue.dequeue();
        switch (msg.type)
        {
        case msg_Exit:
            emuStatus = emuStatus_Exit;
            emuPauseStack = emuPauseStackRunning;

            emuInstance->audioDisable();
            MPInterface::Get().End(emuInstance->instanceID);
            break;

        case msg_EmuRun:
            emuStatus = emuStatus_Running;
            emuPauseStack = emuPauseStackRunning;
            emuActive = true;

            emuInstance->audioEnable();
            emit windowEmuStart();
            break;

        case msg_EmuPause:
            emuPauseStack++;
            if (emuPauseStack > emuPauseStackPauseThreshold) break;

            prevEmuStatus = emuStatus;
            emuStatus = emuStatus_Paused;

            if (prevEmuStatus != emuStatus_Paused)
            {
                emuInstance->audioDisable();
                emit windowEmuPause(true);
                emuInstance->osdAddMessage(0, "Paused");
            }
            break;

        case msg_EmuUnpause:
            if (emuPauseStack < emuPauseStackPauseThreshold) break;

            emuPauseStack--;
            if (emuPauseStack >= emuPauseStackPauseThreshold) break;

            emuStatus = prevEmuStatus;

            if (emuStatus != emuStatus_Paused)
            {
                emuInstance->audioEnable();
                emit windowEmuPause(false);
                emuInstance->osdAddMessage(0, "Resumed");
            }
            break;

        case msg_EmuStop:
            if (msg.param.value<bool>())
                emuInstance->nds->Stop();
            emuStatus = emuStatus_Paused;
            emuActive = false;

            emuInstance->audioDisable();
            emit windowEmuStop();
            break;

        case msg_EmuFrameStep:
            emuStatus = emuStatus_FrameStep;
            break;

        case msg_EmuReset:
            emuInstance->reset();

            emuStatus = emuStatus_Running;
            emuPauseStack = emuPauseStackRunning;
            emuActive = true;

            emuInstance->audioEnable();
            emit windowEmuReset();
            emuInstance->osdAddMessage(0, "Reset");
            break;

        case msg_InitGL:
            emuInstance->initOpenGL(msg.param.value<int>());
            useOpenGL = true;
            break;

        case msg_DeInitGL:
            emuInstance->deinitOpenGL(msg.param.value<int>());
            if (msg.param.value<int>() == 0)
                useOpenGL = false;
            break;

        case msg_BorrowGL:
            emuInstance->releaseGL();
            glborrow = true;
            break;

        case msg_BootROM:
            msgResult = 0;
            if (!emuInstance->loadROM(msg.param.value<QStringList>(), true, msgError))
                break;

            assert(emuInstance->nds != nullptr);
            emuInstance->nds->Start();
            msgResult = 1;
            break;

        case msg_BootFirmware:
            msgResult = 0;
            if (!emuInstance->bootToMenu(msgError))
                break;

            assert(emuInstance->nds != nullptr);
            emuInstance->nds->Start();
            msgResult = 1;
            break;

        case msg_InsertCart:
            msgResult = 0;
            if (!emuInstance->loadROM(msg.param.value<QStringList>(), false, msgError))
                break;

            msgResult = 1;
            break;

        case msg_EjectCart:
            emuInstance->ejectCart();
            break;

        case msg_InsertGBACart:
            msgResult = 0;
            if (!emuInstance->loadGBAROM(msg.param.value<QStringList>(), msgError))
                break;

            msgResult = 1;
            break;

        case msg_InsertGBAAddon:
            msgResult = 0;
            emuInstance->loadGBAAddon(msg.param.value<int>(), msgError);
            msgResult = 1;
            break;

        case msg_EjectGBACart:
            emuInstance->ejectGBACart();
            break;

        case msg_SaveState:
            msgResult = emuInstance->saveState(msg.param.value<QString>().toStdString());
            break;

        case msg_LoadState:
            msgResult = emuInstance->loadState(msg.param.value<QString>().toStdString());
            break;

        case msg_UndoStateLoad:
            emuInstance->undoStateLoad();
            msgResult = 1;
            break;

        case msg_ImportSavefile:
            {
                msgResult = 0;
                auto f = Platform::OpenFile(msg.param.value<QString>().toStdString(), Platform::FileMode::Read);
                if (!f) break;

                u32 len = FileLength(f);

                std::unique_ptr<u8[]> data = std::make_unique<u8[]>(len);
                Platform::FileRewind(f);
                Platform::FileRead(data.get(), len, 1, f);

                assert(emuInstance->nds != nullptr);
                emuInstance->nds->SetNDSSave(data.get(), len);

                CloseFile(f);
                msgResult = 1;
            }
            break;

        case msg_EnableCheats:
            emuInstance->enableCheats(msg.param.value<bool>());
            break;
        }

        msgSemaphore.release();
    }
    msgMutex.unlock();

    if (glborrow)
    {
        glBorrowMutex.lock();
        glBorrowCond.wait(&glBorrowMutex);
        glBorrowMutex.unlock();
    }
}

bool EmuThread::pluginShouldFastForward()
{
    return emuInstance->plugin->ShouldTerminateIngameCutscene() && emuInstance->plugin->RunningReplacementCutscene();
}

void EmuThread::refreshPluginGameScene()
{
    bool updated = emuInstance->plugin->refreshGameScene();
    if (updated && SHOW_GAME_SCENE)
    {
        emuInstance->osdAddMessage(0, emuInstance->plugin->getGameSceneName());
    }
}

void EmuThread::refreshPluginState()
{
    bool enableInvisibleFastMode = false;
    bool disableInvisibleFastMode = false;

     // Background music replacement
    {
        auto* plugin = emuInstance->plugin;

        if (plugin->shouldStopBackgroundMusic()) {
            u16 bgm = plugin->getBackgroundMusicToStop();
            bool bShouldStoreResumePos = plugin->getStoreBackgroundMusicPosition();
            u32 fadeOutDuration = plugin->getBackgroundMusicFadeOutToApply();
            emit windowStopBgmMusic(bgm, bShouldStoreResumePos, fadeOutDuration);
        }

        if (plugin->shouldStartBackgroundMusic()) {
            u16 bgm = plugin->getCurrentBackgroundMusic();
            const std::string& path = plugin->getCurrentBackgroundMusicFilePath();

            bool bShouldStoreResumePos = plugin->getResumeFromPositionBackgroundMusic();
            u8 volume = plugin->getCurrentBgmMusicVolume();
            u32 delayAtStart = plugin->getBgmDelayAtStart();
    
            QString filePath = QString::fromUtf8(path.c_str());
            emit windowStartBgmMusic(bgm, volume, bShouldStoreResumePos, delayAtStart, filePath);
        }

        if (plugin->shouldPauseBackgroundMusic()) {
            emit windowPauseBgmMusic();
        }

        if (plugin->shouldResumeBackgroundMusic()) {
            emit windowUnpauseBgmMusic();
        }

        if (plugin->shouldUpdateBackgroundMusicVolume()) {
            u8 volume = plugin->getCurrentBgmMusicVolume();
            emit windowUpdateBgmMusicVolume(volume);
        }
    }

    if (emuInstance->plugin->isMouseCursorGrabbed()) {
        QPoint point = mainWindow->panel->mapToGlobal(mainWindow->panel->rect().center());
        QCursor::setPos(point);
    }
    if (emuInstance->plugin->ShouldGrabMouseCursor()) {
        mainWindow->panel->setMouseTracking(true);
        mainWindow->panel->grabMouse();
        mainWindow->panel->setCursor(Qt::BlankCursor);
    }
    if (emuInstance->plugin->ShouldReleaseMouseCursor()) {
        mainWindow->panel->unsetCursor();
        mainWindow->panel->releaseMouse();
    }


    if (emuInstance->plugin->ShouldStopReplacementCutscene()) {
        emit windowStopVideo();
    }

    if (emuInstance->plugin->ShouldPauseReplacementCutscene()) {
        emit windowPauseVideo();
    }

    if (emuInstance->plugin->ShouldUnpauseReplacementCutscene()) {
        emit windowUnpauseVideo();
    }

    if (emuInstance->plugin->ShouldReturnToGameAfterCutscene()) {
        emuStatus = emuStatus_Running;
        disableInvisibleFastMode = true;
    }

    if (emuInstance->plugin->ShouldUnmuteAfterCutscene()) {
        emuStatus = emuStatus_Running;
        disableInvisibleFastMode = true;
    }


    if (emuInstance->plugin->ShouldTerminateIngameCutscene()) {
        enableInvisibleFastMode = true;
    }

    if (emuInstance->plugin->ShouldStartReplacementCutscene()) {
        auto cutscene = emuInstance->plugin->CurrentCutscene();
        if (cutscene != nullptr) {
            std::string path = emuInstance->plugin->replacementCutsceneFilePath(cutscene);
            if (path != "") {
                // disabling fast-foward, otherwise it will affect the cutscenes
                emuInstance->setVSyncGL(true);

                emuStatus = emuStatus_Paused;
                QString filePath = QString::fromUtf8(path.c_str());
                emit windowStartVideo(filePath);
            }
        }
    }

    if (emuInstance->plugin->StartedReplacementCutscene()) {
        emuStatus = emuStatus_Running;
        enableInvisibleFastMode = true;
    }

    if (emuInstance->plugin->StoppedIngameCutscene()) {
        emuStatus = emuStatus_Paused;
    }

    if (enableInvisibleFastMode)
    {
        emuInstance->audioVolume = 0;

        emuInstance->targetFPS = 1000.0;

        int newVideoRenderer = renderer3D_Software;
        if (videoRenderer != newVideoRenderer) {
            videoRenderer = newVideoRenderer;
            updateRenderer();
        }
    }
    if (disableInvisibleFastMode)
    {
        Config::Table& globalCfg = emuInstance->getGlobalConfig();

        auto& instcfg = emuInstance->getLocalConfig();
        emuInstance->audioVolume = instcfg.GetInt("Audio.Volume");

        emuInstance->targetFPS = globalCfg.GetDouble("TargetFPS");

        int newVideoRenderer = globalCfg.GetInt("3D.Renderer");
        if (videoRenderer != newVideoRenderer) {
            videoRenderer = newVideoRenderer;
            updateRenderer();
        }
    }
}

void EmuThread::changeWindowTitle(char* title)
{
    emit windowTitleChange(QString(title));
}

void EmuThread::initContext(int win)
{
    sendMessage({.type = msg_InitGL, .param = win});
    waitMessage();
}

void EmuThread::deinitContext(int win)
{
    sendMessage({.type = msg_DeInitGL, .param = win});
    waitMessage();
}

void EmuThread::borrowGL()
{
    sendMessage(msg_BorrowGL);
    waitMessage();
}

void EmuThread::returnGL()
{
    glBorrowMutex.lock();
    glBorrowCond.wakeAll();
    glBorrowMutex.unlock();
}

void EmuThread::emuRun()
{
    sendMessage(msg_EmuRun);
    waitMessage();
}

void EmuThread::emuPause(bool broadcast)
{
    if (emuInstance->plugin != nullptr && emuInstance->plugin->isReady() && emuInstance->plugin->togglePause()) {
        return;
    }

    sendMessage(msg_EmuPause);
    waitMessage();

    if (broadcast)
        emuInstance->broadcastCommand(InstCmd_Pause);
}

void EmuThread::emuUnpause(bool broadcast)
{
    if (emuInstance->plugin != nullptr && emuInstance->plugin->isReady() && emuInstance->plugin->togglePause()) {
        return;
    }

    sendMessage(msg_EmuUnpause);
    waitMessage();

    if (broadcast)
        emuInstance->broadcastCommand(InstCmd_Unpause);
}

void EmuThread::emuTogglePause(bool broadcast)
{
    if (emuStatus == emuStatus_Paused)
        emuUnpause(broadcast);
    else
        emuPause(broadcast);
}

void EmuThread::emuStop(bool external)
{
    sendMessage({.type = msg_EmuStop, .param = external});
    waitMessage();
}

void EmuThread::emuExit()
{
    sendMessage(msg_Exit);
    waitAllMessages();
}

void EmuThread::emuFrameStep()
{
    if (emuPauseStack < emuPauseStackPauseThreshold)
        sendMessage(msg_EmuPause);
    sendMessage(msg_EmuFrameStep);
    waitAllMessages();
}

void EmuThread::emuReset()
{
    sendMessage(msg_EmuReset);
    waitMessage();
}

bool EmuThread::emuIsRunning()
{
    return emuStatus == emuStatus_Running;
}

bool EmuThread::emuIsActive()
{
    return emuActive;
}

int EmuThread::bootROM(const QStringList& filename, QString& errorstr)
{
    sendMessage({.type = msg_BootROM, .param = filename});
    waitMessage();
    if (!msgResult)
    {
        errorstr = msgError;
        return msgResult;
    }

    sendMessage(msg_EmuRun);
    waitMessage();
    errorstr = "";
    return msgResult;
}

int EmuThread::bootFirmware(QString& errorstr)
{
    sendMessage(msg_BootFirmware);
    waitMessage();
    if (!msgResult)
    {
        errorstr = msgError;
        return msgResult;
    }

    sendMessage(msg_EmuRun);
    waitMessage();
    errorstr = "";
    return msgResult;
}

int EmuThread::insertCart(const QStringList& filename, bool gba, QString& errorstr)
{
    MessageType msgtype = gba ? msg_InsertGBACart : msg_InsertCart;

    sendMessage({.type = msgtype, .param = filename});
    waitMessage();
    errorstr = msgResult ? "" : msgError;
    return msgResult;
}

void EmuThread::ejectCart(bool gba)
{
    sendMessage(gba ? msg_EjectGBACart : msg_EjectCart);
    waitMessage();
}

int EmuThread::insertGBAAddon(int type, QString& errorstr)
{
    sendMessage({.type = msg_InsertGBAAddon, .param = type});
    waitMessage();
    errorstr = msgResult ? "" : msgError;
    return msgResult;
}

int EmuThread::saveState(const QString& filename)
{
    sendMessage({.type = msg_SaveState, .param = filename});
    waitMessage();
    return msgResult;
}

int EmuThread::loadState(const QString& filename)
{
    sendMessage({.type = msg_LoadState, .param = filename});
    waitMessage();
    return msgResult;
}

int EmuThread::undoStateLoad()
{
    sendMessage(msg_UndoStateLoad);
    waitMessage();
    return msgResult;
}

int EmuThread::importSavefile(const QString& filename)
{
    sendMessage(msg_EmuReset);
    sendMessage({.type = msg_ImportSavefile, .param = filename});
    waitMessage(2);
    return msgResult;
}

void EmuThread::enableCheats(bool enable)
{
    sendMessage({.type = msg_EnableCheats, .param = enable});
    waitMessage();
}

void EmuThread::updateRenderer()
{
    if (videoRenderer != lastVideoRenderer)
    {
        switch (videoRenderer)
        {
            case renderer3D_Software:
                emuInstance->nds->GPU.SetRenderer3D(std::make_unique<SoftRenderer>());
                break;
            case renderer3D_OpenGL:
                emuInstance->nds->GPU.SetRenderer3D(GLRenderer::New(emuInstance->plugin));
                break;
            case renderer3D_OpenGLCompute:
                emuInstance->nds->GPU.SetRenderer3D(ComputeRenderer::New(emuInstance->plugin));
                break;
            default: __builtin_unreachable();
        }
    }
    lastVideoRenderer = videoRenderer;

    auto& cfg = emuInstance->getGlobalConfig();
    switch (videoRenderer)
    {
        case renderer3D_Software:
            static_cast<SoftRenderer&>(emuInstance->nds->GPU.GetRenderer3D()).SetThreaded(
                    cfg.GetBool("3D.Soft.Threaded"),
                    emuInstance->nds->GPU);
            break;
        case renderer3D_OpenGL:
            static_cast<GLRenderer&>(emuInstance->nds->GPU.GetRenderer3D()).SetRenderSettings(
                    cfg.GetBool("3D.GL.BetterPolygons"),
                    cfg.GetInt("3D.GL.ScaleFactor"));
            break;
        case renderer3D_OpenGLCompute:
            static_cast<ComputeRenderer&>(emuInstance->nds->GPU.GetRenderer3D()).SetRenderSettings(
                    cfg.GetInt("3D.GL.ScaleFactor"),
                    cfg.GetBool("3D.GL.HiresCoordinates"));
            break;
        default: __builtin_unreachable();
    }
}

void EmuThread::compileShaders()
{
    int currentShader, shadersCount;
    u64 startTime = SDL_GetPerformanceCounter();
    // kind of hacky to look at the wallclock, though it is easier than
    // than disabling vsync
    do
    {
        emuInstance->nds->GPU.GetRenderer3D().ShaderCompileStep(currentShader, shadersCount);
    } while (emuInstance->nds->GPU.GetRenderer3D().NeedsShaderCompile() &&
             (SDL_GetPerformanceCounter() - startTime) * perfCountsSec < 1.0 / 6.0);
    emuInstance->osdAddMessage(0, "Compiling shader %d/%d", currentShader+1, shadersCount);
}
