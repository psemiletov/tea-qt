#ifndef TZIPPER_H
#define TZIPPER_H


#include <QObject>
#include <QFileInfo>
#include <QStringList>

#include "quazip.h"
#include "quazipfile.h"


class CZipper: public QObject
{
  Q_OBJECT

public:

   QString string_data;
   QString archive_fullpath;
   QString archive_name;
  
   QStringList files_list;
   
   bool pack_prepared();
   bool zip_directory (const QString &archpath, const QString &dir2pack);
   bool read_as_utf8 (const QString &archname, const QString &fname);
   bool unzip (const QString &archpath, const QString &destdir);
   QStringList unzip_list (const QString &archpath);

signals:

   void new_iteration (const QFileInfo &fi);
   void unpack_iteration (const QString &fname);

};

QByteArray gzip_deflateFile (const QString &fileName);

#endif // TZIPPER_H
