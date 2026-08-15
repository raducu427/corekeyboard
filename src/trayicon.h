/*
  *
  * This file is a part of CoreKeyboard.
  * An on-screenkeyboard for C Suite.
  * Copyright 2019 CuboCore Group
  *
  *
  *
  * This program is free software; you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation; either version 3 of the License, or
  * (at your option) any later version.
  *
  *
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  *
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, see {http://www.gnu.org/licenses/}.
  *
  */

#pragma once

#include <QWidget>
#include <QMenu>
#include <QCoreApplication>
#include <QSystemTrayIcon>

#include "settings.h"

#include "corekeyboard.h"
#include "cprime/messageengine.h"

class trayicon : public QSystemTrayIcon {
	Q_OBJECT

public:
    QAction *r, *w, *o;
    settings smi;

	trayicon(QWidget *parent) : QSystemTrayIcon(parent)
	{
        setIcon(QIcon::fromTheme(appID,QIcon::fromTheme("preferences-keyboard")));
		show();

		connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(activationHandler(QSystemTrayIcon::ActivationReason)));

		QMenu *menu = new QMenu("TrayMenu");

        QMenu *kbLayouts = new QMenu("Set Layout");

        r =  new QAction("Daemon mode", this);
        r->setCheckable(true);
        r->setChecked(( bool )smi.getValue("CoreKeyboard", "DaemonMode"));
        connect(r, SIGNAL(triggered()), this, SLOT(daemonMode()));


        w =  new QAction("Window mode", this);
        w->setCheckable(true);
        w->setChecked(( bool )smi.getValue("CoreKeyboard", "WindowMode"));
        connect(w, SIGNAL(triggered()), this, SLOT(windowMode()));


        o =  new QAction("Opaque mode", this);
        o->setCheckable(true);
        o->setChecked(( bool )smi.getValue("CoreKeyboard", "OpaqueMode"));
        connect(o, SIGNAL(triggered()), this, SLOT(opaqueMode()));


        kbLayouts->addAction(QIcon(), "Romanian (RO)",this, [=](){ emit keymapSwitch(1);});
        kbLayouts->addAction(QIcon(), "English (US)",this, [=](){ emit keymapSwitch(2);});


        menu->addAction(QIcon(), "&Toggle Visible", this, &trayicon::toggleShowHide);
//        menu->addAction(QIcon(), "&Toggle Autosuggest", this, &trayicon::toggleAutosuggest);
        menu->addAction(QIcon(), "Switch &Mode", this, &trayicon::switchMode);
        menu->addAction(QIcon(), "Switch &Type", this, &trayicon::switchType);
        menu->addAction(r);
        menu->addAction(w);
        menu->addAction(o);
        menu->addMenu(kbLayouts);
		menu->addSeparator();
		menu->addAction(QIcon::fromTheme("application-quit"), "&Quit CoreKeyboard", QCoreApplication::instance(), SLOT(quit()));

		setContextMenu(menu);
	}

private Q_SLOTS:
	void activationHandler(QSystemTrayIcon::ActivationReason reason)
	{
		if (reason == QSystemTrayIcon::Trigger)
		{

			if (( int )smi.getValue("CoreApps", "UIMode") == 2)
			{
				contextMenu()->exec(QCursor::pos());
			}

			else
			{
				emit toggleShowHide();
			}
		}
	}

    void daemonMode()
    {
        if(r->isChecked())
           {
              smi.setValue("CoreKeyboard", "DaemonMode", true);
              qWarning() << "true";
           }
           else
           {
              smi.setValue("CoreKeyboard", "DaemonMode", false);
              qWarning() << "false";
           }

           CPrime::MessageEngine::showMessage(appID, "CoreKeyboard", "Need to restart the app", "Changes will take effect after the restart of the app");
    }


    void windowMode()
    {
           if(w->isChecked())
           {
              smi.setValue("CoreKeyboard", "WindowMode", true);
              qWarning() << "true";
           }
           else
           {
              smi.setValue("CoreKeyboard", "WindowMode", false);
              qWarning() << "false";
           }

           CPrime::MessageEngine::showMessage(appID, "CoreKeyboard", "Need to restart the app", "Changes will take effect after the restart of the app");
    }


    void opaqueMode()
    {
           if(o->isChecked())
           {
              smi.setValue("CoreKeyboard", "OpaqueMode", true);
              qWarning() << "true";
           }
           else
           {
              smi.setValue("CoreKeyboard", "OpaqueMode", false);
              qWarning() << "false";
           }

           CPrime::MessageEngine::showMessage(appID, "CoreKeyboard", "Need to restart the app", "Changes will take effect after the restart of the app");
    }


Q_SIGNALS:
	void toggleShowHide();
	void toggleAutosuggest();
    void switchMode();
    void minimize();
    void switchType();
	void keymapSwitch(int);
};
