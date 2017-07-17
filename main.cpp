/***************************************************************************
 *   Copyleft 2007-2017 by Peter Semiletov <peter.semiletov@gmail.com>     *
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

#ifndef Q_OS_OS2
#include "single_application_shared.h"
#endif

extern rvln *mainWindow; 

int main (int argc, char *argv[])
{
// Q_INIT_RESOURCE (rlvn);
   
 // qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));

   
#if defined(Q_OS_OS2)

  QApplication app (argc, argv);
  qApp->setApplicationName ("TEA");
    
  mainWindow = new rvln();
  mainWindow->show();

  return app.exec();
  
#endif
  //Q_INIT_RESOURCE (rlvn);

#if defined(Q_OS_WIN)
  
  CSingleApplication app (argc, argv, "tea unique id 1977");
  
  if (app.isRunning())
     {
       
       if (argc > 1)
          for (int i = 1; i < argc; i++) 
               app.sendMessage (QString(argv[i]));
       return 0;
     }

#endif


#if defined(Q_OS_UNIX)

 CSingleApplicationShared app (argc, argv, "tea unique id 1977");
 
 if (app.alreadyExists())
    {
     if (argc > 1)
        for (int i = 1; i < argc; i++) 
             app.sendMessage (QString(argv[i]));
             
     return 0;
    }
    
#endif     
     

  mainWindow = new rvln();



#if defined(Q_OS_WIN)
 QObject::connect(&app, SIGNAL(messageAvailable(QString)), mainWindow, SLOT(receiveMessage(QString)));
#endif


#if defined(Q_OS_UNIX)

 QObject::connect(&app, SIGNAL(messageAvailable(QStringList)), mainWindow, SLOT(receiveMessageShared(QStringList)));


// QObject::connect(&app, SIGNAL(signal_commit_data()), mainWindow, SLOT(app_commit_data()));
// QObject::connect(&app, SIGNAL(commitDataRequest(QSessionManager & )), mainWindow, SLOT(slot_commitDataRequest(QSessionManager & )));

#endif

   
  mainWindow->show();
  
  return app.exec();   
}
