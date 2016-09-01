/*
this code is Public Domain
*/


#include "gui_utils.h"
#include "utils.h"

#include <QDir>
#include <QFileInfoList>
#include <QFileInfo>

#include <QLabel>

#include <QPushButton>
#include <QMenu>
#include <QStringList>
#include <QAction>
#include <QListWidgetItem>

void create_menu_from_list (QObject *handler,
                            QMenu *menu,
                            const QStringList &list,
                            const char *method
                           )
{
  menu->setTearOffEnabled (true);

  foreach (QString s, list)
          {
           if (! s.startsWith("#"))
              {
               QAction *act = new QAction (s, menu->parentWidget());
               handler->connect (act, SIGNAL(triggered()), handler, method);
               menu->addAction (act);
              }
          }
}


void create_menu_from_dir (QObject *handler,
                           QMenu *menu,
                           const QString &dir,
                           const char *method
                           )
{
  menu->setTearOffEnabled (true);
  QDir d (dir);
  QFileInfoList lst_fi = d.entryInfoList (QDir::NoDotAndDotDot | QDir::AllEntries,
                                          QDir::DirsFirst | QDir::IgnoreCase |
                                          QDir::LocaleAware | QDir::Name);

  foreach (QFileInfo fi, lst_fi)
         {
          if (fi.isDir())
             {
              QMenu *mni_temp = menu->addMenu (fi.fileName());
              create_menu_from_dir (handler, mni_temp,
                                    fi.filePath(), method);
             }
          else
              {
               QAction *act = new QAction (fi.fileName(), menu->parentWidget());
               act->setData (fi.filePath());
               handler->connect (act, SIGNAL(triggered()), handler, method);
               menu->addAction (act);
              }
         }
}

//uses dir name as menuitem, no recursion
void create_menu_from_dir_dir (QObject *handler,
                               QMenu *menu,
                               const QString &dir,
                               const char *method
                               )
{
  menu->setTearOffEnabled (true);
  QDir d (dir);
  QFileInfoList lst_fi = d.entryInfoList (QDir::NoDotAndDotDot | QDir::Dirs,
                                          QDir::IgnoreCase | QDir::LocaleAware | QDir::Name);

  foreach (QFileInfo fi, lst_fi)
         {
          if (fi.isDir())
             {
              QAction *act = new QAction (fi.fileName(), menu->parentWidget());
              act->setData (fi.filePath());
              handler->connect (act, SIGNAL(triggered()), handler, method);
              menu->addAction (act);
             }
         }
}


QImage image_scale_by (const QImage &source,
                       bool by_side,
                       int value,
                       Qt::TransformationMode mode)
{
  bool horisontal = (source.width() > source.height());

  int width;
  int height;

  if (by_side)
     {
      width = value;
      height = value;
     }
   else
       {
        width = get_value (source.width(), value);
        height = get_value (source.height(), value);
       }

  if (horisontal)
     return source.scaledToWidth (width, mode);
  else
      return source.scaledToHeight (height, mode);
}


QLineEdit* new_line_edit (QBoxLayout *layout, const QString &label, const QString &def_value)
{
  QHBoxLayout *lt_h = new QHBoxLayout;
  QLabel *l = new QLabel (label);

  QLineEdit *r = new QLineEdit;
  r->setText (def_value);

  lt_h->insertWidget (-1, l, 0, Qt::AlignLeft);
  lt_h->insertWidget (-1, r, 1, Qt::AlignLeft);

  layout->addLayout (lt_h);

  return r;
}


QSpinBox* new_spin_box (QBoxLayout *layout, const QString &label, int min, int max, int value, int step)
{
  QHBoxLayout *lt_h = new QHBoxLayout;
  QLabel *l = new QLabel (label);

  QSpinBox *r = new QSpinBox;

  r->setSingleStep (step);
  r->setRange (min, max);
  r->setValue (value);

 // lt_h->addWidget (l, 1);
 // lt_h ->addWidget (r);
 
 
  lt_h->insertWidget (-1, l, 0, Qt::AlignLeft);
  lt_h->insertWidget (-1, r, 1, Qt::AlignLeft);


  layout->addLayout (lt_h, 1);

  return r;
}


QComboBox* new_combobox (QBoxLayout *layout,
                         const QString &label,
                         const QStringList &items,
                         const QString &def_value)
{
  QHBoxLayout *lt_h = new QHBoxLayout;
  QLabel *l = new QLabel (label);

  QComboBox *r = new QComboBox;

  r->addItems (items);
  r->setCurrentIndex (r->findText (def_value));

  //lt_h->addWidget (l, 1, Qt::AlignLeft);
  //lt_h->addWidget (r);

  lt_h->insertWidget (-1, l, 0, Qt::AlignLeft);
  lt_h->insertWidget (-1, r, 1, Qt::AlignLeft);

  layout->addLayout (lt_h);
  
  return r;
}

QComboBox* new_combobox (QBoxLayout *layout,
                         const QString &label,
                         const QStringList &items,
                         int index)
{
  QHBoxLayout *lt_h = new QHBoxLayout;
  QLabel *l = new QLabel (label);

  QComboBox *r = new QComboBox;

  r->addItems (items);
  r->setCurrentIndex (index);

  //lt_h->addWidget (l, 1, Qt::AlignLeft);
  //lt_h->addWidget (r);

  lt_h->insertWidget (-1, l, 0, Qt::AlignLeft);
  lt_h->insertWidget (-1, r, 1, Qt::AlignLeft);

  layout->addLayout (lt_h);

  return r;
}


/*
CTextListWindow::CTextListWindow (const QString &title, const QString &label_text): QDialog (0) 
{
  //setAttribute (Qt::WA_DeleteOnClose);
  QVBoxLayout *lt = new QVBoxLayout;

  QLabel *l = new QLabel (label_text);

  list = new QListWidget (this);
   
  lt->addWidget (l);
  lt->addWidget (list);
    
  QHBoxLayout *lt_h = new QHBoxLayout;
  
  QPushButton *bt_apply = new QPushButton (tr ("OK"));
  QPushButton *bt_exit = new QPushButton (tr ("Exit"));

  connect (list, SIGNAL(itemActivated (QListWidgetItem *)), this, SLOT(accept()));


  connect (bt_apply, SIGNAL(clicked()), this, SLOT(accept()));
  connect (bt_exit, SIGNAL(clicked()), this, SLOT(reject()));

  lt_h->addWidget (bt_apply);
  lt_h->addWidget (bt_exit);

  lt->addLayout (lt_h);

  setLayout (lt);
    
  setWindowTitle (title);
}

*/
