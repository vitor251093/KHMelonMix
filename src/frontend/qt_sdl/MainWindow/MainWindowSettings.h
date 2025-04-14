/*
    Copyright 2016-2023 melonDS team

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

#ifndef MAINWINDOWSETTINGS_H
#define MAINWINDOWSETTINGS_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QAudioOutput>
#include <QPushButton>
#include <initializer_list>

#include "Config.h"

class EmuInstance;

namespace melonMix {
class AudioPlayer;
}

namespace Ui { class MainWindowSettings; }
class MainWindowSettings;

class MainWindowSettings : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindowSettings(EmuInstance* inst, QWidget* parent);
    ~MainWindowSettings();

public slots:
    void asyncStartBgmMusic(QString bgmMusicFilePath);
    void asyncStopBgmMusic();
    void asyncPauseBgmMusic();
    void asyncUnpauseBgmMusic();

    void startBgmMusic(QString bgmMusicFilePath);
    void stopBgmMusic();
    void pauseBgmMusic();
    void unpauseBgmMusic();


    void asyncStartVideo(QString videoFilePath);
    void asyncStopVideo();
    void asyncPauseVideo();
    void asyncUnpauseVideo();

    void startVideo(QString videoFilePath);
    void stopVideo();
    void pauseVideo();
    void unpauseVideo();

protected:
    void keyPressEvent(QKeyEvent* event) override;

    QWidget* settingsWidget;
    QStackedWidget* settingWidgetOptions;
    bool showingSettings;

    void initWidgets();

    virtual void showGame() = 0;

private:
    QScopedPointer<Ui::MainWindowSettings> ui;
    Config::Table& localCfg;
    EmuInstance* emuInstance;

    QScopedPointer<QVideoWidget> playerWidget;
    QScopedPointer<QAudioOutput> playerAudioOutput;
    QScopedPointer<QMediaPlayer> player;

    QScopedPointer<melonMix::AudioPlayer> bgmPlayer;

    void createVideoPlayer();

};

#endif // MAINWINDOWSETTINGS_H
