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

#include "suggestions.h"


Suggestions::Suggestions(QString wordsFile)
{
	QSettings map(wordsFile, QSettings::NativeFormat);

	Q_FOREACH (QString key, map.allKeys())
	{
		wordMap[key] = map.value(key).toStringList();
	}
}


QStringList Suggestions::wordsFor(QString start)
{
	QStringList goodWords;

	Q_FOREACH (QString word, wordMap.value(start.at(0)))
	{
		if (word.startsWith(start, Qt::CaseInsensitive))
		{
			goodWords << word;
		}
	}

	return goodWords;
}
