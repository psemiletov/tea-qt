/***************************************************************************
 *   2007-2023 by Peter Semiletov                                          *
 *   peter.semiletov@gmail.com                                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 **************************************************************************/

/* some code by
Copyright (C) 2006-2008 Trolltech ASA. All rights reserved.
*/

/*
code from qPEditor:
Copyright (C) 2007 by Michael Protasov, mik-protasov@anyqsoft.com
*/

/*
Diego Iastrubni <elcuco@kde.org> //some GPL'ed code from new-editor-diego-3,
found on qtcentre forum
*/

/*
code from qwriter:
*   Copyright (C) 2009 by Gancov Kostya                                   *
*   kossne@mail.ru                                                        *
*/

#include <bitset>
#include <algorithm>

#include <QTextCodec>
#include <QApplication>
#include <QClipboard>
#include <QSettings>
#include <QLabel>
#include <QPainter>
#include <QDebug>
#include <QUrl>
#include <QDir>
#include <QMimeData>
#include <QFile>
#include <QMessageBox>
#include <QTimer>

#include "document.h"
#include "utils.h"
#include "gui_utils.h"

#include "pugixml.hpp"


#define SK_A 38
#define SK_D 40
#define SK_W 25
#define SK_S 39
#define SK_E 26
#define SK_C 54


QHash <QString, QString> global_palette;
QSettings *settings;
QMenu *menu_current_files;
int recent_list_max_items;
bool b_recent_off;
bool b_destroying_all;


QStringList text_get_bookmarks (const QString &s, const QString &sstart, const QString &send)
{
  QStringList result;

  int c = s.size();
  int i = 0;

  while (i < c)
        {
         int start = s.indexOf (sstart, i, Qt::CaseInsensitive);

         if (start == -1)
             break;

         int end = s.indexOf (send, start + sstart.length());
         if (end == -1)
             break;

         result.prepend (s.mid (start + sstart.length(), (end - start) - send.length()));

         i = end + 1;
        }

  return result;
}


QTextCharFormat tformat_from_style (const QString &fontstyle, const QString &color, int darker_val)
{
  QTextCharFormat tformat;
  tformat.setForeground (QBrush (QColor (color).darker (darker_val)));

  if (fontstyle.isEmpty())
     return tformat;

  if (fontstyle.contains ("bold"))
      tformat.setFontWeight (QFont::Bold);

  if (fontstyle.contains ("italic"))
      tformat.setFontItalic (true);

  if (fontstyle.contains ("underline"))
      tformat.setFontUnderline (true);

  if (fontstyle.contains ("strikeout"))
      tformat.setFontStrikeOut (true);

  return tformat;
}


CSyntaxHighlighter::CSyntaxHighlighter (QTextDocument *parent, CDocument *doc, const QString &fname): QSyntaxHighlighter (parent)
{
  document = doc;
  casecare = true;
}


#if QT_VERSION < 0x050000

CSyntaxHighlighterQRegExp::CSyntaxHighlighterQRegExp (QTextDocument *parent, CDocument *doc, const QString &fname):
                                                      CSyntaxHighlighter (parent, doc, fname)
{
  document = doc;
  load_from_xml_pugi (fname);
}

/*
void CSyntaxHighlighterQRegExp::load_from_xml (const QString &fname)
{
  if (! file_exists (fname))
     return;

  cs = Qt::CaseSensitive;

  comment_start_expr = make_pair (QRegExp(), false);
  comment_end_expr = make_pair (QRegExp(), false);

  QString temp = qstring_load (fname);
  if (temp.isEmpty())
     return;

  int darker_val = settings->value ("darker_val", 100).toInt();

  QXmlStreamReader xml (temp);

  while (! xml.atEnd())
        {
         xml.readNext();

         QString tag_name = xml.name().toString().toLower();

         if (xml.isStartElement())
            {
             //if (tag_name == "document")
               // {
               //  exts = xml.attributes().value ("exts").toString();
               //  langs = xml.attributes().value ("langs").toString();
              //  }

             if (tag_name == "item")
                {
                 QString attr_type = xml.attributes().value ("type").toString();
                 QString attr_name = xml.attributes().value ("name").toString();

                 if (attr_name == "options")
                    {
                     QString s_casecare = xml.attributes().value ("casecare").toString();
                     if (! s_casecare.isEmpty())
                        if (s_casecare == "0" || s_casecare == "false")
                           casecare = false;

                     if (! casecare)
                        cs = Qt::CaseInsensitive;
                     }

                 if (attr_type == "keywords")
                    {
                     QString color = hash_get_val (global_palette, xml.attributes().value ("color").toString(), "darkBlue");
                     QTextCharFormat fmt = tformat_from_style (xml.attributes().value ("fontstyle").toString(), color, darker_val);

		     QString element = xml.readElementText().trimmed().remove('\n');
		     if (element.isEmpty())
		         continue;

                     QRegExp rg (element, cs);

                     if (! rg.isValid())
                         qDebug() << "! valid " << rg.pattern();
                     else
                         hl_rules.push_back (make_pair (rg, fmt));

                     } //keywords
                 else
                 if (attr_type == "item")
                    {
                     QString color = hash_get_val (global_palette, xml.attributes().value ("color").toString(), "darkBlue");
                     QTextCharFormat fmt = tformat_from_style (xml.attributes().value ("fontstyle").toString(), color, darker_val);
                     QString element = xml.readElementText().trimmed().remove('\n');
		     if (element.isEmpty())
		         continue;

		     QRegExp rg (element, cs);
                     if (rg.isValid())
                         hl_rules.push_back (make_pair (rg, fmt));
                    }
                 else
                 if (attr_type == "mcomment-start")
                    {
                     QString color = hash_get_val (global_palette, xml.attributes().value ("color").toString(), "gray");
                     QTextCharFormat fmt = tformat_from_style (xml.attributes().value ("fontstyle").toString(), color, darker_val);

                     fmt_multi_line_comment = fmt;
                     QString element = xml.readElementText().trimmed().remove('\n');
                     if (! element.isEmpty())
                        //commentStartExpression = QRegExp (element, cs, QRegExp::RegExp);
                       {
                        comment_start_expr.first = QRegExp (element, cs, QRegExp::RegExp);
                        comment_start_expr.second = true;
                       }

                    }
                 else
                 if (attr_type == "mcomment-end")
                    {
                     QString element = xml.readElementText().trimmed().remove('\n');
                     if (! element.isEmpty())
                        //commentEndExpression = QRegExp (element, cs, QRegExp::RegExp);
                       {
                        comment_end_expr.first = QRegExp (element, cs, QRegExp::RegExp);
                        comment_end_expr.second = true;
                       }

                    }
                 else
                 if (attr_type == "comment")
                    {
                     QString element = xml.readElementText().trimmed().remove('\n');
                     if (element.isEmpty())
		        continue;

                     if (xml.attributes().value ("name").toString() == "cm_mult")
                         comment_mult = element;
                     else
                         if (xml.attributes().value ("name").toString() == "cm_single")
                            comment_single = element;
                    }
                }//item

       }//is start

  if (xml.hasError())
     qDebug() << "xml parse error";

  } //cycle
}
*/

void CSyntaxHighlighterQRegExp::highlightBlock (const QString &text)
{
 
  if (hl_rules.size() == 0)
      return;
/*
  for (auto &p: hl_rules)
     {
      int index  = text.indexOf (p.first);
       
      while (index >= 0)
            {
             int length = p.first.matchedLength();
             if (length == 0)
               continue;

             setFormat (index, length, p.second);
             index = text.indexOf (p.first, index + length);
            }
      }*/

  for (vector <pair <QRegExp, QTextCharFormat> >::iterator p = hl_rules.begin(); p != hl_rules.end(); ++p)
      {
       int index  = text.indexOf (p->first);

       while (index >= 0)
             {
             int length = p->first.matchedLength();
             if (length == 0)
               continue;

             setFormat (index, length, p->second);
             index = text.indexOf (p->first, index + length);
            }
      }


  if (! comment_start_expr.second && ! comment_end_expr.second)
     return;


  setCurrentBlockState (0);

  int startIndex = 0;

  //if (commentStartExpression.isEmpty() || commentEndExpression.isEmpty())
    // return;

  if (previousBlockState() != 1)
     startIndex = text.indexOf (comment_start_expr.first);

  while (startIndex >= 0)
        {
         int endIndex = comment_end_expr.first.indexIn (text, startIndex);

         int commentLength;

         if (endIndex == -1)
            {
             setCurrentBlockState (1);
             commentLength = text.length() - startIndex;
            }
         else
             commentLength = endIndex - startIndex + comment_end_expr.first.matchedLength();

         setFormat (startIndex, commentLength, fmt_multi_line_comment);
         startIndex = text.indexOf (comment_start_expr.first, startIndex + commentLength);
        }
}


class CXMLHL_walker: public pugi::xml_tree_walker
{
public:

  CSyntaxHighlighterQRegExp *hl;
  int darker_val;

  bool for_each (pugi::xml_node& node);
};


