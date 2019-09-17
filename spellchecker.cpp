/***************************************************************************
 *   2007-2019 by Peter Semiletov                                          *
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

#ifdef H_DEPRECATED
#include <string>
#include <vector>
#endif

#ifdef HUNSPELL_ENABLE
#include <hunspell/hunspell.hxx>
#endif

#include <QTextCodec>
#include <QDir>
#include <QRegExp>

#include "spellchecker.h"
#include "utils.h"



#ifdef ASPELL_ENABLE

CAspellchecker::CAspellchecker (const QString &lang, const QString &path,
                                const QString &user_path):
                                CSpellchecker (lang, path, user_path)
{
  qDebug() << "CAspellchecker::CAspellchecker - start";
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


CAspellchecker::~CAspellchecker()
{
  if (speller)
     delete_aspell_speller (speller);

  if (spell_config)
     delete_aspell_config (spell_config);
}


void CAspellchecker::add_to_user_dict (const QString &word)
{
  if (! initialized || ! speller)
     return;

  if (word.isEmpty())
     return;

  QByteArray bw = word.toUtf8();

  aspell_speller_add_to_personal (speller, bw.data(), bw.size());
  aspell_speller_save_all_word_lists (speller);
}


void CAspellchecker::remove_from_user_dict (const QString &word)
{
  //const AspellWordList *awl = aspell_speller_personal_word_list (speller);
}


void CAspellchecker::change_lang (const QString &lang)
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


QStringList CAspellchecker::get_speller_modules_list()
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


bool CAspellchecker::check (const QString &word)
{
  if (! initialized || ! speller)
      return false;

  if (word.isEmpty())
     return false;

   return aspell_speller_check (speller, word.toUtf8().data(), -1);
}


QStringList CAspellchecker::get_suggestions_list (const QString &word)
{
  QStringList l;

  if (! initialized || word.isEmpty() || !speller)
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

CHunspellChecker::CHunspellChecker (const QString &lang, const QString &path, const QString &user_path): CSpellchecker (lang, path, user_path)
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
  QString fname_userdict_pure;

  fname_aff = fname_aff.replace ("/", "\\");
  fname_dict = fname_dict.replace ("/", "\\");
  fname_userdict = fname_userdict.replace ("/", "\\");
  fname_userdict_pure = fname_userdict;

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

  if (dir_exists (dict_dir))
     initialized = true;

  speller = new Hunspell (fname_aff.toUtf8().data(), fname_dict.toUtf8().data());

#if !defined (H_DEPRECATED)
  encoding = speller->get_dic_encoding();
#else
  str_encoding = speller->get_dict_encoding();
#endif


#if defined(Q_OS_UNIX)

  if (initialized)
  if (file_exists (fname_userdict))
     {
      speller->add_dic (fname_userdict.toUtf8().data());

#if ! defined (H_DEPRECATED)
      user_words = qstring_load (fname_userdict, encoding).split ("\n");
#else
      user_words = qstring_load (fname_userdict, str_encoding.data()).split ("\n");

#endif
      user_words.removeFirst();
     }
#else  //! UNIX

  if (initialized)
  if (file_exists (fname_userdict_pure))
     {
      speller->add_dic (fname_userdict.toUtf8().data());
#if ! defined (H_DEPRECATED)
      user_words = qstring_load (fname_userdict, encoding).split ("\n");
#else
      user_words = qstring_load (fname_userdict, str_encoding.data()).split ("\n");
#endif

      user_words.removeFirst();
     }

#endif

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
#if ! defined (H_DEPRECATED)
      qstring_save (filename, user_words.join ("\n"), encoding);
#else
      qstring_save (filename, user_words.join ("\n"), str_encoding.data());
#endif

     }

  delete speller;
}


void CHunspellChecker::change_lang (const QString &lang)
{
  delete speller;

  initialized = false;
  lng = lang;
  user_words.clear();

  if (dir_exists (dict_dir))
     initialized = true;

#if defined(Q_OS_WIN) || defined(Q_OS_OS2)

  QString fname_aff = dict_dir + QDir::separator() + lng + ".aff";
  QString fname_dict = dict_dir + QDir::separator() + lng + ".dic";
  QString fname_userdict = user_dir + QDir::separator() + lng + ".dic";
  QString fname_userdict_pure;

  fname_aff = fname_aff.replace ("/", "\\");
  fname_dict = fname_dict.replace ("/", "\\");
  fname_userdict = fname_userdict.replace ("/", "\\");
  fname_userdict_pure = fname_userdict;

#else

  QString fname_aff = dict_dir + QDir::separator() + lng + ".aff";
  QString fname_dict = dict_dir + QDir::separator() + lng + ".dic";
  QString fname_userdict = user_dir + QDir::separator() + lng + ".dic";

#endif

  speller = new Hunspell (fname_aff.toUtf8().data(), fname_dict.toUtf8().data());

#if ! defined (H_DEPRECATED)
  encoding = speller->get_dic_encoding();
#else
  str_encoding = speller->get_dict_encoding();
#endif

#if defined(Q_OS_UNIX)

  if (initialized && file_exists (fname_userdict))
     {
      speller->add_dic (fname_userdict.toUtf8().data());

#if ! defined (H_DEPRECATED)
      user_words = qstring_load (fname_userdict, encoding).split ("\n");
#else
      user_words = qstring_load (fname_userdict, str_encoding.data()).split ("\n");

#endif
      user_words.removeFirst(); //зачем я это закомментировал раньше?
     }
#else //! UNIX
  if (initialized && file_exists (fname_userdict_pure))
     {
      speller->add_dic (fname_userdict.toUtf8().data());
#if ! defined (H_DEPRECATED)
      user_words = qstring_load (fname_userdict, encoding).split ("\n");
#else
      user_words = qstring_load (fname_userdict, str_encoding.data()).split ("\n");
#endif
      user_words.removeFirst(); //зачем я это закомментировал раньше?
     }

#endif
}


void CHunspellChecker::add_to_user_dict (const QString &word)
{
  if (! initialized || word.isEmpty())
     return;

#if !defined (H_DEPRECATED)
  QTextCodec *codec = QTextCodec::codecForName (encoding);
#else
  QTextCodec *codec = QTextCodec::codecForName (str_encoding.data());
#endif

  QByteArray es = codec->fromUnicode (word);
  speller->add (es.data());
  user_words.append (word);
}


bool CHunspellChecker::check (const QString &word)
{
  if (! initialized)
      return false;

#if ! defined (H_DEPRECATED)
 QTextCodec *codec = QTextCodec::codecForName (encoding);
#else
 QTextCodec *codec = QTextCodec::codecForName (str_encoding.data());
#endif

 QByteArray es = codec->fromUnicode (word);

#ifndef H_DEPRECATED

   return speller->spell (es.constData());
  //return speller->spell (es.data()); //old way
#else
  return speller->spell (QString(es).toStdString());
#endif
}


void CHunspellChecker::remove_from_user_dict (const QString &word)
{
  if (! initialized || word.isEmpty())
      return;

#ifndef H_DEPRECATED
  QTextCodec *codec = QTextCodec::codecForName (encoding);
#else
 QTextCodec *codec = QTextCodec::codecForName (str_encoding.data());
#endif

  QByteArray es = codec->fromUnicode (word);
  speller->remove (es.data());
  int i = user_words.indexOf (word);
  if (i != -1)
     user_words.removeAt (i);
}


QStringList CHunspellChecker::get_speller_modules_list()
{
  QStringList sl;

  QDir dir (dict_dir);
  if (! dir.exists())
     return sl;

  QStringList filters;

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
  QStringList sl;

  if (! initialized || word.isEmpty())
     return sl;

#if !defined (H_DEPRECATED)
  QTextCodec *codec = QTextCodec::codecForName (encoding);
#else
  QTextCodec *codec = QTextCodec::codecForName (str_encoding.data());
#endif

  QByteArray es = codec->fromUnicode (word);

#ifndef H_DEPRECATED
  char **slst;

  int size = speller->suggest (&slst, es.data());

  for (int i = 0; i < size; i++)
      sl.append (codec->toUnicode (slst[i]));

  speller->free_list (&slst, size);

#else

  std::vector<std::string> suglist = speller->suggest (QString(es).toStdString());

  sl.reserve (suglist.size());
  for (size_t i = 0, sz = suglist.size(); i < sz; ++i)
      sl.append (QString::fromStdString (suglist[i]));

#endif

  return sl;
}

#endif
