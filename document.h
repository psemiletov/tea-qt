/***************************************************************************
 *   2007-2017 by Peter Semiletov <peter.semiletov@gmail.com>                            *
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


#include "logmemo.h"
#include "tio.h"
#include "todo.h"


#include <QPoint>

#include <QStatusBar>
#include <QMainWindow>
#include <QTabWidget>
#include <QPlainTextEdit>
#include <QSyntaxHighlighter>
//#include <QDebug>

#if QT_VERSION >= 0x050000

#include <QRegularExpression>

#endif


class LineNumberArea;
class document_holder;
class CDocument;


class CTEAEdit: public QPlainTextEdit
{
  Q_OBJECT
  

public:

  QString indent_val;  
  QList <QTextEdit::ExtraSelection> extraSelections;
  QTextEdit::ExtraSelection brace_selection;

  
  //rect selection

  QPoint rect_sel_start;
  QPoint rect_sel_end;
 
  CDocument *doc;

  QColor currentLineColor;
  QColor brackets_color;
  QColor margin_color;
  QColor linenums_bg;
  QColor text_color;

  QColor sel_text_color;
  QColor sel_back_color;


  QWidget *lineNumberArea;

  bool highlightCurrentLine;
  bool hl_brackets;
  bool draw_margin;
  bool draw_linenums;
  
  bool use_hl_wrap;
  bool wrap;

  bool auto_indent;
  bool spaces_instead_of_tabs;
  int tab_sp_width; //in spaces
  int brace_width; //in pixels
  int margin_pos; //in chars
  int margin_x; //in pixels


  CTEAEdit (QWidget *parent = 0);

  void setCurrentLineColor (const QColor &newColor);
  void set_brackets_color (const QColor &newColor);
  void set_show_linenums (bool enable);
  void set_show_margin (bool enable);
  void set_margin_pos (int mp);
  void set_hl_cur_line (bool enable);
  void set_hl_brackets (bool enable);

  void indent();
  void un_indent();
  void calc_auto_indent();
  void setup_brace_width();

  void text_replace (const QString &s);

  void rect_sel_reset();
  void rect_sel_replace (const QString &s, bool insert = false);
  //void rect_sel_insert (const QString &s);
  
  
  void update_ext_selections();
  
  
  void update_rect_sel();
  
  QString get_rect_sel();

  void rect_sel_cut (bool just_del = false);


  void braceHighlight();
  
  void lineNumberAreaPaintEvent(QPaintEvent *event);
  int lineNumberAreaWidth();


  Q_INVOKABLE bool has_rect_selection();
  

protected:

  bool canInsertFromMimeData (const QMimeData *source);
  void insertFromMimeData (const QMimeData *source);
  
  QMimeData* createMimeDataFromSelection(); 

  void paintEvent(QPaintEvent *event);
  void keyPressEvent (QKeyEvent *event);    
  void resizeEvent(QResizeEvent *event);
  
  /*
 bool event(QEvent *ev) {
        if (ev->type() == QEvent::KeyboardLayoutChange) {
            qDebug() << "kb layout changed";
        } else {
            return QWidget::event(ev);
        }

    }
*/
public slots:

  void updateLineNumberAreaWidth (int newBlockCount);
  void cb_cursorPositionChanged();
  void updateLineNumberArea (const QRect &, int);
  
  void slot_selectionChanged();
  
 // void copy();
  
};


class CSyntaxHighlighter: public QSyntaxHighlighter
{
public:
 
  CSyntaxHighlighter (QTextDocument *parent = 0, CDocument *doc = 0, const QString &fname = "none");
 
  CDocument *document;
  bool casecare;
  bool wrap;
  
  int xml_format;
  
  QString exts;
  QString langs;
    
  QString cm_mult;
  QString cm_single;
 
  QTextCharFormat singleLineCommentFormat;
  QTextCharFormat multiLineCommentFormat;
};


