/***************************************************************************
 *   2007-2024 by Peter Semiletov                                          *
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


#ifdef HUNSPELL_ENABLE

#ifdef H_DEPRECATED
#include <string>
#include <vector>
#endif

#include <hunspell/hunspell.hxx>

#endif


#ifdef NUSPELL_ENABLE

#include <vector>
#include <iostream>
#include <string>

#endif


#include <QDir>
#include <QMessageBox>
#include <QObject>

#include "spellchecker.h"
#include "utils.h"
#include "enc.h"


#ifdef ASPELL_ENABLE


QString aspell_default_dict_path()
{
  QString r;
  r = "/usr/lib/aspell-0.60";

#if defined(Q_OS_OS2)

  r = qEnvironmentVariable ("unixroot", "c:") + "\\usr\\lib\\aspell-0.60";

#endif

#if defined(Q_OS_WIN)

  r = "C:\\Program Files\\Aspell";

#endif

  return r;
}


void CAspellchecker::load_dict()
{
  loaded = false;

  if (! spell_config)
     return;

  if (speller)
     {
      delete_aspell_speller (speller);
      speller = 0;
     }

  aspell_config_replace (spell_config, "lang", language.toUtf8().data());
  aspell_config_replace (spell_config, "encoding", "UTF-8");

#if defined(Q_OS_WIN) || defined(Q_OS_OS2)

  QString data_path = dir_dicts + "\\data";
  QString dict_path = dir_dicts + "\\dict";

  data_path = data_path.replace ("/", "\\");
  dict_path = dict_path.replace ("/", "\\");

  aspell_config_replace (spell_config, "data-dir", data_path.toLatin1().data());
  aspell_config_replace (spell_config, "dict-dir", dict_path.toLatin1().data());

#endif

  ret = new_aspell_speller (spell_config);
  if (! ret)
     return;

  if (aspell_error (ret) != 0)
     {
      //g_print ("Error: %s\n", aspell_error_message (ret));
      delete_aspell_can_have_error (ret);
      return;
     }

  speller = to_aspell_speller (ret);

  loaded = true;
}


CAspellchecker::CAspellchecker (const QString &lang, const QString &path,
                                const QString &user_path):
                                CSpellchecker (lang, path, user_path)
{
  ret = 0;
  speller = 0;
  spell_config = 0;

  loaded = false;

  spell_config = new_aspell_config();
  if (! spell_config)
     return;
}


CAspellchecker::~CAspellchecker()
{
  if (speller)
     delete_aspell_speller (speller);

  if (spell_config)
     delete_aspell_config (spell_config);
}


void CAspellchecker::change_lang (const QString &lang)
{
  if (lang.isEmpty() || language == lang)
     return;

  language = lang;
}


void CAspellchecker::get_speller_modules_list()
{
   modules_list.clear();

   if (! spell_config)
      return;

  AspellDictInfoList *dlist;
  AspellDictInfoEnumeration *dels;
  const AspellDictInfo *entry;

  dlist = get_aspell_dict_info_list (spell_config);
  dels = aspell_dict_info_list_elements (dlist);

  while ((entry = aspell_dict_info_enumeration_next (dels)) != 0)
        {
//       if (entry)
         modules_list.prepend (entry->name);
        }

  delete_aspell_dict_info_enumeration (dels);
}


bool CAspellchecker::check (const QString &word)
{
  if (modules_list.size() == 0)
     {
      //QMessageBox::about (0, "!", QObject::tr ("Please set up spell checker dictionaries at\n Options - Functions page"));
      return false;
     }

  if (word.isEmpty())
     return false;

  if (! loaded)
     load_dict();

  if (! loaded)
      false; //still not loaded

  if (user_dict_checker.check (word))
     return true;

  if (speller)
     return aspell_speller_check (speller, word.toUtf8().data(), -1);
  else
      return false;
}


QStringList CAspellchecker::get_suggestions_list (const QString &word)
{
  QStringList l;

  if (! loaded || word.isEmpty() || ! speller)
     return l;

  const AspellWordList *suggestions = aspell_speller_suggest (speller, word.toUtf8().data(), -1);

  if (! suggestions)
     return l;

  AspellStringEnumeration *elements = aspell_word_list_elements (suggestions);
  const char *wrd;

  while ((wrd = aspell_string_enumeration_next (elements)))
         l.prepend (QString::fromUtf8 (wrd));

  delete_aspell_string_enumeration (elements);

  return l;
}


void CAspellchecker::add_to_user_dict (const QString &word)
{
  if (! loaded || ! speller)
     return;

  if (word.isEmpty())
     return;

   user_dict_checker.add_word (word);
}


void CAspellchecker::remove_from_user_dict (const QString &word)
{
  if (word.isEmpty())
     return;

  user_dict_checker.remove_word (word);
}

#endif


/****************** HUNSPELL **********************************/

