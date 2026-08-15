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

/*
  * TODO
  *
  * Currently modifier keys are not supported.
  * We have a list of the modifiers in the keymap files.
  * Based on the click (short/long), we should set them into checked/locked state.
  * If the modifier is in locked state, then it will be unlocked upon a subsequent click.
  * If the modifier is in checked state, then it will be release with any mouse click.
  *
  * Record the time when the first click was registered. Then when the key is released,
  * check the time. If the elapsed time is less than the threshold, set it as checked;
  * otherwise, set it as locked. On our keyboard, simultaneous clicks are not needded.
  */

#include "corekeyboard.h"

#include <X11/extensions/XTest.h>

CoreKeyboard::CoreKeyboard() : QWidget()
	, smi(new settings)
{
    loadSettings();
	loadKeymap();
	relayKeyboard();


    mFont = QFontDatabase::systemFont(QFontDatabase::GeneralFont);

    if (not mFont.family().length())
    {
        mFont = QFont("Cantarell", 9);
    }

    setFont(mFont);

    setAttribute(Qt::WA_ShowWithoutActivating);

    if(mOpaqueMode){
        setWindowOpacity(qreal(75)/100);
    }

    if(not mWindowMode){
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus | Qt::BypassWindowManagerHint);
        setAttribute(Qt::WA_X11NetWmWindowTypeDock, true);
        setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
        setMouseTracking(true);
    } else{
        setWindowFlags(Qt::WindowStaysOnTopHint);
    }

    if (mMode){
        modeFixed();
    } else{
        modeCompact();
    }

	// For screen rotation
    QScreen *scrn = qApp->primaryScreen();

    connect(scrn, &QScreen::availableGeometryChanged, [this, scrn](const QRect& geometry) {
        qDebug() << "orientation change " << geometry;
        QSize size = scrn->size();

        if (mMode){
            resize(size.width(), size.height() * .3);
            setMinimumSize(size.width(), size.height() * .3);
            setGeometry(0, size.height() * .7, size.width(), size.height() * .3);
            qApp->processEvents();
        } else{
            setMinimumSize(size.width()*0.2, size.height()*0.15);
            resize(windowSize);
            move(size.width() - width(), size.height() - height());
            qApp->processEvents();
        }
    });
}


CoreKeyboard::~CoreKeyboard()
{
	delete smi;
}


void CoreKeyboard::loadSettings()
{
    mAutoSuggest = smi->getValue("CoreKeyboard", "AutoSuggest");
    mType = smi->getValue("CoreKeyboard", "Type");
    mMode = smi->getValue("CoreKeyboard", "Mode");
    keymapNumber = smi->getValue("CoreKeyboard", "KeymapNumber");
    mWindowMode = smi->getValue("CoreKeyboard", "WindowMode");
    mOpaqueMode = smi->getValue("CoreKeyboard", "OpaqueMode");
    windowSize = smi->getValue("CoreKeyboard", "WindowSize");
}

void CoreKeyboard::keymapSwitch(int n)
{
    keymapNumber = n;
    smi->setValue("CoreKeyboard", "KeymapNumber", keymapNumber);
    loadKeymap();
    relayKeyboard();
    show();
}


void CoreKeyboard::loadKeymap()
{
	QString keymapName;

    if (mType){
        if(keymapNumber == 1)keymapName = ":/resources/ro_RO_mobile.keymap";
        else keymapName = ":/resources/en_US_mobile.keymap";
    } else {
        if(keymapNumber == 1)keymapName = ":/resources/ro_RO.keymap";
        else keymapName = ":/resources/en_US.keymap";
    }

	QSettings keymapSett(keymapName, QSettings::IniFormat);

	/* All the KeySym to char data ins under the section Data */
	keymapSett.beginGroup("Data");

	/* Load the data to memory */
    Q_FOREACH (QString key, keymapSett.allKeys())
	{
        keyChart[key.toInt()] = keymapSett.value(key).toStringList();
	}

	/* Close the group */
	keymapSett.endGroup();

	Q_FOREACH (QString page, keymapSett.value("Pages").toStringList())
	{
		Page pg;
		Q_FOREACH (QString row, keymapSett.value("Rows").toStringList())
		{
			pg[row] = keymapSett.value(page + "/" + row).toStringList();
		}

		mKeyMap[page.toInt()] = pg;
	}

	QStringList modKeys = keymapSett.value("Modifiers").toStringList();

	Q_FOREACH (QString key, modKeys)
	{
		modifiers << key.toInt();
	}

	QStringList letterKeys = keymapSett.value("Letters").toStringList();

	Q_FOREACH (QString key, letterKeys)
	{
		letters << key.toInt();
	}

	caps = keymapSett.value("Caps").toInt();
	ctrl = keymapSett.value("Ctrl").toInt();
	shift = keymapSett.value("Shift").toInt();
}


