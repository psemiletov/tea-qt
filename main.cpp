/***************************************************************************
 *   Copyleft 2007-2021 by Peter Semiletov <peter.semiletov@gmail.com>     *
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

#include <cstring>

#include <QApplication>
#include <QDebug>

#include "rvln.h"

#include "single_application_shared.h"


extern rvln *mainWindow; 

int main (int argc, char *argv[])
{
   
#if defined (NO_SINGLE_APP)

  QApplication app (argc, argv);
  qApp->setApplicationName ("TEA");
    
  mainWindow = new rvln();
  mainWindow->show();

  return app.exec();
  
#else
 CSingleApplicationShared app (argc, argv, "tea unique id 1977");
 
 bool single_mode = true;
 
  if (argc > 1)
     if (strcmp(argv[1], "--m") == 0)
        single_mode = false;
    
  if (single_mode && app.alreadyExists())
     {
      if (argc > 1)
         for (int i = 1; i < argc; i++) 
              app.sendMessage (QString(argv[i]));
             
      return 0;
     }
    
#endif
    

  mainWindow = new rvln();


#if !defined (NO_SINGLE_APP)
 QObject::connect(&app, SIGNAL(messageAvailable(QStringList)), mainWindow, SLOT(receiveMessageShared(QStringList)));
#endif

   
  mainWindow->show();
  
  return app.exec();   
}

