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

#include <QDebug>
#include <QTime>
#include <QTextBlock>
#include <QTextCursor>

#if QT_VERSION >= 0x050000
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#endif

#include "logmemo.h"


CLogMemo::CLogMemo (QWidget *parent): QPlainTextEdit (parent)
{
  setObjectName ("logmemo");
  no_jump = false;
  terminal_output = false;

  setFocusPolicy (Qt::ClickFocus);
  setUndoRedoEnabled (false);
  setReadOnly (true);

  setTextInteractionFlags (Qt::LinksAccessibleByMouse | Qt::TextSelectableByMouse |
                           Qt::TextSelectableByKeyboard);
}

#if QT_VERSION >= 0x050000
void CLogMemo::log_terminal (const QString &text)
{
  if (no_jump)
     return;

  QString txt = text;
  QString tb = txt;

  QRegularExpression re ("\\w+\\.\\w+:\\d+:\\d+:");
  QRegularExpressionMatch match = re.match (txt, 1);

  if (match.hasMatch())
     {
      QString matched = match.captured (0);
      matched = matched.remove (matched.size() - 1, 1);
      tb.replace (matched, "<b>" + matched + "</b>");
     }

  txt = tb;

  QTextCursor cr = textCursor();
  cr.movePosition (QTextCursor::Start);
  cr.movePosition (QTextCursor::Down, QTextCursor::MoveAnchor, 0);
  setTextCursor (cr);

  textCursor().insertHtml (txt + "<br>");

  cr = textCursor();
  cr.movePosition (QTextCursor::Start);
  cr.movePosition (QTextCursor::Down, QTextCursor::MoveAnchor, 0);
  setTextCursor (cr);
}
#endif

void CLogMemo::log (const QString &text)
{

#if QT_VERSION >= 0x050000
  if (terminal_output)
     {
      log_terminal (text);
      return;
     }
#endif

  if (no_jump)
     return;

  QTextCursor cr = textCursor();
  cr.movePosition (QTextCursor::Start);
  cr.movePosition (QTextCursor::Down, QTextCursor::MoveAnchor, 0);
  setTextCursor (cr);

  if (! terminal_output)
     {
      QTime t = QTime::currentTime();
      textCursor().insertHtml ("[" + t.toString("hh:mm:ss") + "] " + text + "<br>");
     }
  else
      textCursor().insertHtml (text + "<br>");

  cr = textCursor();
  cr.movePosition (QTextCursor::Start);
  cr.movePosition (QTextCursor::Down, QTextCursor::MoveAnchor, 0);
  setTextCursor (cr);
}


void CLogMemo::mouseDoubleClickEvent (QMouseEvent *event)
{
  QTextCursor cur = cursorForPosition (event->pos());
  QString txt = toPlainText();
  int pos = cur.position();

  int idx_right = txt.indexOf (" ", pos);
  if (idx_right == -1)
     {
      event->accept();
      return;
     }

  int idx_left = 0;

  for (int i = pos; i != -1; i--)
      {
       if (txt[i] == ' ')
          {
           idx_left = i;
           break;
          }
      }

  txt = txt.mid (idx_left, idx_right - idx_left + 1);

  emit double_click (txt.simplified());

  event->accept();
}
