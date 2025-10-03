/*
    Copyright 2025 Melon Mix team

    This file is part of Melon Mix.

    Melon Mix is free software: you can redistribute it and/or modify it under
    the terms of the GNU General Public License as published by the Free
    Software Foundation, either version 3 of the License, or (at your option)
    any later version.

    Melon Mix is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with Melon Mix. If not, see http://www.gnu.org/licenses/.
*/

#ifndef PLUGINSETTINGSDIALOG_H
#define PLUGINSETTINGSDIALOG_H

#include <QDialog>
#include <QButtonGroup>

namespace Ui { class PluginSettingsDialog; }
class PluginSettingsDialog;
class EmuInstance;

class PluginSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PluginSettingsDialog(QWidget* parent);
    ~PluginSettingsDialog();

    static PluginSettingsDialog* currentDlg;
    static PluginSettingsDialog* openDlg(QWidget* parent)
    {
        if (currentDlg)
        {
            currentDlg->activateWindow();
            return currentDlg;
        }

        currentDlg = new PluginSettingsDialog(parent);
        currentDlg->show();
        return currentDlg;
    }
    static void closeDlg()
    {
        currentDlg = nullptr;
    }

signals:
    void updatePluginSettings();

private slots:
    void on_PluginSettingsDialog_accepted();
    void on_PluginSettingsDialog_rejected();

    void on_cbEnhancedGraphics_stateChanged(int state);
    void on_cbSingleScreenMode_stateChanged(int state);
    void on_cbFFLoadingScreens_stateChanged(int state);
    void on_cbxAudioPack_currentTextChanged(const QString &text);

    void on_sbHUDSize_valueChanged(int value);
    void on_sbCameraSensitivity_valueChanged(int value);
private:
    void setEnabled();

    Ui::PluginSettingsDialog* ui;
    EmuInstance* emuInstance;

    bool isPluginLoaded;
    int oldEnhancedGraphics;
    int oldSingleScreenMode;
    int oldFFLoadingScreens;
    std::string oldAudioPack;
    int oldHUDSize;
    int oldCameraSensitivity;
};

#endif // PLUGINSETTINGSDIALOG_H