void CoreKeyboard::relayKeyboard()
{
	layout.clear();
	keypress.clear();

	int cols = 0;

	Q_FOREACH (QString row, mKeyMap[mPage].keys())
	{
		cols = (mKeyMap[mPage][row].count() > cols ? mKeyMap[mPage][row].count() : cols);
	}

	/* cols is the maximum number of columns */
	qreal keywidth = 1.0 * width() / cols;

	/* mKeyMap[ mPage ].keys() list the rows */
	qreal keyheight = height() / mKeyMap[mPage].keys().count();

	/* List of the rows of this page */
	QStringList rows = mKeyMap[mPage].keys();

	for (int row = 0; row < rows.count(); row++)
	{
		/* List of all the keys of this row */
		QStringList keys = mKeyMap[mPage][rows.at(row)];

		QList<int> fatKeys;
		Q_FOREACH (QString key, keys)
		{
			if ((key.toInt() > 1000) and (key.toInt() < 2000))
			{
				fatKeys << key.toInt();
			}
		}

		qreal extraSpace = 0, fatSpace = 0;
		if (fatKeys.count())
		{
			extraSpace = 0;
			fatSpace   = (width() - keys.count() * keywidth) / fatKeys.count();
		}

		else
		{
			/* Some rows have lesser number of keys, give a space at the beginning and end */
			extraSpace = (width() - keys.count() * keywidth) / 2;
			fatSpace   = 0;
		}

		for (int x = 0; x < keys.count(); x++)
		{
			qreal xPos   = extraSpace + x * keywidth;
			qreal yPos   = row * keyheight;
			qreal kwidth = keywidth;
			if (fatKeys.contains(keys[x].toInt()))
			{
				kwidth     += fatSpace;
				extraSpace += fatSpace;
			}
			layout.insert(keys[x].toInt(), QRectF(QPointF(xPos, yPos), QSizeF(kwidth, keyheight)));
		}
	}

	/* Scale the font size */
	qreal maxPtSize = qMin(keywidth, keyheight) * 0.4;

    mFont.setPointSize(maxPtSize);
    setFont(mFont);

	repaint();
}


void CoreKeyboard::resizeEvent(QResizeEvent *rEvent)
{
	rEvent->accept();
	relayKeyboard();
}


void CoreKeyboard::mousePressEvent(QMouseEvent *mEvent)
{
	Q_FOREACH (int key, layout.uniqueKeys())
	{
		keypress[key] = false;

		bool hasKey = false;
		Q_FOREACH (QRectF rect, layout.values(key))
		{
			hasKey |= rect.contains(mEvent->pos());
		}

		if (hasKey)
		{
			int realKey = 0;
			/* Virtual release of the actual key */
			if (key > 50000)
			{
				realKey = key - 50000;
			}

			/* Ctrl: Contains some symbol or text */
			else if (key > 37000)
			{
				realKey = key - 37000;
			}

			/* Fat keys: Remove 1000 to get its actual value */
			else if (key > 1000)
			{
				realKey = key - 1000;
			}

			/* A normal key */
			else
			{
				realKey = key;
			}

			// Allow pressing the key if it is not a modifier key
			// or if it is, and it currently isn't being pressed,
			// or if it is the shift key while at shiftMode '1',
			// which would about to be '2'
			bool doPress = !modifiers.contains(realKey)
				|| !modPress.contains(realKey)
				|| (realKey == shift && shiftMode == 1);

			if ((key != 777) and (key != 888) and (key != 999))
			{
                Display *display = qApp->nativeInterface<QNativeInterface::QX11Application>()->display();

				/* If the key value is greater than 50000, press 'Shift' */
				if (key > 50000)
				{
					XTestFakeKeyEvent(display, shift, true, 0);
				}

				/* If the key value is greater than 37000, press 'Ctrl' */
				else if (key > 37000)
				{
					XTestFakeKeyEvent(display, ctrl, true, 0);
				}

                /* Virtual press of the actual key */
                XTestFakeKeyEvent(display, realKey, doPress, 0);
			}
			else if (key == 888)
			{
                Display *display = qApp->nativeInterface<QNativeInterface::QX11Application>()->display();

				// Clear pressed shift if we are switching to second layout (mobile)
				// Second layout already provides shift-modified characters,
				// so no need to change the first row
                XTestFakeKeyEvent(display, shift, false, 0);

            	modPress.remove(shift);
				shiftMode = 0;
			}

			keypress[key] = doPress;

			if (realKey == caps)
			{
				capsOn = !capsOn;
			}
			else if (modifiers.contains(realKey))
			{
				/* If pressing, add key to modPress */
				if (keypress[key])
				{
					/* If using the shift key before mode 2, move to next stage */
					if (realKey == shift && shiftMode < 2)
					{
						shiftMode++;
					}

					modPress << realKey;
				}
				/* Otherwise, remove key from the modPress */
				else
				{
					/* Set shift mode to 0 if we are dealing with the shift key */
					if (realKey == shift)
					{
						shiftMode = 0;
					}

					modPress.remove(realKey);
				}
			}
		}
	}

	repaint();
	mEvent->accept();
}


