/***************************************************************************
 *   2007-2022 by Peter Semiletov                                          *
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
 ***************************************************************************/


/***************************************************************************
some code is taken from Scribus::util.cpp:
                             -------------------
    begin                : Fri Sep 14 2001
    copyright            : (C) 2001 by Franz Schmid
    email                : Franz.Schmid@altmuehlnet.de
 ***************************************************************************/
/* roman.c by Adam Rogoyski (apoc@laker.net) Temperanc on EFNet irc
 * Copyright (C) 1998 Adam Rogoyski
 * Converts Decimal numbers to Roman Numerals and Roman Numberals to
 * Decimals on the command line or in Interactive mode.
 * Uses an expanded Roman Numeral set to handle numbers up to 999999999
*/


#include <algorithm>

#include <QString>
#include <QMap>
#include <QDebug>
//#include <QStringBuilder>

#if QT_VERSION < 0x050000
#include <QRegExp>
#else
#include <QRegularExpression>
#endif



#include "textproc.h"
#include "utils.h"

using namespace std;



bool qstring_length_less_than (const QString& v1, const QString& v2)
{
   return v1.length() < v2.length();
}

/*
QString int_to_binary (int n)
{
  std::bitset<sizeof (int)> bt (n);
  return QString::fromStdString (bt.to_string<char,std::string::traits_type,std::string::allocator_type>());
}
*/


int str_fuzzy_search (const QString &s, const QString &text_to_find, int start_pos, double q)
{
  if (s.isEmpty() || text_to_find.isEmpty())
     return -1;

  int counter;
  int result = -1;

  bool jump = false;

  int end_pos = s.length() - 1;

  for (int i = start_pos; i < end_pos; i++)
      {
       if (jump)
          break;

       counter = 0;
       for (int j = 0; j < text_to_find.length(); j++)
           {
            if (s[i + j] == text_to_find[j])
               counter++;

            if (get_percent ((double)text_to_find.length(), (double)counter) >= q)
               {
                result = i;
                jump = true;
                break;
               }
           }
      }

  return result;
}


QString apply_table (const QString &s, const QString &fname, bool use_regexp)
{
  QHash<QString, QString> h = hash_load_keyval (fname);

  QString result = s;

  for (int i = 0; i < h.size(); i++)
      {
       QString key = h.keys()[i];

       if (use_regexp)
#if QT_VERSION < 0x050000
           result.replace (QRegExp (key), h.value (key));
       else
#else
           result.replace (QRegularExpression (key), h.value (key));
       else
#endif
           result.replace (key, h.value (key));
      }

  return result;
}


QString strip_html (const QString &source)
{
  bool do_copy = true;
  QString dest;

  for (int i = 0; i < source.length(); i++)
      {
       if (source[i] == '<')
          do_copy = false;
       else
       if (source[i] == '>')
          {
           do_copy = true;
           if (i < source.length() - 1)
              i++;
           else
               break;
          }

       if (do_copy)
          dest += source[i]; //% from QStringBuilder

//          dest = dest % source[i]; //% from QStringBuilder
      }

  return dest;
}