#ifdef HUNSPELL_ENABLE

void CHunspellChecker::add_to_user_dict (const QString &word)
{
  if (! loaded || ! speller)
     return;

  if (word.isEmpty())
     return;

  user_dict_checker.add_word (word);
}


void CHunspellChecker::remove_from_user_dict (const QString &word)
{
  if (word.isEmpty())
     return;

  user_dict_checker.remove_word (word);
}


void CHunspellChecker::load_dict()
{
  loaded = false;

  if (! dir_exists (dir_dicts) || language.isEmpty())
     return;

  if (speller)
     delete speller;

#if defined(Q_OS_WIN) || defined(Q_OS_OS2)

  QString fname_aff = dir_dicts + QDir::separator() + language + ".aff";
  QString fname_dict = dir_dicts + QDir::separator() + language + ".dic";

  fname_aff = fname_aff.replace ("/", "\\");
  fname_dict = fname_dict.replace ("/", "\\");

#else

  QString fname_aff = dir_dicts + QDir::separator() + language + ".aff";
  QString fname_dict = dir_dicts + QDir::separator() + language + ".dic";

#endif

//need for Unicode path
#if defined(Q_OS_WIN)

  fname_aff = "\\\\?\\" + fname_aff;
  fname_dict = "\\\\?\\" + fname_dict;

#endif

  speller = new Hunspell (fname_aff.toUtf8().data(), fname_dict.toUtf8().data());

#if !defined (H_DEPRECATED)
  encoding = speller->get_dic_encoding();
#else
  str_encoding = speller->get_dict_encoding();
 // std::cout << "str_encoding : " << str_encoding << std::endl;
  //qDebug() << "str_encoding : " << str_encoding;

#endif

  loaded = true;
}


CHunspellChecker::CHunspellChecker (const QString &lang, const QString &dir_path, const QString &dir_user): CSpellchecker (lang, dir_path, dir_user)
{
  speller = 0;
  encoding = 0;
  loaded = false;
}


CHunspellChecker::~CHunspellChecker()
{
  delete speller;
}


void CHunspellChecker::change_lang (const QString &lang)
{
  if (language == lang)
      return;

  language = lang;
}

/*
void CHunspellChecker::add_to_user_dict (const QString &word)
{
  if (! loaded || word.isEmpty())
     return;

  QByteArray es = word.toUtf8();
  speller->add (es.data());
  user_words.append (word);
  save_user_dict();
}
*/

bool CHunspellChecker::check (const QString &word)
{
  if (modules_list.size() == 0)
     {
    //  QMessageBox::about (0, "!", QObject::tr ("Please set up spell checker dictionaries at\n Options - Functions page"));
      return false;
     }

  if (word.isEmpty())
     return false;

  if (! loaded)
     load_dict();

  if (! loaded)
     {
      qDebug() << "still not loaded!";
      return false;
     }

  if (user_dict_checker.check (word))
     return true;


    QByteArray es = word.toUtf8();

#ifndef H_DEPRECATED

   return speller->spell (es.constData());

   //return speller->spell (es.data()); //old way
#else

  return speller->spell (QString(es).toStdString());
 //  return speller->spell (word.toStdString());

#endif
}


