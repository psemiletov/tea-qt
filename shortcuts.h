/***************************************************************************
 *   2007-2021 by Peter Semiletov                                          *
 *   peter.semiletov@gmail.com                                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#ifndef SHORTCUTS_H
#define SHORTCUTS_H

#include <QObject>
#include <QHash>
#include <QLineEdit>


class CShortcutEntry: public QLineEdit
{
  Q_OBJECT

public:

  void keyPressEvent (QKeyEvent *event);
};


class CShortcuts: public QObject
{
  Q_OBJECT

public:

  QWidget *w;
  QString fname;
  QHash <QString, QString> hash;
  QStringList captions;

  CShortcuts (QWidget *widget);
  void captions_iterate();
  QAction* find_by_caption (const QString &text);
  QAction* find_by_shortcut (const QString &shcut);

  QKeySequence find_seq_by_caption (const QString &text);
  void set_new_shortcut (const QString &menuitem, const QString &shcut);

  void save_to_file (const QString &file_name);
  void load_from_file (const QString &file_name);
};

#endif