QString qstringlist_process (const QString &s, const QString &params, int mode)
{
  QStringList sl;
  QStringList l;
  QString result;

  if (mode != QSTRL_PROC_FLT_WITH_SORTCASECARE_SEP && mode != QSTRL_PROC_LIST_FLIP_SEP)
      sl = s.split (QChar::ParagraphSeparator);

  switch (mode)
         {
          case QSTRL_PROC_FLT_WITH_SORTCASECARE_SEP:
                                                    {
                                                     if (s.indexOf (params) == -1)
                                                        return s;

                                                     QStringList t = s.split (params);
                                                     t.sort();
                                                     result = t.join (params);
                                                     return result;
                                                    };

          case QSTRL_PROC_LIST_FLIP_SEP:  {
                                           if (s.indexOf (params) == -1)
                                              return s;

                                           QStringList t = s.split (params);
                                           t.sort();

                                           for (int i = 0; i < t.size(); i++)
                                                l.prepend (t.at(i));

                                           result = l.join (params);
                                           return result;
                                          };


          case QSTRL_PROC_FLT_WITH_SORTNOCASECARE:
                                                 {
                                                  QMap <QString, QString> map;

                                                  for (int i = 0; i < sl.size(); i++)
                                                      map.insert (sl[i].toLower(), sl[i]);

                                                  for (QMap<QString, QString>::const_iterator i = map.constBegin();
                                                       i != map.constEnd();
                                                       ++i)
                                                       l.append (i.value());

                                                  break;
                                                 }

          case QSTRL_PROC_FLT_WITH_SORTLEN:
                                                 {
                                                  l = sl;
                                                  std::sort (l.begin(), l.end(), qstring_length_less_than);
                                                  break;
                                                 }


          case QSTRL_PROC_FLT_REMOVE_EMPTY:
                                           {
                                            for (QList <QString>::iterator i = sl.begin(); i != sl.end(); ++i)
                                                if (! i->isEmpty())
                                                   l.append (*i);

                                            break;
                                           };


          case QSTRL_PROC_FLT_REMOVE_DUPS:
                                          {
                                           for (QList <QString>::iterator i = sl.begin(); i != sl.end(); ++i)
                                               if (! l.contains (*i))
                                                  l.append (*i);

                                           break;
                                          };

          case QSTRL_PROC_REMOVE_FORMATTING:
                                           {
                                            for (QList <QString>::iterator i = sl.begin(); i != sl.end(); ++i)
                                                 l.append (i->simplified());

                                            break;
                                           };

           case QSTRL_PROC_FLT_WITH_REGEXP:
                                          {
#if QT_VERSION < 0x050000
                                           l = sl.filter (QRegExp (params));
#else
                                           l = sl.filter (QRegularExpression (params));
#endif
                                           break;
                                          }

           case QSTRL_PROC_FLT_WITH_SORTCASECARE:
                                                 {
                                                  l = sl;
                                                  l.sort();
                                                  break;
                                                 }

           case QSTRL_PROC_LIST_FLIP:
                                     {
                                      for (QList <QString>::iterator i = sl.begin(); i != sl.end(); ++i)
                                           l.prepend (*i);

                                      break;
                                     }

           case QSTRL_PROC_FLT_LESS:
                                    {
                                     int t = params.toInt();

                                     for (QList <QString>::iterator i = sl.begin(); i != sl.end(); ++i)
                                          if (i->size() > t)
                                              l.append (*i);

                                     break;
                                    }

           case QSTRL_PROC_FLT_GREATER:
                                    {
                                     int t = params.toInt();

                                     for (QList <QString>::iterator i = sl.begin(); i != sl.end(); ++i)
                                          if (i->size() < t)
                                              l.append (*i);

                                     break;
                                    }
         }

  result = l.join ("\n");
  return result;
}


QString string_reverse (const QString &s)
{
  QString sn;

  int c = s.length() - 1;
  int x = 0;

  for (int i = c; i > -1; i--)
       sn[x++] = s.at(i);

 return sn;
}


QString conv_quotes (const QString &source, const QString &c1, const QString &c2)
{
  QString x;
  QString dest;

  bool flag = true;
  int c = source.size() - 1;
  for (int i = 0; i <= c; i++)
      {
       if (source.at(i) == '\"')
          {
           if (flag)
              x = c1;
           else
               x = c2;

           flag = ! flag;
           dest += x;
          }
       else
           dest += source[i];
      }

  return dest;
}


QStringList html_get_by_patt (const QString &s, const QString &spatt)
{
  QStringList result;

  int c = s.size();
  int i = 0;

  while (i < c)
        {
         int start = s.indexOf (spatt, i, Qt::CaseInsensitive);

         if (start == -1)
             break;

         int end = s.indexOf ('"', start + spatt.size());
         if (end == -1)
             break;

         result.prepend (s.mid (start + spatt.size(), (end - start) - spatt.size()));

         i = end + 1;
        }

  return result;
}


QStringList anagram (const QString &s)
{
  QString input = s;
  QStringList sl;

  sort (input.begin(), input.end());

  do
    sl.append (input);
  while (next_permutation (input.begin(), input.end()));

  return sl;
}

/*
from:

* roman.c by Adam Rogoyski (apoc@laker.net) Temperanc on EFNet irc
 * Copyright (C) 1998 Adam Rogoyski
 * Converts Decimal numbers to Roman Numerals and Roman Numberals to
 * Decimals on the command line or in Interactive mode.
 * Uses an expanded Roman Numeral set to handle numbers up to 999999999
*/

#define FROM_ROMAN_I 1
#define FROM_ROMAN_V 5
#define FROM_ROMAN_X 10
#define FROM_ROMAN_L 50
#define FROM_ROMAN_C 100
#define FROM_ROMAN_D 500
#define FROM_ROMAN_M 1000
#define FROM_ROMAN_P 5000
#define FROM_ROMAN_Q 10000
#define FROM_ROMAN_R 50000
#define FROM_ROMAN_S 100000
#define FROM_ROMAN_T 500000
#define FROM_ROMAN_U 1000000
#define FROM_ROMAN_B 5000000
#define FROM_ROMAN_W 10000000
#define FROM_ROMAN_N 50000000
#define FROM_ROMAN_Y 100000000
#define FROM_ROMAN_Z 500000000

