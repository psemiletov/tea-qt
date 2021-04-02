#include <QDir>
#include <QSettings>
#include <QDebug>

#include "tzipper.h"
#include "utils.h"


extern QSettings *settings;

//including a gunzip snippet from OSDaB
//by Angius Fabrizio


QString qstring_get_last_after (const QString &s, const QString &sep)
{
  int i = s.lastIndexOf (sep);
  if (i == -1)
     return QString();

  i++;

  QString t = s.right (s.size() - i);
  return t;
}


bool CZipper::zip_directory (const QString &archpath, const QString &dir2pack)
{
  QString archname = qstring_get_last_after (dir2pack, "/");

  QString zipname (archpath); //zip name has a full path ending with .zip
  zipname.append ("/").append (archname).append (".zip");

  QuaZip zip (zipname);
  zip.setFileNameCodec (QTextCodec::codecForName((settings->value ("zip_charset_out", "UTF-8").toString().trimmed().toLatin1().data())));

  if (! zip.open (QuaZip::mdCreate))
     return false;

  QDir dir (dir2pack);

  QFileInfoList files = dir.entryInfoList();
  QFile inFile;
  QuaZipFile outFile(&zip);

  for (QList <QFileInfo>::iterator file = files.begin(); file != files.end(); ++file)
      {
       if (! file->isFile())
          continue;

       inFile.setFileName (file->absoluteFilePath());

       if (! inFile.open (QIODevice::ReadOnly))
          return false;

       QString outfname (archname);
       outfname.append ("/").append (file->fileName());

       if (! outFile.open (QIODevice::WriteOnly, QuaZipNewInfo (outfname, inFile.fileName())))
          return false;

       QByteArray ba = inFile.readAll();
       outFile.write (ba);
       outFile.close();

       if (outFile.getZipError() != UNZ_OK)
          return false;

       inFile.close();
       emit new_iteration (*file); //GOOD?
      }

  zip.close();

  if (zip.getZipError() != 0)
     return false;

  return true;
}


bool CZipper::read_as_utf8 (const QString &archname, const QString &fname)
{
  QuaZip zip (archname);
  zip.setFileNameCodec (QTextCodec::codecForName((settings->value ("zip_charset_in", "UTF-8").toString().trimmed().toLatin1().data())));

  if (! zip.open (QuaZip::mdUnzip))
      return false;

  zip.setCurrentFile (fname);

  if (! zip.hasCurrentFile())
      return false;

  QuaZipFileInfo info;
  if (! zip.getCurrentFileInfo (&info))
     return false;

  QuaZipFile file (&zip);
  if (! file.open (QIODevice::ReadOnly))
      return false;

  QByteArray ba = file.readAll();

  string_data = QString::fromUtf8 (ba.data());

  file.close();
  zip.close();

  return true;
}


//a snippet from OSDaB
//by Angius Fabrizio
QByteArray gzip_deflateFile (const QString &fileName)
{
  gzFile gzDoc;
  char buff[4097];
  int i;
  QByteArray fname = fileName.toLatin1();

  gzDoc = gzopen(fname.constData(), "rb");
  if (! gzDoc)
     return QByteArray();

  QByteArray data;

  while ((i = gzread (gzDoc, &buff, 4096)) > 0)
        {
         buff[i] = '\0';
         data.append (buff);
        }

  gzclose(gzDoc);

  return data;
}


bool CZipper::pack_prepared()
{
  if (archive_fullpath.isEmpty())
     return false;

  QString zipname (archive_fullpath);
  QuaZip zip (zipname);
  zip.setFileNameCodec (QTextCodec::codecForName((settings->value ("zip_charset_out", "UTF-8").toString().trimmed().toLatin1().data())));

  if (! zip.open (QuaZip::mdCreate))
      return false;

  QFile inFile;
  QuaZipFile outFile (&zip);

  for (int i = 0; i < files_list.size(); i++)
      {
       QFileInfo file (files_list[i]);
       if (! file.isFile())
          continue;

       inFile.setFileName (file.absoluteFilePath());
       if (! inFile.open (QIODevice::ReadOnly))
          return false;

       QString outfname (archive_name);
       outfname.append ("/").append (file.fileName());

       if (! outFile.open (QIODevice::WriteOnly, QuaZipNewInfo (outfname, inFile.fileName())))
          return false;

       QByteArray ba = inFile.readAll();
       outFile.write (ba);
       outFile.close();

       if (outFile.getZipError() != UNZ_OK)
          return false;

       inFile.close();

       emit new_iteration (file);
      }

  zip.close();

  if (zip.getZipError() != 0)
     return false;

  return true;
}


