/*
DJVU read code taken fromdvutxt.c:

//C- DjVuLibre-3.5
//C- Copyright (c) 2002  Leon Bottou and Yann Le Cun.
//C- Copyright (c) 2001  AT&T

//C- DjVuLibre-3.5 is derived from the DjVu(r) Reference Library from
//C- Lizardtech Software.  Lizardtech Software has authorized us to
//C- replace the original DjVu(r) Reference Library notice by the following
//C- text (see doc/lizard2002.djvu and doc/lizardtech2007.djvu):
//C-
//C-  ------------------------------------------------------------------
//C- | DjVu (r) Reference Library (v. 3.5)
//C- | Copyright (c) 1999-2001 LizardTech, Inc. All Rights Reserved.
//C- | The DjVu Reference Library is protected by U.S. Pat. No.
//C- | 6,058,214 and patents pending.
//C- |
//C- | This software is subject to, and may be distributed under, the
//C- | GNU General Public License, either Version 2 of the license,
//C- | or (at your option) any later version. The license should have
//C- | accompanied the software or you may obtain a copy of the license
//C- | from the Free Software Foundation at http://www.fsf.org .
//C- |
//C- | The computer code originally released by LizardTech under this
//C- | license and unmodified by other parties is deemed "the LIZARDTECH
//C- | ORIGINAL CODE."  Subject to any third party intellectual property
//C- | claims, LizardTech grants recipient a worldwide, royalty-free,
//C- | non-exclusive license to make, use, sell, or otherwise dispose of
//C- | the LIZARDTECH ORIGINAL CODE or of programs derived from the
//C- | LIZARDTECH ORIGINAL CODE in compliance with the terms of the GNU
//C- | General Public License.   This grant only confers the right to
//C- | infringe patent claims underlying the LIZARDTECH ORIGINAL CODE to
//C- | the extent such infringement is reasonably necessary to enable
//C- | recipient to make, have made, practice, sell, or otherwise dispose
//C- | of the LIZARDTECH ORIGINAL CODE (or portions thereof) and not to
//C- | any greater extent that may be necessary to utilize further
//C- | modifications or combinations.
//C- |
//C- | The LIZARDTECH ORIGINAL CODE is provided "AS IS" WITHOUT WARRANTY
//C- | OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
//C- | TO ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF
//C- | MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
//C- +------------------------------------------------------------------
*/

#include <iostream>


#include <QFile>
#include <QUrl>

//#include <QXmlStreamReader>
#include <QDataStream>
#include <QTextStream>
#include <QDebug>
#include <QTextCodec>
#include <QTextBrowser>
#include <QSettings>

#if QT_VERSION >= 0x050000
#include <QRegularExpression>
#else
#include <QRegExp>
#endif


//////////////////FOR PDF
#include <stdio.h>
//YOur project must also include zdll.lib (ZLIB) as a dependency.
//ZLIB can be freely downloaded from the internet, www.zlib.org
//Use 4 byte struct alignment in your project!
#include <zlib.h>
//////////////////

/*
with cmake - Qt5/Qt6 poppler
with qmake - Qt4/Qt5 poppler
*/

#if defined (POPPLER_ENABLE)

#if QT_VERSION_MAJOR == 6
#include <poppler-qt6.h>
#endif

#if QT_VERSION_MAJOR == 5
#include <poppler-qt5.h>
#endif

#if QT_VERSION_MAJOR == 4
#include <poppler-qt4.h>
#endif

#endif


#if defined (DJVU_ENABLE)
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <fcntl.h>
#include <errno.h>
#include <libdjvu/miniexp.h>
#include <libdjvu/ddjvuapi.h>
#endif


#include "tio.h"
#include "utils.h"
#include "tzipper.h"
#include "textproc.h"

#include "pugixml.hpp"

using namespace std;

extern QSettings *settings;


class CXML_walker: public pugi::xml_tree_walker
{
public:

  QString *text;
  QStringList paragraphs;
  bool fine_spaces;
  bool for_each (pugi::xml_node& node);
};


bool CXML_walker::for_each (pugi::xml_node &node)
{
  if (node.type() != pugi::node_element)
      return true;

  QString node_name = node.name();

  if (paragraphs.contains (node_name, Qt::CaseInsensitive))
     {
      QString t = node.text().as_string();

      if (! t.isEmpty())
         {
          if (fine_spaces)
            text->append ("   ");

          text->append (t);
          if (t.size() > 1)
             text->append ("\n");
         }
      }

  return true;
}


QString extract_text_from_html (const QString &string_data)
{
  QTextBrowser br;
  br.setHtml (string_data);
  return br.toPlainText();
}


