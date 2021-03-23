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

#ifdef HUNSPELL_ENABLE
#include <hunspell/hunspell.hxx>
#endif


#include <QDebug>
#include <QString>
#include <QStringList>
#include <QHash>


class CSpellchecker
{

public:

  QString language;
  QString dir_dicts;
  QString dir_user_dicts;
  QStringList modules_list;

  bool loaded;


  CSpellchecker (const QString &lang,
                 const QString &dir_path,
                 const QString &dir_user): language (lang), dir_dicts (dir_path), dir_user_dicts (dir_user), loaded(false)
                 {};

  virtual ~CSpellchecker() {};

  virtual void load_dict() = 0; //uses current language
  virtual void save_user_dict() = 0; //uses current language
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

  void save_user_dict() {}; //not needed due to Aspell implementation
  void load_dict();
  void change_lang (const QString &lang);
  void add_to_user_dict (const QString &word);
  void remove_from_user_dict (const QString &word);
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

  void save_user_dict();
  void load_dict();
  void change_lang (const QString &lang);
  void add_to_user_dict (const QString &word);
  void remove_from_user_dict (const QString &word);
  bool check (const QString &word);
  void get_speller_modules_list();
  QStringList get_suggestions_list (const QString &word);
};

QString hunspell_default_dict_path(); 


#endif

#endif
