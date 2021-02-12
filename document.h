/***************************************************************************
 *   2007-2021 by Peter Semiletov <peter.semiletov@gmail.com>                            *
 *                                                                         *
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

/*
Copyright (C) 2006-2008 Trolltech ASA. All rights reserved.
*/

/*
Diego Iastrubni <elcuco@kde.org> //some GPL'ed code from new-editor-diego-3, found on qtcentre forum
*/

/*
code from qwriter:
 *   Copyright (C) 2009 by Gancov Kostya                                   *
 *   kossne@mail.ru                                                        *
*/


#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <vector>

#include <QPoint>
#include <QStatusBar>
#include <QMainWindow>
#include <QTabWidget>
#include <QPlainTextEdit>
#include <QSyntaxHighlighter>
#include <QMultiHash> 

#if QT_VERSION >= 0x050000
#include <QRegularExpression>
#else
#include <QRegExp>
#endif

#if defined (JOYSTICK_SUPPORTED)
#include "myjoystick.h"
#endif

#include "logmemo.h"
#include "tio.h"
#include "todo.h"


class CDox;
class CDocument;
class CLineNumberArea;


class CSyntaxHighlighter: public QSyntaxHighlighter
{
public:

  CDocument *document;
  bool casecare;
  int xml_format;

  QString exts;
  QString langs;
  QString cm_mult;
  QString cm_single;

  QTextCharFormat singleLineCommentFormat;
  QTextCharFormat multiLineCommentFormat;

  CSyntaxHighlighter (QTextDocument *parent = 0, CDocument *doc = 0, const QString &fname = "none");
};


#if QT_VERSION < 0x050000
class CSyntaxHighlighterQRegExp: public CSyntaxHightlighter
{
  Q_OBJECT

protected:

  void highlightBlock (const QString &text);

public:

  struct HighlightingRule
         {
          QRegExp pattern;
          QTextCharFormat format;
         };

  QRegExp commentStartExpression;
  QRegExp commentEndExpression;

  Qt::CaseSensitivity cs;

  std::vector <HighlightingRule> highlightingRules;

  CSyntaxHighlighterQRegExp (QTextDocument *parent = 0, CDocument *doc = 0, const QString &fname = "none");
  void load_from_xml (const QString &fname);
};
#endif


#if QT_VERSION >= 0x050000
class CSyntaxHighlighterQRegularExpression: public CSyntaxHighlighter
{
  Q_OBJECT

protected:

  void highlightBlock (const QString &text);

public:

  struct HighlightingRule
        {
         QRegularExpression pattern;
         QTextCharFormat format;
        };

  QRegularExpression commentStartExpression;
  QRegularExpression commentEndExpression;
  QRegularExpression::PatternOptions pattern_opts;

  QTextCharFormat singleLineCommentFormat;
  QTextCharFormat multiLineCommentFormat;

  std::vector <HighlightingRule> highlightingRules;

  CSyntaxHighlighterQRegularExpression (QTextDocument *parent = 0, CDocument *doc = 0, const QString &fname = "none");
  void load_from_xml (const QString &fname);
};
#endif


class CDocument: public QPlainTextEdit
{
  Q_OBJECT

private:

  QWidget *line_num_area;
  QList <QTextEdit::ExtraSelection> extra_selections;
  QTextEdit::ExtraSelection brace_selection;

protected:

  QMimeData* createMimeDataFromSelection() const;
  bool canInsertFromMimeData (const QMimeData *source) const;
  void insertFromMimeData (const QMimeData *source);
  void paintEvent (QPaintEvent *event);
  void keyPressEvent (QKeyEvent *event);
  void resizeEvent (QResizeEvent *event);
  void wheelEvent (QWheelEvent *e);

public:

  CDox *holder; //uplink
  QWidget *tab_page; //pointer

  CSyntaxHighlighter *highlighter;

  bool cursor_xy_visible;

  QString eol;
  QStringList labels;

  QString markup_mode;
  QString file_name;
  QString text_to_search;
  QString charset;
  int position;

  bool highlight_current_line;
  bool hl_brackets;
  bool draw_margin;
  bool draw_linenums;
  bool auto_indent;
  bool spaces_instead_of_tabs;
  int tab_sp_width; //in spaces
  int brace_width; //in pixels
  int margin_pos; //in chars
  int margin_x;  //in pixels

  QString indent_val;

  QPoint rect_sel_start; //rect selection
  QPoint rect_sel_end;   //rect selection

  QColor current_line_color;
  QColor brackets_color;
  QColor margin_color;
  QColor linenums_bg;
  QColor text_color;

  QColor sel_text_color;
  QColor sel_back_color;


  CDocument (CDox *hldr, QWidget *parent = 0);
  ~CDocument();

  Q_INVOKABLE QString get() const; //return selected text
  Q_INVOKABLE void put (const QString &value); //replace selection or insert text at cursor

