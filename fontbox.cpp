/*
this code is Public Domain
*/

#include <QVBoxLayout>
#include <QDebug>
#include <QSpinBox> 
#include <QFont> 
#include <QFontComboBox>

#include "fontbox.h"


CFontBox::CFontBox (QWidget *parent): QWidget (parent)
{
  QVBoxLayout *h_box = new QVBoxLayout;
  setLayout (h_box);
 
  QFontComboBox *cf = new QFontComboBox(); 

  QSpinBox *spb_font_size = new QSpinBox (this);
  spb_font_size->setRange (6, 64);

  spb_font_size->setValue (12);
  connect (spb_font_size, SIGNAL(valueChanged (int)), this, SLOT(slot_font_size_changed (int )));
  
  h_box->addWidget (cf);
  h_box->addWidget (spb_font_size);
  
  h_box->addWidget (&edit);
  
  edit.setPlainText (tr("Example string"));
   
  connect (cf, SIGNAL(currentIndexChanged (const QString &)),
           this, SLOT(slot_fontname_changed(const QString &)));

  setWindowTitle (tr("Font gallery"));  
  setAttribute(Qt::WA_DeleteOnClose, true);
}


void CFontBox::slot_fontname_changed (const QString &text)
{
  QFont f = edit.font();
  f.setFamily (text);
  edit.setFont (f);
}


void CFontBox::slot_font_size_changed (int i)
{
  QFont f = edit.font();
  f.setPointSize (i);
  edit.setFont (f);
}
