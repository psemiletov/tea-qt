/*
 * taken from http://www.qtcentre.org/wiki/index.php?title=SingleApplication
 * LGPL'ed source code
 * modified by Peter Semiletov
 * 
 */

#ifndef SINGLE_APP_H
#define SINGLE_APP_H

#include <QApplication>
#include <QLocalServer>

#ifndef Q_OS_OS2
#include <QSharedMemory>
#endif


class CSingleApplication : public QApplication
{
  Q_OBJECT

private:

#ifndef Q_OS_OS2
  QSharedMemory sharedMemory;
#endif        

  bool _isRunning;
  QString _uniqueKey;

  QLocalServer *localServer;
  static const int timeout = 1000;

public:

  CSingleApplication (int &argc, char *argv[], const QString uniqueKey);
  bool isRunning();
  bool sendMessage(const QString &message);
  static int cursorFlashTime() {return 0;}; 

public slots:

  void receiveMessage();

signals:

  void messageAvailable(QString message);

};

#endif // SINGLE_APP_H

