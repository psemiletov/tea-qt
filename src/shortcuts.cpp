/***************************************************************************
 *   Copyleft 2007-2019 by Peter Semiletov                                 *
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


#include <QString>
#include <QDebug>

#include <QKeyEvent>
#include <QAction>
#include <QStringList>
#include <QKeySequence>

#include "shortcuts.h"
#include "utils.h"
//#include "gui_utils.h"



QString mod_to_string (Qt::KeyboardModifiers k)
{
  QString s;

  if (k & Qt::ShiftModifier)
     s += "Shift+";

  if (k & Qt::ControlModifier)
     s += "Ctrl+";

  if (k & Qt::AltModifier)
     s += "Alt+";

  if (k & Qt::MetaModifier)
     s+= "Meta+";

  return s;
}


QString keycode_to_string (int k)
{
//  return QKeySequence(k).toString(QKeySequence::NativeText);

  QString s;

  switch (k)
         {
          case Qt::Key_F1:
                          s = "F1";
                          break;

          case Qt::Key_F2:
                          s = "F2";
                          break;

          case Qt::Key_F3:
                          s = "F3";
                          break;

          case Qt::Key_F4:
                          s = "F4";
                          break;

          case Qt::Key_F5:
                          s = "F5";
                          break;

          case Qt::Key_F6:
                          s = "F6";
                          break;

          case Qt::Key_F7:
                          s = "F7";
                          break;

          case Qt::Key_F8:
                          s = "F8";
                          break;

          case Qt::Key_F9:
                          s = "F9";
                          break;

          case Qt::Key_F10:
                          s = "F10";
                          break;

          case Qt::Key_F11:
                          s = "F11";
                          break;

          case Qt::Key_F12:
                          s = "F12";
                          break;

          default:
                  s = QChar (k);
         }

  return s;
}


CShortcuts::CShortcuts (QWidget *widget)
{
  w = widget;
}


//FIXME
void CShortcuts::captions_iterate()
{
  captions.clear();
  QList <QAction *> a = w->findChildren <QAction *>();

  for (int i = 0; i < a.size(); i++) 
      if (a.at(i))
          if (! a.at(i)->text().isEmpty())
              captions.prepend (a.at(i)->text());

  captions.sort();
  captions.removeDuplicates(); //nasty hack
}


QAction* CShortcuts::find_by_caption (const QString &text)
{
  QList<QAction *> a = w->findChildren<QAction *>();

  for (int i = 0; i < a.size(); i++) 
       if (a.at(i)->text() == text)
          return a.at(i);

  return 0;
}


QAction* CShortcuts::find_by_shortcut (const QString &shcut)
{
  QList<QAction *> a = w->findChildren<QAction *>();

  for (int i = 0; i < a.size(); i++) 
       if (a.at(i)->shortcut().toString() == shcut)
          return a.at(i);

  return 0;
}


QKeySequence CShortcuts::find_seq_by_caption (const QString &text)
{
  QAction *a = find_by_caption (text);

  if (a)
     return a->shortcut();

  return QKeySequence::fromString ("Ctrl+Alt+Z");
}


void CShortcuts::set_new_shortcut (const QString &menuitem, const QString &shcut)
{
  QAction *b = find_by_shortcut (shcut);
  if (b)
     b->setShortcut (QKeySequence(""));

  QAction *a = find_by_caption (menuitem);
  if (a)
     a->setShortcut (QKeySequence (shcut));
}


void CShortcuts::save_to_file (const QString &file_name)
{
  QList<QAction *> a = w->findChildren<QAction *>();
  QString s;

   for (int i = 0; i < a.size(); i++) 
       if (! a.at(i)->shortcut().toString().isEmpty())
           s.append (a.at(i)->text()).append ("=").append (a.at(i)->shortcut().toString()).append ("\n");

  qstring_save (file_name, s);
}


void CShortcuts::load_from_file (const QString &file_name)
{
  if (! file_exists (file_name))
     return;

  QHash <QString, QString> hash = hash_load_keyval (file_name);

  QList<QAction *> a = w->findChildren<QAction *>();

  for (int i = 0; i < a.size(); i++) 
      {
       QAction *at = a.at(i);

       if (hash.contains (at->text()))
           at->setShortcut (QKeySequence (hash.value (at->text())));
      }
}


void CShortcutEntry::keyPressEvent (QKeyEvent *event)
{
  event->accept();
  QString text = mod_to_string (event->modifiers()) + keycode_to_string (event->key());
  setText (text);
}
