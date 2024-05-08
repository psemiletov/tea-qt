/*
this code is Public Domain
Peter Semiletov
*/



#include <QTextStream>
//#include <QTextCodec>
#include <QDebug>
#include <QDir>
#include <QImageReader>
#include <QImage>
#include <QProcess>
#include <QApplication>

#include <fstream>

#include "utils.h"
//#include "enc.h"

using namespace std;

bool boring;


/* file utils */

bool has_css_file (const QString &path)
{
  if (path.isEmpty())
      return false;

  QDir d (path);
  QStringList l = d.entryList();

  for (int i = 0; i < l.size(); i++)
      {
       if (l.at(i).endsWith(".css", Qt::CaseInsensitive))
           return true;
      }

  return false;
}

/*
QString guess_enc_for_file (const QString &fname)
{
  if (fname.isEmpty())
      return QString();

  QString enc = "UTF-8";

  QProcess p;
  p.start ("enca", QStringList() << "-i" << fname);

  if (! p.waitForStarted() || ! p.waitForFinished() )
     return "err";

  QString s = p.readAllStandardOutput();
  if (! s.isEmpty())
     enc = s.trimmed();

  return enc;
}
*/

bool file_is_writable (const QString &fname)
{
  if (fname.isEmpty())
      return false;

  QFile f (fname);
  return f.isWritable();
}


bool file_is_readable (const QString &fname)
{
  if (fname.isEmpty())
      return false;

  QFile f (fname);
  return f.isReadable();
}


bool path_is_file (const QString &fname)
{
  if (fname.isEmpty())
      return false;

  QFileInfo fi (fname);
  return fi.isFile();
}


bool path_is_dir (const QString &fname)
{
  if (fname.isEmpty())
      return false;

  QFileInfo fi (fname);
  return fi.isDir();
}


bool path_is_abs (const QString &fname)
{
  if (fname.isEmpty())
    return false;

  if (fname[0] == '/' || fname.indexOf (':') == 1)
    return true;
  else
    return false;
}


bool dir_exists (const QString &path)
{
  if (path.isEmpty())
      return false;

  QDir d (path);
  return d.exists();
}


bool file_exists (const QString &fileName)
{
  if (fileName.isEmpty())
     return false;

  return QFile::exists (fileName);
}


QString change_file_ext (const QString &s, const QString &ext)
{
  int i = s.lastIndexOf (".");
  if (i == -1)
     return (s + "." + ext);

  QString r (s);
  r.truncate (++i);

  r.append (ext);
  return r;
}


QString file_get_ext (const QString &file_name)
{
  if (file_name.isEmpty())
      return QString();

 /* int i = file_name.lastIndexOf (".");
  if (i == -1)
      return QString();

  return file_name.mid (i + 1).toLower();
*/
  QFileInfo fi (file_name);
  return fi.completeSuffix();
}


QStringList read_dir_entries (const QString &path)
{
  QDir dir (path);
  return dir.entryList (QDir::AllEntries | QDir::NoDotAndDotDot);
}


/* io utils */


bool qstring_save (const QString &fileName, const QString &data)
{
  if (fileName.isEmpty())
      return false;

  QFile file (fileName);
  if (! file.open (QFile::WriteOnly))
      return false;

 // if (enc == "UTF-8")
   // {
     QByteArray ba = data.toUtf8();
     file.write (ba);
     file.close();
     return true;
    //}



  //return false;
}


QString qstring_load (const QString &fileName)
{
  if (fileName.isEmpty())
      return QString();

  QFile file (fileName);
  QString result;

  if (! file.open (QFile::ReadOnly))
      return QString();

  QByteArray ba = file.readAll();
  file.close();


 // if (enc == "UTF-8")
      result = QString::fromUtf8 (ba.data());

  //if (enc == "UTF-16")
    //  result = QString::fromUtf16 ((char16_t *)ba.data());


  return result;
}



/*
bool qstring_save (const QString &fileName, const QString &data, const char *enc)
{
  if (fileName.isEmpty())
      return false;

  QFile file (fileName);
  if (! file.open (QFile::WriteOnly))
      return false;

  QTextCodec *codec = QTextCodec::codecForName (enc);
  if (! codec)
     return false;


  QByteArray ba = codec->fromUnicode (data);

  file.write (ba);
  file.close();

  return true;
}


QString qstring_load (const QString &fileName, const char *enc)
{
  if (fileName.isEmpty())
      return QString();

  QFile file (fileName);

  if (! file.open (QFile::ReadOnly))
      return QString();

  QByteArray ba = file.readAll();
  QTextCodec *codec = QTextCodec::codecForName (enc);

  if (! codec)
     return QString();

  file.close();

  return codec->toUnicode (ba);
}
*/

