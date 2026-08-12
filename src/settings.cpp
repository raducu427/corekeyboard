/*
  * This file is a part of CoreKeyboard.
  * An on-screenkeyboard for C Suite.
  * Copyright 2019 CuboCore Group
  *
  * This program is free software; you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation; either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, see {http://www.gnu.org/licenses/}.
  */


#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QFontDatabase>

#include <cprime/variables.h>
#include <cprime/themefunc.h>
#include <cprime/filefunc.h>

#include "settings.h"

settings::settings()
{
    defaultSett = QDir(CPrime::Variables::CC_Library_ConfigDir()).filePath("coreapps.conf");
    cSetting = new QSettings(defaultSett, QSettings::NativeFormat);

    // set some default settings that are user specific
    if (not CPrime::FileUtils::exists(defaultSett)) {
        qDebug() << "Settings file " << cSetting->fileName();
        CPrime::FileUtils::setupFolder(CPrime::FolderSetup::ConfigFolder);
    }

    checkAndSetCSuiteEntries(*cSetting);
    setAppDefaultSettings(*cSetting);

}

settings::~settings()
{
    delete cSetting;
}

// Check CSuite's default settings
void settings::checkAndSetCSuiteEntries(QSettings& settings)
{
    // Check if "CoreApps/KeepActivities" exists
    if (not settings.contains("CoreApps/KeepActivities")) {
        settings.setValue("CoreApps/KeepActivities", true);
    }

    // Check if "CoreApps/EnableExperimental" exists
    if (not settings.contains("CoreApps/EnableExperimental")) {
        settings.setValue("CoreApps/EnableExperimental", false);
    }

    // Check if "CoreApps/AutoDetect" exists
    if (not settings.contains("CoreApps/AutoDetect")) {
        settings.setValue("CoreApps/AutoDetect", true);
    }

    // Check if "CoreApps/DisableTrashConfirmationMessage" exists
    if (not settings.contains("CoreApps/DisableTrashConfirmationMessage")) {
        settings.setValue("CoreApps/DisableTrashConfirmationMessage", false);
    }

    if (autoUIMode() == 2) {
        // Check if "CoreApps/IconViewIconSize" exists
        if (not settings.contains("CoreApps/IconViewIconSize")) {
            settings.setValue("CoreApps/IconViewIconSize", QSize(56, 56));
        }

        // Check if "CoreApps/ListViewIconSize" exists
        if (not settings.contains("CoreApps/ListViewIconSize")) {
            settings.setValue("CoreApps/ListViewIconSize", QSize(48, 48));
        }

        // Check if "CoreApps/ToolsIconSize" exists
        if (not settings.contains("CoreApps/ToolsIconSize")) {
            settings.setValue("CoreApps/ToolsIconSize", QSize(48, 48));
        }

    } else {
        // Check if "CoreApps/IconViewIconSize" exists
        if (not settings.contains("CoreApps/IconViewIconSize")) {
            settings.setValue("CoreApps/IconViewIconSize", QSize(48, 48));
        }

        // Check if "CoreApps/ListViewIconSize" exists
        if (not settings.contains("CoreApps/ListViewIconSize")) {
            settings.setValue("CoreApps/ListViewIconSize", QSize(32, 32));
        }

        // Check if "CoreApps/ToolsIconSize" exists
        if (not settings.contains("CoreApps/ToolsIconSize")) {
            settings.setValue("CoreApps/ToolsIconSize", QSize(24, 24));
        }
    }

    cSetting->sync();
}


// Check app's default settings
void settings::setAppDefaultSettings(QSettings &settings)
{
    // CoreKeyboard

    // Check if "CoreKeyboard/Mode" exists
    if (not settings.contains("CoreKeyboard/Mode")) {
        settings.setValue("CoreKeyboard/Mode", true);
    }

    // Check if "CoreKeyboard/AutoSuggest" exists
    if (not settings.contains("CoreKeyboard/AutoSuggest")) {
        settings.setValue("CoreKeyboard/AutoSuggest", false);
    }

    // Check if "CoreKeyboard/DaemonMode" exists
    if (not settings.contains("CoreKeyboard/DaemonMode")) {
        settings.setValue("CoreKeyboard/DaemonMode", false);
    }

    // Check if "CoreKeyboard/KeymapNumber" exists
    if (not settings.contains("CoreKeyboard/KeymapNumber")) {
        settings.setValue("CoreKeyboard/KeymapNumber", 1);
    }

    // Check if "CoreKeyboard/Type" exists
    if (not settings.contains("CoreKeyboard/Type")) {
        settings.setValue("CoreKeyboard/Type", false);
    }

    // Check if "CoreKeyboard/WindowMode" exists
    if (not settings.contains("CoreKeyboard/WindowMode")) {
        settings.setValue("CoreKeyboard/WindowMode", false);
    }

    // Check if "CoreKeyboard/OpaqueMode" exists
    if (not settings.contains("CoreKeyboard/OpaqueMode")) {
        settings.setValue("CoreKeyboard/OpaqueMode", false);
    }

    // Check if "CoreKeyboard/WindowSize" exists
    if (!settings.contains("CoreKeyboard/WindowSize")) {
        settings.setValue("CoreKeyboard/WindowSize", QSize(400, 200));
    }

    cSetting->sync();
}

int settings::autoUIMode() const
{
    if (CPrime::ThemeFunc::getFormFactor() == CPrime::FormFactor::Mobile) {
        return 2; // Mobile
    } else if (CPrime::ThemeFunc::getFormFactor() == CPrime::FormFactor::Tablet &&  CPrime::ThemeFunc::getTouchMode() == true) {
        return 1; // Tablet
    } else {
        return 0; // Desktop
    }
}

settings::cProxy settings::getValue(const QString &appName, const QString &key,
                                    const QVariant &defaultValue)
{
    if (appName == "CoreApps" && key == "UIMode") { // Wants to get CoreApps/UIMode
        // Check whether CoreApps/AutoDetect is On
        bool isAutoDetect = cSetting->value("CoreApps/AutoDetect").toBool();

        if (isAutoDetect)
            return cProxy { cSetting, "Dummy", autoUIMode() };
    }

    return cProxy{ cSetting, appName + "/" + key, defaultValue };
}

void settings::setValue(const QString &appName, const QString &key, QVariant value)
{
    cSetting->setValue(appName + "/" + key, value);
}

QString settings::defaultSettingsFilePath() const
{
    return cSetting->fileName();
}
