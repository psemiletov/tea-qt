#ifndef FONTBOX_H
#define FONTBOX_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QObject>
#include <QFontComboBox>
#include <QSpinBox>


class CFontBox: public QWidget
{
Q_OBJECT 
 
public slots:
 
  void slot_fontname_changed (const QString &text);
  void slot_font_size_changed (int i);
   
public:

 QPlainTextEdit edit;
 QFontComboBox *cf;
 QSpinBox *spb_font_size;

 CFontBox (QWidget *parent = 0); 
 
};

#endif

