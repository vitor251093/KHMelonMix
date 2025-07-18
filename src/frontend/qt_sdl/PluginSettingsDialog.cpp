/*
    Copyright 2016-2024 melonDS team

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

#include <QFileDialog>
#include <QtGlobal>

#include "types.h"
#include "Platform.h"
#include "Config.h"
#include "GPU.h"
#include "main.h"

#include "PluginSettingsDialog.h"
#include "ui_PluginSettingsDialog.h"



PluginSettingsDialog* PluginSettingsDialog::currentDlg = nullptr;

void PluginSettingsDialog::setEnabled()
{
    if (!isPluginLoaded)
    {
        ui->cbSingleScreenMode->setEnabled(false);
        ui->cbFFLoadingScreens->setEnabled(false);
        ui->cbxAudioPack->setEnabled(false);
        ui->sbHUDSize->setEnabled(false);
        ui->sbCameraSensitivity->setEnabled(false);
        return;
    }

    auto plugin = emuInstance->plugin;
    std::string root = plugin->tomlUniqueIdentifier();

    auto& cfg = emuInstance->getGlobalConfig();
    bool singleScreenModeEnabled = !cfg.GetBool(root + ".DisableEnhancedGraphics");

    ui->sbHUDSize->setEnabled(singleScreenModeEnabled);
}

PluginSettingsDialog::PluginSettingsDialog(QWidget* parent) : QDialog(parent), ui(new Ui::PluginSettingsDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    emuInstance = ((MainWindow*)parent)->getEmuInstance();

    auto plugin = emuInstance->plugin;
    isPluginLoaded = plugin != nullptr;

    if (isPluginLoaded)
    {
        std::string root = plugin->tomlUniqueIdentifier();

        auto& cfg = emuInstance->getGlobalConfig();
        oldSingleScreenMode = !cfg.GetBool(root + ".DisableEnhancedGraphics");
        oldFFLoadingScreens = cfg.GetBool(root + ".FastForwardLoadingScreens");
        oldAudioPack = cfg.GetString(root + ".AudioPack");
        oldHUDSize = cfg.GetInt(root + ".HUDScale");
        oldHUDSize = (oldHUDSize == 0) ? 4 : oldHUDSize;
        oldCameraSensitivity = cfg.GetInt(root + ".CameraSensitivity");
        oldCameraSensitivity = (oldCameraSensitivity == 0) ? plugin->DefaultCameraSensitivity : oldCameraSensitivity;

        ui->cbSingleScreenMode->setChecked(oldSingleScreenMode != 0);
        ui->cbFFLoadingScreens->setChecked(oldFFLoadingScreens != 0);

        auto audioPackNames = plugin->audioPackNames();
        ui->cbxAudioPack->addItem(QString::fromStdString("None"));
        for (const std::string& name : audioPackNames) {
            ui->cbxAudioPack->addItem(QString::fromStdString(name));
        }
        QString qSelected = QString::fromStdString(oldAudioPack);
        int index = ui->cbxAudioPack->findText(qSelected);
        if (index >= 0) {
            ui->cbxAudioPack->setCurrentIndex(index);
        }

        ui->sbHUDSize->setValue(oldHUDSize);
        ui->sbCameraSensitivity->setValue(oldCameraSensitivity);
    }

    setEnabled();
}

PluginSettingsDialog::~PluginSettingsDialog()
{
    delete ui;
}

void PluginSettingsDialog::on_PluginSettingsDialog_accepted()
{
    Config::Save();

    closeDlg();
}

void PluginSettingsDialog::on_PluginSettingsDialog_rejected()
{
    if (!((MainWindow*)parent())->getEmuInstance())
    {
        closeDlg();
        return;
    }

    if (!isPluginLoaded)
    {
        closeDlg();
        return;
    }

    auto plugin = emuInstance->plugin;
    std::string root = plugin->tomlUniqueIdentifier();

    auto& cfg = emuInstance->getGlobalConfig();
    cfg.SetBool(root + ".DisableEnhancedGraphics", !oldSingleScreenMode);
    cfg.SetBool(root + ".FastForwardLoadingScreens", oldFFLoadingScreens);
    cfg.SetString(root + ".AudioPack", oldAudioPack);
    cfg.SetInt(root + ".HUDScale", oldHUDSize);
    cfg.SetInt(root + ".CameraSensitivity", oldCameraSensitivity);

    emit updatePluginSettings();

    closeDlg();
}

void PluginSettingsDialog::on_cbSingleScreenMode_stateChanged(int state)
{
    auto plugin = emuInstance->plugin;
    std::string root = plugin->tomlUniqueIdentifier();

    auto& cfg = emuInstance->getGlobalConfig();
    cfg.SetBool(root + ".DisableEnhancedGraphics", state == 0);

    emit updatePluginSettings();
}

void PluginSettingsDialog::on_cbFFLoadingScreens_stateChanged(int state)
{
    auto plugin = emuInstance->plugin;
    std::string root = plugin->tomlUniqueIdentifier();

    auto& cfg = emuInstance->getGlobalConfig();
    cfg.SetBool(root + ".FastForwardLoadingScreens", state != 0);

    emit updatePluginSettings();
}

void PluginSettingsDialog::on_cbxAudioPack_currentTextChanged(const QString &text)
{
    auto plugin = emuInstance->plugin;
    std::string root = plugin->tomlUniqueIdentifier();

    auto& cfg = emuInstance->getGlobalConfig();
    cfg.SetString(root + ".AudioPack", text.toStdString());

    emit updatePluginSettings();
}

void PluginSettingsDialog::on_sbHUDSize_valueChanged(int val)
{
    auto plugin = emuInstance->plugin;
    std::string root = plugin->tomlUniqueIdentifier();

    auto& cfg = emuInstance->getGlobalConfig();
    cfg.SetInt(root + ".HUDScale", val);

    emit updatePluginSettings();
}

void PluginSettingsDialog::on_sbCameraSensitivity_valueChanged(int val)
{
    auto plugin = emuInstance->plugin;
    std::string root = plugin->tomlUniqueIdentifier();

    auto& cfg = emuInstance->getGlobalConfig();
    cfg.SetInt(root + ".CameraSensitivity", val);

    emit updatePluginSettings();
}
