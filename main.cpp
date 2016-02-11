/***************************************************************************
 *   Copyleft 2007-2014 by Peter Semiletov <tea@list.ru>                   *
 *                                                                         *
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
 ***************************************************************************/


#include <QApplication>
#include <QDebug>

#include "rvln.h"

#include "single_application.h"


extern rvln *mainWindow; 

int main (int argc, char *argv[])
{
// Q_INIT_RESOURCE (rlvn);
   
   
#if defined(Q_OS_OS2)

  CApplication app (argc, argv);
  qApp->setApplicationName ("TEA");
  
  //rvln mw;

  //mw.show();
  
  mainWindow = new rvln();
  mainWindow->show();

  return app.exec();
  
#else
  
  //Q_INIT_RESOURCE (rlvn);
  
  SingleApplication app (argc, argv, "tea unique id 1977");
  
  if (app.isRunning())
     {
       
       if (argc > 1)
          for (int i = 1; i < argc; i++) 
               app.sendMessage (QString(argv[i]));
       
       return 0;
     }
  
  
  //rvln *mainWindow = new rvln();
  mainWindow = new rvln();

  QObject::connect(&app, SIGNAL(messageAvailable(QString)), mainWindow, SLOT(receiveMessage(QString)));
  
  mainWindow->show();

  
  return app.exec();   
  
#endif  
}