QString extract_text_from_xml_pugi (const QString &string_data, const QStringList &tags)
{
  QString data;

  pugi::xml_document doc;

  pugi::xml_parse_result result = doc.load_buffer (string_data.utf16(),
                                                   string_data.size() * 2,
                                                   pugi::parse_default,
                                                   pugi::encoding_utf16);

  if (! result)
     {
      qDebug() << "NOT PARSED";
      return data;
     }

  CXML_walker walker;
  walker.text = &data;
  walker.fine_spaces = settings->value ("show_ebooks_fine", "0").toBool();
  walker.paragraphs.append (tags);

  doc.traverse (walker);

  return data;
}



/*
QString extract_text_from_xml (const QString &string_data, const QStringList &tags)
{
  QString data;
  QXmlStreamReader xml (string_data);

  bool tt = false;

  while (! xml.atEnd())
        {
         xml.readNext();

         QString tag_name = xml.qualifiedName().toString().toLower();

         for (QList <QString>::const_iterator ts = tags.begin(); ts != tags.end(); ++ts)
             {
              if (xml.isStartElement() && tag_name == *ts)
                  tt = true;

              if (xml.isEndElement() && tag_name == *ts)
                  tt = false;
             }

         if (tt && xml.isCharacters())
            {
             QString s = xml.text().toString();
             if (! s.isEmpty())
               {
                data.append (s);
                data.append("\n");
               }
             }
        }

   if (xml.hasError())
      qDebug() << "xml parse error";

  return data;
}
*/


bool CTioPlainText::load (const QString &fname)
{
  QFile file (fname);

  if (! file.open (QFile::ReadOnly))
     {
      error_string = file.errorString();
      return false;
     }

  QByteArray block = file.read (4096);

  eol = "\n";

  if (block.indexOf ("\r\n") != -1)
     eol = "\r\n";
  else
  if (block.indexOf ('\n') != -1)
     eol = "\n";
  else
  if (block.indexOf ('\r') != -1)
     eol = "\r";

  file.seek(0);

  QByteArray ba = file.readAll();
  QTextCodec *codec = QTextCodec::codecForName (charset.toUtf8().data());
  data = codec->toUnicode (ba);

  if (eol == "\r\n")
     data.replace (eol, "\n");
  else
  if (eol == "\r")
     data.replace (eol, "\n");

  file.close();

  return true;
}


bool CTioPlainText::save (const QString &fname)
{
  QFile file (fname);
  if (! file.open (QFile::WriteOnly))
     {
      error_string = file.errorString();
      return false;
     }

  QTextCodec *codec = QTextCodec::codecForName(charset.toUtf8().data());
  QByteArray ba = codec->fromUnicode(data);

  file.write(ba);
  file.close();

  return true;
}


CTioHandler::CTioHandler()
{
  default_handler = new CTioPlainText;

  list.push_back (default_handler);
  list.push_back (new CTioGzip);
  list.push_back (new CTioXMLZipped);
 // list.push_back (new CTioODT);
  list.push_back (new CTioABW);
  list.push_back (new CTioFB2);
  list.push_back (new CTioRTF);

#if QT_VERSION >= 0x050000
  list.push_back (new CTioEpub);
#endif

#if defined (POPPLER_ENABLE)
  list.push_back (new CTioPDF);
#endif

#if defined (DJVU_ENABLE)
  list.push_back (new CTioDJVU);
#endif
}


CTioHandler::~CTioHandler()
{
  if (list.size() > 0)
     for (vector <size_t>::size_type i = 0; i < list.size(); i++)
          delete list[i];
}

/*
CTio* CTioHandler::get_for_fname (const QString &fname)
{
//  qDebug() << "CTioHandler::get_for_fname ";

  CTio *instance;
  QString ext = file_get_ext (fname).toLower();

//  qDebug() << "ext: " << ext;

  for (vector <size_t>::size_type i = 0; i < list.size(); i++)
      {
       instance = list.at (i);
       if (instance->extensions.indexOf (ext) != -1)
          return instance;
      }

  return default_handler;
}
*/


CTio* CTioHandler::get_for_fname (const QString &fname)
{
  CTio *instance = 0;

  for (vector <size_t>::size_type i = 0; i < list.size(); i++)
      {
       instance = list.at (i);

       for (int i = 0; i < instance->extensions.size(); i++)
           {
            QString ext = "." + instance->extensions[i];
            if (fname.endsWith (ext))
                return instance;
           }
     }

  return default_handler;
}


CTioPlainText::CTioPlainText()
{
  ronly = false;
}


CTioGzip::CTioGzip()
{
  ronly = true;
  extensions.append ("gz");
}


bool CTioGzip::load (const QString &fname)
{
  QByteArray a = gzip_deflateFile (fname);
  data = a.data();
  return true;
}


bool CTioReadOnly::save (const QString &fname)
{
  error_string = tr ("Saving for this format is not supported");
  return false;
}


CTioABW::CTioABW()
{
  ronly = true;
  extensions.append ("abw");
}


bool CTioABW::load (const QString &fname)
{
  QString temp = qstring_load (fname);

  QStringList tags;
  tags.append ("p");

  data = extract_text_from_xml_pugi (temp, tags);

  return true;
}

/*
bool CTioODT::load (const QString &fname)
{
  data.clear();

  CZipper zipper;

  if (! zipper.read_as_utf8 (fname, "content.xml"))
     {
      qDebug() << "cannot read content.xml";
      return false;
     }

  QXmlStreamReader xml (zipper.string_data);

  bool tt = false;
  while (! xml.atEnd())
        {
         xml.readNext();

//         QString tag_name = xml.qualifiedName().toString().toLower();

         QString prefix = xml.prefix().toString().toLower();

         QString tag_name = xml.name().toString().toLower();

//         if (tag_name.isEmpty())
  //           continue;
            qDebug() << "prefix:" << xml.prefix().toString();

            qDebug() << "tag_name:" << tag_name;
            qDebug() << xml.text().toString();



         if (xml.isStartElement())
            {
             if (prefix == "text" && (tag_name == "s" || tag_name == "p"))
                {
                 QXmlStreamAttributes attrs = xml.attributes();
                 if (attrs.hasAttribute("text:c"))
                    {
                     QString av = attrs.value ("text:c").toString();
                     QString fillval;
                     fillval = fillval.fill (' ', av.toInt());
                     data.append (fillval);
                    }
                 }
            }

         if (xml.isEndElement())
            {
             if (prefix == "text" && (tag_name == "p" || tag_name == "s"))
                if (tag_name != "span")
                   tt = true;
            }

         if (xml.isCharacters() && tt)
            {
             tt = false;
             data.append (xml.text().toString());
             data.append ("\n");
            }

        }

   data = data.trimmed();

   if (xml.hasError())
      qDebug() << "xml parse error";

  return true;
}
*/

/*
bool CTioODT::load (const QString &fname)
{
  data.clear();

  CZipper zipper;

  if (! zipper.read_as_utf8 (fname, "content.xml"))
     {
      qDebug() << "cannot read content.xml";
      return false;
     }

  QXmlStreamReader xml (zipper.string_data);

  bool tt = false;
  while (! xml.atEnd())
        {
         xml.readNext();

         QString tag_name = xml.qualifiedName().toString().toLower();


//         if (tag_name.isEmpty())
  //           continue;
//            qDebug() << "prefix:" << xml.prefix().toString();

  //            qDebug() << "tag_name:" << tag_name;
           //   qDebug() << xml.text().toString();



         if (xml.isStartElement())
            {
             if (tag_name == "text:s" || tag_name == "text:p")
                {
                 QXmlStreamAttributes attrs = xml.attributes();
                 if (attrs.hasAttribute("text:c"))
                    {
                     QString av = attrs.value ("text:c").toString();
                     QString fillval;
                     fillval = fillval.fill (' ', av.toInt());
                     data.append (fillval);
                    }
                 }
            }

         if (xml.isEndElement())
            {
             if ((tag_name.startsWith ("text")) && tag_name != "text:span")
                tt = true;
            }

         if (xml.isCharacters() && tt)
            {
             tt = false;
             QString t = xml.text().toString();

             if (! t.isEmpty())
                 {
                  data.append (t);
                  if (t.size() > 1)
                     data.append ("\n");
               }
            }
        }

   data = data.trimmed();

   if (xml.hasError())
      qDebug() << "xml parse error";

  return true;
}
*/


/*
class CODT_walker: public pugi::xml_tree_walker
{
public:

  QString *text;
  bool fine_spaces;

  bool begin (pugi::xml_node &node);
  bool end (pugi::xml_node &node);
  bool for_each (pugi::xml_node& node);
};


bool CODT_walker::begin (pugi::xml_node &node)
{
 // std::cout << "begin node name = " << node.name() << std::endl;
  return true;
}


bool CODT_walker::end (pugi::xml_node &node)
{
//  std::cout << "end node name = " << node.name() << std::endl;
  return true;
}


bool CODT_walker::for_each (pugi::xml_node &node)
{
  if (node.type() != pugi::node_element)
      return true;

  QString node_name = node.name();

  if (node_name == "text:p" || node_name == "text:s")
     {
      QString t = node.text().as_string();

     // qDebug() << t;

      if (! t.isEmpty())
         {
          if (fine_spaces)
             text->append ("   ");

          text->append (t);
          if (t.size() > 1)
             text->append ("\n");
         }
      }

  return true;
}



bool CTioODT::load (const QString &fname)
{
  data.clear();

  CZipper zipper;

  if (! zipper.read_as_utf8 (fname, "content.xml"))
     {
      qDebug() << "cannot read content.xml";
      return false;
     }

  pugi::xml_document doc;
  //pugi::xml_parse_result result = doc.load_buffer (zipper.string_data.utf16(),
    //                                               zipper.string_data.size(),
      //                                             pugi::parse_default,
        //                                           pugi::encoding_utf16);






  pugi::xml_parse_result result = doc.load_buffer (zipper.string_data.toUtf8().data(),
                                                   zipper.string_data.toUtf8().size());


   if (! result)
       {
        qDebug() << "cannot parse " << fname;
        return false;
       }

   CODT_walker walker;
   walker.text = &data;
   walker.fine_spaces = settings->value ("show_ebooks_fine", "0").toBool();

   doc.traverse (walker);


  return true;
}



CTioODT::CTioODT()
{
  ronly = true;

  extensions.append ("odt");
  extensions.append ("sxw");
}
*/