void CoreKeyboard::mouseReleaseEvent(QMouseEvent *mEvent)
{
	Q_FOREACH (int key, layout.uniqueKeys())
	{
		bool hasKey = keypress[key];

		keypress[key] = false;

		if (hasKey)
		{
			int realKey = 0;
			/* Virtual release of the actual key */
			if (key > 50000)
			{
				realKey = key - 50000;
			}

			/* Ctrl: Contains some symbol or text */
			else if (key > 37000)
			{
				realKey = key - 37000;
			}

			/* Fat keys: Remove 1000 to get its actual value */
			else if (key > 1000)
			{
				realKey = key - 1000;
			}

			/* A normal key */
			else
			{
				realKey = key;
			}

			/* If the current key release is not a modifier */
			if ((key != 777) and (key != 888) and (key != 999) and (not modifiers.contains(realKey)))
			{
                Display *display = qApp->nativeInterface<QNativeInterface::QX11Application>()->display();

				/* Virtual release of the actual key */
				XTestFakeKeyEvent(display, realKey, false, 0);

				/* If the key value is greater than 50000, release 'Shift' */
				if (key > 50000)
				{
					XTestFakeKeyEvent(display, shift, false, 0);
				}

				/* If the key value is greater than 37000, release 'Ctrl' */
				else if (key > 37000)
				{
					XTestFakeKeyEvent(display, ctrl, false, 0);
				}

				/* Release all modifiers if any are pressed */
				Q_FOREACH (int key, modPress)
				{
					if (key == shift)
					{
						// Do not release shift key is in mode '2' and we are
						// not pressing the caps key
						if (shiftMode == 2 && realKey != caps)
						{
							continue;
						}
						else
						{
							shiftMode = 0;
						}
					}

					XTestFakeKeyEvent(display, key, false, 0);
				}

				modPress.clear();

				// If on shift mode 2, add it back to modPress
				if (shiftMode == 2)
				{
					modPress << shift;
				}
			}

			else
			{
				switch (realKey)
				{
				case 777:
					hide();
					break;

				case 888:
					mPage = (mPage == 1 ? 2 : 1);
					relayKeyboard();
					break;
				}
			}

			mEvent->accept();
		}
	}

	repaint();
	mEvent->accept();
}


