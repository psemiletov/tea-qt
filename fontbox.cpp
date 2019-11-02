/*
this code is Public Domain
*/

#include <QVBoxLayout>
#include <QFont>

#include "fontbox.h"


CFontBox::CFontBox (QWidget *parent): QWidget (parent)
{
  QVBoxLayout *h_box = new QVBoxLayout;
  setLayout (h_box);

  cf = new QFontComboBox;

  spb_font_size = new QSpinBox (this);
  spb_font_size->setRange (6, 64);

  spb_font_size->setValue (12);
  connect (spb_font_size, SIGNAL(valueChanged (int)), this, SLOT(slot_font_size_changed (int )));

  h_box->addWidget (cf);
  h_box->addWidget (spb_font_size);
  h_box->addWidget (&edit);

  edit.setPlainText (tr ("Example string"));

  connect (cf, SIGNAL(currentIndexChanged (const QString &)),
           this, SLOT(slot_fontname_changed(const QString &)));

  setWindowTitle (tr ("Font gallery"));  
  setAttribute (Qt::WA_DeleteOnClose, true);
}


void CFontBox::slot_fontname_changed (const QString &text)
{
  edit.setFont (QFont (cf->currentText(), spb_font_size->value()));
}


void CFontBox::slot_font_size_changed (int i)
{
  edit.setFont (QFont (cf->currentText(), spb_font_size->value()));
}
