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

#include <QSharedMemory>


class CSingleApplicationShared: public QApplication
{
  Q_OBJECT

private:

  QSharedMemory sharedMemory;

  bool bAlreadyExists;

public:

  CSingleApplicationShared (int &argc, char *argv[], const QString &uniqueKey);
  ~CSingleApplicationShared() {/*qDebug() << "~CSingleApplicationShared";*/};  

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
       return ! alreadyExists();
      }

  bool sendMessage (const QString &message);
        
        
public slots:

  void checkForMessage();

signals:

  void messageAvailable(const QStringList &messages);
  void signal_commit_data();
 
};


#endif // SINGLE_APP_SHARED_H
