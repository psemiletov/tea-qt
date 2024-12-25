/*
this code is Public Domain
*/

#include <QFileInfoList>
#include <QFileInfo>
#include <QLabel>
#include <QPushButton>
#include <QMenu>
#include <QStringList>
#include <QAction>

#include "gui_utils.h"
#include "utils.h"



/*
void create_menu_from_list (QObject *handler,
                            QMenu *menu,
                            const QStringList &list,
                            const char *method
                           )
{
  menu->setTearOffEnabled (true);

  for (QList <QString>::const_iterator i = list.begin(); i != list.end(); ++i)
      {
       if (! i->startsWith ("#"))
          {
            QAction *act = new QAction (*i, menu);
            act->setData (*i);

           //QAction *act = menu->addAction (*i);
           //act->setData (*i);
           handler->connect (act, SIGNAL(triggered()), handler, method);
           menu->addAction (act);
          }
      }
}
*/

void create_menu_from_list (QObject *handler,
                            QMenu *menu,
                            const QStringList &list,
                            const char *method
                           )
{
  menu->setTearOffEnabled (true);

 // qDebug() << "=========== create_menu_from_list START=========";

  for (QList <QString>::const_iterator i = list.begin(); i != list.end(); ++i)
      {
       if (! i->startsWith ("#"))
          {
//           qDebug() << "*i" << *i;

           QAction *act = menu->addAction (*i);
           act->setData (*i);
           handler->connect (act, SIGNAL(triggered()), handler, method);
          }
      }

  //qDebug() << "=========== create_menu_from_list END=========";

}


//uses dir name as menuitem, no recursion
void create_menu_from_themes (QObject *handler,
                              QMenu *menu,
                              const QString &dir,
                              const char *method
                              )
{
  menu->setTearOffEnabled (true);
  QDir d (dir);
  QFileInfoList lst_fi = d.entryInfoList (QDir::NoDotAndDotDot | QDir::Dirs,
                                          QDir::IgnoreCase | QDir::LocaleAware | QDir::Name);

  for (QList <QFileInfo>::iterator fi = lst_fi.begin(); fi != lst_fi.end(); ++fi)
      {
       if (fi->isDir())
          {
           if (has_css_file (fi->absoluteFilePath()))
              {
               QAction *act = menu->addAction (fi->fileName());
               act->setData (fi->filePath());
               handler->connect (act, SIGNAL(triggered()), handler, method);
              }
           else
               {
                QMenu *mni_temp = menu->addMenu (fi->fileName());
                create_menu_from_themes (handler, mni_temp, fi->filePath(), method);
               }
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
  if (! d.exists())
     return;

  QFileInfoList lst_fi = d.entryInfoList (QDir::NoDotAndDotDot | QDir::AllEntries,
                                          QDir::DirsFirst | QDir::IgnoreCase |
                                          QDir::LocaleAware | QDir::Name);

  for (QList <QFileInfo>::iterator fi = lst_fi.begin(); fi != lst_fi.end(); ++fi)
      {
       if (fi->isDir())
          {
           QMenu *mni_temp = menu->addMenu (fi->fileName());
           create_menu_from_dir (handler, mni_temp, fi->filePath(), method);
          }
       else
           {
            QAction *act = menu->addAction (fi->fileName());
            act->setData (fi->filePath());
            handler->connect (act, SIGNAL(triggered()), handler, method);
           }
      }
}


QImage image_scale_by (const QImage &source,
                       bool by_side,
                       int value,
                       Qt::TransformationMode mode)
{
  if (source.isNull())
     return source;

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

  lt_h->insertWidget (-1, l, 0, Qt::AlignLeft);
  lt_h->insertWidget (-1, r, 1, Qt::AlignLeft);

  layout->addLayout (lt_h);

  return r;
}


QComboBox* new_combobox_from_vector (QBoxLayout *layout,
                        const QString &label,
                        std::vector <std::string> items,
                        int index)
{
    //qDebug() << "new_combobox_from_vector items.size:" << items.size();

    QHBoxLayout *lt_h = new QHBoxLayout;
    QLabel *l = new QLabel (label);
    QComboBox *r = new QComboBox;

    //r->addItems (items);

    for (size_t i = 0; i < items.size(); i++)
       {
        r->addItem (QString::fromStdString (items[i]));
       }

    if (index < items.size() || index > -1)
        r->setCurrentIndex (index);

    lt_h->insertWidget (-1, l, 0, Qt::AlignLeft);
    lt_h->insertWidget (-1, r, 1, Qt::AlignLeft);

    layout->addLayout (lt_h);

    return r;
}