QString qstring_load_first_line (const QString &fileName)
{
  if (fileName.isEmpty())
      return QString();

  QFile file (fileName);

  if (! file.open (QFile::ReadOnly | QFile::Text))
     return QString();

  QTextStream in(&file);

  return in.readLine();
}


QByteArray file_load (const QString &fileName)
{
  if (fileName.isEmpty())
      return QByteArray();

  QFile file (fileName);
  QByteArray b;

  if (! file.open (QFile::ReadOnly))
      return b;

  b = file.readAll();

  return b;
}


/* string/stringlist utils */


bool ends_with (std::string const &value, std::string const &ending)
{
  if (ending.size() > value.size())
     return false;

  return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}



std::string string_file_load (const std::string &fname)
{
 if (fname.empty())
    return std::string();

 std::ifstream t (fname.c_str());
 std::string s ((std::istreambuf_iterator<char>(t)),
                 std::istreambuf_iterator<char>());

 return s;
}


void strlist_swap (QStringList &l, int a, int b)
{
  QString t = l[a];
  l[a] = l[b];
  l[b] = t;
}


QString string_between (const QString &source,
                        const QString &sep1,
                        const QString &sep2)
{
  QString result;
  int pos1 = source.indexOf (sep1);
  if (pos1 == -1)
     return result;

  int pos2 = source.indexOf (sep2, pos1 + sep1.size());
  if (pos2 == -1)
     return result;

  pos1 += sep1.size();

  result = source.mid (pos1, pos2 - pos1);
  return result;
}


bool char_is_bad (const QChar &c)
{
  if (! c.isNull() && ! c.isLetter())
     return true;

  return false;
}


void qstring_list_print (const QStringList &l)
{
  for (int i = 0; i < l.size(); i++)
      qDebug() << l[i];
}


QStringList bytearray_to_stringlist (const QList<QByteArray> &a)
{
  QStringList r;

  for (int i = 0; i < a.size(); i++)
       r.append (a.at(i).data());

  return r;
}


/* hash utils */

QString hash_get_val (QHash<QString, QString> &h,
                      const QString &key,
                      const QString &def_val)
{
  QString result = h.value (key);
  if (result.isEmpty())
     {
      result = def_val;
      h.insert (key, def_val);
     }

  return result;
}


QString qstring_load_value (const QString &fileName, const QString &key, const QString &def)
{
  QHash <QString, QString> h = hash_load_keyval (fileName);
  return hash_get_val (h, key, def);
}


QHash <QString, QString> hash_load_keyval (const QString &fname)
{
  QHash <QString, QString> result;

  if (! file_exists (fname))
     return result;

  QStringList l = qstring_load (fname).split ("\n");

  for (QList <QString>::iterator i = l.begin(); i != l.end(); ++i)
      {
       QStringList sl = i->split ("=");
       if (sl.size() > 1)
          result.insert (sl.at(0), sl.at(1));
      }

  return result;
}


void hash_save_keyval (const QString &fname, const QHash <QString, QString> &h)
{
  QFile::remove (fname);

  QStringList l;

  QHash<QString, QString>::const_iterator i = h.constBegin();
  while (i != h.constEnd())
        {
         l+= (i.key() + "=" + i.value());
         ++i;
        }

  qstring_save (fname, l.join ("\n"));
}


/* image utils */

bool is_image (const QString &filename)
{
  if (filename.isEmpty())
     return false;

  QList <QByteArray> a = QImageReader::supportedImageFormats();

  for (QList <QByteArray>::iterator i = a.begin(); i != a.end(); ++i)
      {
       QString t (i->data());

       if (t == "pdf") //hack for qt6
          continue;

       if (filename.endsWith (t.prepend ("."), Qt::CaseInsensitive))
          return true;
      }

  return false;
}