class CSyntaxHighlighterQRegExp: public CSyntaxHighlighter
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
    
  CSyntaxHighlighterQRegExp (QTextDocument *parent = 0, CDocument *doc = 0, const QString &fname = "none");

  QVector <HighlightingRule> highlightingRules;
 
  void load_from_xml (const QString &fname);
};


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
    
  CSyntaxHighlighterQRegularExpression (QTextDocument *parent = 0, CDocument *doc = 0, const QString &fname = "none");

  QVector <HighlightingRule> highlightingRules;

  QTextCharFormat singleLineCommentFormat;
  QTextCharFormat multiLineCommentFormat;

  void load_from_xml (const QString &fname);
};

#endif


class CDocument: public QObject
{
  Q_OBJECT

public:

  bool cursor_xy_visible;

  QHash <QString, QString> fnameswoexts;
  
  QString eol;
  
  document_holder *holder;
  CSyntaxHighlighter *highlighter;
  CTEAEdit *textEdit;

  QStringList labels;

  QString markup_mode;
  QString file_name;
  QString text_to_search;
  QString charset;
  QWidget *tab_page;
  int position;

  CDocument (QObject *parent = 0);
  ~CDocument();

  Q_INVOKABLE QString get_selected_text() const;
  Q_INVOKABLE void set_selected_text (const QString &value);

  void create_new();
  Q_INVOKABLE void set_tab_caption (const QString &fileName);
  Q_INVOKABLE bool save_with_name (const QString &fileName, const QString &codec);
  Q_INVOKABLE bool save_with_name_plain (const QString &fileName);
  Q_INVOKABLE void goto_pos (int pos);
  Q_INVOKABLE QString get_filename_at_cursor();
  Q_INVOKABLE QStringList get_words();
  Q_INVOKABLE void set_hl (bool mode_auto = true, const QString &theext = "txt");
  Q_INVOKABLE bool open_file (const QString &fileName, const QString &codec);
  Q_INVOKABLE void insert_image (const QString &full_path);
  Q_INVOKABLE void set_markup_mode();
  Q_INVOKABLE void reload (const QString &enc);
  Q_INVOKABLE int get_tab_idx();
  Q_INVOKABLE QString get_triplex();
  Q_INVOKABLE void update_status();
  Q_INVOKABLE void update_title (bool fullname = true);


  Q_INVOKABLE void update_labels();
};


class document_holder: public QObject
{
  Q_OBJECT

public:

  CTioHandler tio_handler;

  QString fname_current_session;

 // QHash <QString, QString> palette;
  QHash <QString, QString> hls;

  QLabel *l_status_bar;
  QString dir_config;

  QString fname_crapbook;

  CTodo todo;

  QString markup_mode;
  QStatusBar *status_bar;
  CLogMemo *log;
  QList <CDocument*> list;
  QMainWindow *parent_wnd;
  QTabWidget *tab_widget;

  QMenu *recent_menu;
  QStringList recent_files;
  QString recent_list_fname;

  
  ~document_holder();

  Q_INVOKABLE CDocument* create_new();
  Q_INVOKABLE CDocument* open_file (const QString &fileName, const QString &codec);
  Q_INVOKABLE CDocument* open_file_triplex (const QString &triplex);
  Q_INVOKABLE CDocument* get_document_by_fname (const QString &fileName);

  void reload_recent_list();

  Q_INVOKABLE void save_to_session (const QString &fileName);
  Q_INVOKABLE void load_from_session (const QString &fileName);
  
  //QHash <QString, QString> load_eclipse_theme_xml (const QString &fname);

 //void load_palette (const QString &fileName);

  void close_by_idx (int i);
  Q_INVOKABLE void close_current();
  Q_INVOKABLE void apply_settings();
  Q_INVOKABLE void apply_settings_single (CDocument *d);

  void add_to_recent (CDocument *d);
  void update_recent_menu();

  void update_current_files_menu();

  Q_INVOKABLE CDocument* get_current();
 

public slots:
 
  void open_recent();
  void open_current();
};


class CLineNumberArea: public QWidget
{
public:

  CTEAEdit *codeEditor;

  CLineNumberArea (CTEAEdit *editor = 0): QWidget (editor)
                 {
                  codeEditor = editor;
                 }

  QSize sizeHint() const {
                          return QSize(codeEditor->lineNumberAreaWidth(), 0);
                         }

protected:

  void paintEvent (QPaintEvent *event)
      {
       codeEditor->lineNumberAreaPaintEvent (event);
      }

};

  

#endif
