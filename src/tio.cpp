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
//#include <QTextCodec>
#include <QTextBrowser>
#include <QSettings>

#if QT_VERSION >= 0x050000
#include <QRegularExpression>
#else
#include <QRegExp>
#endif

#include <algorithm>

//////////////////FOR PDF
#include <stdio.h>
//////////////////

#include <sstream>


#if defined (POPPLER_ENABLE)
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>
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

#include "pugixml.hpp"

#include "tio.h"
#include "utils.h"
#include "zip.h"

#include "enc.h"


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
      //QString t = QString::fromUtf8 (node.text().get());

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


std::vector <std::string> split_string_to_vector (const string& s, const string& delimeter, const bool keep_empty)
{
  vector <string> result;

  if (delimeter.empty())
     {
      result.push_back (s);
      return result;
     }

  string::const_iterator substart = s.begin(), subend;

  while (true)
        {
         subend = search (substart, s.end(), delimeter.begin(), delimeter.end());

         string temp (substart, subend);

         if (keep_empty || ! temp.empty())
             result.push_back (temp);

         if (subend == s.end())
             break;

         substart = subend + delimeter.size();
        }

  return result;
}


//from https://stackoverflow.com/questions/13739924/remove-all-xml-tags-from-a-stdstring
std::string xml_strip(std::string &xmlBuffer)
{
    bool copy = true;
    std::string plainString = "";   
    std::stringstream convertStream;

    // remove all xml tags
    for (int i=0; i < xmlBuffer.length(); i++)
    {                   
        convertStream << xmlBuffer[i];

        if(convertStream.str().compare("<") == 0) copy = false;
        else if(convertStream.str().compare(">") == 0) 
        {
            copy = true;
            convertStream.str(std::string());
            continue;
        }

        if (copy) 
           plainString.append(convertStream.str());       
        //
        
        convertStream.str(std::string());
    }

    return plainString;
}


/*
vector <string> split_string_to_vector (const string& s, const string& delimeter, const bool keep_empty)
{
  vector <string> result;

  if (delimeter.empty())
     {
      result.push_back (s);
      return result;
     }

  string::const_iterator substart = s.begin(), subend;

  while (true)
        {
         subend = search (substart, s.end(), delimeter.begin(), delimeter.end());

         string temp (substart, subend);

         if (keep_empty || ! temp.empty())
             result.push_back (temp);

         if (subend == s.end())
             break;

         substart = subend + delimeter.size();
        }

  return result;
}
*/

std::string join_lines (const std::vector <std::string> &lst, const std::string &delim)
{
  std::string ret;
  
  //for (const auto &s : lst) 
  for (size_t i = 0; i < lst.size(); i++)
      {
       if (! ret.empty())
           ret += delim;
          
       //ret += s;
       ret += lst[i]; 
     }
  return ret;
}

std::string xml_strip_remove_empty_lines (std::string &xmlBuffer)
{
  std::string s = xml_strip (xmlBuffer);
  
  std::vector <std::string> v = split_string_to_vector (s, "\n", false);

  return join_lines (v, "\n");
  
  //str.find_first_not_of(" \t\n\v\f\r") != std::string::npos

}




std::string html_strip (const std::string &source)
{
  std::string html = source;

  while (html.find ('<') != std::string::npos)
        {
         size_t startpos = html.find ('<');
         size_t endpos = html.find ('>') + 1;

         if (endpos != std::string::npos)
            html.erase (startpos, endpos - startpos);
        }

  return html;
}


template <typename T> void remove_duplicates(std::vector<T>& vec)
{
  std::sort(vec.begin(), vec.end());
  vec.erase (std::unique (vec.begin(), vec.end()), vec.end());
}


std::vector <std::string> extract_hrefs (const std::string &source, const std::string &prefix)
{
  std::vector <std::string> result;

  size_t i = 0;
  size_t limit = source.size() - 1;
  std::string signature_str = "<item href=\"";
  size_t signature_size = signature_str.size();


  while (i < limit)
        {
         size_t pos_start = source.find (signature_str, i);
         if (pos_start == string::npos)
            break;

         size_t pos_end = source.find ('\"', pos_start + signature_size);
         if (pos_end == string::npos)
            break;

        //else

         std::string url = source.substr (pos_start + signature_size, pos_end - (pos_start + signature_size));

         if (ends_with (url, "html") || ends_with (url, "xhtml"))
            if (url.rfind("wrap", 0) != 0)
                 result.push_back (prefix + url);

         i = pos_end + 1;
        }

  return result;
}


