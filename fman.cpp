 /**************************************************************************
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
 **************************************************************************/

#include <QStandardItem>
#include <QFileInfoList>
#include <QUrl>
#include <QMimeData>
#include <QDebug>
#include <QApplication>
#include <QDateTime>
#include <QSettings>
#include <QMouseEvent>

#include "fman.h"
#include "utils.h"
//#include "logmemo.h"


extern QSettings *settings;

void CFMan::dir_up()
{
  if (dir.isRoot())
     return;

  QString oldcurdir = dir.dirName();

  dir.cdUp();
  nav (dir.path());

  QModelIndex index = index_from_name (oldcurdir);

  selectionModel()->setCurrentIndex(index, QItemSelectionModel::Rows | QItemSelectionModel::NoUpdate);
  scrollTo (index);
}


void CFMan::nav (const QString &path)
{
  if (path.isEmpty())
     return;

  QString p = path;
  p = p.remove ("file://");

  if (p.contains ("%"))
     p = QUrl::fromPercentEncoding (p.toLatin1().constData());


//  if (path.startsWith ("file://"))
  //   p = p.remove (0, 7);

  dir.setPath (p);
  if (! dir.exists())
      return;

  setModel (0);

  QDir::SortFlags sort_flags;// = 0;

  if (sort_order == Qt::DescendingOrder)
     sort_flags |= QDir::Reversed;

  if (sort_mode == 0)
     sort_flags |= QDir::Name;

  if (sort_mode == 1)
     sort_flags |= QDir::Size;

  if (sort_mode == 2)
     sort_flags |= QDir::Time;

  sort_flags |= QDir::DirsFirst;
  sort_flags |= QDir::IgnoreCase;
  sort_flags |= QDir::LocaleAware;

  mymodel->removeRows (0, mymodel->rowCount());

  QFileInfoList lst = dir.entryInfoList (QDir::Dirs | QDir::Hidden | QDir::NoDotAndDotDot|
                                         QDir::Files | QDir::Drives,
                                         sort_flags);

#if defined(Q_OS_WIN) || defined(Q_OS_OS2)
  if (path != "/")
     append_dot_entry ("..");
#else
  if (path.size() != 2)
     append_dot_entry ("..");
#endif

/*
  NOT GOOD FOR OS/2

  if (! dir.isRoot())
    append_dot_entry ("..");
*/

  for (int i = 0; i < lst.size(); i++)
       add_entry (lst.at(i));

  setModel (mymodel);
  connect (selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(fman_currentChanged(QModelIndex,QModelIndex)));
  emit dir_changed (p);
}


const QModelIndex CFMan::index_from_name (const QString &name)
{
  QList <QStandardItem *> lst = mymodel->findItems (name);

  if (lst.size() > 0)
     return mymodel->indexFromItem (lst[0]);
  else
      return QModelIndex();
}


void CFMan::tv_activated (const QModelIndex &index)
{
  QString item_string = index.data().toString();

  QString dpath = dir.path();

  if (dpath.size() > 1)
     if (dpath.endsWith("/") || dpath.endsWith("\\"))
        dpath.truncate(dpath.size() - 1);

  QString full_path;

  if (dpath == "/")
     full_path = "/" + item_string;
  else
     full_path = dpath + "/" + item_string;

  if (item_string == ".." && dir.path() != "/")
     {
      dir_up();
      return;
     }

  if (is_dir (full_path))
     {
      nav (full_path);
      QModelIndex idx = mymodel->index (0, 0);
      selectionModel()->setCurrentIndex (idx, QItemSelectionModel::Select | QItemSelectionModel::Rows);
     }
  else
      emit file_activated (full_path);
}


