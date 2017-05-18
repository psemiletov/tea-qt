/***************************************************************************
 *   2007-2017 by Peter Semiletov                                          *
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


#include "logmemo.h"

#include <QTime>
#include <QTextCursor>


CLogMemo::CLogMemo ( QWidget * parent): QPlainTextEdit (parent)
   
//CLogMemo::CLogMemo (QPlainTextEdit *m)
{
//  memo = m;
  
  setObjectName ("logmemo");
  /*
  memo->setFocusPolicy (Qt::ClickFocus);
  memo->setUndoRedoEnabled (false);
  memo->setReadOnly (true);
  
  memo->setTextInteractionFlags (Qt::LinksAccessibleByMouse | Qt::TextSelectableByMouse | 
                                 Qt::TextSelectableByKeyboard);*/
  
  setFocusPolicy (Qt::ClickFocus);
  setUndoRedoEnabled (false);
  setReadOnly (true);
  
  setTextInteractionFlags (Qt::LinksAccessibleByMouse | Qt::TextSelectableByMouse | 
                                 Qt::TextSelectableByKeyboard);

}


void CLogMemo::log (const QString &text)
{
   /*
  QTextCursor cr = memo->textCursor();
  cr.movePosition (QTextCursor::Start);
  cr.movePosition (QTextCursor::Down, QTextCursor::MoveAnchor, 0);
  memo->setTextCursor (cr);

  QTime t = QTime::currentTime();

  memo->textCursor().insertHtml ("[" + t.toString("hh:mm:ss") + "] " + text + "<br>");

  cr = memo->textCursor();
  cr.movePosition (QTextCursor::Start);
  cr.movePosition (QTextCursor::Down, QTextCursor::MoveAnchor, 0);
  memo->setTextCursor (cr);*/
   
   QTextCursor cr = textCursor();
  cr.movePosition (QTextCursor::Start);
  cr.movePosition (QTextCursor::Down, QTextCursor::MoveAnchor, 0);
  setTextCursor (cr);

  QTime t = QTime::currentTime();

  textCursor().insertHtml ("[" + t.toString("hh:mm:ss") + "] " + text + "<br>");

  cr = textCursor();
  cr.movePosition (QTextCursor::Start);
  cr.movePosition (QTextCursor::Down, QTextCursor::MoveAnchor, 0);
  setTextCursor (cr);
}
