/***************************************************************************
 *   2007-2021 by Peter Semiletov                                          *
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

#ifndef SPELLCHECKER_H
#define SPELLCHECKER_H

#ifdef ASPELL_ENABLE
#include "aspell.h"
#endif

#ifdef NUSPELL_ENABLE
#include <nuspell/dictionary.hxx>
#include <nuspell/finder.hxx>
#endif


#ifdef HUNSPELL_ENABLE
#include <filesystem>
#include <hunspell/hunspell.hxx>
#endif


#include <QDebug>
#include <QString>
#include <QStringList>
#include <QHash>

#include "utils.h"


class CSpellchecker
{

public:

  QString user_dict_filename;

  QString language;
  QString dir_dicts;
  QString dir_user_dicts;
  QStringList modules_list;

  QStringList user_dict;

  bool loaded;


  CSpellchecker (const QString &lang,
                 const QString &dir_path,
                 const QString &dir_user): language (lang), dir_dicts (dir_path), dir_user_dicts (dir_user), loaded(false)
                 {
                  user_dict_filename = dir_user + "/user_dict.txt";

                  if (file_exists (user_dict_filename))
                     {
                      QString file_content = qstring_load (user_dict_filename);
                      user_dict = file_content.split ("\n");
                     }

                };

  virtual ~CSpellchecker() {
                            if (user_dict.size() > 0)
                                qstring_save (user_dict_filename, user_dict.join ("\n"));
                            };

  virtual void load_dict() = 0; //uses current language
  virtual void change_lang (const QString &lang) = 0; //set current language
  virtual void add_to_user_dict (const QString &word) = 0;
  virtual void remove_from_user_dict (const QString &word) = 0;
  virtual bool check (const QString &word) = 0;
  virtual void get_speller_modules_list() = 0;
  virtual QStringList get_suggestions_list (const QString &word) = 0;
};


#ifdef ASPELL_ENABLE

class CAspellchecker: public CSpellchecker
{

public:

  AspellConfig *spell_config;
  AspellCanHaveError *ret;
  AspellSpeller *speller;

  CAspellchecker (const QString &lang, const QString &dir_path = "", const QString &dir_user = "");
  ~CAspellchecker();

  void add_to_user_dict (const QString &word);
  void remove_from_user_dict (const QString &word);


  void load_dict();
  void change_lang (const QString &lang);
  bool check (const QString &word);
  void get_speller_modules_list();
  QStringList get_suggestions_list (const QString &word);
};

QString aspell_default_dict_path();

#endif


#ifdef HUNSPELL_ENABLE

class CHunspellChecker: public CSpellchecker
{

public:

  Hunspell *speller;

  QStringList user_words;
  const char *encoding; //depercated (old Hunspell versions) but supported
  std::string str_encoding; //new approach

  CHunspellChecker (const QString &lang, const QString &dir_path = "", const QString &dir_user = "");
  ~CHunspellChecker();

  void add_to_user_dict (const QString &word);
  void remove_from_user_dict (const QString &word);

  void load_dict();
  void change_lang (const QString &lang);
  bool check (const QString &word);
  void get_speller_modules_list();
  QStringList get_suggestions_list (const QString &word);
};

QString hunspell_default_dict_path(); 


#endif


#ifdef NUSPELL_ENABLE

class CNuspellChecker: public CSpellchecker
{

public:

  nuspell::Dictionary *speller;
  std::vector<std::filesystem::path> dirs;
  std::filesystem::path dict_path;

  QStringList user_words;
  std::string str_encoding;

  CNuspellChecker (const QString &lang, const QString &dir_path = "", const QString &dir_user = "");
  ~CNuspellChecker();

  void add_to_user_dict (const QString &word);
  void remove_from_user_dict (const QString &word);

  void load_dict();
  void change_lang (const QString &lang);
  bool check (const QString &word);
  void get_speller_modules_list();
  QStringList get_suggestions_list (const QString &word);
};


#endif


#endif
