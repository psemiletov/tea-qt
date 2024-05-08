/*
this code is Public Domain
Peter Semiletov
*/

#ifndef UTILS_H
#define UTILS_H

#include <vector>

#include <QObject>
#include <QHash>
#include <QFileInfo>
#include <QStringList>

#if QT_VERSION < 0x050000
#include <QRegExp>
#else
#include <QRegularExpression>
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

#if QT_VERSION < 0x050000
  std::vector <QRegExp> patterns;
#else
  std::vector <QRegularExpression> patterns;
#endif

  CFTypeChecker();

  bool check (const QString &fname) const; 
};


/* file utils */

bool has_css_file (const QString &path);
//QString guess_enc_for_file (const QString &fname);

bool file_is_writable (const QString &fname);
bool file_is_readable (const QString &fname);
bool path_is_file (const QString &fname);
bool path_is_dir (const QString &fname);
bool path_is_abs (const QString &fname);

bool dir_exists (const QString &path);
bool file_exists (const QString &fileName);

QString change_file_ext (const QString &s, const QString &ext);
QString file_get_ext (const QString &file_name);

QStringList read_dir_entries (const QString &path);


/* io utils */

bool qstring_save (const QString &fileName, const QString &data);
QString qstring_load (const QString &fileName);
QString qstring_load_first_line (const QString &fileName);
QByteArray file_load (const QString &fileName);

/* string/stringlist utils */

bool ends_with (std::string const &value, std::string const &ending);
std::string string_file_load (const std::string &fname);
//bool str_check (char *s1, char *s2, int size);
QString string_between (const QString &source, const QString &sep1, const QString &sep2);
bool char_is_bad (const QChar &c);
void qstring_list_print (const QStringList &l);
QStringList bytearray_to_stringlist (const QList<QByteArray> &a);
void strlist_swap (QStringList &l, int a, int b);


/* hash utils */

QString hash_get_val (QHash<QString, QString> &h, const QString &key, const QString &def_val);
QString qstring_load_value (const QString &fileName, const QString &key, const QString &def);
QHash<QString, QString> hash_load_keyval (const QString &fname);
void hash_save_keyval (const QString &fname, const QHash <QString, QString> &h);


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
  return QFileInfo (fileName).path();
}


inline QString qstring_clear (const QString &s)
{
  QString t = s;
  return t.remove ("&");
}


#endif
