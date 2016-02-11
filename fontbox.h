#ifndef FONTBOX_H
#define FONTBOX_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QObject>

class CFontBox: public QWidget
{
Q_OBJECT 
 
public slots:
 
   void slot_fontname_changed (const QString &text);
   void slot_font_size_changed (int i);
   
public:

 QPlainTextEdit edit;

 CFontBox (QWidget *parent = 0); 
 
};


#endif