void CoreKeyboard::paintEvent(QPaintEvent *pEvent)
{
	QPainter painter(this);
	bool shiftPressed = modPress.contains(shift);

	Q_FOREACH (int key, layout.uniqueKeys())
	{
		int realKey = 0;
		/* Shift: Contains an alternate text */
		if (key > 50000)
		{
			realKey = key - 50000;
		}

		/* Ctrl: Contains some symbol or text */
		else if (key > 37000)
		{
			realKey = key - 37000;
		}

		/* Fat keys: Remove 1000 to get its actual value */
		else if (key > 1000)
		{
			realKey = key - 1000;
		}

		/* A normal key */
		else
		{
			realKey = key;
		}

		/* A key having value 0, will be our spacer */
		if (key == 0)
		{
			continue;
		}

		Q_FOREACH (QRectF layoutRect, layout.values(key))
		{
			/* Draw the key background: pressed */
			if (keypress.value(key) or modPress.contains(realKey))
			{
				painter.save();
				painter.setPen(Qt::NoPen);
				painter.setBrush(QColor(Qt::darkGray));
				painter.drawRect(layoutRect.adjusted(4, 4, -1, -1));
				painter.restore();
			}

			/* Draw the key background: normal */
			else
			{
				painter.save();
				painter.setPen(Qt::NoPen);
				painter.setBrush(palette().color(QPalette::Window).darker());
				painter.drawRect(layoutRect.adjusted(4, 4, -1, -1));
				painter.setBrush(palette().color(QPalette::Window).darker());
				painter.drawRect(layoutRect.adjusted(2, 2, -3, -3));
				painter.restore();
			}

			/* Draw the key text: pressed */
			painter.save();
			painter.setRenderHints(QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
			QString keyTxt;

			keyTxt =
				/* If the key is greater than 50000, */
				key > 50000
				// or if the shift key is on (no caps and is not the caps key)
				// and an alt exists,
				|| (!capsOn && shiftPressed
					&& realKey != shift && realKey != caps
					&& keyChart[realKey].size() > 1)
				/* or if the caps is on (no shift) and it is a letter, */
				|| (capsOn && !shiftPressed
					&& letters.contains(realKey))
				// or if the caps is on (w/ shift) and it is non-letter
				// (but non-shift key) with an alt,
				|| (capsOn && shiftPressed
					&& realKey != shift && !letters.contains(realKey)
					&& keyChart[realKey].size() > 1)
				/* or if the caps is on and is the caps key, */
				|| (capsOn && realKey == caps)
				/* or if the shift key on shiftMode '2', */
				|| (realKey == shift && shiftMode == 2)
				/* Then draw the alt, else the primary */
				? keyChart[realKey].value(1)
				: keyChart[realKey].value(0);

			if (not keyTxt.contains("/") or (keyTxt == "/"))
			{
				painter.drawText(layoutRect, Qt::AlignCenter, keyTxt);
			}

			else
			{
				QRectF imgRect = layoutRect;
				qreal  size    = qMin(imgRect.width(), imgRect.height()) * 0.5;

				QImage img(keyTxt);
				img = img.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

				imgRect.setX(imgRect.x() + (layoutRect.width() - img.width()) / 2);
				imgRect.setY(imgRect.y() + (layoutRect.height() - img.height()) / 2);
				imgRect.setSize(QSizeF(size, size));

				painter.drawImage(imgRect, img);
			}

			painter.restore();
		}
	}

	painter.end();

	pEvent->accept();
}


void CoreKeyboard::toggleShowHide()
{
	if (isVisible()){
        hide();
    } else{
		show();
	}
}

void CoreKeyboard::toggleAutosuggest()
{
    if (mAutoSuggest){
        mAutoSuggest = false;
    }else{
        mAutoSuggest = true;
    }
    smi->setValue("CoreKeyboard", "AutoSuggest", mAutoSuggest);
}

void CoreKeyboard::modeCompact()
{
    QSize size = qApp->primaryScreen()->size();

    setMinimumSize(size.width()*0.2, size.height()*0.15);
    resize(windowSize);
    move(size.width() - width(), size.height() - height());
    qApp->processEvents();
}

void CoreKeyboard::modeFixed()
{
    QSize size = qApp->primaryScreen()->size();

    resize(size.width(), size.height() * .3);
    setMinimumSize(size.width(), size.height() * .3);
    setGeometry(0, size.height() * .7, size.width(), size.height() * .3);
    qApp->processEvents();
}

void CoreKeyboard::switchMode()
{
    if (mMode){
        modeCompact();
        mMode = false;
        smi->setValue("CoreKeyboard", "Mode", false);
    } else {
        modeFixed();
        mMode = true;
        smi->setValue("CoreKeyboard", "Mode", true);
	}

    loadKeymap();
    relayKeyboard();
	show();
}

void CoreKeyboard::switchType()
{
    if (mType == true){
        smi->setValue("CoreKeyboard", "Type", false);
        mType = false;
    } else {
        mType = true;
        smi->setValue("CoreKeyboard", "Type", true);
    }

    loadKeymap();
    relayKeyboard();
    show();
}

void CoreKeyboard::changeEvent(QEvent *event)
{
	if ((event->type() == QEvent::ActivationChange) and (!isActiveWindow()))
	{
		setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus);
		show();
		event->accept();
	}

	else
	{
		QWidget::changeEvent(event);
		event->accept();
	}
}


void CoreKeyboard::closeEvent(QCloseEvent *cEvent)
{
    qDebug()<< "save window stats"<< this->size() << this->isMaximized();

    smi->setValue("CoreKeyboard", "WindowSize", this->size());
}
