#include "tio.h"
#include "utils.h"
#include "tzipper.h"

#include <QXmlStreamReader>
#include <QTextStream>
#include <QDebug>


bool CTioPlainText::load (const QString &fname)
{

 //qDebug() << "EOL detection - start";

  QFile feol_detector (fname);

  if (! feol_detector.open (QFile::ReadOnly))
     {
      error_string = feol_detector.errorString();
      return false;
     }

  QByteArray block = feol_detector.read (4096);

  eol = "\n";

  if (block.indexOf ("\r\n") != -1)
     eol = "\r\n";
  else
      {
       if (block.indexOf ('\n') != -1)
          eol = "\n";
       else
           if (block.indexOf ('\r') != -1)
              eol = "\r";
      }   
  /*
   if (eol == "\n")
     qDebug() << "LF";
   if (eol == "\r\n")
     qDebug() << "CRLF";
   if (eol == "\r")
     qDebug() << "CR";

  qDebug() << "EOL detection - end";
*/
  feol_detector.close();

  QFile file (fname);

  if (! file.open (QFile::ReadOnly/* | QFile::Text)*/))
     {
      error_string = file.errorString();
      return false;
     }

  QTextStream in (&file);
  in.setCodec (charset.toUtf8().data());
  data = in.readAll();

  if (eol == "\r\n")
     data.replace (eol, "\n");
  else
  if (eol == "\r")
     data.replace (eol, "\n");

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

  QTextStream out (&file);
  out.setCodec (charset.toUtf8().data());
  out << data;

  return true;
}


CTioHandler::CTioHandler()
{
  default_handler = new CTioPlainText;
  list.append (default_handler);
  list.append (new CTioGzip);
  list.append (new CTioXMLZipped);
  list.append (new CTioODTSpecial);
  list.append (new CTioABW);
  list.append (new CTioFB2);
  list.append (new CTioRTF);
}


CTioHandler::~CTioHandler()
{
  while (! list.isEmpty())
       delete list.takeFirst();
}


CTio* CTioHandler::get_for_fname (const QString &fname)
{
  CTio *instance;
  QString ext = file_get_ext (fname).toLower();

  for (int i = 0; i < list.size(); i++)
      {
       instance = list.at (i);
       if (instance->extensions.indexOf (ext) != -1)
          return instance;
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
  error_string = tr ("saving of this format is not supported");
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

  data.clear();

  QXmlStreamReader xml (temp);

  bool tt = false;

  while (! xml.atEnd())
        {
         xml.readNext();

         QString tag_name = xml.qualifiedName().toString().toLower();

         if (xml.isStartElement())
             if (tag_name == "p")
                tt = true;

         if (xml.isEndElement())
            if (tag_name == "p")
               tt = false;

         if (tt && xml.isCharacters())
            {
             QString s = xml.text().toString();
             if (! s.isEmpty())
               {
                data.append (xml.text().toString());
                data.append("\n");
               }
             }
        }

   if (xml.hasError())
      qDebug() << "xml parse error";

  return true;
}


bool CTioODTSpecial::load (const QString &fname)
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
         
         if (xml.isStartElement()) 
            {
             if (tag_name == "text:s")
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
             if (tag_name.startsWith ("text") && tag_name != "text:span")
                tt = true;
            }
      
         
         if (xml.isCharacters() && tt)
            {
             tt = false;
             data.append (xml.text().toString());
             data.append ("\n");
            }
        }
    
   if (xml.hasError()) 
      qDebug() << "xml parse error";
   
  return true;
}


CTioODTSpecial::CTioODTSpecial()
{
  ronly = true;

  extensions.append ("odt");
  extensions.append ("sxw");
}


CTioXMLZipped::CTioXMLZipped()
{
  ronly = true;

  extensions.append ("kwd");
  extensions.append ("docx");
}


