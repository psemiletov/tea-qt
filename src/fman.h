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


#ifndef FMAN_H
#define FMAN_H


#include <QObject>
#include <QStandardItemModel>
#include <QTreeView>
#include <QHeaderView>
#include <QDir>
#include <QDrag>
#include <QPainter>



class CFMan: public QTreeView
{
  Q_OBJECT

public:

//  CZipper zipper;
  QDir dir;
  
  int sort_mode;
  Qt::SortOrder sort_order;
  
  QStandardItemModel *mymodel;
    
  CFMan (QWidget *parent = 0);

  void add_entry (const QFileInfo &fi);
  void append_dot_entry (const QString &fname);

  const QModelIndex index_from_name (const QString &name);
  const QModelIndex index_from_idx (int idx);
  int get_sel_index();
  
  void nav (const QString &path);

  QString get_sel_fname();
  QStringList get_sel_fnames();

public slots:

  void tv_activated (const QModelIndex &index);
  void refresh();
  void dir_up();
  void fman_currentChanged (const QModelIndex &current, const QModelIndex &previous);
  void header_view_sortIndicatorChanged (int logicalIndex, Qt::SortOrder order);
  
signals:

 void file_activated (const QString &path);
 void dir_changed (const QString &path);
 void current_file_changed (const QString &path, const QString &just_name);

protected:

  void mouseMoveEvent (QMouseEvent *event);
  void keyPressEvent (QKeyEvent *event);
  void drawRow (QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif
