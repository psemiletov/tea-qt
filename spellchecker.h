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

  bool initialized;

  CSpellchecker (const QString &lang,
                 const QString &path,
                 const QString &user): initialized {false} {};

  virtual ~CSpellchecker() {};

  virtual void change_lang (const QString &lang) = 0;
  virtual void add_to_user_dict (const QString &word) = 0;
  virtual void remove_from_user_dict (const QString &word) = 0;

  virtual bool check (const QString &word) = 0;

  virtual QStringList get_speller_modules_list() = 0;
  virtual QStringList get_suggestions_list (const QString &word) = 0;
};


#ifdef ASPELL_ENABLE

class CAspellchecker: public CSpellchecker
{

public:

  AspellConfig *spell_config;
  AspellCanHaveError *ret;
  AspellSpeller *speller;

  CAspellchecker (const QString &lang, const QString &path = "", const QString &user_path = "");
  ~CAspellchecker();

  void change_lang (const QString &lang);
  void add_to_user_dict (const QString &word);
  void remove_from_user_dict (const QString &word);

  bool check (const QString &word);

  QStringList get_speller_modules_list();
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
  QString user_dir;
  QString lng;

  const char *encoding;
  std::string str_encoding;

  QString dict_dir;

  CHunspellChecker (const QString &lang, const QString &path = "", const QString &user_path = "");
  ~CHunspellChecker();

  void change_lang (const QString &lang);
  void add_to_user_dict (const QString &word);
  void remove_from_user_dict (const QString &word);

  bool check (const QString &word);

  QStringList get_speller_modules_list();
  QStringList get_suggestions_list (const QString &word);
};

QString hunspell_default_dict_path(); 


#endif



#endif
