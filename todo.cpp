#include "utils.h"
#include "todo.h"



void CTodo::check_timeout()
{
  QDate cur_date = QDate::currentDate();
  if (cur_date.daysTo (prev_date) > 0)
      load_dayfile();
  
  QString time = QTime::currentTime().toString ("HH:mm");
  
  if (table.contains (time))
     {
      text_info->clear();
      text_info->insertHtml ("<b>" + time + "</b><br>");
      text_info->insertPlainText (table[time]);
     
      w->move (QPoint (1, 1));
     
      w->show();
      w->raise();
      w->activateWindow();
     } 
}


CTodo::~CTodo()
{
  delete w;
}


CTodo::CTodo()
{
  w = new QWidget (0, Qt::Window);
  w->resize (QSize (320, 240));
  w->setWindowTitle (tr ("Attention! Attention!"));
  text_info = new QTextEdit (w);   
  text_info->setReadOnly (true);
  text_info->resize (w->size());
    
  prev_date = QDate::currentDate();
  
  connect (&timer, SIGNAL(timeout()), this, SLOT(check_timeout()));
  timer.setInterval (1000 * 60);
  timer.start();
}


void CTodo::load_dayfile()
{
  QString fname = dir_days + "/" + QDate::currentDate().toString ("yyyy-MM-dd");
  
  if (! file_exists (fname))
     {
      table.clear();
      return;
     }
  
  QString s = qstring_load (fname);
  
  QStringList sl = s.split ('[');
  for (int i = 0; i < sl.size(); i++)
      {
       int br = sl[i].indexOf (']');
       if (br == -1)
          continue;
          
       QString time = sl[i].left (br);
       QString text = sl[i].right (sl[i].size() - br - 1);

       table.insert (time, text);
      }
}
