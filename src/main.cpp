/*
  *
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

#include "corekeyboard.h"
#include "trayicon.h"
#include "settings.h"

#include <QApplication>
#include <QMessageBox>

#include <cprime/capplication.h>

const QString appID = QStringLiteral(APP_ID);


int main(int argc, char **argv)
{
	CPrime::CApplication app("CoreKeyboard", argc, argv);

	// Set application info
	app.setOrganizationName("CuboCore");
	app.setApplicationName("CoreKeyboard");
	app.setApplicationVersion(QStringLiteral(VERSION_TEXT));
	app.setDesktopFileName(appID);
	app.setQuitOnLastWindowClosed(false);

	CoreKeyboard k;

    QObject::connect(&app, &CPrime::CApplication::messageReceived, [&k]() {
        if(k.isVisible()){
            k.hide();
        } else{
            k.show();
        }
	});

	if (app.isRunning())
	{
        return not app.sendMessage("App is already running. Using existing Instance.");
	}

    settings smi;
    if (( bool )smi.getValue("CoreKeyboard", "DaemonMode") == false)
    {
        k.show();
	}

	/* Start the tray icon */
	trayicon tray(&k);

	/** Connect the signals that we've defined in trayicon class to respective slots in CoreKeyboard class */
	QObject::connect(&tray, &trayicon::toggleShowHide, &k, &CoreKeyboard::toggleShowHide);
    QObject::connect(&tray, &trayicon::switchMode, &k, &CoreKeyboard::switchMode);
    QObject::connect(&tray, &trayicon::switchType, &k, &CoreKeyboard::switchType);
    QObject::connect(&tray, &trayicon::toggleAutosuggest, &k, &CoreKeyboard::toggleAutosuggest);
    QObject::connect(&tray, &trayicon::keymapSwitch, &k, &CoreKeyboard::keymapSwitch);

	tray.show();

	return app.exec();
}