void CHunspellChecker::get_speller_modules_list()
{
  modules_list.clear();

  QDir dir (dir_dicts);
  if (! dir.exists())
     return;

  QStringList filters;

  filters << "*.dic";

  dir.setSorting (QDir::Name);
  QFileInfoList fil = dir.entryInfoList (filters);

  for (int i = 0; i < fil.size(); i++)
      {
       modules_list.append (fil[i].baseName());
      }
}


QStringList CHunspellChecker::get_suggestions_list (const QString &word)
{
  QStringList sl;

  if (! loaded || word.isEmpty())
     return sl;

  QByteArray es = word.toUtf8();

#ifndef H_DEPRECATED

  char **slst;
  int size = speller->suggest (&slst, es.data());

  for (int i = 0; i < size; i++)
      sl.append (QString::fromUtf8 (slst[i]));

  speller->free_list (&slst, size);

#else

  std::vector<std::string> suglist = speller->suggest (QString(es).toStdString());

  sl.reserve (suglist.size());
  for (size_t i = 0, sz = suglist.size(); i < sz; ++i)
      sl.append (QString::fromStdString (suglist[i]));

#endif

  return sl;
}


QString hunspell_default_dict_path()
{
  QString r;
  r = "/usr/share/hunspell";

#if defined(Q_OS_OS2)
  r = qEnvironmentVariable ("unixroot", "c:") + "\\usr\\share\\myspell";
#endif

#if defined(Q_OS_WIN)
#endif

  return r;
}

#endif



#ifdef NUSPELL_ENABLE

void CNuspellChecker::add_to_user_dict (const QString &word)
{
  if (! loaded || ! speller)
     return;

  if (word.isEmpty())
     return;

  user_dict_checker.add_word (word);
}


void CNuspellChecker::remove_from_user_dict (const QString &word)
{
  if (word.isEmpty())
     return;

  user_dict_checker.remove_word (word);
}


void CNuspellChecker::load_dict()
{
  loaded = false;

  //qDebug() << "true CNuspellChecker::load_dict";

  dict_path = nuspell::search_dirs_for_one_dict (dirs, language.toStdString());

  //qDebug() << "dict_path: " << dict_path.string();

  if (dict_path.empty() )
     {
      qDebug() << "dict_path.empty()";
      return; // Return error because we can not find the requested
	          // dictionary.
     }

  speller = new nuspell::Dictionary;


  try {
       speller->load_aff_dic(dict_path);
      }
  catch (const nuspell::Dictionary_Loading_Error& e)
        {
	     std::cout << e.what() << '\n';
	     return;
        }

  loaded = true;
}


CNuspellChecker::CNuspellChecker (const QString &lang, const QString &dir_path, const QString &dir_user): CSpellchecker (lang, dir_path, dir_user)
{
//  qDebug() << "CNuspellChecker::CNuspellChecker ";

  loaded = false;
  speller = 0;

  nuspell::append_default_dir_paths (dirs);
  nuspell::append_libreoffice_dir_paths (dirs);
  //qDebug << "dirs.size: " << dirs.size();

  //auto dict_list = nuspell::search_default_dirs_for_dicts();
}


CNuspellChecker::~CNuspellChecker()
{
  delete speller;
}


void CNuspellChecker::change_lang (const QString &lang)
{
  if (language.isEmpty() || language == lang)
     return;

  language = lang;
}


bool CNuspellChecker::check (const QString &word)
{
  if (modules_list.size() == 0)
      return false;

  if (! loaded)
     load_dict();

  if (! loaded)
      //still not loaded!
      return false;

  if (user_dict_checker.check (word))
     return true;

  return speller->spell (word.toStdString());
}