bool CXMLHL_walker::for_each (pugi::xml_node &node)
{
  if (node.type() != pugi::node_element)
      return true;

  QString tag_name = node.name();

  if (tag_name == "item")
     {
      pugi::xml_attribute attr = node.attribute ("type");
      QString attr_type = attr.as_string();
      attr = node.attribute ("name");
      QString attr_name = attr.as_string();

      if (attr_name == "options")
         {
          attr = node.attribute ("casecare");

          QString s_casecare = attr.as_string();
          if (! s_casecare.isEmpty())
             if (s_casecare == "0" || s_casecare == "false")
                hl->casecare = false;

          if (! hl->casecare)
              hl->cs = Qt::CaseInsensitive;
         }

      if (attr_type == "keywords")
         {
          attr = node.attribute ("color");
          QString color = hash_get_val (global_palette, attr.as_string(), "darkBlue");

          attr = node.attribute ("fontstyle");
          QTextCharFormat fmt = tformat_from_style (attr.as_string(), color, darker_val);

          QString t = node.text().as_string();
          QString element = t.trimmed().remove('\n');
          if (element.isEmpty())
             return true;

          QRegExp rg (element, hl->cs);

          if (! rg.isValid())
             qDebug() << "! valid " << rg.pattern();
          else
              hl->hl_rules.push_back (make_pair (rg, fmt));

          } //keywords
      else
      if (attr_type == "item")
         {
          attr = node.attribute ("color");
          QString color = hash_get_val (global_palette, attr.as_string(), "darkBlue");

          attr = node.attribute ("fontstyle");
          QTextCharFormat fmt = tformat_from_style (attr.as_string(), color, darker_val);

          QString t = node.text().as_string();
          QString element = t.trimmed().remove('\n');
          if (element.isEmpty())
             return true;

          QRegExp rg (element, hl->cs);
          if (rg.isValid())
              hl->hl_rules.push_back (make_pair (rg, fmt));
          }
       else
       if (attr_type == "mcomment-start")
          {
           attr = node.attribute ("color");

           QString color = hash_get_val (global_palette, attr.as_string(), "gray");

           attr = node.attribute ("fontstyle");

           QString fontstyle = attr.as_string();
           QTextCharFormat fmt = tformat_from_style (fontstyle, color, darker_val);

           hl->fmt_multi_line_comment = fmt;

           QString t = node.text().as_string();
           QString element = t.trimmed().remove('\n');
           if (element.isEmpty())
              return true;

           if (! element.isEmpty())
              {
               hl->comment_start_expr.first = QRegExp (element, hl->cs, QRegExp::RegExp);
               hl->comment_start_expr.second = true;
             }
          }
       else
       if (attr_type == "mcomment-end")
          {
           QString t = node.text().as_string();
           QString element = t.trimmed().remove('\n');
           if (element.isEmpty())
              return true;

           if (! element.isEmpty())
             {
              hl->comment_end_expr.first = QRegExp (element, hl->cs, QRegExp::RegExp);
              hl->comment_end_expr.second = true;
             }
          }
       else
       if (attr_type == "comment")
          {
           attr = node.attribute ("name");
           QString name = attr.as_string();

           QString t = node.text().as_string();
           QString element = t.trimmed().remove('\n');
           if (element.isEmpty())
              return true;

           if (name == "cm_mult")
               hl->comment_mult = element;
           else
           if (name == "cm_single")
               hl->comment_single = element;
          }

      }//item

  return true;
}


void CSyntaxHighlighterQRegExp::load_from_xml_pugi (const QString &fname)
{
  if (! file_exists (fname))
     return;

  cs = Qt::CaseSensitive;

  comment_start_expr = make_pair (QRegExp(), false);
  comment_end_expr = make_pair (QRegExp(), false);

  QString temp = qstring_load (fname);
  if (temp.isEmpty())
     return;

  int darker_val = settings->value ("darker_val", 100).toInt();

  pugi::xml_document doc;
  pugi::xml_parse_result result = doc.load_buffer (temp.utf16(),
                                                   temp.size() * 2,
                                                   pugi::parse_default,
                                                   pugi::encoding_utf16);

   //pugi::xml_parse_result result = doc.load_buffer (temp.toUtf8().data(),
     //                                               temp.toUtf8().size());

   if (! result)
      return;

   CXMLHL_walker walker;
   walker.darker_val = darker_val;
   walker.hl = this;

   doc.traverse (walker);
}

#endif


#if QT_VERSION >= 0x050000

CSyntaxHighlighterQRegularExpression::CSyntaxHighlighterQRegularExpression (QTextDocument *parent, CDocument *doc, const QString &fname):
                                                                            CSyntaxHighlighter (parent, doc, fname)
{
  document = doc;
  load_from_xml_pugi (fname);
}

/*
void CSyntaxHighlighterQRegularExpression::load_from_xml (const QString &fname)
{
  if (! file_exists (fname))
     return;

  casecare = true;

  comment_start_expr = make_pair (QRegularExpression(), false);
  comment_end_expr = make_pair (QRegularExpression(), false);

  QString temp = qstring_load (fname);
  if (temp.isEmpty())
     return;

  int darker_val = settings->value ("darker_val", 100).toInt();

  QXmlStreamReader xml (temp);

  while (! xml.atEnd())
        {
         xml.readNext();

         QString tag_name = xml.name().toString().toLower();

         if (xml.isStartElement())
            {
             if (tag_name == "item")
                {
                 QString attr_type = xml.attributes().value ("type").toString();
                 QString attr_name = xml.attributes().value ("name").toString();

                 if (attr_name == "options")
                    {
                     QString s_casecare = xml.attributes().value ("casecare").toString();
                     if (! s_casecare.isEmpty())
                        if (s_casecare == "0" || s_casecare == "false")
                           casecare = false;

                     if (! casecare)
                        pattern_opts = pattern_opts | QRegularExpression::CaseInsensitiveOption;
                    }

                 if (attr_type == "keywords")
                    {
                     QString color = hash_get_val (global_palette, xml.attributes().value ("color").toString(), "darkBlue");
                     QTextCharFormat fmt = tformat_from_style (xml.attributes().value ("fontstyle").toString(), color, darker_val);

                     QString element = xml.readElementText().trimmed().remove('\n');
                     if (element.isEmpty())
                        continue;

                     QRegularExpression rg = QRegularExpression (element, pattern_opts);

                     if (! rg.isValid())
                        qDebug() << "! valid " << rg.pattern();
                     else
                          hl_rules.push_back (make_pair (rg, fmt));

                     } //keywords
                 else
                 if (attr_type == "item")
                    {
                     QString color = hash_get_val (global_palette, xml.attributes().value ("color").toString(), "darkBlue");
                     QTextCharFormat fmt = tformat_from_style (xml.attributes().value ("fontstyle").toString(), color, darker_val);

                     QString element = xml.readElementText().trimmed().remove('\n');
                     if (element.isEmpty())
                        continue;

                     QRegularExpression rg = QRegularExpression (element, pattern_opts);
                     if (! rg.isValid())
                        qDebug() << "! valid " << rg.pattern();
                     else
                         hl_rules.push_back (make_pair (rg, fmt));
                    }
                  else
                  if (attr_type == "mcomment-start")
                     {
                      QString color = hash_get_val (global_palette, xml.attributes().value ("color").toString(), "gray");
                      QTextCharFormat fmt = tformat_from_style (xml.attributes().value ("fontstyle").toString(), color, darker_val);

                      fmt_multi_line_comment = fmt;

                      QString element = xml.readElementText().trimmed().remove('\n');
                      if (element.isEmpty())
                         continue;

                      QRegularExpression rg = QRegularExpression (element, pattern_opts);
                      if (! rg.isValid())
                         qDebug() << "! valid " << rg.pattern();
                      else
                          {
                           comment_start_expr.first = rg;
                           comment_start_expr.second = true;
                          }
                     }
                  else
                  if (attr_type == "mcomment-end")
                     {
                      QString element = xml.readElementText().trimmed().remove('\n');
                      if (element.isEmpty())
                         continue;

                      QRegularExpression rg = QRegularExpression (element, pattern_opts);
                      if (! rg.isValid())
                         qDebug() << "! valid " << rg.pattern();
                      else
                          {
                           comment_end_expr.first = rg;
                           comment_end_expr.second = true;
                          }

                     }
                  else
                  if (attr_type == "comment")
                     {
                      if (xml.attributes().value ("name").toString() == "cm_mult")
                          comment_mult = xml.readElementText().trimmed();
                      else
                      if (xml.attributes().value ("name").toString() == "cm_single")
                          comment_single = xml.readElementText().trimmed();
                     }

                 }//item

       }//end of "is start"

   if (xml.hasError())
      qDebug() << "xml parse error";

  } //cycle
}
*/


class CXMLHL_walker: public pugi::xml_tree_walker
{
public:

  CSyntaxHighlighterQRegularExpression *hl;
  int darker_val;

  bool for_each (pugi::xml_node& node);
};


