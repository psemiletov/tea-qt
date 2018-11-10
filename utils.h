/*
this code is Public Domain
Peter Semiletov
*/

#ifndef UTILS_H
#define UTILS_H

#include <QObject>
#include <QHash>
#include <QFileInfo>
#include <QStringList>


#if QT_VERSION >= 0x040500
#include <QMimeDatabase>
#endif


class CFilesList: public QObject
{
public:

  QStringList list;

  void get (const QString &path);
  void iterate (QFileInfo &fi);
};


class CFTypeChecker: public QObject
{
public:

  QStringList lexts;
  QStringList lnames;

#if QT_VERSION >= 0x040500
  QMimeDatabase db;
#endif

  CFTypeChecker (const QStringList &fnames, const QStringList &exts);
  bool check (const QString &fname);
  QStringList get_supported_exts();
};


/* file utils */

bool file_is_writable (const QString &fname);
bool file_is_readable (const QString &fname);

bool dir_exists (const QString &path);
bool file_exists (const QString &fileName);

QString change_file_ext (const QString &s, const QString &ext);
QString file_get_ext (const QString &file_name);

QStringList read_dir_entries (const QString &path);


/* io utils */

bool qstring_save (const QString &fileName, const QString &data, const char *enc = "UTF-8");
QString qstring_load (const QString &fileName, const char *enc = "UTF-8");
QString qstring_load_first_line (const QString &fileName);
QByteArray file_load (const QString &fileName);


/* string/stringlist utils */

QString string_between (const QString &source, const QString &sep1, const QString &sep2);
bool char_is_bad (const QChar &c);
void qstring_list_print (const QStringList &l);
QStringList bytearray_to_stringlist (QList<QByteArray> a);


/* hash utils */

QString hash_get_val (QHash<QString, QString> &h, const QString &key, const QString &def_val);
QString qstring_load_value (const QString &fileName, const QString &key, const QString &def);
QString hash_keyval_to_string (const QHash<QString, QString> &h);

QHash<QString, QString> hash_load (const QString &fname);
QHash<QString, QString> hash_load_keyval (const QString &fname);
QHash<QString, QString> stringlist_to_hash (const QStringList &l);


/* image utils */

bool is_image (const QString &filename);
QString get_insert_image (const QString &file_name, const QString &full_path, const QString &markup_mode);


/* inlines */


inline int get_value (int total, int perc)
{
  return static_cast <int> (total * perc / 100);
}


inline double get_value (double total, double perc)
{
  return (total * perc / 100);
}


inline double get_percent (double total, double value)
{
  return (value / total) * 100;
}


inline float get_percent (float total, float value)
{
  return (value / total) * 100;
}


inline bool is_dir (const QString &path)
{
  return QFileInfo(path).isDir();
}


inline QString get_file_path (const QString &fileName)
{
  return QFileInfo (fileName).absolutePath();
}


#endif
