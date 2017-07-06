/*
 * taken from http://berenger.eu/blog/c-qt-singleapplication-single-app-instance/
 * code by Berenger Bramas
 * modified by Peter Semiletov
 * 
 */

#ifndef SINGLE_APP_SHARED_H
#define SINGLE_APP_SHARED_H


#include <QApplication>
#include <QStringList>
#include <QLocalServer>

#ifndef Q_OS_OS2
#include <QSharedMemory>
#endif


class CSingleApplicationShared: public QApplication
{
        Q_OBJECT
public:
        CSingleApplicationShared (int &argc, char *argv[], const QString uniqueKey);
           
        ~CSingleApplicationShared() {qDebug() << "~CSingleApplicationShared";};  

        static int cursorFlashTime() 
               {
                return 0;
               } 
        
        bool alreadyExists() const
             {
              return bAlreadyExists;
              }
        
        bool isMasterApp() const
             {
              return !alreadyExists();
              }

        bool sendMessage(const QString &message);
        
        
public slots:
        void checkForMessage();
 
signals:
        void messageAvailable(const QStringList& messages);
        void signal_commit_data();
 
private:
        bool bAlreadyExists;
        
#ifndef Q_OS_OS2
        
        QSharedMemory sharedMemory;
#endif        
};


#endif // SINGLE_APP_SHARED_H