QString extract_text_from_xml_pugi (const char *string_data, size_t datasizebytes, const QStringList &tags)
{
  QString data;

  pugi::xml_document doc;

  pugi::xml_parse_result result = doc.load_buffer (string_data,
                                                   datasizebytes,
                                                   pugi::parse_default,
                                                   pugi::encoding_utf8);
  if (! result)
     {
      std::cout << "NOT PARSED\n";
      return QString("");
     }

  CXML_walker walker;
  walker.text = &data;
  walker.fine_spaces = settings->value ("show_ebooks_fine", "0").toBool();
  walker.paragraphs.append (tags);

  doc.traverse (walker);

  return data;
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
QString extract_text_from_html (const QString &string_data)
{
  QTextBrowser br;
  br.setHtml (string_data);
  return br.toPlainText();
}
*/


bool CTioPlainText::load (const QString &fname)
{
  data.clear();

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

  /*
  QByteArray ba = file.readAll();
  QTextCodec *codec = QTextCodec::codecForName (charset.toUtf8().data());
  data = codec->toUnicode (ba);
*/

  QByteArray ba = file.readAll();

  //qDebug() << "charset:" << charset;

//ok
  if (charset == "UTF-8")
     {
      //ushort* filedata = TextConverter::ConvertFromUTF8ToUTF16 (ba.data());
      //data = QString::fromUtf16 (filedata);
      //delete [] filedata;
      data = QString::fromUtf8 (ba.data());

     }

 if (charset == "UTF-16")
     data = QString::fromUtf16 ((UTF16TEXT *)ba.data());


  //ok
  if (charset == "CP-1251")
     {
      UTF16TEXT* filedata = CTextConverter::ConvertFromCP1251ToUTF16 (ba.data());
      data = QString::fromUtf16 (filedata);
      delete [] filedata;
     }


  //ok
  if (charset == "CP-866")
     {
      UTF16TEXT* filedata = CTextConverter::ConvertFromDOS866ToUTF16 (ba.data());

      data = QString::fromUtf16 (filedata);
      delete [] filedata;
     // std::cout << *filedata << std::endl;
   //   qDebug() << ba.data();
  //    std::wcout << reinterpret_cast<wchar_t*>(filedata) << std::endl;
     }

  if (charset == "KOI8-R")
     {
      UTF16TEXT* filedata = CTextConverter::ConvertFromKOI8RToUTF16 (ba.data());
      data = QString::fromUtf16 (filedata);
      delete [] filedata;
     // std::cout << *filedata << std::endl;
   //   qDebug() << ba.data();
  //    std::wcout << reinterpret_cast<wchar_t*>(filedata) << std::endl;
     }




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


   qDebug() << "Saving at encoding: " << charset;
//  QTextCodec *codec = QTextCodec::codecForName(charset.toUtf8().data());
//  QByteArray ba = codec->fromUnicode(data);

  if (charset == "UTF-8")
    {
     QByteArray ba = data.toUtf8();
     file.write(ba);
     file.close();
     return true;
    }


  if (charset == "UTF-16")
    {
     QByteArray ba = QByteArray((const char*)data.utf16(), (data.length() + 1) * 2);
     file.write (ba);
     file.close();
     return true;
    }

   //ok
  if (charset == "CP-1251")
    {
     char* text = CTextConverter::ConvertFromUTF16ToCP1251 ((UTF16TEXT*)data.utf16());
     file.write (text);
     file.close();
     delete [] text;
     return true;
    }


  if (charset == "KOI8-R") //ok
    {
     char* text = CTextConverter::ConvertFromUTF16ToKOI8R ((UTF16TEXT*)data.utf16());
     file.write (text);
     file.close();
     delete [] text;
     return true;
    }


  if (charset == "CP-866") //ноль
     {
      char* text = CTextConverter::ConvertFromUTF16ToCP866 ((UTF16TEXT*)data.utf16());
      file.write (text);
      file.close();
      delete [] text;
      return true;
     }

  return false;
}


CTioHandler::CTioHandler()
{
  default_handler = new CTioPlainText;

  list.push_back (default_handler);
  //list.push_back (new CTioGzip);
  list.push_back (new CTioXMLZipped);
  list.push_back (new CTioABW);
  list.push_back (new CTioFB2);
  list.push_back (new CTioRTF);
  list.push_back (new CTioEpub);

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


CTio* CTioHandler::get_for_fname (const QString &fname)
{
  CTio *instance = 0;

  //с нуля лишено смысла, ибо list[0] содержит умолчальный обработчик
  //в котором не создан список расширений файлов для обработки
  for (vector <size_t>::size_type i = 0; i < list.size(); i++)
      {
       instance = list.at (i);

       for (int j = 0; j < instance->extensions.size(); j++)
           {
            QString ext = "." + instance->extensions[j];
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
  data.clear();


/*  QByteArray a = gzip_deflateFile (fname);
  data = a.data();
  return true;*/
  return false;
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
  data.clear();

  QString temp = qstring_load (fname);

  QStringList tags;
  tags.append ("c");

  data = extract_text_from_xml_pugi (temp, tags);

  return true;
}


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

  std::string fn = fname.toStdString();

  QStringList tags;
  std::string source_fname;

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
      tags.append ("text:p");
      tags.append ("text:s");
     }


  void *buf = NULL;
  size_t bufsize = 0;

  struct zip_t *zip = zip_open (fn.c_str(), 0, 'r');

  if (! zip)
     return false;


  if (zip_entry_open (zip, source_fname.c_str()) < 0)
     return false;


  zip_entry_read (zip, &buf, &bufsize);

  zip_entry_close (zip);

  data = extract_text_from_xml_pugi ((char*)buf, bufsize, tags);

  zip_close (zip);
  free(buf);

  return true;
}

/*
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

}


CCharsetMagic::~CCharsetMagic()
{
  if (signatures.size() > 0)
     for (vector <size_t>::size_type i = 0; i < signatures.size(); i++)
          delete signatures[i];
}
*/

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


//QString CCharsetMagic::guess_for_file (const QString &fname)
//{
 //String enc = "UTF-8";
/*
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
*/
//  return enc;
//}


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



std::string string_between (const std::string &source,
                            const std::string &sep1,
                            const std::string &sep2)
{
    std::string result;
    size_t pos1 = source.find(sep1);
    if (pos1 == std::string::npos)
        return result;

    size_t pos2 = source.find(sep2, pos1 + sep1.size());
    if (pos2 == std::string::npos)
        return result;

    pos1 += sep1.size();

    result = source.substr(pos1, pos2 - pos1);
    return result;
}


bool CTioFB2::load (const QString &fname)
{

//  std::cout << "CTioFB2::load (const QString &fname" << std::endl;

  data.clear();

  QStringList tags;
  tags.append ("p");

  std::string zip_file_name = fname.toStdString();

  if (fname.endsWith ("fb2"))
     {
      std::string stemp = string_file_load (zip_file_name);

      if (stemp.find ("encoding=\"windows-1251\"")  != std::string::npos)
         {
  //        std::cout << "1251^^^^^^^^^^^^^^^^^^^^^" << std::endl;
          UTF16TEXT *t_utf16 = CTextConverter::ConvertFromCP1251ToUTF16 (stemp.c_str());
          QString td = QString::fromUtf16 (t_utf16);
          delete [] t_utf16;

          data = extract_text_from_xml_pugi (td, tags);

          if (data.isEmpty())
             return false;
          else
              return true;
         }


      data = extract_text_from_xml_pugi (stemp.c_str(), stemp.size(), tags);

      if (data.isEmpty())
         return false;
      else
          return true;
     }

   // else
 // if (fname.endsWith (".fb2.zip") || fname.endsWith (".fbz"))

  std::string source_fname; //достаем его из зипа

  //we can have malformed internal filename, so find the first fb2 at the archive

  struct zip_t *zip = zip_open (zip_file_name.c_str(), 0, 'r');

  if (! zip)
     return false;

  int n = zip_entries_total(zip);

  for (int i = 0; i < n; ++i)
      {
       zip_entry_openbyindex (zip, i);

       const char *name = zip_entry_name (zip);

       std::string tname = name;

       if (ends_with (tname, "fb2"))
          {
           source_fname = tname;
           zip_entry_close(zip);
           break;
          }

        //int isdir = zip_entry_isdir(zip);
        //unsigned long long size = zip_entry_size(zip);
        //unsigned int crc32 = zip_entry_crc32(zip);
       zip_entry_close(zip);
      }


  if (source_fname.empty())
      return false;

  void *buf = NULL;
  size_t bufsize;


  if (zip_entry_open (zip, source_fname.c_str()) < 0)
     return false;

  zip_entry_read (zip, &buf, &bufsize);

  zip_entry_close (zip);

  zip_close(zip);

 // QString enc = string_between (tmp, "encoding=\"", "\"");

  //bool need_to_recode = false;
  //if (strstr ((char*)buf, "encoding=\"windows-1251\""))
//     need_to_recode = true;

  std::string stemp =(char*)buf;

  free (buf);

  if (stemp.find ("encoding=\"windows-1251\"")  != std::string::npos)
     {
  //   std::cout << "1251^^^^^^^^^^^^^^^^^^^^^ ZIPPED" << std::endl;
      UTF16TEXT *t_utf16 = CTextConverter::ConvertFromCP1251ToUTF16 (stemp.c_str());
      QString td = QString::fromUtf16 (t_utf16);
      delete [] t_utf16;

      data = extract_text_from_xml_pugi (td, tags);

      if (data.isEmpty())
         return false;
      else
          return true;
     }


  data = extract_text_from_xml_pugi (stemp.c_str(), stemp.size(), tags);

  if (data.isEmpty())
     return false;
  else
      return true;
}

/*
  pugi::xml_document doc;

  pugi::xml_parse_result result = doc.load_buffer ((char*)buf, bufsize,
                                                   pugi::parse_default,
                                                   pugi::encoding_utf8);


  free(buf);

  if (! result)
     return false;


//  if (stemp.find ("encoding=\"windows-1251\"")  != std::string::npos)

  CFB2_walker walker;
  walker.text = &data;
  walker.fine_spaces = settings->value ("show_ebooks_fine", "0").toBool();

  doc.traverse (walker);
*/



/*
bool CTioFB2::load (const QString &fname)
{

  std::cout << "CTioFB2::load (const QString &fname" << std::endl;

  data.clear();

  QStringList tags;
  tags.append ("p");


  std::string zip_file_name = fname.toStdString();

  if (fname.endsWith ("fb2"))
     {
      std::string stemp = string_file_load (zip_file_name);

      if (stemp.find ("encoding=\"windows-1251\"")  != std::string::npos)
         {
          std::cout << "1251^^^^^^^^^^^^^^^^^^^^^" << std::endl;
          UTF16TEXT *t_utf16 = CTextConverter::ConvertFromCP1251ToUTF16 (stemp.c_str());
          QString td = QString::fromUtf16 (t_utf16);
          delete [] t_utf16;

          data = extract_text_from_xml_pugi (td, tags);

          if (data.isEmpty())
             return false;
          else
              return true;
         }


      data = extract_text_from_xml_pugi (stemp.c_str(), stemp.size(), tags);

      if (data.isEmpty())
         return false;
      else
          return true;
     }

   // else
 // if (fname.endsWith (".fb2.zip") || fname.endsWith (".fbz"))



  std::string source_fname; //достаем его из зипа

  //we can have malformed internal filename, so find the first fb2 at the archive

  struct zip_t *zip = zip_open (zip_file_name.c_str(), 0, 'r');

  if (! zip)
     return false;


  int n = zip_entries_total(zip);

  for (int i = 0; i < n; ++i)
      {
       zip_entry_openbyindex (zip, i);

       const char *name = zip_entry_name (zip);

       std::string tname = name;

       if (ends_with (tname, "fb2"))
          {
           source_fname = tname;
           zip_entry_close(zip);
           break;
          }

        //int isdir = zip_entry_isdir(zip);
        //unsigned long long size = zip_entry_size(zip);
        //unsigned int crc32 = zip_entry_crc32(zip);
       zip_entry_close(zip);
      }


  if (source_fname.empty())
      return false;


  void *buf = NULL;
  size_t bufsize;


  if (zip_entry_open (zip, source_fname.c_str()) < 0)
     return false;

  zip_entry_read (zip, &buf, &bufsize);

  zip_entry_close (zip);

  zip_close(zip);

 // QString enc = string_between (tmp, "encoding=\"", "\"");


  bool need_to_recode = false;
  if (strstr (buf, "encoding=\"windows-1251\""))
     need_to_recode = true;


  pugi::xml_document doc;

  pugi::xml_parse_result result = doc.load_buffer ((char*)buf, bufsize,
                                                   pugi::parse_default,
                                                   pugi::encoding_utf8);


  free(buf);

  if (! result)
     return false;


//  if (stemp.find ("encoding=\"windows-1251\"")  != std::string::npos)

  CFB2_walker walker;
  walker.text = &data;
  walker.fine_spaces = settings->value ("show_ebooks_fine", "0").toBool();

  doc.traverse (walker);


  return true;
}*/
/*
bool CTioFB2::load (const QString &fname)
{
  data.clear();


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

  QString strCopy;strCopy.reserve (length);

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

//BROKEN
bool CTioRTF::load (const QString &fname)
{

  data.clear();

  QByteArray ba = file_load (fname);
/*
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

  return true;*/

  return false;
}

#if defined (POPPLER_ENABLE)

CTioPDF::CTioPDF()
{
  ronly = true;
  extensions.append ("pdf");
}


bool CTioPDF::load (const QString &fname)
{
  data.clear();


  poppler::document *d = poppler::document::load_from_file (fname.toStdString ());

  if (! d)
     return false;

  if (d->is_locked())
     {
      delete d;
      return false;
     }

  int pages_count = d->pages();

  for (int i = 0; i < pages_count; i++)
      {
       poppler::page *p = d->create_page (i);

       if (! p)
          continue;

       poppler::ustring text_from_page = p->text();

       poppler::byte_array ba = text_from_page.to_utf8();

       char *str = &*ba.begin();
       data += QString::fromUtf8 (str);

       delete p;
      }

  delete d;
  return true;
}

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
   data.clear();


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



CTioEpub::CTioEpub()
{
  ronly = true;
  extensions.append ("epub");
}


std::vector <std::string> extract_src_from_toc (const std::string &source, const std::string &prefix)
{
  std::vector <std::string> result;

  size_t i = 0;
  size_t limit = source.size() - 1;
  std::string signature_str = "src=\"";
  size_t signature_size = signature_str.size();

  while (i < limit)
        {
         size_t pos_start = source.find (signature_str, i);
         if (pos_start == std::string::npos)
            break;

         size_t pos_end = source.find ('\"', pos_start + signature_size);
         if (pos_end == std::string::npos)
            break;

         //else

         std::string url = source.substr (pos_start + signature_size, pos_end - (pos_start + signature_size));
         
//         qDebug() << "url:" << url;

         //find "#" if any
         //remove after # to the end of string

         size_t pos_part = url.find ('#');
         if (pos_end != std::string::npos)
             url = url.substr (0, pos_part);

         if (ends_with (url, "html") || ends_with (url, "htm") || ends_with (url, "xhtml") || ends_with (url, "xml"))
            {
             std::string url_to_add = prefix + url;
             result.push_back (url_to_add);
            }

         i = pos_end + 1;
        }

  return result;
}


std::vector <std::string> extract_src_from_opf (const std::string &source, const std::string &prefix)
{
  std::vector <std::string> result;

  size_t i = 0;
  size_t limit = source.size() - 1;
  std::string signature_str = "href=\"";
  size_t signature_size = signature_str.size();

  qDebug() << "limit: " << limit;  

  while (i < limit)
        {
         size_t pos_start = source.find (signature_str, i);
         
         qDebug() << "pos_start: " << pos_start;  
         
         if (pos_start == std::string::npos)
            break;

         size_t pos_end = source.find ('\"', pos_start + signature_size);
         
         qDebug() << "pos_end: " << pos_end;  
 
         
         if (pos_end == std::string::npos)
            break;

         //else

         

         std::string url = source.substr (pos_start + signature_size, pos_end - (pos_start + signature_size));
         
        // qDebug() << "url:" << url;

         //find "#" if any
         //remove after # to the end of string

         size_t pos_part = url.find ('#');
         if (pos_end != std::string::npos)
             url = url.substr (0, pos_part);

         if (ends_with (url, "html") || ends_with (url, "htm") || ends_with (url, "xhtml") || ends_with (url, "xml"))
            {
             std::string url_to_add = prefix + url;
             result.push_back (url_to_add);
            }

         i = pos_end + 1;
        }

  return result;
}


/*

переписать алгоритм!

1 делаем анзип
2 открываем META-INF/container.xml
3 читаем оттуда full-path: <rootfile full-path="OPS/package.opf" media-type="application/oebps-package+xml" />            
4. Открываем full-path
5. Парсим все <item href="images/image089.jpg" id="image089" media-type="image/jpeg"/>              
6. проверяем является ли расширение href == xml, xhtml, html, htm
7. если да, создаем url и добавляем в список
*/


std::string str_between(const std::string &source,
                           const std::string &sep1,
                           const std::string &sep2)
{
    std::string result;
    size_t pos1 = source.find(sep1);
    if (pos1 == std::string::npos)
        return result;

    size_t pos2 = source.find(sep2, pos1 + sep1.size());
    if (pos2 == std::string::npos)
        return result;

    pos1 += sep1.size();

    result = source.substr(pos1, pos2 - pos1);
    return result;
}

bool CTioEpub::load (const QString &fn)
{

  qDebug() << "CTioEpub::load " << fn;

  data.clear();


  std::string fname = fn.toStdString();
  std::vector <std::string> tags;

  struct zip_t *zip = zip_open (fname.c_str(), 0, 'r');

  
  if (! zip)
     return false;
  
  
  //read META-INF/container.xml

  void *buffer = NULL;
  size_t bufsize;

  std::string subdir;

  if (zip_entry_open (zip, "META-INF/container.xml") < 0) //не можем открыть просто toc?
      return false;

  zip_entry_read (zip, &buffer, &bufsize);

  zip_entry_close (zip);

  if (bufsize == 0)
    return false;

  
 // done with container_xml
  std::string content ((char*)buffer);
  free (buffer);

  //qDebug() << "buffer:"  << buffer;


//читаем оттуда full-path: <rootfile full-path="OPS/package.opf" media-type="application/oebps-package+xml" />

   std::string rootfile_path = str_between (content, "rootfile full-path=\"", "\"");

   //get subdir!
   
   size_t separator_pos = rootfile_path.find ("/");
   
   if (separator_pos != std::string::npos)
      {
       subdir = rootfile_path.substr (0, separator_pos + 1);
      // qDebug() << "subdir: " << subdir;
      }
   

  
//read rootfile

  if (zip_entry_open (zip, rootfile_path.c_str()) < 0) //не можем открыть просто toc?
      return false;

  zip_entry_read (zip, &buffer, &bufsize);

  zip_entry_close (zip);

  if (bufsize == 0)
    return false;

  std::string root_file_buffer ((char*)buffer);
  free (buffer);



  std::vector <std::string> urls = extract_src_from_opf (root_file_buffer, subdir);
//  remove_duplicates (urls);

  
  //HERE WE ALREADY PARSED URLS

  
  if (urls.size() == 0)
     return false;

  tags.push_back ("p");

  //read urls from epub

  
  
  for (size_t i = 0; i < urls.size(); i++)
      {
        //check duplicated urls, skip dups
        if (i + 1 != urls.size())
           if (urls[i] == urls[i+1])
              continue;
        
       // qDebug() << "i: " << i;
 

//       std::cout << "open: " << urls[i] << std::endl;

       void *temp = NULL;

       if (zip_entry_open (zip, urls[i].c_str()) >= 0)
          {
         //  qDebug() << "opened " << urls[i].c_str();
  
            
           zip_entry_read (zip, &temp, &bufsize);
           zip_entry_close (zip);

//           qDebug() << "readed and closed";
           
           std::string st; 
           
           if (temp)
              st = (char*) temp;
            
            
           //std::string st_cleaned = html_strip (st); 

           std::string st_cleaned; 
           
          // if (ends_with (urls[i].c_str(), "xml") || ends_with (urls[i].c_str(), "xhtml"))
              
           
           //st_cleaned = xml_strip (st); 
              
             st_cleaned =  xml_strip_remove_empty_lines (st);
              
           //else
             //  st_cleaned = html_strip (st); 

           
  //         qDebug() << "cleanded";
           
           
          //print_lines (file_lines);

          //if (file_lines.size() > 0)
           //  lines.insert(std::end(lines), std::begin(file_lines), std::end(file_lines));

           data += QString::fromStdString (st_cleaned);
           data += "\n";
           free (temp);
          }
      }

  zip_close(zip);

  return true;
}


/*
bool CTioEpub::load (const QString &fn)
{

  qDebug() << "CTioEpub::load " << fn;

  data.clear();


  std::string fname = fn.toStdString();
  std::vector <std::string> tags;

  struct zip_t *zip = zip_open (fname.c_str(), 0, 'r');

  qDebug() << "1";
  
  if (! zip)
     return false;
  
  qDebug() << "2";
  
  //read toc.ncx

  void *toc = NULL;
  size_t bufsize;

   
 
  
  std::string subdir;

  if (zip_entry_open (zip, "toc.ncx") < 0) //не можем открыть просто toc?
     {
      subdir = "OEBPS/";
  
       qDebug() << "3";
      
      if (zip_entry_open (zip, "OEBPS/toc.ncx") < 0) //пробуем еще так
         {
         qDebug() << "4";
  
         
          if (zip_entry_open (zip, "OPS/toc.ncx") < 0) //и так
             return false;

           qDebug() << "5";
  

         subdir = "OPS/";
        }
     }


   qDebug() << "subdir:" << subdir;
     
 
  zip_entry_read (zip, &toc, &bufsize);

  zip_entry_close (zip);

  if (bufsize == 0)
    return false;

  
 // done with toc
  std::string content ((char*)toc);
  free (toc);

  std::vector <std::string> urls = extract_src_from_toc (content, subdir);
//  remove_duplicates (urls);

 
  
  //HERE WE ALREADY PARSED URLS

//  qDebug() << "urls.size(): " << urls.size();

  
  if (urls.size() == 0)
     return false;

  tags.push_back ("p");

  //read urls from epub

  
  
  for (size_t i = 0; i < urls.size(); i++)
      {
        //check duplicated urls, skip dups
        if (i + 1 != urls.size())
           if (urls[i] == urls[i+1])
              continue;
        
       // qDebug() << "i: " << i;
 

//       std::cout << "open: " << urls[i] << std::endl;

       void *temp = NULL;

       if (zip_entry_open (zip, urls[i].c_str()) >= 0)
          {
         //  qDebug() << "opened " << urls[i].c_str();
  
            
           zip_entry_read (zip, &temp, &bufsize);
           zip_entry_close (zip);

//           qDebug() << "readed and closed";
           
           std::string st; 
           
           if (temp)
              st = (char*) temp;
            
            
           //std::string st_cleaned = html_strip (st); 

           std::string st_cleaned; 
           
          // if (ends_with (urls[i].c_str(), "xml") || ends_with (urls[i].c_str(), "xhtml"))
              
           
           //st_cleaned = xml_strip (st); 
              
             st_cleaned =  xml_strip_remove_empty_lines (st);
              
           //else
             //  st_cleaned = html_strip (st); 

           
  //         qDebug() << "cleanded";
           
           
          //print_lines (file_lines);

          //if (file_lines.size() > 0)
           //  lines.insert(std::end(lines), std::begin(file_lines), std::end(file_lines));

           data += QString::fromStdString (st_cleaned);
           data += "\n";
           free (temp);
          }
      }

  zip_close(zip);

  return true;
}
*/

/*
QStringList CTioHandler::get_supported_exts()
{
  QStringList l;

  for (std::vector <CTio*>::iterator t = list.begin(); t != list.end(); ++t)
       for (int i = 0; i < (*t)->extensions.size(); i++)
           l.append ((*t)->extensions[i]);

  l.append ("txt");
  return l;
}
*/