bool CXMLHL_walker::for_each (pugi::xml_node &node)
{
  if (node.type() != pugi::node_element)
      return true;

//  std::cout << "for_each name = " << node.name() << std::endl;

  QString tag_name = node.name();

  if (tag_name == "item")
     {
      pugi::xml_attribute attr = node.attribute ("type");
      QString attr_type = attr.as_string();
      attr = node.attribute ("name");
      QString attr_name = attr.as_string();

      if (attr_name == "options")
         {
//          attr = node.attribute ("casecare");
          QString s_casecare = node.attribute ("casecare").as_string();
          if (! s_casecare.isEmpty())
             if (s_casecare == "0" || s_casecare == "false")
                hl->casecare = false;

          if (! hl->casecare)
             hl->pattern_opts = hl->pattern_opts | QRegularExpression::CaseInsensitiveOption;
         }

      if (attr_type == "keywords")
         {
          //attr = node.attribute ("color");
          QString color = hash_get_val (global_palette, node.attribute ("color").as_string(), "darkBlue");

          //attr = node.attribute ("fontstyle");
          QTextCharFormat fmt = tformat_from_style (node.attribute ("fontstyle").as_string(), color, darker_val);

          QString t = node.text().as_string();
          QString element = t.trimmed().remove('\n');
          if (element.isEmpty())
             return true;

          QRegularExpression rg = QRegularExpression (element, hl->pattern_opts);

          if (! rg.isValid())
             qDebug() << "! valid " << rg.pattern();
          else
              hl->hl_rules.push_back (make_pair (rg, fmt));

          } //keywords
      else
      if (attr_type == "item")
         {
//          attr = node.attribute ("color");
          QString color = hash_get_val (global_palette, node.attribute ("color").as_string(), "darkBlue");

//          attr = node.attribute ("fontstyle");
          QTextCharFormat fmt = tformat_from_style (node.attribute ("fontstyle").as_string(), color, darker_val);

          QString t = node.text().as_string();
          QString element = t.trimmed().remove('\n');
          if (element.isEmpty())
             return true;

          QRegularExpression rg = QRegularExpression (element, hl->pattern_opts);
          if (! rg.isValid())
             qDebug() << "! valid " << rg.pattern();
          else
              hl->hl_rules.push_back (make_pair (rg, fmt));
         }
       else
       if (attr_type == "mcomment-start")
          {
  //         attr = node.attribute ("color");
           QString color = hash_get_val (global_palette, node.attribute ("color").as_string(), "gray");

           //attr = node.attribute ("fontstyle");
           QString fontstyle = node.attribute ("fontstyle").as_string();
           QTextCharFormat fmt = tformat_from_style (fontstyle, color, darker_val);

           hl->fmt_multi_line_comment = fmt;

           QString t = node.text().as_string();
           QString element = t.trimmed().remove('\n');
           if (element.isEmpty())
              return true;

           QRegularExpression rg = QRegularExpression (element, hl->pattern_opts);
           if (! rg.isValid())
              qDebug() << "! valid " << rg.pattern();
           else
               {
                hl->comment_start_expr.first = rg;
                hl->comment_start_expr.second = true;
               }
          }
       else
       if (attr_type == "mcomment-end")
          {
           QString t = node.text().as_string();
           QString element = t.trimmed().remove('\n');
           if (element.isEmpty())
              return true;

           QRegularExpression rg = QRegularExpression (element, hl->pattern_opts);
           if (! rg.isValid())
              qDebug() << "! valid " << rg.pattern();
           else
               {
                hl->comment_end_expr.first = rg;
                hl->comment_end_expr.second = true;
               }
          }
       else
       if (attr_type == "comment")
          {
           //attr = node.attribute ("name");
           QString name = node.attribute ("name").as_string();

           QString t = node.text().as_string();
           QString element = t.trimmed().remove('\n');
           if (element.isEmpty())
              return true;

           if (name == "cm_mult")
               hl->comment_mult = element;
           else
           if (name == "cm_single")
               hl->comment_single = element;
          }

      }//item

  return true;
}


void CSyntaxHighlighterQRegularExpression::load_from_xml_pugi (const QString &fname)
{
  if (! file_exists (fname))
     return;

  casecare = true;

  comment_start_expr = make_pair (QRegularExpression(), false);
  comment_end_expr = make_pair (QRegularExpression(), false);

  QString temp = qstring_load (fname);
  if (temp.isEmpty())
     return;

  int darker_val = settings->value ("darker_val", 100).toInt();

  pugi::xml_document doc;
  pugi::xml_parse_result result = doc.load_buffer (temp.utf16(),
                                                   temp.size() * 2,
                                                   pugi::parse_default,
                                                   pugi::encoding_utf16);
  if (! result)
     return;

   CXMLHL_walker walker;
   walker.darker_val = darker_val;
   walker.hl = this;

   doc.traverse (walker);
}


void CSyntaxHighlighterQRegularExpression::highlightBlock (const QString &text)
{
  if (hl_rules.size() == 0)
      return;

  for (vector <pair <QRegularExpression, QTextCharFormat> >::iterator p = hl_rules.begin(); p != hl_rules.end(); ++p)
      {
       QRegularExpressionMatch m = p->first.match (text);

       int index = m.capturedStart();

       while (index >= 0)
             {
              int length = m.capturedLength();
              if (length == 0)
                 continue;

              setFormat (index, length, p->second);
              m = p->first.match (text, index + length);
              index = m.capturedStart();
             }
      }
  /*
  for (const auto& p: hl_rules)
      {
       QRegularExpressionMatch m = p.first.match (text);

       int index = m.capturedStart();

       while (index >= 0)
             {
              int length = m.capturedLength();
              if (length == 0)
                 continue;

              setFormat (index, length, p.second);
              m = p.first.match (text, index + length);
              index = m.capturedStart();
             }
       }
*/

  if (! comment_start_expr.second && ! comment_end_expr.second)
     return;

  setCurrentBlockState (0);

  int startIndex = 0;

  QRegularExpressionMatch m_start = comment_start_expr.first.match (text);

  if (previousBlockState() != 1)
      startIndex = m_start.capturedStart();

  while (startIndex >= 0)
        {
         QRegularExpressionMatch m_end = comment_end_expr.first.match (text, startIndex);

         int endIndex = m_end.capturedStart();

         int commentLength;

         if (endIndex == -1)
            {
             setCurrentBlockState (1);
             commentLength = text.length() - startIndex;
            }
         else
             commentLength = endIndex - startIndex + m_end.capturedLength();

         setFormat (startIndex, commentLength, fmt_multi_line_comment);

         m_start = comment_start_expr.first.match (text, startIndex + commentLength);

         startIndex = m_start.capturedStart();
        }
}
#endif


QMimeData* CDocument::createMimeDataFromSelection() const
{
  if (has_rect_selection())
     {
      QMimeData *md = new QMimeData;
      md->setText (rect_sel_get());
      return md;
     }

  return QPlainTextEdit::createMimeDataFromSelection();
}


bool CDocument::canInsertFromMimeData (const QMimeData *source) const
{
//  if (source->hasFormat ("text/uri-list"))
  //    return true;
  //else
  return true;
}


void CDocument::insertFromMimeData (const QMimeData *source)
{
  QString fname;
  QFileInfo info;

  bool b_ins_plain_text = ! source->hasUrls();

  if (source->hasUrls() && source->urls().at(0).scheme() != "file")
      b_ins_plain_text = true;

  if (b_ins_plain_text)
     {
      QPlainTextEdit::insertFromMimeData (source);
      return;
     }

   QList<QUrl> l = source->urls();

   for (QList <QUrl>::iterator u = l.begin(); u != l.end(); ++u)
       {
        fname = u->toLocalFile();
        info.setFile(fname);
        if (info.isFile())
           holder->open_file (fname, "UTF-8");
       }
}


void CDocument::paintEvent (QPaintEvent *event)
{
  if (draw_margin || highlight_current_line)
     {
      QPainter painter (viewport());

      if (highlight_current_line)
         {
          QRect r = cursorRect();
          r.setX (0);
          r.setWidth (viewport()->width());
          painter.fillRect (r, QBrush (current_line_color));
         }

      if (draw_margin)
         {
          QPen pen = painter.pen();
          pen.setColor (margin_color);
          painter.setPen (pen);
          painter.drawLine (margin_x, 0, margin_x, viewport()->height());
         }
     }

  QPlainTextEdit::paintEvent (event);
}


void CDocument::keyPressEvent (QKeyEvent *event)
{
 // qDebug() << event->nativeScanCode() ;

 // qDebug() << int_to_binary (event->nativeModifiers());
  // std::bitset<32> btst (event->nativeModifiers());

   /*
   for (std::size_t i = 0; i < btst.size(); ++i) {
        qDebug() << "btst[" << i << "]: " << btst[i];
    }
   */

   //LSHIFT = 0
//LCTRL = 2
//LALT = 3
  //LWIN = 6

  if (settings->value ("wasd", "0").toBool())
     {
      bitset <32> btst (event->nativeModifiers());
      QTextCursor cr = textCursor();

      QTextCursor::MoveMode m = QTextCursor::MoveAnchor;

      if (btst[3] == 1 || btst[6] == 1) //LALT or LWIN
         {
          int visible_lines;

          if (btst[6] == 1)
             m = QTextCursor::KeepAnchor;

          switch (event->nativeScanCode())
                 {
                  case SK_W:
                            cr.movePosition (QTextCursor::Up, m);
                            setTextCursor (cr);
                            return;

                  case SK_S:
                            cr.movePosition (QTextCursor::Down, m);
                            setTextCursor (cr);
                            return;

                  case SK_A:
                            cr.movePosition (QTextCursor::Left, m);
                            setTextCursor (cr);
                            return;

                  case SK_D:
                            cr.movePosition (QTextCursor::Right, m);
                            setTextCursor (cr);
                            return;

                  case SK_E:
                            visible_lines = height() / fontMetrics().height();
                            cr.movePosition (QTextCursor::Down, m, visible_lines);
                            setTextCursor (cr);
                            return;

                  case SK_C:
                            visible_lines = height() / fontMetrics().height();
                            cr.movePosition (QTextCursor::Up, m, visible_lines);
                            setTextCursor (cr);
                            return;
                }
       }
    }


  if (auto_indent && event->key() == Qt::Key_Return)
     {
      calc_auto_indent();
      QPlainTextEdit::keyPressEvent (event);
      textCursor().insertText (indent_val);
      return;
     }

  if (event->key() == Qt::Key_Tab)
     {
      indent();
      event->accept();
      return;
     }

  if (event->key() == Qt::Key_Backtab)
     {
      un_indent();
      event->accept();
      return;
     }

  if (event->key() == Qt::Key_Insert)
     {
      setOverwriteMode (! overwriteMode());
      event->accept();
      return;
     }
/*

#if QT_VERSION >= 0x050000

  if (event->key() == Qt::Key_C && (event->modifiers().testFlag(Qt::ControlModifier)))
     {
      QString t = get();

      if (t.isEmpty())
         {
          event->accept();
          return;
         }

#if defined(Q_OS_WIN) || defined(Q_OS_OS2)
      t = t.replace (QChar::ParagraphSeparator, "\r\n");
#elif defined(Q_OS_UNIX)
      t = t.replace (QChar::ParagraphSeparator, "\n");
#endif

      QClipboard *clipboard = QApplication::clipboard();

      clipboard->setText (t);

      event->accept();
      return;
     }
#endif
*/


  if (event->key() == Qt::Key_C && (event->modifiers().testFlag(Qt::ControlModifier)))
     {
      ed_copy();
      event->accept();
      return;
     }


  if (event->key() == Qt::Key_X && (event->modifiers().testFlag(Qt::ControlModifier)))
     {
      ed_cut();
      event->accept();
      return;
     }

  if (event->key() == Qt::Key_V && (event->modifiers().testFlag(Qt::ControlModifier)))
     {
      ed_paste();
      event->accept();
      return;
     }


  QPlainTextEdit::keyPressEvent (event);
}


