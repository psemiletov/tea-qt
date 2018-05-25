/***************************************************************************
 *   2007-2017 by Peter Semiletov                                          *
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
#include <hunspell/hunspell.hxx>
#endif

#ifdef SPELLCHECK_ENABLE
#include "spellchecker.h"
#endif

#include "utils.h"

#include <QTextCodec>
#include <QDir>
#include <QRegExp>

#ifndef H_DEPRECATED
#include <string>
#include <vector>
#endif


#ifdef ASPELL_ENABLE

CSpellchecker::CSpellchecker (const QString &lang, const QString &path,
                              const QString &user_path):
                              ASpellchecker (lang, path, user_path)
{
  ret = 0;
  speller = 0;
  spell_config = 0;

  initialized = false;
  
  spell_config = new_aspell_config();
  if (! spell_config)
     return;

  aspell_config_replace (spell_config, "lang", lang.toUtf8().data());
  aspell_config_replace (spell_config, "encoding", "UTF-8");

#if defined(Q_OS_WIN) || defined(Q_OS_OS2)

  QString data_path = path + "\\data";
  QString dict_path = path + "\\dict";

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
  initialized = true;
}


CSpellchecker::~CSpellchecker()
{
  if (speller)
     delete_aspell_speller (speller);

  if (spell_config)
     delete_aspell_config (spell_config);
}


void CSpellchecker::add_to_user_dict (const QString &word)
{
  if (! initialized || ! speller)
     return;
  
  if (word.isEmpty())
     return;

  QByteArray bw = word.toUtf8();

  aspell_speller_add_to_personal (speller, bw.data(), bw.size());
  aspell_speller_save_all_word_lists (speller);
}


void CSpellchecker::remove_from_user_dict (const QString &word)
{
  //const AspellWordList *awl = aspell_speller_personal_word_list (speller);
}


void CSpellchecker::change_lang (const QString &lang)
{
  if (! spell_config)
     return;
     
  if (lang.isEmpty())
     return;   

  aspell_config_replace (spell_config, "lang", lang.toUtf8().data());
  aspell_config_replace (spell_config, "encoding", "UTF-8");

  if (speller)
     delete_aspell_speller (speller);

  ret = new_aspell_speller (spell_config);

  if (aspell_error (ret) != 0)
     {
      //g_print ("Error: %s\n", aspell_error_message (ret));
      delete_aspell_can_have_error (ret);
      return;
     }

  speller = to_aspell_speller (ret);
  initialized = true;
}


QStringList CSpellchecker::get_speller_modules_list()
{
  QStringList l;

  if (! spell_config)
    return l;

  AspellDictInfoList *dlist;
  AspellDictInfoEnumeration *dels;
  const AspellDictInfo *entry;

  dlist = get_aspell_dict_info_list (spell_config);

  dels = aspell_dict_info_list_elements (dlist);

  while ((entry = aspell_dict_info_enumeration_next (dels)) != 0)
        {
         if (entry)
            l.prepend (entry->name);
        }

  delete_aspell_dict_info_enumeration (dels);

  return l;
}


bool CSpellchecker::check (const QString &word)
{
  if (! initialized || ! speller)
      return false;
   
  if (word.isEmpty())
     return false;

   return aspell_speller_check (speller, word.toUtf8().data(), -1);
}


QStringList CSpellchecker::get_suggestions_list (const QString &word)
{
  QStringList l;

  if (! initialized)
     return l;
  
  if (word.isEmpty() || ! speller)
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

#endif


#ifdef HUNSPELL_ENABLE

CHunspellChecker::CHunspellChecker (const QString &lang, const QString &path, const QString &user_path): ASpellchecker (lang, path, user_path)
{
  qDebug() << "CHunspellChecker::CHunspellChecker - start";

  initialized = false;

  dict_dir = path;
  user_dir = user_path; 
  lng = lang;


#if defined(Q_OS_WIN) || defined(Q_OS_OS2)

  QString fname_aff = path + QDir::separator() + lng + ".aff";
  QString fname_dict = path + QDir::separator() + lng + ".dic";
  QString fname_userdict = user_path + QDir::separator() + lng + ".dic";

  fname_aff = fname_aff.replace ("/", "\\");
  fname_dict = fname_dict.replace ("/", "\\");
  fname_userdict = fname_userdict.replace ("/", "\\");

#else

  QString fname_aff = path + QDir::separator() + lng + ".aff";
  QString fname_dict = path + QDir::separator() + lng + ".dic";
  QString fname_userdict = user_path + QDir::separator() + lng + ".dic";

#endif


#if defined(Q_OS_WIN)

  fname_aff = "\\\\?\\" + fname_aff;
  fname_dict = "\\\\?\\" + fname_dict;
  fname_userdict = "\\\\?\\" + fname_userdict;

#endif


//qDebug() << fname_aff;
//qDebug() << fname_dict;
//qDebug() << fname_userdict;


  speller = new Hunspell (fname_aff.toUtf8().data(), fname_dict.toUtf8().data());
  encoding = speller->get_dic_encoding();


//    const char *c_aff = fname_aff.toLocal8Bit().constData();
  //  const char *c_dict = fname_dict.toLocal8Bit().constData();

//    speller = new Hunspell (c_aff, c_dict);

  
  if (file_exists (fname_userdict))
     {
      speller->add_dic (fname_userdict.toUtf8().data());
      user_words = qstring_load (fname_userdict, encoding).split ("\n");
      user_words.removeFirst();
     }


  initialized = true;

//  if (file_exists (fname_dict))
  //    initialized = true;
//  else
  //    qDebug() << "Hunspell engine is not initilized with a proper dictionary file " << fname_dict;

  qDebug() << "CHunspellChecker::CHunspellChecker - end";

}


CHunspellChecker::~CHunspellChecker()
{
#if defined(Q_OS_WIN) || defined(Q_OS_OS2)

  QString filename = user_dir + QDir::separator() + lng + ".dic";
  filename = filename.replace ("/", "\\");

#else

  QString filename = user_dir + QDir::separator() + lng + ".dic";

#endif

  if (user_words.size() > 0)
     {
      user_words.prepend (QString::number (user_words.size()));
      qstring_save (filename, user_words.join ("\n"), encoding);
     }

  delete speller;
}


void CHunspellChecker::change_lang (const QString &lang)
{
  delete speller;

  initialized = false;

  lng = lang;

  user_words.clear(); 

#if defined(Q_OS_WIN) || defined(Q_OS_OS2)

  QString fname_aff = dict_dir + QDir::separator() + lng + ".aff";
  QString fname_dict = dict_dir + QDir::separator() + lng + ".dic";
  QString fname_userdict = user_dir + QDir::separator() + lng + ".dic";

  fname_aff = fname_aff.replace ("/", "\\");
  fname_dict = fname_dict.replace ("/", "\\");
  fname_userdict = fname_userdict.replace ("/", "\\");


#else

  QString fname_aff = dict_dir + QDir::separator() + lng + ".aff";
  QString fname_dict = dict_dir + QDir::separator() + lng + ".dic";
  QString fname_userdict = user_dir + QDir::separator() + lng + ".dic";


#endif

  speller = new Hunspell (fname_aff.toUtf8().data(), fname_dict.toUtf8().data());
  encoding = speller->get_dic_encoding();

  if (file_exists (fname_userdict))
     {
      speller->add_dic (fname_userdict.toUtf8().data());
      user_words = qstring_load (fname_userdict, encoding).split ("\n");  
      user_words.removeFirst(); //зачем я это закомментировал раньше?
     } 

  initialized = true;

//  qDebug() << "encoding: " << QString (encoding);
/*  if (file_exists (fname_dict))
      initialized = true;
  else
      qDebug() << "Hunspell engine is not initilized with a proper dictionary file " << fname_dict;

  qDebug() << "restarting hunspell with " << fname_aff << " " << fname_dict;*/
}