CTioXMLZipped::CTioXMLZipped()
{
  ronly = true;

  extensions.append ("kwd");
  extensions.append ("docx");
  extensions.append ("odt");
  extensions.append ("sxw");
}


bool CTioXMLZipped::load (const QString &fname)
{
  data.clear();

  QStringList tags;
  QString source_fname;

  QString ext = file_get_ext (fname);

  if (ext == "kwd")
     {
      source_fname = "maindoc.xml";
      tags.append ("text");
     }
  else
  if (ext == "docx")
     {
      source_fname = "word/document.xml";
      tags.append ("w:t");
     }
  else
  if (ext == "odt" || ext == "sxw" )
     {
      source_fname = "content.xml";
      tags.append ( "text:p");
      tags.append ( "text:s");
     }


  CZipper zipper;
  if (! zipper.read_as_utf8 (fname, source_fname))
      return false;

  data = extract_text_from_xml_pugi (zipper.string_data, tags);

  return true;
}


CCharsetMagic::CCharsetMagic()
{
  QStringList fnames = read_dir_entries (":/encsign");

  CSignaturesList *koi8u = NULL;
  CSignaturesList *koi8r = NULL;

  for (int i = 0; i < fnames.size(); i++)
      {

       QString fn = fnames.at(i);
       QString fname = ":/encsign/";
       fname.append (fn);

       QByteArray a = file_load (fname);
       QList<QByteArray> bsl = a.split (',');

       CSignaturesList *sl = new CSignaturesList;
       sl->encname = fn;

       if (fn == "KOI8-R")
          koi8r = sl;

       if (fn == "KOI8-U")
          koi8u = sl;

       //fill with signatures
       for (int j = 0; j < bsl.count(); j++)
           sl->words.append (bsl[j]);

       signatures.push_back (sl);
      }

//the following is needed to correct detection KOI8-R/U

  std::vector<CSignaturesList*>::iterator it1 = std::find(signatures.begin(), signatures.end(), koi8u);
  std::vector<CSignaturesList*>::iterator it2 = std::find(signatures.begin(), signatures.end(), koi8r);
  std::swap (*it1, *it2);

//  qDebug() << "signatures.size: " << signatures.size();
/*
  int ku = signatures.indexOf (koi8u);
  int kr = signatures.indexOf (koi8r);

//#if QT_VERSION >= 0x051300
#if (QT_VERSION_MAJOR >= 5 && QT_VERSION_MINOR >= 13)
  signatures.swapItemsAt (ku, kr);
#else
  signatures.swap (ku, kr);
#endif
*/
}


CCharsetMagic::~CCharsetMagic()
{
  if (signatures.size() > 0)
     for (vector <size_t>::size_type i = 0; i < signatures.size(); i++)
          delete signatures[i];
}


//from https://stackoverflow.com/questions/28270310/how-to-easily-detect-utf8-encoding-in-the-string
bool is_valid_utf8 (const char *string)
{
    if (! string)
        return true;

    const unsigned char * bytes = (const unsigned char *)string;
    unsigned int cp;
    int num;

    while (*bytes != 0x00)
    {
        if ((*bytes & 0x80) == 0x00)
        {
            // U+0000 to U+007F
            cp = (*bytes & 0x7F);
            num = 1;
        }
        else if ((*bytes & 0xE0) == 0xC0)
        {
            // U+0080 to U+07FF
            cp = (*bytes & 0x1F);
            num = 2;
        }
        else if ((*bytes & 0xF0) == 0xE0)
        {
            // U+0800 to U+FFFF
            cp = (*bytes & 0x0F);
            num = 3;
        }
        else if ((*bytes & 0xF8) == 0xF0)
        {
            // U+10000 to U+10FFFF
            cp = (*bytes & 0x07);
            num = 4;
        }
        else
            return false;

        bytes += 1;
        for (int i = 1; i < num; ++i)
        {
            if ((*bytes & 0xC0) != 0x80)
                return false;
            cp = (cp << 6) | (*bytes & 0x3F);
            bytes += 1;
        }

        if ((cp > 0x10FFFF) ||
            ((cp >= 0xD800) && (cp <= 0xDFFF)) ||
            ((cp <= 0x007F) && (num != 1)) ||
            ((cp >= 0x0080) && (cp <= 0x07FF) && (num != 2)) ||
            ((cp >= 0x0800) && (cp <= 0xFFFF) && (num != 3)) ||
            ((cp >= 0x10000) && (cp <= 0x1FFFFF) && (num != 4)))
            return false;
    }

    return true;
}