QString get_insert_image (const QString &file_name, const QString &full_path, const QString &markup_mode)
{
  if (! is_image (full_path))
     return QString();

  QFileInfo inf (file_name);
  QDir dir (inf.absolutePath());

  QImage img;
  img.load (full_path);
  QString result;

  if (markup_mode == "HTML")
     result = QString ("<img src=\"%1\" alt=\"\" width=\"%2\" height=\"%3\">").arg (
                        dir.relativeFilePath (full_path)).arg (img.width()).arg (img.height());
  else
  if (markup_mode == "XHTML")
     result = QString ("<img src=\"%1\" border=\"0\" alt=\"\" width=\"%2\" height=\"%3\" />").arg (
                       dir.relativeFilePath (full_path)).arg (img.width()).arg (img.height());
  else
  if (markup_mode == "Docbook")
     result = QString ("<mediaobject><imageobject>\n<imagedata fileref=\"%1\"/>\n</imageobject></mediaobject>").arg (
                        dir.relativeFilePath (full_path)) ;
  else
  if (markup_mode == "LaTeX")
      result = QString ("\\includegraphics{%1}").arg (dir.relativeFilePath (full_path));
  else
  if (markup_mode == "Lout")
      result = QString ("@IncludeGraphic {%1}").arg (dir.relativeFilePath (full_path));
  else
  if (markup_mode == "Markdown")
      result = QString ("![alt_text](%1)").arg (dir.relativeFilePath (full_path));

  return result;
}


/* class functions */

void CFilesList::iterate (QFileInfo &fi)
{
  qApp->processEvents();

  if (boring)
     return;

  if (fi.isDir())
     {
      QDir d (fi.absoluteFilePath());
      QFileInfoList lfi= d.entryInfoList (QDir::Dirs | QDir::Files | QDir::Readable | QDir::NoDotAndDotDot);

      for (int i = 0; i < lfi.count(); i++)
           iterate (lfi[i]);
     }
  else
  if (fi.isFile())
      list.append (fi.absoluteFilePath());
}


void CFilesList::get (const QString &path)
{
  if (path.isEmpty())
     return;

  list.clear();
  QDir d (path);
  QFileInfoList lfi= d.entryInfoList (QDir::Dirs | QDir::Files | QDir::Readable | QDir::NoDotAndDotDot);

  for (int i = 0; i < lfi.count(); i++)
      iterate (lfi[i]);
}



#if QT_VERSION < 0x050000
#define ADDTEXTTYPE(expr) patterns.push_back (QRegExp (expr, Qt::CaseInsensitive));
#else
#define ADDTEXTTYPE(expr) patterns.push_back (QRegularExpression (expr, QRegularExpression::CaseInsensitiveOption));
#endif


CFTypeChecker::CFTypeChecker()
{
  ADDTEXTTYPE (".*(readme|news|changelog|todo)$");
  ADDTEXTTYPE ("^\\..*(rc)$");
  ADDTEXTTYPE ("^.*\\.(txt|conf|md|ini|bat|cfg|sbv|log|odt|docx|kwd|fb2|fb2.zip|fbz|abw|rtf|epub|sxw)$");
  ADDTEXTTYPE ("^.*\\.(cpp|c|h|hh|cxx|hpp|cc|m|mm)$");
  ADDTEXTTYPE ("^.*\\.(htm|html|xml|xhtml|ts|osm|xsl)$");

#if defined(POPPLER_ENABLE)
  ADDTEXTTYPE ("^.*\\.(pdf)$");
#endif
#if defined(DJVU_ENABLE)
  ADDTEXTTYPE ("^.*\\.(djvu)$");
#endif

  ADDTEXTTYPE ("^.*\\.(awk)$");
  ADDTEXTTYPE ("^.*\\.(sh)$");
  ADDTEXTTYPE ("^.*\\.(bas|bi|vbs|vbe)$");
  ADDTEXTTYPE ("^.*\\.(d)$");
  ADDTEXTTYPE ("^.*\\.(f|for|f90|f95)$");
  ADDTEXTTYPE ("^.*\\.(java|js)$");
  ADDTEXTTYPE ("^.*\\.(ly)$");
  ADDTEXTTYPE ("^.*\\.(lout)$");
  ADDTEXTTYPE ("^.*\\.(lua)$");
  ADDTEXTTYPE ("^.*\\.(asm)$");
  ADDTEXTTYPE ("^.*\\.(nsi)$");
  ADDTEXTTYPE ("^.*\\.(pp|pas|dpr)$");
  ADDTEXTTYPE ("^.*\\.(pl|pm)$");
  ADDTEXTTYPE ("^.*\\.(php)$");
  ADDTEXTTYPE ("^.*\\.(po)$");
  ADDTEXTTYPE ("^.*\\.(py)$");
  ADDTEXTTYPE ("^.*\\.(r)$");
  ADDTEXTTYPE ("^.*\\.(sd7)$");
  ADDTEXTTYPE ("^.*\\.(tex|lyx)$");
  ADDTEXTTYPE ("^.*\\.(vala)$");
  ADDTEXTTYPE ("^.*\\.(v)$");
  ADDTEXTTYPE ("^.*\\.(wiki)$");
}



