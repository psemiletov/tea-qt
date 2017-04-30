// "single_application.cpp"

/*
 * taken from http://www.qtcentre.org/wiki/index.php?title=SingleApplication
 * LGPL'ed source code
 * modified by Peter Semiletov
 * 
 */


#include <QLocalSocket>
#include <QDebug>

#include "single_application.h"

CSingleApplication::CSingleApplication(int &argc, char *argv[], const QString uniqueKey) : QApplication(argc, argv), _uniqueKey(uniqueKey)
{

#ifndef Q_OS_OS2

  // qDebug() << "SingleApplication::SingleApplication";

  sharedMemory.setKey(_uniqueKey);
  
  if (sharedMemory.attach())
     _isRunning = true;
  else
      {
       _isRunning = false;
       // create shared memory.
  
       if (! sharedMemory.create(1))
          {
           qDebug() << "Unable to create single instance.";
           return;
          }
          
       // create local server and listen to incomming messages from other instances.
       localServer = new QLocalServer (this);
       connect (localServer, SIGNAL(newConnection()), this, SLOT(receiveMessage()));
       localServer->listen (_uniqueKey);
      }
      
#endif      
}

// public slots.

void CSingleApplication::receiveMessage()
{
  QLocalSocket *localSocket = localServer->nextPendingConnection();
  if (localSocket == 0)
      return;
  
  if (! localSocket->waitForReadyRead (timeout))
      {
       qDebug() << localSocket->errorString();
       return;
      }
      
  QByteArray byteArray = localSocket->readAll();
  QString message = QString::fromUtf8(byteArray.constData());
  emit messageAvailable(message);
  localSocket->disconnectFromServer();
  
  delete localSocket;
}

// public functions.

bool CSingleApplication::isRunning()
{
  return _isRunning;
}


bool CSingleApplication::sendMessage(const QString &message)
{
  if (! _isRunning)
     return false;
  
  if (message.isNull() || message.isEmpty())
     return false;
        
  QLocalSocket localSocket (this);
  localSocket.connectToServer (_uniqueKey, QIODevice::WriteOnly);
   
  if (! localSocket.waitForConnected (timeout))
     {
      qDebug() << localSocket.errorString();
      return false;
     }
     
  localSocket.write (message.toUtf8());
  
  if (! localSocket.waitForBytesWritten (timeout))
     {
      qDebug() << localSocket.errorString();
      return false;
     }
     
  localSocket.disconnectFromServer();
  return true;
}