QString CCharsetMagic::guess_for_file (const QString &fname)
{
  QString enc = "UTF-8";

  QByteArray bafile = file_load (fname);
  QString ext = file_get_ext (fname);

  if (ext == "html" ||
      ext == "htm" ||
      ext == "xhtml")
     {
      QTextCodec *defcodec = QTextCodec::codecForName ("UTF-8");
      QTextCodec *codec = QTextCodec::codecForHtml (bafile, defcodec);
      return codec->name();
     }

  if (is_valid_utf8 (bafile.data()))
     return enc;

  if (signatures.size() > 0)
     for (vector <size_t>::size_type i = 0; i < signatures.size(); i++)
          for (int x = 0; x < signatures[i]->words.count(); x++)
              {
               if (bafile.contains (signatures[i]->words[x]))
                  {
                   enc = signatures[i]->encname;
                   return enc;
                  }
              }

  return enc;
}


CTioFB2::CTioFB2()
{
  ronly = true;
  extensions.append ("fb2");
  extensions.append ("fbz");
  extensions.append ("fb2.zip");
}


class CFB2_walker: public pugi::xml_tree_walker
{
public:

  QString *text;
  bool fine_spaces;

  bool for_each (pugi::xml_node& node);
};


bool CFB2_walker::for_each (pugi::xml_node &node)
{
  if (node.type() != pugi::node_element)
      return true;

  QString node_name = node.name();

  if (node_name == "p")
     {
      if (fine_spaces)
        text->append ("   ");

      QString t = node.text().as_string();
      text->append (t);
      text->append ("\n");
     }

  if (node_name == "title" || node_name == "section" || node_name == "empty-line")
      text->append ("\n"); //НЕ ДОБАВЛЯЕТСЯ?

  return true;
}



bool CTioFB2::load (const QString &fname)
{
  data.clear();

  //QString ext = file_get_ext (fname);

  //qDebug () << "ext: " << ext;


  QString temp;

  CZipper zipper;

  if (fname.endsWith (".fb2.zip") || fname.endsWith (".fbz"))
  //if (ext == "fb2.zip" || ext == "fbz")
     {
      CZipper zipper;
      QFileInfo f (fname);

      QString source_fname = f.baseName() + ".fb2";

      if (! zipper.read_as_utf8 (fname, source_fname))
          return false;

      temp = zipper.string_data;
     }

 if (fname.endsWith ("fb2"))
    {
     QByteArray ba = file_load (fname);
     if (ba.isEmpty())
        return false;

     //read encoding:

     QString enc = string_between (QString (ba), "encoding=\"", "\"");
     if (enc.isEmpty())
        enc = "UTF-8";

     QTextCodec *codec = QTextCodec::codecForName (enc.toLatin1().data());
     temp = codec->toUnicode (ba);
   }

// qDebug() << temp;

  pugi::xml_document doc;
//  pugi::xml_parse_result result = doc.load_buffer (temp.utf16(), temp.size() * 2,
  //                                                 pugi::parse_default,
    //                                               pugi::encoding_utf16);

  pugi::xml_parse_result result = doc.load_buffer (temp.utf16(), temp.size() * 2,
                                                   pugi::parse_default,
                                                   pugi::encoding_utf16);


  if (! result)
     return false;

  //qDebug () << "2";

  CFB2_walker walker;
  walker.text = &data;
  walker.fine_spaces = settings->value ("show_ebooks_fine", "0").toBool();

  doc.traverse (walker);

  return true;
}


/*
bool CTioFB2::load (const QString &fname)
{
  data.clear();

  QString ext = file_get_ext (fname);

  QString temp;

  CZipper zipper;

  if (ext == "fb2.zip" || ext == "fbz")
     {
      CZipper zipper;
      QFileInfo f (fname);

      QString source_fname = f.baseName() + ".fb2";

      if (! zipper.read_as_utf8 (fname, source_fname))
          return false;

      temp = zipper.string_data;
     }

 if (ext == "fb2")
    {
     QByteArray ba = file_load (fname);
     if (ba.isEmpty())
        return false;

     //read encoding:

     QString enc = string_between (QString (ba), "encoding=\"", "\"");
     if (enc.isEmpty())
        enc = "UTF-8";

     QTextCodec *codec = QTextCodec::codecForName (enc.toLatin1().data());
     temp = codec->toUnicode (ba);
   }


  QString ts = "p";

  QXmlStreamReader xml (temp);

  bool tt = false;

  while (! xml.atEnd())
        {
         xml.readNext();

         QString tag_name = xml.qualifiedName().toString().toLower();

         if (xml.isStartElement())
             if (tag_name == ts)
                tt = true;

         if (xml.isEndElement())
            {
             if (tag_name == ts)
                 tt = false;

             if (tag_name == "title")
                 data.append ("\n");

             if (tag_name == "section")
                 data.append ("\n");

             if (tag_name == "empty-line")
                 data.append ("\n");
            }

         if (tt && xml.isCharacters())
            {
             QString s = xml.text().toString();
             if (! s.isEmpty())
                {
                 data.append ("   ");
                 data.append (s);
                 data.append ("\n");
                }
            }
        }

  if (xml.hasError())
    qDebug() << "xml parse error";

  return true;
}
*/

//www.codeguru.com/forum/archive/index.php/t-201658.html
//rewritten by Peter Semiletov
QString rtf_strip (const QString &rtf)
{
  int length = rtf.size();

  if (length < 4)
     return QString();

  int start = 0;

  start = rtf.indexOf ("\\pard");
  if ( start < 1)
     return QString();

  QString strCopy;
  strCopy.reserve (length);

  int k = 0;

  bool slash = false; //is backslash followed by the space
  bool figure_opened = false; //is opening figure brace followed by the space
  bool figure_closed = false; //is closing brace followed by the space
  bool first_space = false; //else spaces are in plain text and must be included to the result

  QChar ch;
  for (int j = start; j < length; j++)
      {
       ch = rtf.at (j);

       if (ch == '\\')//we are looking at the backslash
          {
           first_space = true;
           slash = true;
          }
       else
       if (ch == '{')
          {
           first_space = true;
           figure_opened = true;
          }
      else
      if (ch == '}')
         {
          first_space = true;
          figure_closed = true;
         }
      else
      if (ch == ' ')// &&
          //(rtf.indexOf ("\\datafield", j - 10) + 10) != j)
         {
          slash = false;
          figure_opened = false;
          figure_closed = false;
         }

      if ( ch == '\\')
         {
          QChar chr = rtf.at (j + 1);

          if (chr == '{') //if the text contains symbol '{'
           {
            slash = false;
            figure_opened = false;
            figure_closed = false;
            first_space = false;
            strCopy += '{';
            j++;
            k++;
            continue;
           }

      if (chr == '}') //if the text contains symbol '}'
         {
          slash = false;
          figure_opened = false;
          figure_closed = false;
          first_space = false;
          strCopy += '}';
          j++;
          k++;
          continue;
         }

      if (chr == '\\')//if the text contains symbol '\'
         {
          slash = false;
          figure_opened = false;
          figure_closed = false;
          first_space = false;
          strCopy += '\\';
          j++;
          continue;
         }
    }

    if (rtf.at (j) == '\\' &&
        rtf.at (j + 1) == 'p' &&
        rtf.at (j + 2) == 'a' &&
        rtf.at (j + 3) == 'r' &&
        rtf.at (j + 4) != 'd')
       {
        slash = false;
        figure_opened = false;
        figure_closed = false;
        first_space = false;
        strCopy += '\n';
        j += 4;
        continue;
       }

    if (slash == false &&
        figure_opened == false &&
        figure_closed == false &&
        ch != '\n')
        {
         if (! first_space)
            strCopy += ch;
         else
            first_space = false;
        }
   }

  return strCopy;
}


CTioRTF::CTioRTF()
{
  ronly = true;
  extensions.append ("rtf");
}


bool CTioRTF::load (const QString &fname)
{
  QByteArray ba = file_load (fname);

  QString text;
  text.reserve (ba.size());

  int i = 0;
  int l = ba.size();

  QString ansicgp;
  int n = ba.indexOf ("ansicpg");
  if (n != -1)
     {
      int m = ba.indexOf ('\\', n);
      n += 7;
      ansicgp = ba.mid (n, m - n);
     }

  if (ansicgp.isEmpty()) //assuming unicode
     {
      while (i < l)
             if ((ba.at(i) == '\\') && (ba.at(i + 1) == 'u'))
                {
                 QByteArray ta = ba.mid (i, 7);
                 ta = ta.mid (2, 4);
                 QChar c (ta.toInt());
                 text.append (c);
                 i += 7 + 3;
                }
             else
                 {
                  text.append (ba.at(i));
                  i++;
                 }
     }
  else
      {
       ansicgp.prepend ("CP");

       QTextCodec *codec = QTextCodec::codecForName (ansicgp.toUtf8().data());
       //qDebug() << "not unicode!";

       while (i < l)
       if ((ba.at(i) == '\\') && (ba.at(i + 1) == '\''))
          {
           QByteArray ta = ba.mid (i, 4);
           ta = ta.mid (2, 2);
           QByteArray bh = ta.fromHex (ta);
           text.append (codec->toUnicode (bh));
           i += 4;
          }
       else
           {
            text.append (ba.at(i));
            i++;
           }
       }

  data = rtf_strip (text);

  return true;
}

#if defined (POPPLER_ENABLE)

CTioPDF::CTioPDF()
{
  ronly = true;
  extensions.append ("pdf");
}


#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0))


bool CTioPDF::load (const QString &fname)
{
  Poppler::Document *d = Poppler::Document::load (fname);

  if (! d)
     return false;

  if (d->isLocked())
     {
      delete d;
      return false;
     }


  int pages_count = d->numPages();

  for (int i = 0; i < pages_count; i++)
      {
       Poppler::Page *p = d->page (i);

       QList<Poppler::TextBox*> tb = p->textList();

       for (int j = 0; j < tb.size(); j++)
           {
            data += tb[j]->text();
            //if (tb[j]->hasSpaceAfter())
            data += " ";

            delete tb[j];
           }
      }

  delete d;
  return true;
}

#endif

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0) && QT_VERSION < QT_VERSION_CHECK(7, 0, 0))


bool CTioPDF::load (const QString &fname)
{
  std::unique_ptr<Poppler::Document> d = Poppler::Document::load (fname);

  if (! d)
     return false;

  if (d->isLocked())
     {
    //  delete d;
      return false;
     }


  int pages_count = d->numPages();

  for (int i = 0; i < pages_count; i++)
      {
      std::unique_ptr<Poppler::Page> p = d->page (i);

       std::vector<std::unique_ptr<Poppler::TextBox> > tb = p->textList();

       for (int j = 0; j < tb.size(); j++)
           {
            data += tb[j]->text();
            //if (tb[j]->hasSpaceAfter())
            data += " ";

           // delete tb[j];
           }
      }

 // delete d;
  return true;
}

#endif


#endif


#if defined (DJVU_ENABLE)

const char *detail = 0;
int escape = 0;

QString temp_data_s;

ddjvu_context_t *ctx;
ddjvu_document_t *doc;


void djvumsg_handle()
{
  const ddjvu_message_t *msg;

  if (! ctx)
    return;

  msg = ddjvu_message_wait (ctx);

  while ((msg = ddjvu_message_peek (ctx)))
        {
         if (msg->m_any.tag == DDJVU_ERROR)
            {
             qDebug() << msg->m_error.message;
             qDebug() << msg->m_error.filename << ":" << msg->m_error.lineno;
             return;
            }

         ddjvu_message_pop(ctx);
        }
}


void dopage (int pageno)
{
  miniexp_t r = miniexp_nil;

  const char *lvl = (detail) ? detail : "page";

  while ((r = ddjvu_document_get_pagetext (doc, pageno, lvl)) == miniexp_dummy)
         djvumsg_handle();

  if ((r = miniexp_nth (5, r)) && miniexp_stringp (r))
     {
      const char *s = miniexp_to_str (r);
      if (s)
         {
          temp_data_s.append (s);
          temp_data_s.append ('\n');
         }
     }
}


CTioDJVU::CTioDJVU()
{
  ronly = true;
  extensions.append ("djvu");
}


bool CTioDJVU::load (const QString &fname)
{
  if (! (ctx = ddjvu_context_create ("tea")))
     return false;

  if (! (doc = ddjvu_document_create_by_filename (ctx, fname.toUtf8().data(), TRUE)))
     return false;


  while (! ddjvu_document_decoding_done (doc))
        djvumsg_handle();

  int n = ddjvu_document_get_pagenum (doc);

  for (int i = 0; i < n; i++)
      dopage (i);

  if (doc)
    ddjvu_document_release (doc);

  if (ctx)
    ddjvu_context_release (ctx);

  data = temp_data_s;
  return true;
}

#endif


#if QT_VERSION >= 0x050000


CTioEpub::CTioEpub()
{
  ronly = true;
  extensions.append ("epub");
}



bool CTioEpub::load (const QString &fname)
{
  qDebug() << "CTioEpub::load";

  data.clear();

  qDebug() << "fname: " << fname;

  CZipper zipper;
  if (! zipper.read_as_utf8 (fname, "META-INF/container.xml"))
       return false;

  QStringList html_files;

  QString opf_fname;
  QString opf_dir;

  int start = zipper.string_data.indexOf ("full-path=\"");
  int end = zipper.string_data.indexOf ("\"", start + 11);

  opf_fname = zipper.string_data.mid (start + 11, end - start - 11);
  opf_dir = opf_fname.left (opf_fname.indexOf ("/"));

  std::cout << "opf_fname: " << opf_fname.toStdString() << std::endl;
  std::cout << "opf_dir: " << opf_dir.toStdString() << std::endl;

  //READ FILES LIST. PARSE OPF FILE

  if (! zipper.read_as_utf8 (fname, opf_fname))
       return false;

  qDebug() << "PARSE XML";


  //QString regex = R"(href="([^"]*))";
  QString regex = "(href=\"([^\"]*))";


   QRegularExpression re(regex);
   QRegularExpressionMatchIterator i = re.globalMatch (zipper.string_data);
    while (i.hasNext())
      {
      QRegularExpressionMatch match = i.next();

      QString ts = match.captured().mid(6);

      ts = ts.replace ("%2C", ",");
      ts = ts.replace ("%20", " ");
      ts = ts.replace ("%5B", "[");
      ts = ts.replace ("%5D", "]");


      QString ext = file_get_ext (ts);


      if (ext == "html" || ext == "htm" || ext == "xml")
         html_files.append (opf_dir + "/" + ts);

      //qDebug().noquote() << match.captured();
    }


  //qDebug() << "html_files.size(): " << html_files.size();


/*
  pugi::xml_document doc;
  pugi::xml_parse_result result = doc.load_buffer (zipper.string_data.utf16(),
                                                   zipper.string_data.size(),
                                                   pugi::parse_default | pugi::parse_fragment,
                                                   pugi::encoding_utf16);


  std::cout << "RESULT: " << result.description() << std::endl;
  //std::cout << "Error offset: " << result.offset << "n\n";


  std::cout << "Error description: " << result.description() << "\n";
     std::cout << "Error offset: " << result.offset << " (error at [..." << (zipper.string_data.utf16() + result.offset)
               << "]\n\n";


   if (! result)
      return false;

   qDebug() << "1";

   for (pugi::xml_node_iterator it = doc.begin(); it != doc.end(); ++it)
      {
       QString tag_name = it->name();

       qDebug() << "tag_name: " << tag_name;


       if (tag_name == "item")
          {
           QString attr_href = it->attribute("href").value();
           QString ext = file_get_ext (attr_href);

           qDebug() << "attr_href: " << attr_href;

           if (ext == "html" || ext == "htm" || ext == "xml")
              html_files.append (opf_dir + "/" + attr_href);
           //std::cout << attr_href.toStdString() << std::endl;
          }
        }

//       for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait)
  //        {
    //       std::cout << " " << ait->name() << "=" << ait->value();
      //     }

       //std::cout << std::endl;

*/
   qDebug() << "2";

  QStringList tags;
  tags.append ("p");

  for (int i = 0; i < html_files.size(); i++)
      {
          qDebug() << "3";

          qDebug() << html_files.at(i);

       if (zipper.read_as_utf8 (fname, html_files.at(i)))
          {
          qDebug() << "yes";
          //QString t = extract_text_from_xml_pugi (zipper.string_data, tags);
          QString t = extract_text_from_html (zipper.string_data);
          data += t;
          data += "\n";
         }
       else
         qDebug() << "no";


      }

  return true;
}

/*
bool CTioEpub::load (const QString &fname)
{
  qDebug() << "CTioEpub::load";

  data.clear();

  qDebug() << "fname: " << fname;

  CZipper zipper;
  if (! zipper.read_as_utf8 (fname, "META-INF/container.xml"))
       return false;

  QStringList html_files;

  QString opf_fname;
  QString opf_dir;

  int start = zipper.string_data.indexOf ("full-path=\"");
  int end = zipper.string_data.indexOf ("\"", start + 11);

  opf_fname = zipper.string_data.mid (start + 11, end - start - 11);
  opf_dir = opf_fname.left (opf_fname.indexOf ("/"));

  std::cout << opf_fname.toStdString() << std::endl;
  std::cout << opf_dir.toStdString() << std::endl;

  //READ FILES LIST. PARSE OPF FILE

  if (! zipper.read_as_utf8 (fname, opf_fname))
       return false;

  QXmlStreamReader xml (zipper.string_data);

  while (! xml.atEnd())
        {
         xml.readNext();

         QString tag_name = xml.qualifiedName().toString().toLower();
         if (tag_name == "item")
            {
             QString attr_href = xml.attributes().value ("href").toString();
             QString ext = file_get_ext (attr_href);
             if (ext == "html" || ext == "htm" || ext == "xml")
                html_files.append (opf_dir + "/" + attr_href);
             //std::cout << attr_href.toStdString() << std::endl;
            }
        }

  if (xml.hasError())
      qDebug() << "xml parse error";

  QStringList tags;
  tags.append ("p");

  for (int i = 0; i < html_files.size(); i++)
      {
       if (! zipper.read_as_utf8 (fname, html_files.at(i)))
           return false;

       QString t = extract_text_from_xml (zipper.string_data, tags);
       data += t;
       data += "\n";
      }

  return true;
}
*/

#endif

QStringList CTioHandler::get_supported_exts()
{
  QStringList l;

  for (std::vector <CTio*>::iterator t = list.begin(); t != list.end(); ++t)
       for (int i = 0; i < (*t)->extensions.size(); i++)
           l.append ((*t)->extensions[i]);

  l.append ("txt");
  return l;
}