//based on the code from quazip Copyright (C) 2005-2007 Sergey A. Tachenov
bool CZipper::unzip (const QString &archpath, const QString &destdir)
{
  QuaZip zip (archpath);
  zip.setFileNameCodec (QTextCodec::codecForName((settings->value ("zip_charset_in", "UTF-8").toString().trimmed().toLatin1().data())));

  if (! zip.open (QuaZip::mdUnzip))
      {
       qDebug() << "zip.open() error: " << zip.getZipError();
       return false;
      }

//  qDebug() << "Entries count: " << zip.getEntriesCount();
  //printf("Global comment: %s\n", zip.getComment().toLocal8Bit().constData());

  QuaZipFileInfo info;

  QuaZipFile file (&zip);
  QFile out;
  QString name;

  char c;

  for (bool more = zip.goToFirstFile(); more; more = zip.goToNextFile())
      {
       if (! zip.getCurrentFileInfo (&info))
          {
           qDebug() << "getCurrentFileInfo()" << zip.getZipError();
           return false;
          }
/*
       printf ("%s\t%hu\t%hu\t%hu\t%hu\t%s\t%u\t%u\t%u\t%hu\t%hu\t%u\t%s\t%s\n",
               info.name.toLocal8Bit().constData(),
               info.versionCreated, info.versionNeeded, info.flags, info.method,
               info.dateTime.toString(Qt::ISODate).toLocal8Bit().constData(),
               info.crc, info.compressedSize, info.uncompressedSize, info.diskNumberStart,
               info.internalAttr, info.externalAttr,
               info.comment.toLocal8Bit().constData(), info.extra.constData());
*/
       if (! file.open (QIODevice::ReadOnly))
           {
            qDebug() << "file.open()" << file.getZipError();
            return false;
           }

       name = file.getActualFileName();

       if (file.getZipError() != UNZ_OK)
          {
           qDebug() <<  "file.getFileName()" << file.getZipError();
           return false;
          }

       QString dirn = destdir + QDir::separator() + name;

       if (name.contains (QDir::separator()))
          { // subdirectory support
            // there must be a more elegant way of doing this
            // but I couldn't find anything useful in QDir
           dirn.chop (dirn.length() - dirn.lastIndexOf (QDir::separator()));
           QDir().mkpath (dirn);
          }

       out.setFileName (destdir + QDir::separator() + name);
       out.open (QIODevice::WriteOnly);
       char buf[4096];
       int len = 0;
       while (file.getChar (&c))
             {
              // we could just do this, but it's about 40% slower:
              // out.putChar(c);
              buf[len++] = c;
              if (len >= 4096)
                 {
                  out.write (buf, len);
                  len = 0;
                 }
             }

       if (len > 0)
           out.write (buf, len);

       out.close();

       emit unpack_iteration (name);

       if (file.getZipError() != UNZ_OK)
          {
           qDebug() << "file.getFileName()" << file.getZipError();
           return false;
          }

       if (! file.atEnd())
          {
           qDebug() << "testRead(): read all but not EOF";
           return false;
          }

       file.close();

       if (file.getZipError() != UNZ_OK)
         {
          qDebug() << "file.close()" << file.getZipError();
          return false;
         }
      }

  zip.close();

  if (zip.getZipError() != UNZ_OK)
     {
      qDebug() <<  "zip.close()" << zip.getZipError();
      return false;
     }

  return true;
}


QStringList CZipper::unzip_list (const QString &archpath)
{
  QuaZip zip (archpath);
  zip.setFileNameCodec (QTextCodec::codecForName((settings->value ("zip_charset_in", "UTF-8").toString().trimmed().toLatin1().data())));

  if (! zip.open (QuaZip::mdUnzip))
      {
       qDebug() << "zip.open() error: " << zip.getZipError();
       return QStringList();
      }

  QStringList result;

  QuaZipFileInfo info;

  for (bool more = zip.goToFirstFile(); more; more = zip.goToNextFile())
      {
       if (! zip.getCurrentFileInfo (&info))
           return QStringList();
         //           qDebug() << "getCurrentFileInfo()" << zip.getZipError();

       result.prepend (info.name);
      }

  zip.close();
  return result;
}