int value (char c)
{
  switch (c)
   {
      case 'I':
         return FROM_ROMAN_I;
      case 'V':
         return FROM_ROMAN_V;
      case 'X':
         return FROM_ROMAN_X;
      case 'L':
         return FROM_ROMAN_L;
      case 'C':
         return FROM_ROMAN_C;
      case 'D':
         return FROM_ROMAN_D;
      case 'M':
         return FROM_ROMAN_M;
      case 'P':
         return FROM_ROMAN_P;
      case 'Q':
         return FROM_ROMAN_Q;
      case 'R':
         return FROM_ROMAN_R;
      case 'S':
         return FROM_ROMAN_S;
      case 'T':
         return FROM_ROMAN_T;
      case 'U':
         return FROM_ROMAN_U;
      case 'B':
         return FROM_ROMAN_B;
      case 'W':
         return FROM_ROMAN_W;
      case 'N':
         return FROM_ROMAN_N;
      case 'Y':
         return FROM_ROMAN_Y;
      case 'Z':
         return FROM_ROMAN_Z;
      default:
         return 0;
   }
}


int romanToDecimal (const char *roman)
{
  int decimal = 0;
  for (; *roman; roman++)
      {
      /* Check for four of a letter in a fow */
      if ((*(roman + 1) && *(roman + 2) && *(roman + 3))
         && (*roman == *(roman + 1))
         && (*roman == *(roman + 2))
         && (*roman == *(roman + 3)))
         return 0;
      /* Check for two five type numbers */
      if (  ((*roman == 'V') && (*(roman + 1) == 'V'))
         || ((*roman == 'L') && (*(roman + 1) == 'L'))
         || ((*roman == 'D') && (*(roman + 1) == 'D'))
         || ((*roman == 'P') && (*(roman + 1) == 'P'))
         || ((*roman == 'R') && (*(roman + 1) == 'R'))
         || ((*roman == 'T') && (*(roman + 1) == 'T'))
         || ((*roman == 'B') && (*(roman + 1) == 'B'))
         || ((*roman == 'N') && (*(roman + 1) == 'N'))
         || ((*roman == 'Z') && (*(roman + 1) == 'Z')))
         return 0;
      /* Check for two lower characters before a larger one */
      if ((value(*roman) == value(*(roman + 1))) && (*(roman + 2))
         && (value(*(roman + 1)) < value(*(roman + 2))))
         return 0;
      /* Check for the same character on either side of a larger one */
      if ((*(roman + 1) && *(roman + 2))
         && (value(*roman) == value(*(roman + 2)))
         && (value(*roman) < value(*(roman + 1))))
         return 0;
      /* Check for illegal nine type numbers */
      if (!strncmp(roman, "LXL", 3) || !strncmp(roman, "DCD", 3)
       || !strncmp(roman, "PMP", 3) || !strncmp(roman, "RQR", 3)
       || !strncmp(roman, "TST", 3) || !strncmp(roman, "BUB", 3)
       || !strncmp(roman, "NWN", 3) || !strncmp(roman, "VIV", 3))
         return 0;
      if (value(*roman) < value(*(roman + 1)))
      {
         /* check that subtracted value is at least 10% larger,
            i.e. 1990 is not MXM, but MCMXC */
         if ((10 * value(*roman)) < value(*(roman + 1)))
            return 0;
         /* check for double subtraction, i.e. IVX */
         if (value(*(roman + 1)) <= value(*(roman + 2)))
            return 0;
         /* check for subtracting by a number starting with a 5
            ie.  VX, LD LM */
         if (*roman == 'V' || *roman == 'L' || *roman == 'D'
          || *roman == 'P' || *roman == 'R' || *roman == 'T'
          || *roman == 'B' || *roman == 'N')
            return 0;
         decimal += value (*(roman + 1)) - value (*roman);
         roman++;
      }
      else
      {
         decimal += value (*roman);
      }
   }
   return decimal;
}


