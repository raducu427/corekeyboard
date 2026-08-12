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

#include <QtWidgets>

#include <src/settings.h>

typedef QMap<QString, QStringList>   Page;
typedef QMap<int, Page>              Keymap;

extern const QString appID;

class CoreKeyboard : public QWidget {
	Q_OBJECT

public:
	CoreKeyboard();
	~CoreKeyboard();

private:
	/* Init the settings from cprime */
    void loadSettings();

	/* Load KeyMap */
	void loadKeymap();

	/* Prepare the layout of the keyboard */
	void relayKeyboard();

	/* Enable word suggestions */
	bool mAutoSuggest = false;

	/* Compact or fixed mode */
    bool mMode;

    /* Mobile or Desktop type */
    bool mType;

    /* App UI with half transperancy */
    bool mOpaqueMode;

    /* Show CK with or without window */
    bool mWindowMode;

	/* Font to be used for the keys */
	QFont mFont;

	/* Current page; start with 1 */
    int mPage = 1;

    /* Key Map number*/
    int keymapNumber =1;

    /* Key Map number*/
    QSize windowSize;

	Keymap mKeyMap;

	/* The keysym to character data */
	QMap<int, QStringList> keyChart;

	/* List of keys */
	QSet<int> modifiers;
	QSet<int> letters;
	int ctrl;
	int shift;
	int caps;

	/* A map of the pressed keys */
	QMap<int, bool> keypress;
	QSet<int> modPress;
	bool capsOn;
	int shiftMode;

	/* The rectangle for each key */
	QMultiMap<int, QRectF> layout;

	settings *smi;

    /* Toggle Comapct UI keyboard */
    void modeCompact();

    /* Toggle Fixed UI keyboard */
    void modeFixed();

protected:
	/* Override the default mouse button press events */
    void mousePressEvent(QMouseEvent *mEvent) override;

	/* Override the default mouse button release events */
    void mouseReleaseEvent(QMouseEvent *mEvent) override;

	/* Relay the layout on keyboard resize */
    void resizeEvent(QResizeEvent *rEvent) override;

	/* Paint the keyboard */
    void paintEvent(QPaintEvent *pEvent) override;

    void changeEvent(QEvent *event) override;

    void closeEvent(QCloseEvent *cEvent) override;

public Q_SLOTS:
	/* Toggle between minimized to tray and visible states */
	void toggleShowHide();

	/* Switch between fixed and compact modes */
	void switchMode();

    /* Toggle between Autto suggestion states */
    void toggleAutosuggest();

    /* Switch between mobile and desktop keyboard */
    void switchType();

	/**  Change the keymap */
    void keymapSwitch(int n);
};