void CNuspellChecker::get_speller_modules_list()
{

  modules_list.clear();

  auto dict_list = nuspell::search_default_dirs_for_dicts();

  for (int i = 0; i < dict_list.size(); i++)
      {
       QString dictname = QString::fromStdString (dict_list[i].stem().string());
//       qDebug() << "CNuspellChecker::get_speller_modules_list() - " << dictname;
       modules_list.append (dictname);
      }
}


QStringList CNuspellChecker::get_suggestions_list (const QString &word)
{
  QStringList sl;

  if (! loaded || word.isEmpty())
     return sl;

  QByteArray es = word.toUtf8();

  auto suggestions = std::vector<std::string>();
  speller->suggest(word.toStdString(), suggestions);

  if (suggestions.empty())
	  return sl;

  sl.reserve (suggestions.size());
  for (size_t i = 0, sz = suggestions.size(); i < sz; ++i)
      sl.append (QString::fromStdString (suggestions[i]));


  return sl;
}

#endif


void CPlainSpellchecker::add_word (const QString &word)
{
  //qDebug() << " CPlainSpellchecker::add_word: " << word;
  
  if (word.isEmpty())
     return;
  
  UTF16TEXT key = word.at(0).unicode();
  map[key].append (word);
  
  //qDebug() << "map.size() " << map.size();
  //qDebug() << "map[key].size() " << map[key].size();
  
  
  
}


void CPlainSpellchecker::remove_word (const QString &word)
{
  if (word.isEmpty())
     return;

  UTF16TEXT key = word.at(0).unicode();

  int i = map[key].indexOf (word);
  if (i != -1)
     map[key].removeAt (i);
}



bool CPlainSpellchecker::check (const QString &word)
{
  
  UTF16TEXT key = word[0].unicode();
  return map.value(key).contains (word, Qt::CaseInsensitive);
}


void CPlainSpellchecker::load_from_file (const QString &fname)
{
  if (fname.isEmpty())
      return;

  if (! file_exists (fname))
      return;

  user_dict_filename = fname;

  QString text = qstring_load (fname);

  QStringList sl = text.split ("\n");

  for (int i = 0; i < sl.size(); i++)
      {
       QString s = sl.at(i);

       if (s.isEmpty() || s == "\n")
          continue;

       UTF16TEXT key = s[0].unicode();
       map[key].append (s);
      }
      
      
//    std::cout << "map.size() " << map.size() << std::endl;     
}


void CPlainSpellchecker::save_to_file (const QString &fname)
{
  QString text;
/*
  for (QStringList value : std::as_const (map))
      {
       if (value.size() > 0)
          {
           for (int i = 0; i < value.size(); i++)
               {
                QString t = value.at(i);

                if (! t.isEmpty())
                   {
                    text += value.at(i);
                    text += "\n";
                    qDebug () << value.at (i);
                   }
               }
           }
       }
*/
/*
  QList <QStringList> values = map.values();

  for (int i = 0; i < values.size(); i++)
      {
       qDebug() << "i:" << i; 
       
              
       if (values.at(i).size() > 0)
          {
           for (int j = 0; j < values.at(j).size(); j++)
               {
                QString t = values.at(i).at(j);

                if (! t.isEmpty())
                   {
                    text += t;
                    text += "\n";
                    qDebug () << "at j:" << j << " t:" << t;
                   }
               }
         }

      }

      
  qstring_save (user_dict_filename, text);*/


  QList <QStringList> values = map.values();

  for (int i = 0; i < values.size(); i++)
      {
//       qDebug() << "i:" << i; 
       
       QStringList sl_char = values.at(i);
       
       //qDebug() << "sl_char.size():" << sl_char.size(); 
             
       if (sl_char.size() == 0)
          continue;
             
        for (int j = 0; j < sl_char.size(); j++)
            {
             QString t = sl_char.at(j);
             
         //    qDebug () << "at j:" << j << " t:" << t;
          
             if (! t.isEmpty())
                {
                 text += t;
                 text += "\n";
                }
            }
      }
      
  qstring_save (user_dict_filename, text);
}