//this code is taken from Scribus::util.cpp:
QString arabicToRoman (int i)
{
  QString roman;

  int arabic = i;

  while (arabic - 1000000 >= 0){
  roman += "m";
  arabic -= 1000000;
  }
  while (arabic - 900000 >= 0){
  roman += "cm";
  arabic -= 900000;
  }
  while (arabic - 500000 >= 0){
  roman += "d";
  arabic -= 500000;
  }
  while (arabic - 400000 >= 0){
  roman += "cd";
  arabic -= 400000;
  }
  while (arabic - 100000 >= 0){
  roman += "c";
  arabic -= 100000;
  }
  while (arabic - 90000 >= 0){
  roman += "xc";
  arabic -= 90000;
  }
  while (arabic - 50000 >= 0){
  roman += "l";
  arabic -= 50000;
  }
  while (arabic - 40000 >= 0){
  roman += "xl";
  arabic -= 40000;
  }
  while (arabic - 10000 >= 0){
  roman += "x";
  arabic -= 10000;
  }
  while (arabic - 9000 >= 0){
  roman += "Mx";
  arabic -= 9000;
  }
  while (arabic - 5000 >= 0){
  roman += "v";
  arabic -= 5000;
  }
  while (arabic - 4000 >= 0){
  roman += "Mv";
  arabic -= 4000;
  }
  while (arabic - 1000 >= 0){
  roman += "M";
  arabic -= 1000;
  }
  while (arabic - 900 >= 0){
  roman += "CM";
  arabic -= 900;
  }
  while (arabic - 500 >= 0){
  roman += "D";
  arabic -= 500;
  }
  while (arabic - 400 >= 0){
  roman += "CD";
  arabic -= 400;
  }
  while (arabic - 100 >= 0){
  roman += "C";
  arabic -= 100;
  }
  while (arabic - 90 >= 0){
  roman += "XC";
  arabic -= 90;
  }
  while (arabic - 50 >= 0){
  roman += "L";
  arabic -= 50;
  }
  while (arabic - 40 >= 0){
  roman += "XL";
  arabic -= 40;
  }
  while (arabic - 10 >= 0){
  roman += "X";
  arabic -= 10;
  }
  while (arabic - 9 >= 0){
  roman += "IX";
  arabic -= 9;
  }
  while (arabic - 5 >= 0){
  roman += "V";
  arabic -= 5;
  }
  while (arabic - 4 >= 0){
  roman += "IV";
  arabic -= 4;
  }
  while (arabic - 1 >= 0){
  roman += "I";
  arabic -= 1;
  }
  return roman;
}


QString int_to_binary (int n)
{
  QString result;
  int sz = sizeof (n) * 8 - 1;

  for (int i = sz; i > -1; i--)
      {
       if (n & (1 << i))
           result.append ("1");
       else
           result.append ("0");

       if (i % 4 == 0)
          result.append (" ");
      }

  return result;
}


unsigned int bin_to_decimal (const QString &s)
{
  unsigned int table[31];
  unsigned int c = 1;
  unsigned int result = 0;
  QString sn = string_reverse (s);

  table[0] = 1;

  for (int i = 1; i < 31; i++)
      {
       c *= 2;
       table[i] = c;
      }

  for (int i = 0; i < sn.size(); i++)
      if (sn[i] == '1')
         result += table[i];

  return result;
}


QString str_to_entities (const QString &s)
{
  QString t = s;
  t = t.replace ("&", "&amp;");

  t = t.replace ("\"", "&quot;");
  t = t.replace ("'", "&apos;");

  t = t.replace ("<", "&lt;");
  t = t.replace (">", "&gt;");

  return t;
}


QString morse_from_lang (const QString &s, const QString &lang)
{
  QHash<QString, QString> h = hash_load_keyval (":/text-data/morse-" + lang);

  QString result;
  QString x = s.toUpper();

  int c = x.size();
  for (int i = 0; i < c; i++)
      {
       QString t = h.value (QString (x[i]));
       if (! t.isEmpty())
          result.append (t).append (" ");
      }

  return result;
}


QString morse_to_lang (const QString &s, const QString &lang)
{
  QHash<QString, QString> h = hash_load_keyval (":/text-data/morse-" + lang);

  QStringList sl = s.toUpper().split (" ");

  QString result;

  for (int i = 0; i < sl.size(); i++)
      {
       QString t = h.key (sl[i]);
       if (! t.isEmpty())
          result.append (t);
      }

  return result;
}


//from http://www.cyberforum.ru/cpp-beginners/thread125615.html
int get_arab_num (std::string rom_str)
{
  int res = 0;

  for (size_t i = 0; i < rom_str.length(); ++i)
      {
       switch (rom_str[i])
              {
               case 'M':
                        res += 1000;
                        break;
               case 'D':
                        res += 500;
                        break;
               case 'C':
                        i + 1 < rom_str.length() && (rom_str[i + 1] == 'D'
                        || rom_str[i + 1] == 'M') ? res -= 100 : res += 100;
                        break;
               case 'L':
                        res += 50;
                        break;
               case 'X':
                        i + 1 < rom_str.length()
                        &&  (rom_str[i + 1] == 'L'
                        || rom_str[i + 1] == 'C') ? res -= 10 : res += 10;
                        break;
               case 'V':
                        res += 5;
                        break;
               case 'I':
                        i + 1 < rom_str.length()
                        &&  (rom_str[i + 1] == 'V'
                        || rom_str[i + 1] == 'X') ? res -= 1 : res += 1;
                        break;

                }//switch
       }//for

  return res;
}