  Q_INVOKABLE bool file_open (const QString &fileName, const QString &codec);
  Q_INVOKABLE bool file_save_with_name (const QString &fileName, const QString &codec);
  Q_INVOKABLE bool file_save_with_name_plain (const QString &fileName);

  Q_INVOKABLE int get_tab_idx();
  Q_INVOKABLE QString get_triplex();

  Q_INVOKABLE QString get_filename_at_cursor();
  Q_INVOKABLE QStringList get_words();

  Q_INVOKABLE void goto_pos (int pos);

  Q_INVOKABLE void set_tab_caption (const QString &fileName);
  Q_INVOKABLE void set_hl (bool mode_auto = true, const QString &theext = "txt");
  Q_INVOKABLE void set_markup_mode();

  Q_INVOKABLE void insert_image (const QString &full_path);
  Q_INVOKABLE void reload (const QString &enc);

  Q_INVOKABLE void update_status();
  Q_INVOKABLE void update_title (bool fullname = true);
  Q_INVOKABLE void update_labels();

  void set_show_linenums (bool enable);
  void set_show_margin (bool enable);
  void set_margin_pos (int mp);
  void set_hl_cur_line (bool enable);
  void set_hl_brackets (bool enable);

  void set_word_wrap (bool wrap);
  bool get_word_wrap();

  void indent();
  void un_indent();
  void calc_auto_indent();

  void setup_brace_width();
  void brace_highlight();

  void update_ext_selections();
  Q_INVOKABLE bool has_rect_selection() const;
  Q_INVOKABLE void rect_sel_reset();
  Q_INVOKABLE void rect_sel_replace (const QString &s, bool insert = false);
  Q_INVOKABLE void rect_sel_upd();
  Q_INVOKABLE QString rect_sel_get() const;
  Q_INVOKABLE void rect_sel_cut (bool just_del = false);

  void lineNumberAreaPaintEvent (QPaintEvent *event);
  int line_number_area_width();

public slots:

  void updateLineNumberAreaWidth();
  void cb_cursorPositionChanged();
  void updateLineNumberArea (const QRect &, int);
  void slot_selectionChanged();
};


class CDox: public QObject
{
  Q_OBJECT

public:

  QStringList recent_files;
  QMultiHash <QString, QString> hls;
  QHash <QString, QString> markup_modes;
  QHash <QString, QString> hash_project;
  std::vector <CDocument*> items;

  CTioHandler tio_handler;
  CTodo todo;

  QString fname_current_session;
  QString fname_current_project;
  QString dir_config;
  QString fname_crapbook;
  QString markup_mode;
  QString recent_list_fname;

  QLabel *l_status_bar;
  QLabel *l_charset;
  CLogMemo *log;
  QMainWindow *parent_wnd;
  QTabWidget *tab_widget;
  QTabWidget *main_tab_widget;
  QMenu *recent_menu;
  QTimer *timer;

#if defined(JOYSTICK_SUPPORTED)
  CJoystick *joystick;
#endif


  CDox();
  ~CDox();

  void update_project (const QString &fileName);
  void reload_recent_list();
  void add_to_recent (CDocument *d);
  void update_recent_menu();
  void update_current_files_menu();

  void move_cursor (QTextCursor::MoveOperation mo);

  Q_INVOKABLE CDocument* create_new();
  Q_INVOKABLE CDocument* open_file (const QString &fileName, const QString &codec);
  Q_INVOKABLE CDocument* open_file_triplex (const QString &triplex);
  Q_INVOKABLE CDocument* get_document_by_fname (const QString &fileName);
  Q_INVOKABLE CDocument* get_current();

  Q_INVOKABLE void close_by_idx (int i);
  Q_INVOKABLE void close_current();
  Q_INVOKABLE void save_to_session (const QString &fileName);
  Q_INVOKABLE void load_from_session (const QString &fileName);
  Q_INVOKABLE void apply_settings();
  Q_INVOKABLE void apply_settings_single (CDocument *d);

#if defined(JOYSTICK_SUPPORTED)

  bool event (QEvent *ev);
  void handle_joystick_event (CJoystickAxisEvent *ev);

#endif


public slots:

  void open_recent();
  void open_current();

  void move_cursor_up();
  void move_cursor_down();
  void move_cursor_left();
  void move_cursor_right();
  void move_cursor_x (double v);
  void move_cursor_y (double v);
};


class CLineNumberArea: public QWidget
{
public:

  CDocument *code_editor;

  CLineNumberArea (CDocument *editor = 0): QWidget (editor), code_editor (editor) {}

  QSize sizeHint() const {
                          return QSize(code_editor->line_number_area_width(), 0);
                         }

protected:

  void paintEvent (QPaintEvent *event)
       {
        code_editor->lineNumberAreaPaintEvent (event);
       }

};

#endif


