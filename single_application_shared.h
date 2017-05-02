/*
 * taken from http://berenger.eu/blog/c-qt-singleapplication-single-app-instance/
 * code by Berenger Bramas
 * modified by Peter Semiletov
 * 
 */

#ifndef SINGLE_APP_SHARED_H
#define SINGLE_APP_SHARED_H

#ifndef Q_OS_OS2

#include <QApplication>
#include <QSharedMemory>
#include <QStringList>
#include <QSharedMemory>
#include <QLocalServer>

class CSingleApplicationShared : public QApplication
{
        Q_OBJECT
public:
        CSingleApplicationShared (int &argc, char *argv[], const QString uniqueKey);

        static int cursorFlashTime() {return 0;}; 
        
         bool alreadyExists() const{
            return bAlreadyExists;
        }
        bool isMasterApp() const{
            return !alreadyExists();
        }

        bool sendMessage(const QString &message);
public slots:
        void checkForMessage();
 
signals:
        void messageAvailable(const QStringList& messages);
 
private:
        bool bAlreadyExists;
        QSharedMemory sharedMemory;
};

#endif //if not def OS/2

#endif // SINGLE_APP_SHARED_H