void CDocument::resizeEvent (QResizeEvent *e)
{
  QPlainTextEdit::resizeEvent (e);
  QRect cr = contentsRect();
  line_num_area->setGeometry (QRect (cr.left(), cr.top(), line_number_area_width(), cr.height()));
}


void CDocument::wheelEvent (QWheelEvent *e)
{
  if (e->modifiers() & Qt::ControlModifier)
     {
#if QT_VERSION < 0x050000
      const int delta = e->delta();
#else
      const int delta = e->angleDelta().y();
#endif
      QFont f = font();
      int sz = f.pointSize();
      
      if (delta > 0)
	  sz++;
      
      if (delta < 0)
	  sz--;
            
      f.setPointSize(sz);
      setFont(f);

      return;
     }

  QPlainTextEdit::wheelEvent(e);
}


void CDocument::contextMenuEvent (QContextMenuEvent *event)
{
  QMenu *menu = new QMenu (this);

  QAction *a = menu->addAction (tr("Copy"));
  connect (a, SIGNAL(triggered()), this, SLOT(ed_copy()));

  a = menu->addAction (tr("Cut"));
  connect (a, SIGNAL(triggered()), this, SLOT(ed_cut()));

  a = menu->addAction (tr("Paste"));
  connect (a, SIGNAL(triggered()), this, SLOT(ed_paste()));

  menu->addSeparator();

  a = menu->addAction (tr("Undo"));
  connect (a, SIGNAL(triggered()), this, SLOT(undo()));

  a = menu->addAction (tr("Redo"));
  connect (a, SIGNAL(triggered()), this, SLOT(redo()));

  menu->exec(event->globalPos());
  delete menu;
}


CDocument::CDocument (CDox *hldr, QWidget *parent): QPlainTextEdit (parent)
{
  holder = hldr;

  highlighter = 0;
  tab_page = 0;
  markup_mode = "HTML";
  file_name = tr ("new[%1]").arg (QTime::currentTime().toString ("hh-mm-ss"));
  cursor_xy_visible = true;
  charset = "UTF-8";
  text_to_search = "";
  position = 0;
  margin_pos = 72;
  margin_x = brace_width * margin_pos;
  draw_margin = false;
  hl_brackets = false;
  auto_indent = false;
  tab_sp_width = 8;
  spaces_instead_of_tabs = true;
  highlight_current_line = false;
  show_tabs_and_spaces = false;

#if defined(Q_OS_WIN) || defined(Q_OS_OS2)
  eol = "\r\n";
#elif defined(Q_OS_UNIX)
  eol = "\n";
#endif

  rect_sel_reset();
  setup_brace_width();

  line_num_area = new CLineNumberArea (this);

  connect (this, SIGNAL(selectionChanged()), this, SLOT(slot_selectionChanged()));
  connect (this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth()));
  connect (this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
  connect (this, SIGNAL(cursorPositionChanged()), this, SLOT(cb_cursorPositionChanged()));

  updateLineNumberAreaWidth();

  document()->setUseDesignMetrics (true);

  QString s_sel_back_color = hash_get_val (global_palette, "sel-background", "black");
  QString s_sel_text_color = hash_get_val (global_palette, "sel-text", "white");

  int darker_val = settings->value ("darker_val", 100).toInt();

  sel_text_color = QColor (s_sel_text_color).darker(darker_val).name();
  sel_back_color = QColor (s_sel_back_color).darker(darker_val).name();
  current_line_color = QColor (hash_get_val (global_palette, "cur_line_color",
                               "#EEF6FF")).darker (settings->value ("darker_val", 100).toInt()).name();

  holder->items.push_back (this);
  int tab_index = holder->tab_widget->addTab (this, file_name);
  tab_page = holder->tab_widget->widget (tab_index);


  //QAction *actCopy;
//  QAction *actCut;
  //QAction *actPaste;

  //actCopy = new QAction (tr("Copy"), this);
  //connect (actCopy, SIGNAL(triggered()), this, SLOT(ed_copy()));

//   setContextMenuPolicy (Qt::PreventContextMenu);

/*
  set dots color, put to hl init
 spaceFormat = QtGui.QTextCharFormat()
        spaceFormat.setForeground(QtCore.Qt.red)
        self.highlightingRules.append((QtCore.QRegExp(" "), spaceFormat))
*/
  setFocus (Qt::OtherFocusReason);
}


CDocument::~CDocument()
{
  if (holder->autosave_files.contains (file_name))
     {
      file_save_with_name (file_name, charset);
      document()->setModified (false);
     }

  if (file_name.endsWith (".notes") && document()->isModified())
     file_save_with_name_plain (file_name);
  else
  if (file_name.startsWith (holder->dir_config) && document()->isModified())
     file_save_with_name_plain (file_name);
  else
      if (document()->isModified() && file_exists (file_name))
         if (QMessageBox::warning (0, "TEA",
                                   tr ("%1 has been modified.\n"
                                       "Do you want to save your changes?").arg (file_name),
                                       QMessageBox::Yes,
                                       QMessageBox::No) == QMessageBox::Yes)
            file_save_with_name (file_name, charset);


  if (! file_name.startsWith (holder->dir_config) && ! file_name.endsWith (".notes"))
     {
      holder->add_to_recent (this);
      holder->update_recent_menu();
     }

  if (file_name.startsWith (holder->todo.dir_days))
      holder->todo.load_dayfile();

//  QMainWindow *w = qobject_cast <QMainWindow *> (holder->parent_wnd);
  QMainWindow *w = holder->parent_wnd;

  w->setWindowTitle ("");

  int i = holder->tab_widget->indexOf (tab_page);
  if (i != -1)
     holder->tab_widget->removeTab (i);
}


QString CDocument::get() const
{
  return textCursor().selectedText();
}


void CDocument::put (const QString &value)
{
  textCursor().insertText (value);
}


void CDocument::ed_copy()
{
  if (! has_selection())
     return;

  QString t = get();

#if defined(Q_OS_UNIX)
    t = t.replace (QChar::ParagraphSeparator, "\n");
#endif

#if defined(Q_OS_WIN) || defined(Q_OS_OS2)
    t = t.replace (QChar::ParagraphSeparator, "\r\n");
#endif

  QClipboard *clipboard = QApplication::clipboard();

  clipboard->setText (t);
}


void CDocument::ed_cut()
{
  if (! has_selection())
     return;

  QString t = get();

#if defined(Q_OS_UNIX)
  t = t.replace (QChar::ParagraphSeparator, "\n");
#endif

#if defined(Q_OS_WIN) || defined(Q_OS_OS2)
  t = t.replace (QChar::ParagraphSeparator, "\r\n");
#endif

  QClipboard *clipboard = QApplication::clipboard();

  clipboard->setText (t);

  textCursor().insertText ("");
}


void CDocument::ed_paste()
{
  QClipboard *clipboard = QApplication::clipboard();
  textCursor().insertText (clipboard->text());
}


bool CDocument::has_selection()
{
  return textCursor().hasSelection();
}

/*
QMenu* CDocument::createStandardContextMenu()
{
  QMenu *m = new QMenu();

  m->addAction (actCopy);

  return m;
}
*/


bool CDocument::file_open (const QString &fileName, const QString &codec)
{
  //qDebug() << "CDocument::file_open " << fileName;

  CTio *tio = holder->tio_handler.get_for_fname (fileName);

  //qDebug() << "tio->metaObject()->className()" << tio->metaObject()->className();

  if (! tio)
      return false;

  if (fileName.contains (holder->dir_config))
      tio->charset = "UTF-8";
  else
      tio->charset = codec;

  if (! tio->load (fileName))
     {
      holder->log->log (tr ("cannot open %1 because of: %2")
                           .arg (fileName)
                           .arg (tio->error_string));
      return false;
     }

  setPlainText (tio->data);

  charset = tio->charset;
  eol = tio->eol;
  file_name = fileName;

  holder->update_project (file_name);

  set_tab_caption (QFileInfo (file_name).fileName());
  set_hl();
  set_markup_mode();
  document()->setModified (false);

  holder->log->log (tr ("%1 is open").arg (file_name));

  QMutableListIterator <QString> i (holder->recent_files);

  while (i.hasNext())
        {
         QStringList lt = i.next().split ("*");
         if (lt.size() > 0)
             if (lt.at(0) == file_name)
                i.remove();
        }

  return true;
}