void CHunspellChecker::add_to_user_dict (const QString &word)
{
  if (! initialized || word.isEmpty())
     return;

  QTextCodec *codec = QTextCodec::codecForName (encoding);
  QByteArray es = codec->fromUnicode (word);
  speller->add (es.data());
  user_words.append (word);
}


bool CHunspellChecker::check (const QString &word)
{
  if (! initialized)
      return false;

 QTextCodec *codec = QTextCodec::codecForName (encoding);
 QByteArray es = codec->fromUnicode (word);
 
#ifndef H_DEPRECATED
  return speller->spell (es.data());
#else  
  return speller->spell (QString(es).toStdString());
#endif
}


void CHunspellChecker::remove_from_user_dict (const QString &word)
{
  if (! initialized || word.isEmpty())
      return;

  QTextCodec *codec = QTextCodec::codecForName (encoding);
  QByteArray es = codec->fromUnicode (word);
  speller->remove (es.data()); 
  int i = user_words.indexOf (word);
  if (i != -1)
     user_words.removeAt (i);
}


QStringList CHunspellChecker::get_speller_modules_list()
{
  QDir dir (dict_dir);

  QStringList filters;
  QStringList sl;
  
  filters << "*.dic";
  
  dir.setSorting (QDir::Name);
  QFileInfoList fil = dir.entryInfoList (filters);
  
  for (int i = 0; i < fil.size(); i++)
      {
       sl.append (fil[i].baseName());
      }

  return sl;
}


QStringList CHunspellChecker::get_suggestions_list (const QString &word)
{
//  if (! initialized)
  //   return QStringList();

  QStringList sl;

  if (word.isEmpty())
     return sl;

  QTextCodec *codec = QTextCodec::codecForName (encoding);
  QByteArray es = codec->fromUnicode (word);

#ifndef H_DEPRECATED
  char **slst;
    
  int size = speller->suggest(&slst, es.data());
  
  for (int i = 0; i < size; i++)
      sl.append (codec->toUnicode (slst[i]));
   
  speller->free_list (&slst, size);
#else



  std::vector<std::string> suglist = speller->suggest (QString(es).toStdString());


//  QList <QString> l;
  sl.reserve (suglist.size());
  for (size_t i = 0, sz = suglist.size(); i < sz; ++i)
      sl.append (QString::fromStdString (suglist[i]));        

//  QList <QString> l = QList <QString>::fromVector(QVector <QString>::fromStdVector (suglist));

// l.reserve(suglist.size());
// std::copy(std::vector.begin(), std::vector.end(), std::back_inserter(l));


 

#endif
  
  return sl;
}


#endif 