void CFMan::add_entry (const QFileInfo &fi)
{
  QList <QStandardItem*> items;

  QStandardItem *item = new QStandardItem (fi.fileName());

  if (fi.isDir())
     {
      QFont f = item->font();
      f.setBold (true);
      item->setFont(f);
     }

  item->setFlags (Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
  items.append (item);

  item = new QStandardItem (QString::number (fi.size()));
  item->setFlags (Qt::ItemIsSelectable | Qt::ItemIsEnabled);
  items.append (item);

  item = new QStandardItem (fi.lastModified().toString ("yyyy-MM-dd"));
  item->setFlags (Qt::ItemIsSelectable | Qt::ItemIsEnabled);
  items.append (item);

  mymodel->appendRow (items);
}


void CFMan::append_dot_entry (const QString &fname)
{
  QList <QStandardItem*> items;

  QStandardItem *item = new QStandardItem (fname);
  item->setFlags (Qt::ItemIsSelectable | Qt::ItemIsEnabled);
  items.append (item);

  item = new QStandardItem ("-");
  item->setFlags (Qt::ItemIsSelectable | Qt::ItemIsEnabled);
  items.append (item);

  item = new QStandardItem ("-");
  item->setFlags (Qt::ItemIsSelectable | Qt::ItemIsEnabled);
  items.append (item);

  mymodel->appendRow (items);
}


void CFMan::header_view_sortIndicatorChanged (int logicalIndex, Qt::SortOrder order)
{
  sort_order = order;
  sort_mode = logicalIndex;

  settings->setValue ("fman_sort_mode", sort_mode);
  settings->setValue ("fman_sort_order", sort_order);

  refresh();
}


CFMan::CFMan (QWidget *parent): QTreeView (parent)
{
  sort_mode = settings->value ("fman_sort_mode", 0).toInt();
  sort_order = Qt::SortOrder (settings->value ("fman_sort_order", 0).toInt());

  mymodel = new QStandardItemModel (0, 3, parent);

  mymodel->setHeaderData (0, Qt::Horizontal, QObject::tr ("Name"));
  mymodel->setHeaderData (1, Qt::Horizontal, QObject::tr ("Size"));
  mymodel->setHeaderData (2, Qt::Horizontal, QObject::tr ("Modified at"));

  setRootIsDecorated (false);
  setAlternatingRowColors (true);
  setAllColumnsShowFocus (true);
  setModel (mymodel);
  setDragEnabled (true);

#if QT_VERSION >= 0x050000

  header()->setSectionResizeMode (QHeaderView::ResizeToContents);
  header()->setSectionsClickable (true);

#else

  header()->setResizeMode (QHeaderView::ResizeToContents);
  header()->setClickable (true);

#endif

  header()->setSortIndicator (sort_mode, sort_order);
  header()->setSortIndicatorShown (true);

  connect (header(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), this, SLOT(header_view_sortIndicatorChanged(int,Qt::SortOrder)));

  header()->setStretchLastSection (false);

  setSelectionMode (QAbstractItemView::ExtendedSelection);
  setSelectionBehavior (QAbstractItemView::SelectRows);

  connect (this, SIGNAL(activated(QModelIndex)), this, SLOT(tv_activated(QModelIndex)));
}


void CFMan::fman_currentChanged (const QModelIndex &current, const QModelIndex &previous )
{
  int row = current.row();
  if (row < 0)
     {
      emit current_file_changed ("", "");
      return;
     }

  QModelIndex i = model()->index (row, 0);
  QString item_string = i.data().toString();
  QString full_path = dir.path() + "/" + item_string;

  emit current_file_changed (full_path, item_string);
}


QString CFMan::get_sel_fname()
{
  if (! selectionModel()->hasSelection())
      return QString();

  QModelIndex index = selectionModel()->currentIndex();
  QString item_string = index.data().toString();
  return dir.path() + "/" + item_string; //return the full path
}


QStringList CFMan::get_sel_fnames()
{
  if (! selectionModel()->hasSelection())
      return QStringList();

  QModelIndexList il = selectionModel()->QItemSelectionModel::selectedRows (0);
  QStringList li;

  for (QList <QModelIndex>::iterator i = il.begin(); i != il.end(); ++i)
      {
       QString item_string = i->data().toString();
       if (item_string != "..")
          {
           QString full_path = dir.path() + "/" + item_string;
           li.append (full_path);
          }
      }

  return li;
}


void CFMan::refresh()
{
  QString current;

  if (selectionModel()->hasSelection())
     {
      QModelIndex index = selectionModel()->currentIndex();
      current = index.data().toString();
     }

  nav (dir.path());

  QModelIndex index = index_from_name (current);
  selectionModel()->setCurrentIndex (index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
  scrollTo (index);
}


const QModelIndex CFMan::index_from_idx (int idx)
{
  QStandardItem *item = mymodel->item (idx);
  if (item)
     return mymodel->indexFromItem (item);
  else
      return QModelIndex();
}


int CFMan::get_sel_index()
{
  if (! selectionModel()->hasSelection())
     return -1;

  QModelIndex index = selectionModel()->currentIndex();
  return index.row();
}


void CFMan::mouseMoveEvent (QMouseEvent *event)
{
  if (! (event->buttons() & Qt::LeftButton))
     return;

  QStringList l = get_sel_fnames();
  if (l.size() < 1)
     return;

  QDrag *drag = new QDrag (this);
  QMimeData *mimeData = new QMimeData;

  QList <QUrl> url_list;

  for (int i = 0; i < l.size(); i++)
       url_list.append (QUrl::fromLocalFile (l[i]));

  mimeData->setUrls (url_list);
  drag->setMimeData (mimeData);

  if (drag->exec (Qt::CopyAction |
                  Qt::MoveAction |
                  Qt::LinkAction) == Qt::MoveAction)
     refresh();

  event->accept();
}


void CFMan::keyPressEvent (QKeyEvent *event)
{
  //заменить это фуфло на          selectionModel()->setCurrentIndex (indexBelow (currentIndex()), QItemSelectionModel::Rows | QItemSelectionModel::Toggle);
/*
  if (event->key() == Qt::Key_Insert)
     {
      bool sel = false;
      QModelIndex index = selectionModel()->currentIndex();
      int row = index.row();

      if (selectionModel()->isSelected (index))
         sel = true;

      sel = ! sel;

      if (sel)
         selectionModel()->select (index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
      else
          selectionModel()->select (index, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);

      if (row < mymodel->rowCount() - 1)
         {
          QModelIndex newindex = mymodel->index (++row, 0);
          selectionModel()->setCurrentIndex (newindex, QItemSelectionModel::Current | QItemSelectionModel::Rows);
          scrollTo (newindex);
         }

      event->accept();
      return;
     }
*/

  if (event->key() == Qt::Key_Insert)
     {
      if (currentIndex().row() == mymodel->rowCount() - 1)
         {
          event->accept();
          return;
         }

      selectionModel()->setCurrentIndex (indexBelow (currentIndex()), QItemSelectionModel::Rows | QItemSelectionModel::Toggle);

      event->accept();
      return;
     }


  if (event->key() == Qt::Key_Backspace)
     {
      dir_up();
      event->accept();
      return;
     }


  if (event->key() == Qt::Key_Return)
     {
      tv_activated (currentIndex());
      event->accept();
      return;
     }


  if (event->key() == Qt::Key_Up)
     {
      if (currentIndex().row() == 0)
         {
          event->accept();
          return;
         }


      if (event->modifiers() & Qt::ShiftModifier)
         selectionModel()->setCurrentIndex (indexAbove (currentIndex()), QItemSelectionModel::Rows | QItemSelectionModel::Toggle);
      else
          selectionModel()->setCurrentIndex (indexAbove (currentIndex()), QItemSelectionModel::Rows | QItemSelectionModel::NoUpdate);

      event->accept();
      return;
     }


  if (event->key() == Qt::Key_Down)
     {
      if (currentIndex().row() == mymodel->rowCount() - 1)
         {
          event->accept();
          return;
         }

      if (event->modifiers() & Qt::ShiftModifier)
         selectionModel()->setCurrentIndex (indexBelow (currentIndex()), QItemSelectionModel::Rows | QItemSelectionModel::Toggle);
      else
           selectionModel()->setCurrentIndex (indexBelow(currentIndex()), QItemSelectionModel::Rows | QItemSelectionModel::NoUpdate);

      event->accept();
      return;
     }


  if (event->key() == Qt::Key_PageUp)
     {
      QModelIndex idx = moveCursor (QAbstractItemView::MovePageUp, Qt::NoModifier);
      selectionModel()->setCurrentIndex (idx, QItemSelectionModel::Rows | QItemSelectionModel::NoUpdate);
      event->accept();
      return;
     }


  if (event->key() == Qt::Key_PageDown)
     {
      QModelIndex idx = moveCursor (QAbstractItemView::MovePageDown, Qt::NoModifier);
      selectionModel()->setCurrentIndex (idx, QItemSelectionModel::Rows | QItemSelectionModel::NoUpdate);
      event->accept();
      return;
     }


  if (event->key() == Qt::Key_End)
     {
      QModelIndex idx = mymodel->index (mymodel->rowCount() - 1, 0);
      selectionModel()->setCurrentIndex (idx, QItemSelectionModel::Rows | QItemSelectionModel::NoUpdate );
      event->accept();
      return;
     }

   if (event->key() == Qt::Key_Home)
      {
       QModelIndex idx = mymodel->index (0, 0);
       selectionModel()->setCurrentIndex (idx, QItemSelectionModel::Rows | QItemSelectionModel::NoUpdate);
       event->accept();
       return;
      }


  QTreeView::keyPressEvent (event);
}


void CFMan::drawRow (QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  if (index.row() == currentIndex().row())
     {
      QStyleOptionViewItem current_option = option;
      QTreeView::drawRow (painter, current_option, index);

      QStyleOptionFocusRect o;
      o.rect = option.rect.adjusted(1,1,-1,-1);
      o.state |= QStyle::State_KeyboardFocusChange;
      o.state |= QStyle::State_Item;

      //o.backgroundColor = palette().color(QPalette::Background);
      //o.backgroundColor = QColor ("red");

      QApplication::style()->drawPrimitive (QStyle::PE_FrameFocusRect, &o, painter);

      QRect r = option.rect.adjusted (1, 1, -1,-1);
      painter->drawRect (r);
     }
  else
      QTreeView::drawRow (painter, option, index);
}