/*
CFTypeChecker::CFTypeChecker()
{

#if QT_VERSION < 0x050000

  patterns.push_back (QRegExp (".*(readme|news|changelog|todo)$", Qt::CaseInsensitive));
  patterns.push_back (QRegExp ("^\\..*(rc)$", Qt::CaseInsensitive));
  patterns.push_back (QRegExp ("^.*\\.(txt|conf|md|ini|bat|cfg|sbv|log|odt|docx|kwd|fb2|abw|rtf|epub|sxw|fb2.zip)$", Qt::CaseInsensitive));
  patterns.push_back (QRegExp ("^.*\\.(cpp|c|h|hh|cxx|hpp|cc|m|mm)$", Qt::CaseInsensitive));
  patterns.push_back (QRegExp ("^.*\\.(htm|html|xml|xhtml|ts|osm|xsl)$", Qt::CaseInsensitive));

#if defined(POPPLER_ENABLE)
  patterns.push_back (QRegExp ("^.*\\.(pdf)$", Qt::CaseInsensitive));
#endif

#if defined(DJVU_ENABLE)
  patterns.push_back (QRegExp ("^.*\\.(djvu)$", Qt::CaseInsensitive));
#endif

  patterns.push_back (QRegExp ("^.*\\.(awk)$", Qt::CaseInsensitive));
  patterns.push_back (QRegExp ("^.*\\.(sh)$", Qt::CaseInsensitive));
  patterns.push_back (QRegExp ("^.*\\.(bas|bi|vbs|vbe)$", Qt::CaseInsensitive));
  patterns.push_back (QRegExp ("^.*\\.(d)$", Qt::CaseInsensitive));
  patterns.push_back (QRegExp ("^.*\\.(f|for|f90|f95)$", Qt::CaseInsensitive));
  patterns.push_back (QRegExp ("^.*\\.(java|js)$", Qt::CaseInsensitive));
  patterns.push_back (QRegExp ("^.*\\.(ly)$", Qt::CaseInsensitive));
  patterns.push_back (QRegExp ("^.*\\.(lout)$", Qt::CaseInsensitive));
  patterns.push_back (QRegExp ("^.*\\.(lua)$", Qt::CaseInsensitive));
  patterns.push_back (QRegExp ("^.*\\.(asm)$", Qt::CaseInsensitive));
  patterns.push_back (QRegExp ("^.*\\.(nsi)$", Qt::CaseInsensitive));
  patterns.push_back (QRegExp ("^.*\\.(pp|pas|dpr)$", Qt::CaseInsensitive));
  patterns.push_back (QRegExp ("^.*\\.(pl|pm)$", Qt::CaseInsensitive));
  patterns.push_back (QRegExp ("^.*\\.(php)$", Qt::CaseInsensitive));
  patterns.push_back (QRegExp ("^.*\\.(po)$", Qt::CaseInsensitive));
  patterns.push_back (QRegExp ("^.*\\.(py)$", Qt::CaseInsensitive));
  patterns.push_back (QRegExp ("^.*\\.(r)$", Qt::CaseInsensitive));
  patterns.push_back (QRegExp ("^.*\\.(sd7)$", Qt::CaseInsensitive));
  patterns.push_back (QRegExp ("^.*\\.(tex|lyx)$", Qt::CaseInsensitive));
  patterns.push_back (QRegExp ("^.*\\.(vala)$", Qt::CaseInsensitive));
  patterns.push_back (QRegExp ("^.*\\.(v)$", Qt::CaseInsensitive));
  patterns.push_back (QRegExp ("^.*\\.(wiki)$", Qt::CaseInsensitive));

#else

  patterns.push_back (QRegularExpression (".*(readme|news|changelog|todo)$", QRegularExpression::CaseInsensitiveOption));
  patterns.push_back (QRegularExpression ("^\\..*(rc)$", QRegularExpression::CaseInsensitiveOption));
  patterns.push_back (QRegularExpression ("^.*\\.(txt|conf|md|ini|bat|cfg|sbv|log|odt|docx|kwd|fb2|abw|rtf|epub|sxw)$", QRegularExpression::CaseInsensitiveOption));
  patterns.push_back (QRegularExpression ("^.*\\.(cpp|c|h|hh|cxx|hpp|cc|m|mm)$", QRegularExpression::CaseInsensitiveOption));
  patterns.push_back (QRegularExpression ("^.*\\.(htm|html|xml|xhtml|ts|osm|xsl)$", QRegularExpression::CaseInsensitiveOption));

#if defined(POPPLER_ENABLE)
  patterns.push_back (QRegularExpression ("^.*\\.(pdf)$", QRegularExpression::CaseInsensitiveOption));
#endif

#if defined(DJVU_ENABLE)
  patterns.push_back (QRegularExpression ("^.*\\.(djvu)$", QRegularExpression::CaseInsensitiveOption));
#endif

  patterns.push_back (QRegularExpression ("^.*\\.(awk)$", QRegularExpression::CaseInsensitiveOption));
  patterns.push_back (QRegularExpression ("^.*\\.(sh)$", QRegularExpression::CaseInsensitiveOption));
  patterns.push_back (QRegularExpression ("^.*\\.(bas|bi|vbs|vbe)$", QRegularExpression::CaseInsensitiveOption));
  patterns.push_back (QRegularExpression ("^.*\\.(d)$", QRegularExpression::CaseInsensitiveOption));
  patterns.push_back (QRegularExpression ("^.*\\.(f|for|f90|f95)$", QRegularExpression::CaseInsensitiveOption));
  patterns.push_back (QRegularExpression ("^.*\\.(java|js)$", QRegularExpression::CaseInsensitiveOption));
  patterns.push_back (QRegularExpression ("^.*\\.(ly)$", QRegularExpression::CaseInsensitiveOption));
  patterns.push_back (QRegularExpression ("^.*\\.(lout)$", QRegularExpression::CaseInsensitiveOption));
  patterns.push_back (QRegularExpression ("^.*\\.(lua)$", QRegularExpression::CaseInsensitiveOption));
  patterns.push_back (QRegularExpression ("^.*\\.(asm)$", QRegularExpression::CaseInsensitiveOption));
  patterns.push_back (QRegularExpression ("^.*\\.(nsi)$", QRegularExpression::CaseInsensitiveOption));
  patterns.push_back (QRegularExpression ("^.*\\.(pp|pas|dpr)$", QRegularExpression::CaseInsensitiveOption));
  patterns.push_back (QRegularExpression ("^.*\\.(pl|pm)$", QRegularExpression::CaseInsensitiveOption));
  patterns.push_back (QRegularExpression ("^.*\\.(php)$", QRegularExpression::CaseInsensitiveOption));
  patterns.push_back (QRegularExpression ("^.*\\.(po)$", QRegularExpression::CaseInsensitiveOption));
  patterns.push_back (QRegularExpression ("^.*\\.(py)$", QRegularExpression::CaseInsensitiveOption));
  patterns.push_back (QRegularExpression ("^.*\\.(r)$", QRegularExpression::CaseInsensitiveOption));
  patterns.push_back (QRegularExpression ("^.*\\.(sd7)$", QRegularExpression::CaseInsensitiveOption));
  patterns.push_back (QRegularExpression ("^.*\\.(tex|lyx)$", QRegularExpression::CaseInsensitiveOption));
  patterns.push_back (QRegularExpression ("^.*\\.(vala)$", QRegularExpression::CaseInsensitiveOption));
  patterns.push_back (QRegularExpression ("^.*\\.(v)$", QRegularExpression::CaseInsensitiveOption));
  patterns.push_back (QRegularExpression ("^.*\\.(wiki)$", QRegularExpression::CaseInsensitiveOption));

#endif
}
*/

bool CFTypeChecker::check (const QString &fname) const
{

#if QT_VERSION < 0x050000

  for (size_t i = 0; i < patterns.size(); ++i)
      {
       if (patterns[i].exactMatch(fname))
          return true;
      }

#else

  for (size_t i = 0; i < patterns.size(); ++i)
      {
       QRegularExpressionMatch match = patterns[i].match(fname);
       if (match.hasMatch())
          return true;
       }

#endif

  return false;
}