bool CDocument::file_save_with_name (const QString &fileName, const QString &codec)
{
  CTio *tio = holder->tio_handler.get_for_fname (fileName);

  if (! tio)
      return false;

  if (fileName.contains (holder->dir_config))
      tio->charset = "UTF-8";
  else
      tio->charset = codec;

  tio->data = toPlainText();

  if (eol != "\n")
      tio->data.replace ("\n", eol);

  if (! tio->save (fileName))
     {
      holder->log->log (tr ("cannot save %1 because of: %2")
                        .arg (fileName)
                        .arg (tio->error_string));
      return false;
     }

  if (! b_destroying_all)
     {
      charset = tio->charset;
      file_name = fileName;

      set_tab_caption (QFileInfo (file_name).fileName());

      holder->log->log (tr ("%1 is saved").arg (file_name));

      update_title (settings->value ("full_path_at_window_title", 1).toBool());
      update_status();

      document()->setModified (false);

      holder->update_current_files_menu();
      holder->update_project (file_name);
     }

  return true;
}


bool CDocument::file_save_with_name_plain (const QString &fileName)
{
  QFile file (fileName);
  if (! file.open (QFile::WriteOnly))
      return false;

  QTextCodec *codec = QTextCodec::codecForName (charset.toUtf8().data());
  if (! codec)
     return false;

  QByteArray ba = codec->fromUnicode (toPlainText());

  file.write (ba);
  file.close();

  holder->update_current_files_menu();

  return true;
}


int CDocument::get_tab_idx()
{
  return holder->tab_widget->indexOf (tab_page);
}


QString CDocument::get_triplex()
{
  if (! file_exists (file_name))
     return QString ("");

  QString s (file_name);
  s += "*";
  s += charset;
  s += "*";
  s += QString::number (textCursor().position());
  s += "*";

  if (! get_word_wrap())
     s+="0";
  else
      s+="1";

  return s;
}


QString CDocument::get_filename_at_cursor()
{
  if (textCursor().hasSelection())
     {
      QFileInfo nf (file_name);
      QDir cd (nf.absolutePath());
      return cd.cleanPath (cd.absoluteFilePath (textCursor().selectedText()));
     }

  QString s = textCursor().block().text();
  if (s.isEmpty())
     return QString();

  QString x;

  QString sep_start;
  QString sep_end;

  if (markup_mode == "LaTeX")
     {
      sep_start = "{";
      sep_end = "}";
     }
  else
  if (markup_mode == "Markdown")
     {
      sep_start = "(";
      sep_end = ")";
     }

  if (markup_mode == "LaTeX" || markup_mode == "Markdown")
     {
      int pos = textCursor().positionInBlock();

      int end = s.indexOf (sep_end, pos);
      if (end == -1)
         return x;

      int start = s.lastIndexOf (sep_start, pos);
      if (start == -1)
         return x;

      x = s.mid (start + 1, end - (start + 1));

      QFileInfo inf (file_name);
      QDir cur_dir (inf.absolutePath());

      QString result = cur_dir.cleanPath (cur_dir.absoluteFilePath(x));
      if (file_exists (result))
         return result;

      int i = x.lastIndexOf ("/");
      if (i < 0)
         i = x.lastIndexOf ("\\");

      if (i < 0)
         return QString();

      x = x.mid (i + 1);

      result = cur_dir.cleanPath (cur_dir.absoluteFilePath(x));
      return result;
     }
  else
      {
       int pos = textCursor().positionInBlock();

       int end = s.indexOf ("\"", pos);
       if (end == -1)
          return x;

       int start = s.lastIndexOf ("\"", pos);
       if (start == -1)
          return x;

       x = s.mid (start + 1, end - (start + 1));

       if (x.startsWith("#"))
          return x;

       QFileInfo inf (file_name);
       QDir cur_dir (inf.absolutePath());

       return cur_dir.cleanPath (cur_dir.absoluteFilePath(x));
      }
}


