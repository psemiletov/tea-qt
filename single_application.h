// "single_application.h"

/*
 * taken from http://www.qtcentre.org/wiki/index.php?title=SingleApplication
 * LGPL'ed source code
 * modified by Peter Semiletov
 * 
 */


#ifndef SINGLE_APPLICATION_H
#define SINGLE_APPLICATION_H

#include <QApplication>


#ifndef Q_OS_OS2
#include <QSharedMemory>
#endif



#include <QLocalServer>

class SingleApplication : public QApplication
{
        Q_OBJECT
public:
        SingleApplication (int &argc, char *argv[], const QString uniqueKey);

        bool isRunning();
        bool sendMessage(const QString &message);

public slots:
        void receiveMessage();

signals:
        void messageAvailable(QString message);

private:
        bool _isRunning;
        QString _uniqueKey;

#ifndef Q_OS_OS2

        QSharedMemory sharedMemory;
        
#endif        
        QLocalServer *localServer;

        static const int timeout = 1000;
};
#endif // SINGLE_APPLICATION_H

