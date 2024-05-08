/***************************************************************************
 *   2007-2024 by Peter Semiletov <peter.semiletov@gmail.com>              *
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
#include <utility>

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


using namespace std;


class CDox;
class CDocument;
class CLineNumberArea;


class CSyntaxHighlighter: public QSyntaxHighlighter
{
public:

  CDocument *document;
  bool casecare;

  QString comment_mult;
  QString comment_single;

  QTextCharFormat fmt_multi_line_comment;

  CSyntaxHighlighter (QTextDocument *parent = 0, CDocument *doc = 0, const QString &fname = "none");
};


#if QT_VERSION < 0x050000
class CSyntaxHighlighterQRegExp: public CSyntaxHighlighter
{
  Q_OBJECT

protected:

  void highlightBlock (const QString &text);

public:

  vector <pair <QRegExp, QTextCharFormat> > hl_rules;

  pair <QRegExp, bool> comment_start_expr;
  pair <QRegExp, bool> comment_end_expr;

  Qt::CaseSensitivity cs;

  CSyntaxHighlighterQRegExp (QTextDocument *parent = 0, CDocument *doc = 0, const QString &fname = "none");
  //void load_from_xml (const QString &fname);
  void load_from_xml_pugi (const QString &fname);

};
#endif


#if QT_VERSION >= 0x050000
class CSyntaxHighlighterQRegularExpression: public CSyntaxHighlighter
{
  Q_OBJECT

protected:

  void highlightBlock (const QString &text);

public:

  vector <pair <QRegularExpression, QTextCharFormat> > hl_rules;

  QRegularExpression::PatternOptions pattern_opts;

  pair <QRegularExpression, bool> comment_start_expr;
  pair <QRegularExpression, bool> comment_end_expr;

  CSyntaxHighlighterQRegularExpression (QTextDocument *parent = 0, CDocument *doc = 0, const QString &fname = "none");
  //void load_from_xml (const QString &fname);
  void load_from_xml_pugi (const QString &fname);

};
#endif


class CDocument: public QPlainTextEdit
{
  Q_OBJECT

private:

  CLineNumberArea *line_num_area;
  QList <QTextEdit::ExtraSelection> extra_selections;
  QTextEdit::ExtraSelection brace_selection;

protected:

  QAction *actCopy;
  QAction *actCut;
  QAction *actPaste;

  QMimeData* createMimeDataFromSelection() const;
  bool canInsertFromMimeData (const QMimeData *source) const;
  void insertFromMimeData (const QMimeData *source);
  void paintEvent (QPaintEvent *event);
  void keyPressEvent (QKeyEvent *event);
  void resizeEvent (QResizeEvent *event);
  void wheelEvent (QWheelEvent *e);
  void contextMenuEvent (QContextMenuEvent *event);


  //QMenu* createStandardContextMenu();

public:

  CDox *holder; //uplink
  QWidget *tab_page; //pointer

  CSyntaxHighlighter *highlighter;

  QStringList labels;

  QString eol;
  QString markup_mode;
  QString file_name;
  QString text_to_search;
  QString charset;
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

  bool cursor_xy_visible;
  bool highlight_current_line;
  bool hl_brackets;
  bool draw_margin;
  bool draw_linenums;
  bool auto_indent;
  bool spaces_instead_of_tabs;
  bool show_tabs_and_spaces;

  int position;
  int tab_sp_width; //in spaces
  int brace_width; //in pixels
  int margin_pos; //in chars
  int margin_x;  //in pixels

  CDocument (CDox *hldr, QWidget *parent = 0);
  ~CDocument();

  QString get() const; //return selected text
  void put (const QString &value); //replace selection or insert text at cursor

  bool has_selection();


  bool file_open (const QString &fileName, const QString &codec);
  bool file_save_with_name (const QString &fileName, const QString &codec);
  bool file_save_with_name_plain (const QString &fileName);

  int get_tab_idx();
  QString get_triplex();
  QString get_filename_at_cursor();
  QStringList get_words();

  void goto_pos (int pos);

  void set_tab_caption (const QString &fileName);
  void set_hl (bool mode_auto = true, const QString &theext = "txt");
  void set_markup_mode();

  void insert_image (const QString &full_path);
  void reload (const QString &enc);

  void update_status();
  void update_title (bool fullname = true);
  void update_labels();

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

  void rect_block_start();
  void rect_block_end();

  bool has_rect_selection() const;
  void rect_sel_reset();
  void rect_sel_replace (const QString &s, bool insert = false);
  void rect_sel_upd();
  QString rect_sel_get() const;
  void rect_sel_cut (bool just_del = false);

  void lineNumberAreaPaintEvent (QPaintEvent *event);
  int line_number_area_width();

public slots:

  void updateLineNumberAreaWidth();
  void cb_cursorPositionChanged();
  void updateLineNumberArea (const QRect &, int);
  void slot_selectionChanged();

  void ed_copy();
  void ed_cut();
  void ed_paste();


};


class CDox: public QObject
{
  Q_OBJECT

public:

  QStringList recent_files;

//regexp pattern and file name of syntax hl rules
#if QT_VERSION >= 0x050000
  std::vector<std::pair <QRegularExpression, QString> > hl_files;
#else
  std::vector<std::pair <QRegExp, QString> > hl_files;
#endif

  std::vector <CDocument*> items;

  QHash <QString, QString> markup_modes;
  QHash <QString, QString> hash_project;
  QHash <QString, QString> autosave_files;

  CTioHandler tio_handler;
  CTodo todo;

  QString dir_last;
  QString fname_current_session;
  QString fname_current_project;
  QString dir_config;
  QString fname_crapbook;
  QString fname_saved_buffers;

  QString markup_mode;
  QString recent_list_fname;

  QLabel *l_status_bar;
  QLabel *l_charset;

  CLogMemo *log; //uplink
  QMainWindow *parent_wnd; //uplink
  QTabWidget *tab_widget; //uplink
  QTabWidget *main_tab_widget; //uplink
  QMenu *menu_recent; //uplink
  QTimer timer_joystick;
  QTimer timer_autosave;

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

  CDocument* create_new();
  CDocument* open_file (const QString &fileName, const QString &codec);
  CDocument* open_file_triplex (const QString &triplex);
  CDocument* get_document_by_fname (const QString &fileName);
  CDocument* get_current();

  void close_by_idx (int i);
  void close_current();

  void save_to_session (const QString &fileName);
  void load_from_session (const QString &fileName);

  void save_buffers (const QString &fileName);
  void load_from_buffers (const QString &fileName);

  void apply_settings();
  void apply_settings_single (CDocument *d);

#if defined(JOYSTICK_SUPPORTED)
  bool event (QEvent *ev);
  void handle_joystick_event (CJoystickAxisEvent *ev);
#endif

public slots:

  void open_recent();
  void open_current();

  void autosave();

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

  CDocument *code_editor; //uplink

  CLineNumberArea (CDocument *editor = 0): QWidget (editor), code_editor (editor) {}

  QSize sizeHint() const {
                          return QSize (code_editor->line_number_area_width(), 0);
                         }

protected:

  void paintEvent (QPaintEvent *event)
      {
       code_editor->lineNumberAreaPaintEvent (event);
      }

};

#endif