bool CTioXMLZipped::load (const QString &fname)
{
  data.clear();
    
  QString source_fname;
  QString ts;  
  
  QString ext = file_get_ext (fname);

  if (ext == "kwd")
     {
      source_fname = "maindoc.xml";
      ts = "text";
     } 
  else
  if (ext == "docx")
     {
      source_fname = "word/document.xml";
      ts = "w:t";  
     }
  
  CZipper zipper;
  if (! zipper.read_as_utf8 (fname, source_fname))
       return false;
    
  QXmlStreamReader xml (zipper.string_data);
  
  bool tt = false;
  
  while (! xml.atEnd()) 
        {
         xml.readNext();

         QString tag_name = xml.qualifiedName().toString().toLower();
         
         if (xml.isStartElement()) 
             if (tag_name == ts)
                tt = true;
              
         if (xml.isEndElement()) 
            if (tag_name == ts) 
               tt = false; 

         if (tt && xml.isCharacters())
            {
             QString s = xml.text().toString();
             if (! s.isEmpty())
               {
                data.append (xml.text().toString());
                data.append("\n");
               }
             }
        } 
    
   if (xml.hasError()) 
      qDebug() << "xml parse error";
  
  return true;
}


CCharsetMagic::CCharsetMagic()
{
  QStringList fnames = read_dir_entries (":/encsign");
  
  CSignaturesList *koi8u = NULL;
  CSignaturesList *koi8r= NULL;
  
  foreach (QString fn, fnames)
          {
           QString fname (":/encsign");
           fname.append ("/").append (fn);
           QByteArray a = file_load (fname);     
           QList<QByteArray> bsl = a.split ('\n'); 
  
           CSignaturesList *sl = new CSignaturesList;
           sl->encname = fn;
           
           if (fn == "KOI8-R")
              koi8r = sl;

           if (fn == "KOI8-U")
              koi8u = sl;
           
           for (int i = 0; i < bsl.count(); i++) 
               sl->words.append (bsl[i]);        
                      
           signatures.append (sl);          
          }
          
  int ku = signatures.indexOf (koi8u);
  int kr = signatures.indexOf (koi8r);
          
  signatures.swap (ku, kr);      
}


CCharsetMagic::~CCharsetMagic()  
{
  for (int i = 0; i < signatures.count(); i++)
      delete signatures.at (i);
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

  for (int i = 0; i < signatures.count(); i++)
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
}


bool CTioFB2::load (const QString &fname)
{
  data.clear();

  QByteArray ba = file_load (fname);

  //read encoding:

  QString enc = string_between (QString (ba), "encoding=\"", "\"");

  if (enc.isEmpty())
     enc = "UTF-8";

   QTextCodec *codec = QTextCodec::codecForName (enc.toLatin1().data());
   QString temp = codec->toUnicode (ba);

   QString ts = "p";

   QXmlStreamReader xml (temp);

   bool tt = false;
   bool title = false;
   bool section = false;

   while (! xml.atEnd())
         {
          xml.readNext();

          QString tag_name = xml.qualifiedName().toString().toLower();

           if (xml.isStartElement())
              {
               if (tag_name == ts)
                  tt = true;

               if (tag_name == "title")
                  title = true;

               if (tag_name == "section")
                  section = true;

              }

           if (xml.isEndElement())
              {
               if (tag_name == ts)
                 tt = false;

               if (tag_name == "title")
                 {
                  title = false;
                  data.append("\n");
                 }

               if (tag_name == "section")
                 {
                  section = false;
                  data.append("\n");
                 }
              }

           if (tt && xml.isCharacters())
              {
               QString s = xml.text().toString();
               if (! s.isEmpty())
                 {
                  data.append ("   ");
                  data.append (s);
                  data.append("\n");
                 }
               }
          }

  if (xml.hasError())
    qDebug() << "xml parse error";

  return true;
}


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
       qDebug() << "not unicode!";

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




/*

<?xml version="1.0" encoding="windows-1252"?>
<FictionBook xmlns:l="http://www.w3.org/1999/xlink" xmlns:xlink="http://www.w3.org/1999/xlink"
 xmlns="http://www.gribuser.ru/xml/fictionbook/2.0">


*/