QStringList CDocument::get_words()
{
  QStringList result;

  QTextCursor cr = textCursor();

  QString text = toPlainText();

  cr.setPosition (0);
  cr.movePosition (QTextCursor::Start, QTextCursor::MoveAnchor);

  do
    {
     QChar c = text[cr.position()];
     if (char_is_bad (c))
         while (char_is_bad (c))
               {
                cr.movePosition (QTextCursor::NextCharacter);
                c = text[cr.position()];
               }

     cr.movePosition (QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
     c = text[cr.position()];

     QString stext = cr.selectedText();

     if (! stext.isEmpty() && stext.endsWith ("\""))
        {
         cr.movePosition (QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
         stext = cr.selectedText();
        }

     if (! stext.isEmpty())
        result.append (stext);
    }
  while (cr.movePosition (QTextCursor::NextWord));

  return result;
}


void CDocument::goto_pos (int pos)
{
  QTextCursor cr = textCursor();
  cr.setPosition (pos);
  setTextCursor (cr);
}


void CDocument::set_tab_caption (const QString &fileName)
{
  holder->tab_widget->setTabText (get_tab_idx(), fileName);
}


void CDocument::set_hl (bool mode_auto, const QString &theext)
{
  if (highlighter)
     delete highlighter;

  highlighter = 0;

  if (! settings->value ("hl_enabled", 1).toBool())
      return;

  QString ext;

  if (mode_auto)
     ext = file_get_ext (file_name);
  else
      ext = theext;

  if (ext.isEmpty())
     return;

   QString fname;

#if QT_VERSION >= 0x050000

   for (vector <pair <QRegularExpression, QString> >::iterator p = holder->hl_files.begin(); p != holder->hl_files.end(); ++p)
       {
        if (p->first.isValid())
           if (p->first.match(file_name).hasMatch())
              {
               fname = p->second;
               break;
              }
       }

#else

   for (vector<pair<QRegExp, QString> >::iterator p = holder->hl_files.begin(); p != holder->hl_files.end(); ++p)
       {
        if (p->first.isValid())
           if (p->first.exactMatch(file_name))
              {
               fname = p->second;
               break;
              }
      }

#endif

  if (fname.isEmpty() || ! file_exists (fname))
     return;

#if QT_VERSION >= 0x050000
  highlighter = new CSyntaxHighlighterQRegularExpression (document(), this, fname);
#else
  highlighter = new CSyntaxHighlighterQRegExp (document(), this, fname);
#endif
}


void CDocument::set_markup_mode()
{
  markup_mode = holder->markup_mode;

  QString e = file_get_ext (file_name);
  QString t = holder->markup_modes[e];

  if (! t.isEmpty())
     markup_mode = t;
}


void CDocument::insert_image (const QString &full_path)
{
  put (get_insert_image (file_name, full_path, markup_mode));
}


void CDocument::reload (const QString &enc)
{
  if (file_exists (file_name))
      file_open (file_name, enc);
}


void CDocument::update_status()
{
  holder->l_charset->setText (charset);

  if (! cursor_xy_visible)
      return;

  int x = textCursor().position() - textCursor().block().position() + 1;
  int y = textCursor().block().blockNumber() + 1;

  holder->l_status_bar->setText (QString ("%1%2[%3]").arg (
                                 QString::number (y), -10).arg (
                                 QString::number (x), -10).arg (
                                 QString::number (blockCount(), 10)));

  holder->l_charset->setText (charset);
}


void CDocument::update_title (bool fullname)
{
  if (! holder->parent_wnd)
     return;

 // QMainWindow *w = qobject_cast <QMainWindow *> (holder->parent_wnd);

  QMainWindow *w = holder->parent_wnd;

  if (fullname)
      w->setWindowTitle (file_name);
  else
      w->setWindowTitle (QFileInfo (file_name).fileName());
}


void CDocument::update_labels()
{
  labels.clear();
  labels = text_get_bookmarks (toPlainText(),
                               settings->value ("label_start", "[?").toString(),
                               settings->value ("label_end", "?]").toString());
}


void CDocument::set_show_linenums (bool enable)
{
  draw_linenums = enable;
  updateLineNumberAreaWidth();
  update();
}


void CDocument::set_show_margin (bool enable)
{
  draw_margin = enable;
  update();
}


void CDocument::set_margin_pos (int mp)
{
  margin_pos = mp;
  margin_x = brace_width * margin_pos;
  update();
}


void CDocument::set_hl_cur_line (bool enable)
{
  highlight_current_line = enable;
  update();
}


void CDocument::set_hl_brackets (bool enable)
{
  hl_brackets = enable;
  update();
}


void CDocument::set_word_wrap (bool wrap)
{
  if (wrap)
     setWordWrapMode (QTextOption::WrapAtWordBoundaryOrAnywhere);
  else
     setWordWrapMode (QTextOption::NoWrap);
}


bool CDocument::get_word_wrap()
{
  return wordWrapMode() == QTextOption::WrapAtWordBoundaryOrAnywhere;
}


void CDocument::indent()
{
  if (! textCursor().hasSelection())
     {
      QString fl;
      fl = fl.fill (' ', tab_sp_width);

      if (spaces_instead_of_tabs)
         textCursor().insertText (fl);
      else
          textCursor().insertText ("\t");

      return;
     }

  QStringList l = textCursor().selectedText().split (QChar::ParagraphSeparator);
  if (l.size() == 0)
     return;

  QString fl;
  fl = fl.fill (' ', tab_sp_width);

  for (QList <QString>::iterator i = l.begin(); i != l.end(); ++i)
      {
       if (spaces_instead_of_tabs)
           i->prepend (fl);
       else
           i->prepend ("\t");
      }

  textCursor().insertText (l.join ("\n"));

  QTextCursor cur = textCursor();
  cur.movePosition (QTextCursor::Up, QTextCursor::KeepAnchor, l.size() - 1);
  cur.movePosition (QTextCursor::StartOfLine, QTextCursor::KeepAnchor);

  setTextCursor (cur);
}


void CDocument::un_indent()
{
  QStringList l = textCursor().selectedText().split (QChar::ParagraphSeparator);
  if (l.size() == 0)
     return;

  for (QList <QString>::iterator t = l.begin(); t != l.end(); ++t)
      {
       if (! t->isEmpty())
          if (t->at(0) == '\t' || t->at(0) == ' ')
             (*t) = t->mid (1);//eat first
      }

  textCursor().insertText (l.join ("\n"));

  QTextCursor cur = textCursor();
  cur.movePosition (QTextCursor::Up, QTextCursor::KeepAnchor, l.size() - 1);
  cur.movePosition (QTextCursor::StartOfLine, QTextCursor::KeepAnchor);

  setTextCursor (cur);
}


//если строка пустая - пофиг, а надо бы смотреть тогда строку выше
void CDocument::calc_auto_indent()
{
  QTextCursor cur = textCursor();
  cur.movePosition (QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
  int aindent = 0;

  QString s = cur.selectedText();
  int len = s.size();

  QChar t = ' '; //what to detect - space or tab?
  if (s.indexOf ('\t') != -1)
     t = '\t';

  if (t != '\t')
     {
      for (int i = 0; i < len; i++)
          if (! s.at(i).isSpace())
             {
              aindent = i;
              break;
             }
     }
  else
     {
      for (int i = 0; i < len; i++)
          if (s.at(i) != '\t')
             {
              aindent = i;
              break;
             }
     }


  if (aindent != 0)
     indent_val = indent_val.fill (t, aindent);
  else
      indent_val.clear();
}


void CDocument::setup_brace_width()
{
  QFontMetrics *fm = new QFontMetrics (font());
  brace_width = fm->averageCharWidth();
}


void CDocument::brace_highlight()
{
  brace_selection.format.setBackground (brackets_color);

  QTextDocument *doc = document();
  QTextCursor cursor = textCursor();
  QTextCursor beforeCursor = cursor;

  cursor.movePosition (QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
  QString brace = cursor.selectedText();

  beforeCursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
  QString beforeBrace = beforeCursor.selectedText();

  if ((brace != "{") &&
      (brace != "}") &&
      (brace != "[") &&
      (brace != "]") &&
      (brace != "(") &&
      (brace != ")") &&
      (brace != "<") &&
      (brace != ">"))
      {
       if ((beforeBrace == "{")
           || (beforeBrace == "}")
           || (beforeBrace == "[")
           || (beforeBrace == "]")
           || (beforeBrace == "(")
           || (beforeBrace == ")")
           || (beforeBrace == "<")
           || (beforeBrace == ">"))
          {
           cursor = beforeCursor;
           brace = cursor.selectedText();
          }
       else
           return;
       }

  QString openBrace;
  QString closeBrace;

  if ((brace == "{") || (brace == "}"))
     {
      openBrace = "{";
      closeBrace = "}";
     }
  else
  if ((brace == "[") || (brace == "]"))
     {
      openBrace = "[";
      closeBrace = "]";
     }
  else
  if ((brace == "(") || (brace == ")"))
     {
      openBrace = "(";
      closeBrace = ")";
     }
  else
  if ((brace == "<") || (brace == ">"))
     {
      openBrace = "<";
      closeBrace = ">";
     }


  if (brace == openBrace)
     {
      QTextCursor cursor1 = doc->find (closeBrace, cursor);
      QTextCursor cursor2 = doc->find (openBrace, cursor);
      if (cursor2.isNull())
         {
          brace_selection.cursor = cursor;
          extra_selections.append (brace_selection);
          brace_selection.cursor = cursor1;
          extra_selections.append (brace_selection);
          setExtraSelections (extra_selections);
         }
      else
          {
           while (cursor1.position() > cursor2.position())
                 {
                  cursor1 = doc->find (closeBrace, cursor1);
                  cursor2 = doc->find (openBrace, cursor2);
                  if (cursor2.isNull())
                      break;
                 }

           brace_selection.cursor = cursor;
           extra_selections.append (brace_selection);
           brace_selection.cursor = cursor1;
           extra_selections.append (brace_selection);
           setExtraSelections (extra_selections);
          }
       }
   else
       {
        if (brace == closeBrace)
           {
            QTextCursor cursor1 = doc->find (openBrace, cursor, QTextDocument::FindBackward);
            QTextCursor cursor2 = doc->find (closeBrace, cursor, QTextDocument::FindBackward);
            if (cursor2.isNull())
               {
                brace_selection.cursor = cursor;
                extra_selections.append (brace_selection);
                brace_selection.cursor = cursor1;
                extra_selections.append (brace_selection);
                setExtraSelections (extra_selections);
               }
            else
                {
                 while (cursor1.position() < cursor2.position())
                       {
                        cursor1 = doc->find (openBrace, cursor1, QTextDocument::FindBackward);
                        cursor2 = doc->find (closeBrace, cursor2, QTextDocument::FindBackward);
                        if (cursor2.isNull())
                           break;
                       }

                 brace_selection.cursor = cursor;
                 extra_selections.append (brace_selection);
                 brace_selection.cursor = cursor1;
                 extra_selections.append (brace_selection);
                 setExtraSelections (extra_selections);
                }
             }
     }
}


void CDocument::update_ext_selections()
{
  extra_selections.clear();
  setExtraSelections (extra_selections);
  rect_sel_upd();
  brace_highlight();
}


void CDocument::rect_block_start()
{
  int x = textCursor().position() - textCursor().block().position();
  int y = textCursor().block().blockNumber();

  rect_sel_start.setX (x);
  rect_sel_start.setY (y);

  update_ext_selections();
}


void CDocument::rect_block_end()
{
  int x = textCursor().position() - textCursor().block().position();
  int y = textCursor().block().blockNumber();

  rect_sel_end.setX (x);
  rect_sel_end.setY (y);

  update_ext_selections();
}


bool CDocument::has_rect_selection() const
{
  if (rect_sel_start.y() == -1 || rect_sel_end.y() == -1)
     return false;

  return true;
}


void CDocument::rect_sel_reset()
{
  rect_sel_start.setX (-1);
  rect_sel_start.setY (-1);
  rect_sel_end.setX (-1);
  rect_sel_end.setY (-1);
}


void CDocument::rect_sel_replace (const QString &s, bool insert)
{
/*

1. Определить с какой строки начинаем вставку
2. Определить как вставляемый текст перекрывает старый. Если строк в старом меньше (или конец файла),
добавляем в выходной буфер строки.
3. Составляем выходной буфер из строк старого и нового.
4. Заменяем старый текст на выходной буфер.

*/
  int y1 = std::min (rect_sel_start.y(), rect_sel_end.y());
  int y2 = std::max (rect_sel_start.y(), rect_sel_end.y());
  int ydiff = y2 - y1;

  int x1 = std::min (rect_sel_start.x(), rect_sel_end.x());
  int x2 = std::max (rect_sel_start.x(), rect_sel_end.x());

  QStringList sl_source;

  for (int line = y1; line <= y2; line++)
      {
       QTextBlock b = document()->findBlockByNumber (line);
       sl_source.append (b.text());
      }

  QStringList sl_insert = s.split ("\n");

  QStringList sl_dest;

  for (int line = 0; line < sl_insert.size(); line++)
      {
       QString t;

       if (line >= sl_source.size())
          {
           t = sl_insert [line];
           sl_dest.append (t);
           continue;
          }

       t = sl_source[line].left (x1);
       t += sl_insert [line];

       if (! insert)
          t += sl_source[line].mid (x2);
       else
           t += sl_source[line].mid (x1);

       sl_dest.append (t);
      }

  QString new_text = sl_dest.join ("\n");

//теперь выделить при помощи курсора всё от y1 до y2 и обычным способом заменить текст

  QTextCursor cursor = textCursor();

  cursor.movePosition (QTextCursor::Start, QTextCursor::MoveAnchor);
  cursor.movePosition (QTextCursor::NextBlock, QTextCursor::MoveAnchor, y1);
  cursor.movePosition (QTextCursor::NextBlock, QTextCursor::KeepAnchor, ydiff);
  cursor.movePosition (QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

  cursor.removeSelectedText();

  textCursor().insertText (new_text);
}


void CDocument::rect_sel_upd()
{
  if (rect_sel_start.y() == -1 || rect_sel_end.y() == -1)
     return;

  QTextEdit::ExtraSelection rect_selection;

  int y1 = std::min (rect_sel_start.y(), rect_sel_end.y());
  int y2 = std::max (rect_sel_start.y(), rect_sel_end.y());

  int x1 = std::min (rect_sel_start.x(), rect_sel_end.x());
  int x2 = std::max (rect_sel_start.x(), rect_sel_end.x());
  int xdiff = x2 - x1;

  int correction = 0;

  QTextCursor cursor = textCursor();

  cursor.movePosition (QTextCursor::Start, QTextCursor::MoveAnchor);
  cursor.movePosition (QTextCursor::NextBlock, QTextCursor::MoveAnchor, y1);

  for (int y = y1; y <= y2; y++)
      {
       QTextBlock b = document()->findBlockByNumber (y);

       if (b.text().length() == 0)
          {
           correction++;
           continue;
          }

       int sel_len = xdiff;

       if ((b.text().length() - x1) < xdiff)
          sel_len = b.text().length() - x1;

       cursor.movePosition (QTextCursor::Right, QTextCursor::MoveAnchor, x1 + correction);
       cursor.movePosition (QTextCursor::Right, QTextCursor::KeepAnchor, sel_len);

       rect_selection.cursor = cursor;
       rect_selection.format.setBackground (sel_back_color);
       rect_selection.format.setForeground (sel_text_color);

       extra_selections.append (rect_selection);

       cursor.movePosition (QTextCursor::NextBlock, QTextCursor::MoveAnchor);

       if (b.text().length() != 0)
           correction = 0;
      }

  setExtraSelections (extra_selections);
}


QString CDocument::rect_sel_get() const
{
  QString result;

  int y1 = std::min (rect_sel_start.y(), rect_sel_end.y());
  int y2 = std::max (rect_sel_start.y(), rect_sel_end.y());

  int x1 = std::min (rect_sel_start.x(), rect_sel_end.x());
  int x2 = std::max (rect_sel_start.x(), rect_sel_end.x());
  int xdiff = x2 - x1; //sel length

  for (int y = y1; y <= y2; y++)
      {
       QTextBlock b = document()->findBlockByNumber (y);
       QString t = b.text();

       result += t.mid (x1, xdiff);

       if (y != y2)
          result += '\n';
      }

  return result;
}


void CDocument::rect_sel_cut (bool just_del)
{
  int y1 = std::min (rect_sel_start.y(), rect_sel_end.y());
  int y2 = std::max (rect_sel_start.y(), rect_sel_end.y());
  int ydiff = y2 - y1;

  int x1 = std::min (rect_sel_start.x(), rect_sel_end.x());
  int x2 = std::max (rect_sel_start.x(), rect_sel_end.x());

  QStringList sl_source;

  for (int line = y1; line <= y2; line++)
      {
       QTextBlock b = document()->findBlockByNumber (line);
       sl_source.append (b.text());
      }

  QStringList sl_dest;
  QStringList sl_copy;

  for (int line = 0; line < sl_source.size(); line++)
      {
       QString t;

       t = sl_source[line].left (x1);
       t += sl_source[line].mid (x2);
       sl_dest.append (t);

       sl_copy.append (sl_source[line].mid (x1, x2 - x1));
      }

  QString new_text = sl_dest.join ("\n");

//теперь выделить при помощи курсора всё от y1 до y2 и обычным способом заменить текст

  QTextCursor cursor = textCursor();

  cursor.movePosition (QTextCursor::Start, QTextCursor::MoveAnchor);
  cursor.movePosition (QTextCursor::NextBlock, QTextCursor::MoveAnchor, y1);
  cursor.movePosition (QTextCursor::NextBlock, QTextCursor::KeepAnchor, ydiff);
  cursor.movePosition (QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

  cursor.beginEditBlock();

  cursor.removeSelectedText();

  cursor.insertText (new_text);

  cursor.endEditBlock();

  if (! just_del)
     QApplication::clipboard()->setText (sl_copy.join ("\n"));
}


void CDocument::lineNumberAreaPaintEvent (QPaintEvent *event)
{
  if (! draw_linenums)
     return;

  QPainter painter (line_num_area);
  painter.fillRect (event->rect(), linenums_bg);
  painter.setPen (text_color);

  QTextBlock block = firstVisibleBlock();
  int blockNumber = block.blockNumber();
  int top = (int) blockBoundingGeometry (block).translated (contentOffset()).top();
  int bottom = top + (int) blockBoundingRect (block).height();

  int w = line_num_area->width();
  int h = fontMetrics().height();

  while (block.isValid() && top <= event->rect().bottom())
        {
         if (block.isVisible() && bottom >= event->rect().top())
            {
             QString number = QString::number (blockNumber + 1) + " ";
             painter.drawText (0, top, w, h, Qt::AlignRight, number);
            }

         block = block.next();
         top = bottom;
         bottom = top + (int) blockBoundingRect(block).height();
         ++blockNumber;
        }
}


int CDocument::line_number_area_width()
{
  if (! draw_linenums)
     return 0;

  int digits = 1;
  int max = qMax (1, blockCount());

  while (max >= 10)
        {
         max /= 10;
         ++digits;
        }

  return (brace_width * 2) + (brace_width * digits);
}


void CDocument::updateLineNumberAreaWidth()
{
  setViewportMargins (line_number_area_width(), 0, 0, 0);
}


void CDocument::cb_cursorPositionChanged()
{
  viewport()->update();

  update_status();

  if (hl_brackets)
     update_ext_selections();
}


void CDocument::updateLineNumberArea (const QRect &rect, int dy)
{
  if (dy)
     line_num_area->scroll (0, dy);
  else
     line_num_area->update (0, rect.y(), line_num_area->width(), rect.height());

  if (rect.contains (viewport()->rect()))
     updateLineNumberAreaWidth();
}


void CDocument::slot_selectionChanged()
{
  QTextCursor cursor = this->textCursor();

  if (cursor.selectionStart() != cursor.selectionEnd())
     {
      rect_sel_start.setX (-1);
      rect_sel_end.setX (-1);
      update_ext_selections();
     }
}


CDox::CDox()
{
  markup_modes.insert ("htm", "HTML");
  markup_modes.insert ("html", "HTML");
  markup_modes.insert ("xhtml", "XHTML");
  markup_modes.insert ("xml", "Docbook");
  markup_modes.insert ("tex", "LaTeX");
  markup_modes.insert ("lout", "Lout");
  markup_modes.insert ("dokuwiki", "DokuWiki");
  markup_modes.insert ("mediawiki", "MediaWiki");
  markup_modes.insert ("md", "Markdown");
  markup_modes.insert ("markdown", "Markdown");

  menu_recent = 0;
  main_tab_widget = 0;
  tab_widget = 0;
  parent_wnd = 0;
  log = 0;
  l_status_bar = 0;
  l_charset = 0;

  //timer_autosave = new QTimer (this);
  timer_autosave.setInterval (settings->value ("timer_autosave_period", "1000").toInt() * 1000);
  connect(&timer_autosave, SIGNAL(timeout()), this, SLOT(autosave()));

  //timer_joystick = new QTimer (this);
  timer_joystick.setInterval (100);

#if defined(JOYSTICK_SUPPORTED)

  joystick = new CJoystick (0, this);

  if (joystick->initialized)
     {
      connect(&timer_joystick, SIGNAL(timeout()), joystick, SLOT(read_joystick()));

      if (settings->value ("use_joystick", "0").toBool())
         timer_joystick.start();
     }
#endif
}


CDox::~CDox()
{
  b_destroying_all = true;

  if (items.size() > 0)
     for (vector <size_t>::size_type i = 0; i < items.size(); i++)
          delete items[i];

  qstring_save (recent_list_fname, recent_files.join ("\n"));

#if defined(JOYSTICK_SUPPORTED)
  delete joystick;
#endif
}


void CDox::update_project (const QString &fileName)
{
  if (file_get_ext (fileName) == "teaproject")
     {
      fname_current_project = fileName;
      hash_project.clear();
      hash_project = hash_load_keyval (fileName);
     }
}


void CDox::reload_recent_list (void)
{
  if (! file_exists (recent_list_fname))
     return;

  recent_files = qstring_load (recent_list_fname).split ("\n");
}


void CDox::add_to_recent (CDocument *d)
{
  if (b_recent_off)
     return;

  if (! file_exists (d->file_name))
     return;

  QString s (d->file_name);
  s += "*";
  s += d->charset;
  s += "*";
  s += QString::number (d->textCursor().position());
  s += "*";

  if (! d->get_word_wrap())
     s+="0";
  else
      s+="1";

  recent_files.prepend (s);
  if (recent_files.size() > recent_list_max_items)
     recent_files.removeLast();
}


void CDox::update_recent_menu()
{
  menu_recent->clear();
  create_menu_from_list (this, menu_recent, recent_files, SLOT(open_recent()));
}


void CDox::update_current_files_menu()
{
  QStringList current_files;

  if (items.size() > 0)
     for (vector <size_t>::size_type i = 0; i < items.size(); i++)
         current_files.prepend (items[i]->file_name);

  menu_current_files->clear();
  create_menu_from_list (this, menu_current_files, current_files, SLOT(open_current()));
}


void CDox::move_cursor (QTextCursor::MoveOperation mo)
{
  CDocument *d = get_current();

  if (! d)
     return;

  QTextCursor cr = d->textCursor();
  if (cr.isNull())
     return;

  if (mo != QTextCursor::NoMove)
     {
      cr.movePosition (mo, QTextCursor::MoveAnchor);
      d->setTextCursor (cr);
     }
}


CDocument* CDox::create_new()
{
  CDocument *doc = new CDocument (this, 0);
  doc->markup_mode = markup_mode;

  tab_widget->setCurrentIndex (tab_widget->indexOf (doc->tab_page));
  apply_settings_single (doc);

  doc->update_title (settings->value ("full_path_at_window_title", 1).toBool());
  doc->update_status();

  update_current_files_menu();

  return doc;
}


CDocument* CDox::open_file (const QString &fileName, const QString &codec)
{
  if (! file_exists (fileName) || ! path_is_file (fileName))
     return 0;

  if (is_image (fileName))
     {
      CDocument *td = get_current();
      if (td)
         {
          td->insert_image (fileName);
          td->setFocus (Qt::OtherFocusReason);
         }
      return td;
     }


  CDocument *d = get_document_by_fname (fileName);
  if (d)
     {
      tab_widget->setCurrentIndex (tab_widget->indexOf (d->tab_page));
      d->reload (codec);
      return d;
     }


//else truly create the new doc
  d = create_new();
  d->file_open (fileName, codec);

  dir_last = get_file_path (d->file_name);

  d->update_status();
  d->update_title (settings->value ("full_path_at_window_title", 1).toBool());

  main_tab_widget->setCurrentIndex (0);

  update_current_files_menu();

  return d;
}


CDocument* CDox::open_file_triplex (const QString &triplex)
{
  QStringList sl = triplex.split ("*");
  if (sl.size() < 3)
     return 0;

  CDocument *d = open_file (sl[0], sl[1]);
  if (! d)
     return 0;

  d->goto_pos (sl[2].toInt());

  if (sl.size() >= 4)
     {
      if (sl[3] == "1")
         d->set_word_wrap (true);
      else
         d->set_word_wrap (false);
     }

  return d;
}


CDocument* CDox::get_document_by_fname (const QString &fileName)
{
  if (fileName.isEmpty() || items.size() == 0)
     return 0;

  for (vector <CDocument *>::iterator i = items.begin(); i != items.end(); ++i)
       if ((*i)->file_name == fileName)
          return *i;

  return 0;
}


CDocument* CDox::get_current()
{
  int i = tab_widget->currentIndex();
  if (i < 0)
     return 0;

  return items[i];
}


void CDox::close_by_idx (int i)
{
  if (i < 0)
     return;

  delete items[i];
  items.erase (items.begin() + i);

  update_current_files_menu();
}


void CDox::close_current()
{
  close_by_idx (tab_widget->currentIndex());
}


void CDox::save_to_session (const QString &fileName)
{
  if (items.size() == 0)
     return;

  fname_current_session = fileName;
  QString l;

  for (vector <CDocument*>::iterator i = items.begin(); i != items.end(); ++i)
      {
       QString t = (*i)->get_triplex();
       if (! t.isEmpty())
          {
           l += t;
           l += "\n";
          }
      }

  qstring_save (fileName, l.trimmed());
}


void CDox::load_from_session (const QString &fileName)
{
  if (! file_exists (fileName))
     return;

  QStringList l = qstring_load (fileName).split ("\n");
  if (l.size() < 0)
     return;

  for (int i = 0; i < l.size(); i++)
      open_file_triplex (l[i]);

  fname_current_session = fileName;
}

void CDox::save_buffers (const QString &fileName)
{
  QFile::remove (fileName);

  if (items.size() == 0)
     return;

  fname_current_session = fileName;
  QString l;

  for (vector <CDocument*>::iterator i = items.begin(); i != items.end(); ++i)
      {
       if (! file_exists ((*i)->file_name))
          {
           l += (*i)->toPlainText();
           l += '\f';
          }
      }

  qstring_save (fileName, l.trimmed());
}


void CDox::load_from_buffers (const QString &fileName)
{
  if (! file_exists (fileName))
     return;

  QStringList l = qstring_load (fileName).split ("\f");
  if (l.size() < 0)
     return;

  for (int i = 0; i < l.size(); i++)
      {
       CDocument *d = create_new();
       d->put (l[i]);

       QTextCursor cr = d->textCursor();
       if (! cr.isNull())
          {
           cr.movePosition (QTextCursor::Start, QTextCursor::MoveAnchor);
           d->setTextCursor (cr);
          }
      }

  fname_current_session = fileName;
}


void CDox::apply_settings()
{
  if (items.size() == 0)
     return;

  for (vector <size_t>::size_type i = 0; i < items.size(); i++)
      apply_settings_single (items[i]);
}


void CDox::apply_settings_single (CDocument *d)
{
  int darker_val = settings->value ("darker_val", 100).toInt();

  d->setCursorWidth (settings->value ("cursor_width", 2).toInt());
  d->setCenterOnScroll (settings->value ("center_on_scroll", true).toBool());

  QString s_sel_back_color = hash_get_val (global_palette, "sel-background", "black");
  QString s_sel_text_color = hash_get_val (global_palette, "sel-text", "white");

  d->sel_text_color = QColor (s_sel_text_color).darker(darker_val).name();
  d->sel_back_color = QColor (s_sel_back_color).darker(darker_val).name();

  d->tab_sp_width = settings->value ("tab_sp_width", 8).toInt();
  d->spaces_instead_of_tabs = settings->value ("spaces_instead_of_tabs", true).toBool();

#if (QT_VERSION_MAJOR <= 5 && QT_VERSION_MINOR < 10)
  d->setTabStopWidth (d->tab_sp_width * d->brace_width);
#else
  d->setTabStopDistance (d->tab_sp_width * d->brace_width);
#endif

  d->setup_brace_width();

  d->set_show_linenums (settings->value ("show_linenums", false).toBool());

  d->show_tabs_and_spaces = settings->value ("show_tabs_and_spaces", false).toBool();

  QTextOption option = d->document()->defaultTextOption();
  if (d->show_tabs_and_spaces)
     option.setFlags(option.flags() | QTextOption::ShowTabsAndSpaces);
  else
     option.setFlags(option.flags() & ~QTextOption::ShowTabsAndSpaces);

  option.setFlags (option.flags() | QTextOption::AddSpaceForLineAndParagraphSeparators);
  d->document()->setDefaultTextOption (option);



  d->set_show_margin (settings->value ("show_margin", false).toBool());
  d->set_margin_pos (settings->value ("margin_pos", 72).toInt());
  d->highlight_current_line = settings->value ("additional_hl", false).toBool();
  d->hl_brackets = settings->value ("hl_brackets", false).toBool();
  d->brackets_color = QColor (hash_get_val (global_palette, "brackets", "yellow")).darker (darker_val);
  d->current_line_color = QColor (hash_get_val (global_palette, "cur_line_color", "#EEF6FF")).darker (darker_val);

  d->cursor_xy_visible = settings->value ("cursor_xy_visible", "2").toBool();
  d->auto_indent = settings->value ("auto_indent", false).toBool();

  QString text_color = hash_get_val (global_palette, "text", "black");
  QString t_text_color = QColor (text_color).darker(darker_val).name();
  d->text_color = QColor (t_text_color);

  QString back_color = hash_get_val (global_palette, "background", "white");
  d->margin_color = QColor (hash_get_val (global_palette, "margin_color", text_color)).darker(darker_val);
  d->linenums_bg = QColor (hash_get_val (global_palette, "linenums_bg", back_color)).darker(darker_val);
  d->set_word_wrap (settings->value ("word_wrap", true).toBool());

  d->repaint();
  d->set_hl();
}


void CDox::open_recent()
{
  QAction *act = qobject_cast<QAction *>(sender());

  int i = recent_files.indexOf (act->text());
  if (i == -1)
     return;

  CDocument *d = open_file_triplex (recent_files[i]);
  if (d)
      dir_last = get_file_path (d->file_name);

  update_recent_menu();
}


void CDox::open_current()
{
  QAction *act = qobject_cast<QAction *>(sender());

  CDocument *d = get_document_by_fname (act->text());
  if (d)
      tab_widget->setCurrentIndex (tab_widget->indexOf (d->tab_page));
}


void CDox::autosave()
{
  if (! settings->value ("autosave", false).toBool())
      return;

  if (items.size() == 0)
     return;

  for (vector <CDocument *>::iterator i = items.begin(); i != items.end(); ++i)
      {
       if (autosave_files.contains ((*i)->file_name))
          (*i)->file_save_with_name ((*i)->file_name, (*i)->charset);
      }

  save_buffers (fname_saved_buffers);
}


void CDox::move_cursor_up()
{
  move_cursor (QTextCursor::Up);
}


void CDox::move_cursor_down()
{
  move_cursor (QTextCursor::Down);
}


void CDox::move_cursor_left()
{
  move_cursor (QTextCursor::Left);
}


void CDox::move_cursor_right()
{
  move_cursor (QTextCursor::Right);
}


void CDox::move_cursor_x (double v)
{
  if (v < 0)
     move_cursor (QTextCursor::Right);
  if (v > 0)
     move_cursor (QTextCursor::Left);
}


void CDox::move_cursor_y (double v)
{
  if (v < 0)
     move_cursor (QTextCursor::Up);
  if (v > 0)
     move_cursor (QTextCursor::Down);
}


#if defined(JOYSTICK_SUPPORTED)

bool CDox::event (QEvent *ev)
{
  if (static_cast<int>(ev->type() == evtJoystickAxis))
     {
      CJoystickAxisEvent* custom_event = reinterpret_cast<CJoystickAxisEvent*>(ev);
      handle_joystick_event (custom_event);
      custom_event->accept();
      return true;
     }

  return QObject::event(ev);
}


void CDox::handle_joystick_event (CJoystickAxisEvent *event)
{
  QTextCursor::MoveOperation mo = QTextCursor::NoMove;

  if (event->axis == 1 && event->value < 0) //up
     mo = QTextCursor::Up;

  if (event->axis == 1 && event->value > 0) //down
     mo = QTextCursor::Down;

  if (event->axis == 0 && event->value < 0) //left
     mo = QTextCursor::Left;

  if (event->axis == 0 && event->value > 0) //right
     mo = QTextCursor::Right;

  move_cursor (mo);
}

#endif
