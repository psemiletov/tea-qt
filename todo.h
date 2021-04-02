#ifndef TODO_H
#define TODO_H

#include <QObject>
#include <QDate>
#include <QTimer>
#include <QHash>
#include <QWidget>
#include <QTextEdit>
#include <QString>
#include <QMessageBox>


class CTodo: public QObject
{
  Q_OBJECT

public:

  QDate prev_date;
  QTimer timer;
  QString dir_days;
  QHash <QString, QString> table;
  
  QMessageBox msgBox;
  QWidget *w;
  QTextEdit *text_info;
    
  CTodo();
  ~CTodo();
  
  void load_dayfile();

public slots:

  void check_timeout();

};

#endif // TODO_H
