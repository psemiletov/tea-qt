/***************************************************************************
 *   2007-2017 by Peter Semiletov                                          *
 *   peter.semiletov@gmail.com                                                           *
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

/* some code by
Copyright (C) 2006-2008 Trolltech ASA. All rights reserved.
*/

/*
code from qPEditor:
Copyright (C) 2007 by Michael Protasov, mik-protasov@anyqsoft.com
*/

/*
Diego Iastrubni <elcuco@kde.org> //some GPL'ed code from new-editor-diego-3,
found on qtcentre forum
*/

/*
code from qwriter:
*   Copyright (C) 2009 by Gancov Kostya                                   *
*   kossne@mail.ru                                                        *
*/


#include "document.h"
#include "utils.h"
#include "gui_utils.h"

#include "textproc.h"


#include <QApplication>
#include <QClipboard>
#include <QSettings>
#include <QLabel>
#include <QPainter>
#include <QDebug>
#include <QUrl>
#include <QDir>
#include <QXmlStreamReader>
#include <QMimeData>

#include <bitset>
#include <algorithm>

#if QT_VERSION >= 0x050000

#include <QRegularExpression>

#endif


QHash <QString, QString> global_palette;
QSettings *settings;
QMenu *current_files_menu;

int recent_list_max_items;

bool b_recent_off;
bool b_destroying_all;

void CDocument::update_status()
{
  if (! cursor_xy_visible)
     {
      holder->l_status_bar->setText (charset);
      return;
     }

//  QTextBlock block = textEdit->document()->begin();
//  int cursor_offset = textEdit->textCursor().position();
 
 /* 
  int off = 0;
  int y = 1;

  while (cursor_offset >= (off + block.length()))
        {
         off += block.length();
         block = block.next();
         y++;
        }

  int x = cursor_offset - off + 1;
*/
  int x = textEdit->textCursor().position() - textEdit->textCursor().block().position() + 1;
  int y = textEdit->textCursor().block().blockNumber() + 1;

  holder->l_status_bar->setText (QString ("%1:%2 %3").arg (
                                 QString::number (y)).arg (
                                 QString::number (x)).arg (charset));
}


void CDocument::update_title (bool fullname)
{
  QMainWindow *w = qobject_cast <QMainWindow *> (holder->parent_wnd);

  if (fullname)
     w->setWindowTitle (file_name);
  else
      w->setWindowTitle (QFileInfo (file_name).fileName());
}


void CDocument::reload (const QString &enc)
{
  if (file_exists (file_name))
      open_file (file_name, enc);
}


bool CDocument::open_file (const QString &fileName, const QString &codec)
{
  CTio *tio = holder->tio_handler.get_for_fname (fileName);
  
  if (! tio)
      return false;

  if (fileName.contains (holder->dir_config))
      tio->charset = "UTF-8";
  else
      tio->charset = codec;

  if (! tio->load (fileName))
     {
      holder->log->log (tr ("cannot open %1 because of: %2")
                           .arg (fileName)
                           .arg (tio->error_string));
      return false;
     }

  charset = tio->charset;
  eol = tio->eol;

  textEdit->setPlainText (tio->data);

  file_name = fileName;

  set_tab_caption (QFileInfo (file_name).fileName());
  set_hl();
  
  set_markup_mode();
  textEdit->document()->setModified (false);

  holder->log->log (tr ("%1 is open").arg (file_name));

  QMutableListIterator <QString> i (holder->recent_files);

  while (i.hasNext())
        {
         QStringList lt = i.next().split(",");
         if (lt.size() > 0 && lt[0] == file_name) 
            i.remove();
        }
       
  return true;
}


bool CDocument::save_with_name (const QString &fileName, const QString &codec)
{
  CTio *tio = holder->tio_handler.get_for_fname (fileName);
  
  if (! tio)
      return false;

  if (fileName.contains (holder->dir_config))
      tio->charset = "UTF-8";
  else
      tio->charset = codec;

  tio->data = textEdit->toPlainText();
  
   if (eol != "\n")
      tio->data.replace ("\n", eol);

  if (! tio->save (fileName))
     {
      holder->log->log (tr ("cannot save %1 because of: %2")
                        .arg (fileName)
                        .arg (tio->error_string));
      return false;
     }

  charset = tio->charset;
  file_name = fileName;

  set_tab_caption (QFileInfo (file_name).fileName());

  holder->log->log (tr ("%1 is saved").arg (file_name));

  update_title (settings->value ("full_path_at_window_title", 1).toBool());
  update_status();

  textEdit->document()->setModified (false);
  
  holder->update_current_files_menu();

  return true;
}


CDocument::CDocument (QObject *parent): QObject (parent)
{
  QString fname = tr ("new[%1]").arg (QTime::currentTime().toString ("hh-mm-ss"));
  
  fnameswoexts.insert ("configure", "sh");
  fnameswoexts.insert ("install-sh", "sh");
  
  markup_mode = "HTML";
  file_name = fname;
  cursor_xy_visible = true;
  charset = "UTF-8";
  text_to_search = "";
  position = 0;
  
#if defined(Q_OS_WIN) || defined(Q_OS_OS2)

  eol = "\r\n";

//  qDebug() << "defined(Q_OS_WIN) || defined(Q_OS_OS2)";

#elif defined(Q_OS_UNIX)

  eol = "\n";
/*
#elif defined(Q_OS_MAC)

  eol = "\r";
*/
#endif

  //qDebug() << "CDocument()";
/*
  if (eol == "\n")
    qDebug() << "LF";
  if (eol == "\r\n")
    qDebug() << "CRLF";
  if (eol == "\r")
    qDebug() << "CR";*/
}


CDocument::~CDocument()
{
  if (file_name.endsWith (".notes") && textEdit->document()->isModified()) 
     save_with_name_plain (file_name);
  else
  if (file_name.startsWith (holder->dir_config) && textEdit->document()->isModified()) 
     save_with_name_plain (file_name);
  else
  if (textEdit->document()->isModified() && file_exists (file_name))
     {
      if (QMessageBox::warning (0, "TEA",
                                tr ("%1 has been modified.\n"
                                "Do you want to save your changes?").arg (file_name),
                                QMessageBox::Ok | QMessageBox::Default,
                                QMessageBox::Cancel | QMessageBox::Escape) == QMessageBox::Ok)
         save_with_name (file_name, charset);
     }

  if (! file_name.startsWith (holder->dir_config))
  if (! file_name.endsWith (".notes")) 
     {
      holder->add_to_recent (this);
      holder->update_recent_menu();
     }

  if (file_name.startsWith (holder->todo.dir_days))
      holder->todo.load_dayfile();


  QMainWindow *w = qobject_cast <QMainWindow *> (holder->parent_wnd);
  w->setWindowTitle ("");

  int i = holder->tab_widget->indexOf (tab_page);
  if (i != -1)
     holder->tab_widget->removeTab (i);
}


void CDocument::create_new()
{
  textEdit = new CTEAEdit;

  textEdit->doc = this;

  textEdit->currentLineColor = QColor (hash_get_val (global_palette, 
                                             "cur_line_color", 
                                             "#EEF6FF")).darker (settings->value ("darker_val", 100).toInt()).name();

  highlighter = NULL;

  int tab_index = holder->tab_widget->addTab (textEdit, file_name);
  tab_page = holder->tab_widget->widget (tab_index);
   
  textEdit->setFocus (Qt::OtherFocusReason);
}


int CDocument::get_tab_idx()
{
  return holder->tab_widget->indexOf (tab_page);
}


void CDocument::set_tab_caption (const QString &fileName)
{
  holder->tab_widget->setTabText (get_tab_idx(), fileName);
}


void document_holder::reload_recent_list (void)
{
  if (! file_exists (recent_list_fname))
     return;

  recent_files = qstring_load (recent_list_fname).split ("\n");
}


document_holder::~document_holder()
{
  b_destroying_all = true;

  while (! list.isEmpty())
        delete list.takeFirst();

  qstring_save (recent_list_fname, recent_files.join ("\n"));
}


CDocument* document_holder::create_new()
{
  CDocument *doc = new CDocument;

  doc->holder = this;
  doc->markup_mode = markup_mode;
  list.append (doc);

  doc->create_new();

  tab_widget->setCurrentIndex (tab_widget->indexOf (doc->tab_page));
  apply_settings_single (doc);

  doc->update_title (settings->value ("full_path_at_window_title", 1).toBool());
  doc->update_status();
  
  update_current_files_menu();
  
  return doc;
}


CDocument* document_holder::get_document_by_fname (const QString &fileName)
{
  if (fileName.isEmpty())
     return NULL;

  foreach (CDocument *d, list)
          if (d->file_name == fileName)
              return d;

  return NULL;
}


CDocument* document_holder::open_file (const QString &fileName, const QString &codec)
{
  if (! file_exists (fileName))
     return NULL;
  
  if (is_image (fileName))
     {
      CDocument *td = get_current();
      if (td)
         {
          td->insert_image (fileName);
          td->textEdit->setFocus (Qt::OtherFocusReason);
         }
      return td;
     }   
   
  CDocument *d = get_document_by_fname (fileName);
  if (d)
     {
      tab_widget->setCurrentIndex (tab_widget->indexOf (d->tab_page));
      d->reload (codec);
      return d;
     }

  CDocument *doc = create_new();
  doc->open_file (fileName, codec);
  doc->update_status();
  doc->update_title (settings->value ("full_path_at_window_title", 1).toBool());

  tab_widget->setCurrentIndex (tab_widget->indexOf (doc->tab_page));
  
  update_current_files_menu();
  
  return doc;
}


void document_holder::close_by_idx (int i)
{
  if (i < 0)
     return;

  CDocument *d = list[i];
  list.removeAt (i);
  delete d;
  
  update_current_files_menu();
}


void document_holder::close_current()
{
  close_by_idx (tab_widget->currentIndex());
}


CDocument* document_holder::get_current()
{
  int i = tab_widget->currentIndex();
  if (i < 0)
     return NULL;

  return list[i];
}


bool CDocument::save_with_name_plain (const QString &fileName)
{
  QFile file (fileName);
  if (! file.open (QFile::WriteOnly | QFile::Text))
      return false;

  QTextStream out (&file);
  out.setCodec (charset.toUtf8().data());
  out << textEdit->toPlainText();

  holder->update_current_files_menu();
  
  return true;
}


QString CDocument::get_filename_at_cursor()
{
  if (textEdit->textCursor().hasSelection())
    {
     QFileInfo nf (file_name);
     QDir cd (nf.absolutePath());
     return cd.cleanPath (cd.absoluteFilePath(textEdit->textCursor().selectedText()));
    }

  QString s = textEdit->toPlainText();

  QString x;

  if (markup_mode == "LaTeX")
     {
      int pos = textEdit->textCursor().position();

      int end = s.indexOf ("}", pos);
      if (end == -1)
         return x;

     int start = s.lastIndexOf ("{", pos);
     if (start == -1)
        return x;

     x = s.mid (start + 1, end - (start + 1));
     
     QFileInfo inf (file_name);
     QDir cur_dir (inf.absolutePath());
         
     QString result = cur_dir.cleanPath (cur_dir.absoluteFilePath(x));
     if (file_exists (result))
         return result;

     
     int i = x.lastIndexOf ("/");
     if (i < 0)
        i = x.lastIndexOf ("\\");
     
     if (i < 0)
        return QString();

     x = x.mid (i + 1);   
     
     result = cur_dir.cleanPath (cur_dir.absoluteFilePath(x));
     //qDebug() << "in cur dir: " << result;
     return result;
    }
  else
 //   if (markup_mode == "HTML")
   //fallback to HTML markup 
     {
      int pos = textEdit->textCursor().position();

      int end = s.indexOf ("\"", pos);
      if (end == -1)
         return x;

      int start = s.lastIndexOf ("\"", pos);
      if (start == -1)
         return x;

      x = s.mid (start + 1, end - (start + 1));

      if (x.startsWith("#"))
         return x;

      QFileInfo inf (file_name);
      QDir cur_dir (inf.absolutePath());

      return cur_dir.cleanPath (cur_dir.absoluteFilePath(x));
     }
}


CSyntaxHighlighter::CSyntaxHighlighter (QTextDocument *parent, CDocument *doc, const QString &fname): QSyntaxHighlighter (parent) 
{
  //qDebug() << "CSyntaxHighlighter::CSyntaxHighlighter";
  document = doc; 
  xml_format = 0;
  //casecare = true; 
  //wrap = true;
}


CSyntaxHighlighterQRegExp::CSyntaxHighlighterQRegExp (QTextDocument *parent, CDocument *doc, const QString &fname):
                                                      CSyntaxHighlighter (parent, doc, fname)
{
  document = doc;
  casecare = true;
  wrap = true;
  load_from_xml (fname);
}


#if QT_VERSION >= 0x050000

CSyntaxHighlighterQRegularExpression::CSyntaxHighlighterQRegularExpression (QTextDocument *parent, CDocument *doc, const QString &fname):
                                                                            CSyntaxHighlighter (parent, doc, fname)
{
  //qDebug() << "CSyntaxHighlighterQRegularExpression::CSyntaxHighlighterQRegularExpression";

  document = doc;
  //casecare = true;
  //wrap = true;
  load_from_xml (fname);
}

#endif


void CDocument::set_hl (bool mode_auto, const QString &theext)
{
  QString ext;
  
  if (mode_auto)
     ext = file_get_ext (file_name);
  else
      ext = theext;

  if (ext.isEmpty())
     {
      QFileInfo fi (file_name);
      ext = fnameswoexts[fi.fileName()];
     }

  if (ext.isEmpty())
     return;

  QString fname = holder->hls.value (ext);
  
  //qDebug() << "hl fname is " << fname;
  
  if (fname.isEmpty())
     return;

  if (highlighter)
     delete highlighter;
  
#if QT_VERSION >= 0x050000
  
  if (settings->value ("qregexpsyntaxhl", 0).toBool())
     highlighter = new CSyntaxHighlighterQRegExp (textEdit->document(), this, fname);
  else
     highlighter = new CSyntaxHighlighterQRegularExpression (textEdit->document(), this, fname);
  
#else
  
  highlighter = new CSyntaxHighlighterQRegExp (textEdit->document(), this, fname);
  
#endif  

  qDebug() << "highlighter->wrap: " << highlighter->wrap;

  
  if (textEdit->use_hl_wrap)
     {
      if (highlighter->wrap)
         textEdit->setLineWrapMode (QPlainTextEdit::WidgetWidth);
       else
          textEdit->setLineWrapMode (QPlainTextEdit::NoWrap);
     }
  
  qDebug() << "textEdit->use_hl_wrap: " << textEdit->use_hl_wrap;
}


void CDocument::goto_pos (int pos)
{
  QTextCursor cr = textEdit->textCursor();
  cr.setPosition (pos);
  textEdit->setTextCursor (cr);
}


void document_holder::apply_settings_single (CDocument *d)
{
  //QFont f;

  int darker_val = settings->value ("darker_val", 100).toInt();

 


/*  
  f.fromString (settings->value ("editor_font_name", "Monospace").toString());
  f.setPointSize (settings->value ("editor_font_size", "12").toInt());
  d->textEdit->setFont (f);
  d->textEdit->lineNumberArea->setFont (f);
*/

  d->textEdit->setCursorWidth (settings->value ("cursor_width", 2).toInt());
 
  d->textEdit->setCenterOnScroll (settings->value ("center_on_scroll", true).toBool());
  
  d->textEdit->use_hl_wrap = settings->value ("use_hl_wrap", true).toBool();
  
  d->textEdit->wrap = settings->value ("word_wrap", true).toBool();
    
     
  QString s_sel_back_color = hash_get_val (global_palette, "sel-background", "black");
  QString s_sel_text_color = hash_get_val (global_palette, "sel-text", "white");

    
  d->textEdit->sel_text_color = QColor (s_sel_text_color).darker(darker_val).name(); 
  d->textEdit->sel_back_color = QColor (s_sel_back_color).darker(darker_val).name(); 

    
  if (! d->textEdit->use_hl_wrap)
     {
      if (d->textEdit->wrap)
         d->textEdit->setLineWrapMode (QPlainTextEdit::WidgetWidth);
      else
          d->textEdit->setLineWrapMode (QPlainTextEdit::NoWrap);
     }
  
  if (settings->value ("right_to_left", false).toBool())
     d->textEdit->document()->setDefaultTextOption (QTextOption (Qt::AlignRight));
  else
     d->textEdit->document()->setDefaultTextOption (QTextOption (Qt::AlignLeft));
    
 
  d->textEdit->tab_sp_width = settings->value ("tab_sp_width", 8).toInt();
  d->textEdit->spaces_instead_of_tabs = settings->value ("spaces_instead_of_tabs", true).toBool();

  d->textEdit->setTabStopWidth (d->textEdit->tab_sp_width * d->textEdit->brace_width);
  
  d->textEdit->setup_brace_width();

  d->textEdit->set_show_linenums (settings->value ("show_linenums", false).toBool());
  d->textEdit->set_show_margin (settings->value ("show_margin", false).toBool());
  d->textEdit->set_margin_pos (settings->value ("margin_pos", 72).toInt());
  
  
  d->textEdit->highlightCurrentLine = settings->value ("additional_hl", false).toBool();
  d->textEdit->hl_brackets = settings->value ("hl_brackets", false).toBool();

  d->textEdit->currentLineColor = QColor (hash_get_val (global_palette, "cur_line_color", "#EEF6FF")).darker (darker_val);
  d->textEdit->brackets_color = QColor (hash_get_val (global_palette, "brackets", "yellow")).darker (darker_val);
  
  d->cursor_xy_visible = settings->value ("cursor_xy_visible", "2").toBool();
  d->textEdit->auto_indent = settings->value ("auto_indent", false).toBool();

  QString text_color = hash_get_val (global_palette, "text", "black");
  QString t_text_color = QColor (text_color).darker(darker_val).name(); 
    
  d->textEdit->text_color = QColor (t_text_color);

  QString back_color = hash_get_val (global_palette, "background", "white");
  //QString sel_back_color = hash_get_val (global_palette, "sel-background", "black");
  //QString sel_text_color = hash_get_val (global_palette, "sel-text", "white");

  d->textEdit->margin_color = QColor (hash_get_val (global_palette, "margin_color", text_color)).darker(darker_val);
  d->textEdit->linenums_bg = QColor (hash_get_val (global_palette, "linenums_bg", back_color)).darker(darker_val);
  
  //QString t_back_color = QColor (back_color).darker(darker_val).name(); 
  //QString t_sel_text_color = QColor (sel_text_color).darker(darker_val).name(); 
  //QString t_sel_back_color = QColor (sel_back_color).darker(darker_val).name(); 
    
  /*
  QString sheet = QString ("QPlainTextEdit { color: %1 ; background-color: %2 ; selection-color: %3; selection-background-color: %4;}").arg (
                           t_text_color).arg (
                           t_back_color).arg (
                           t_sel_text_color).arg (
                           t_sel_back_color);
                           
  d->textEdit->setStyleSheet (sheet);*/
  d->textEdit->repaint();
  d->set_hl();
}


void document_holder::apply_settings()
{
  foreach (CDocument *d, list)
          apply_settings_single (d);
}


void document_holder::add_to_recent (CDocument *d)
{
  if (b_recent_off)
     return;

  if (! file_exists (d->file_name))
     return;

  QString s (d->file_name);
  s += ",";
  s += d->charset;
  s += ",";
  s += QString ("%1").arg (d->textEdit->textCursor().position());

  recent_files.prepend (s);
//  if (recent_files.size() > settings->value ("recent_list.max_items", 21).toInt())
  if (recent_files.size() > recent_list_max_items)
     recent_files.removeLast();
}


void document_holder::update_recent_menu()
{
  recent_menu->clear();
  create_menu_from_list (this, recent_menu, recent_files, SLOT(open_recent()));
}


void document_holder::open_recent()
{
  QAction *act = qobject_cast<QAction *>(sender());

  int i = recent_files.indexOf (act->text());
  if (i == -1)
     return;

  open_file_triplex (recent_files[i]);
  update_recent_menu();
}


CDocument* document_holder::open_file_triplex (const QString &triplex)
{
  QStringList sl = triplex.split (",");
  if (sl.size() < 3)
     return NULL;

  CDocument *d = open_file (sl[0], sl[1]);
  if (d)
     d->goto_pos (sl[2].toInt());

  return d;
}


QString CDocument::get_triplex()
{
  QString s (file_name);
  s += ",";
  s += charset;
  s += ",";
  s += QString::number (textEdit->textCursor().position()); 
  return s;
}


void CDocument::set_markup_mode()
{
  markup_mode = holder->markup_mode;

  QString e = file_get_ext (file_name);

  if (e == "htm" || e == "html")
    markup_mode = "HTML";
  else
  if (e == "xhtml")
    markup_mode = "XHTML";
  else
  if (e == "xml")
    markup_mode = "Docbook";
  else
  if (e == "tex")
    markup_mode = "LaTeX";
  else
  if (e == "lout")
    markup_mode = "Lout";
  else
  if (e == "dokuwiki")
    markup_mode = "DokuWiki";
  else
  if (e == "mediawiki")
    markup_mode = "MediaWiki";
  else
  if (e == "md" || e == "markdown")
  markup_mode = "Markdown";  
}


void CTEAEdit::cb_cursorPositionChanged()
{
  viewport()->update();
  
  if (doc)
     doc->update_status();
     
  if (hl_brackets)   
     //braceHighlight();   
     update_ext_selections();
}


void CTEAEdit::setCurrentLineColor (const QColor &newColor)
{
  currentLineColor = newColor;
  emit repaint();
}


void CTEAEdit::set_brackets_color (const QColor &newColor)
{
  brackets_color = newColor;
  emit repaint();
}


void CTEAEdit::set_hl_cur_line (bool enable)
{
  highlightCurrentLine = enable;
  emit repaint();
}


void CTEAEdit::set_hl_brackets (bool enable)
{
  hl_brackets = enable;
  emit repaint();
}


void CTEAEdit::set_show_margin (bool enable)
{
  draw_margin = enable;
  emit repaint();
}


void CTEAEdit::set_show_linenums (bool enable)
{
  draw_linenums = enable;
  updateLineNumberAreaWidth (0);
  emit repaint();
}


void CTEAEdit::set_margin_pos (int mp)
{
  margin_pos = mp;
  margin_x = brace_width * margin_pos;
  emit repaint();
}


void CTEAEdit::paintEvent (QPaintEvent *event)
{
  if (draw_margin || highlightCurrentLine)
     {
      QPainter painter (viewport());

      if (highlightCurrentLine)
         {
          QRect r = cursorRect();
          r.setX (0);
          r.setWidth (viewport()->width());
          painter.fillRect (r, QBrush (currentLineColor));
         }

      if (draw_margin)
         {
          QPen pen = painter.pen();
          pen.setColor (margin_color);
          painter.setPen (pen);
          painter.drawLine (margin_x, 0, margin_x, viewport()->height());
         }
     }

  /*
      QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
*/
  QPlainTextEdit::paintEvent (event);

}


void CTEAEdit::setup_brace_width()
{
  QFontMetrics *fm = new QFontMetrics (font());
  brace_width = fm->averageCharWidth();
}


CTEAEdit::CTEAEdit (QWidget *parent): QPlainTextEdit (parent)
{
  rect_sel_reset();
  
  highlightCurrentLine = false;
  setup_brace_width();
  
  lineNumberArea = new CLineNumberArea (this);

  connect(this, SIGNAL(selectionChanged()), this, SLOT(slot_selectionChanged()));
  
  connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
  connect(this, SIGNAL(updateRequest(const QRect &, int)), this, SLOT(updateLineNumberArea(const QRect &, int)));
  updateLineNumberAreaWidth (0);

  margin_pos = 72;
  margin_x = brace_width * margin_pos;
  draw_margin = false;
  hl_brackets = false;
  auto_indent = false;
  use_hl_wrap = false;
  wrap = true;
  tab_sp_width = 8;
  spaces_instead_of_tabs = true;
  
  //document()->setDefaultTextOption(QTextOption(Qt::AlignRight));
 // setCursorWidth (settings->value ("cursor_width", 1).toInt());
  
  
  QString s_sel_back_color = hash_get_val (global_palette, "sel-background", "black");
  QString s_sel_text_color = hash_get_val (global_palette, "sel-text", "white");

  int darker_val = settings->value ("darker_val", 100).toInt();

  sel_text_color = QColor (s_sel_text_color).darker(darker_val).name(); 
  sel_back_color = QColor (s_sel_back_color).darker(darker_val).name(); 
  
  
  connect (this, SIGNAL(cursorPositionChanged()), this, SLOT(cb_cursorPositionChanged()));
}


QStringList CDocument::get_words()
{
  QStringList result;

  QTextCursor cr = textEdit->textCursor();

  QString text = textEdit->toPlainText();

  cr.setPosition (0);
  cr.movePosition (QTextCursor::Start, QTextCursor::MoveAnchor);

  do
    {
     QChar c = text[cr.position()];
     if (char_is_shit (c))
         while (char_is_shit (c))
               {
                cr.movePosition (QTextCursor::NextCharacter);
                c = text[cr.position()];
               }

     cr.movePosition (QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
     c = text[cr.position()];

     QString stext = cr.selectedText();

     if (! stext.isEmpty() && stext.endsWith ("\""))
        {
         cr.movePosition (QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
         stext = cr.selectedText();
        }  
           
     if (! stext.isEmpty())
        result.append (stext);     
    }
  while (cr.movePosition (QTextCursor::NextWord));

  return result;
}


QTextCharFormat tformat_from_style (const QString &fontstyle, const QString &color, int darker_val)
{
  QTextCharFormat tformat;
  tformat.setForeground (QBrush (QColor (color).darker (darker_val)));

  if (fontstyle.isEmpty())
     return tformat; 

  if (fontstyle.contains ("bold"))
      tformat.setFontWeight (QFont::Bold);

  if (fontstyle.contains ("italic"))
      tformat.setFontItalic (true);

  if (fontstyle.contains ("underline"))
      tformat.setFontUnderline (true);

  if (fontstyle.contains ("strikeout"))
      tformat.setFontStrikeOut (true);
  
  return tformat;
}


#if QT_VERSION >= 0x050000

void CSyntaxHighlighterQRegularExpression::load_from_xml (const QString &fname)
{
 qDebug() << "CSyntaxHighlighterQRegularExpression::load_from_xml ";

  wrap = true;
  casecare = true;

  exts = "default";
  langs = "default";
  
  int darker_val = settings->value ("darker_val", 100).toInt();
  
  pattern_opts = 0;
  
  QString temp = qstring_load (fname);
  QXmlStreamReader xml (temp);

  
   while (! xml.atEnd())
        {
         xml.readNext();

         QString tag_name = xml.name().toString().toLower();


         if (xml.isStartElement())
            if (tag_name == "document")
                {
                 exts = xml.attributes().value ("exts").toString();
                 langs = xml.attributes().value ("langs").toString();
                }

         if (xml.isStartElement())
            {

             if (tag_name == "item")
                {
                 QString attr_type = xml.attributes().value ("type").toString();
                 QString attr_name = xml.attributes().value ("name").toString();

                 if (attr_name == "options")
                    {
                     QString s_xml_format = xml.attributes().value ("xml_format").toString();
                     if (! s_xml_format.isEmpty())
                        xml_format = s_xml_format.toInt();
                    
                     QString s_wrap = xml.attributes().value ("wrap").toString();
                     if (! s_wrap.isEmpty())
                        if (s_wrap == "0" || s_wrap == "false")
                           wrap = false;
                        
                     QString s_casecare = xml.attributes().value ("casecare").toString();
                     if (! s_casecare.isEmpty())
                        if (s_casecare == "0" || s_casecare == "false")
                           casecare = false;
                        
                     if (! casecare) 
                       pattern_opts = pattern_opts | QRegularExpression::CaseInsensitiveOption;                    
                     }

                 if (attr_type == "keywords")
                    {
                     QString color = hash_get_val (global_palette, xml.attributes().value ("color").toString(), "darkBlue");

                     QTextCharFormat fmt = tformat_from_style (xml.attributes().value ("fontstyle").toString(), color, darker_val);

                     if (xml_format == 0)
                        {
                         QStringList keywordPatterns = xml.readElementText().trimmed().split(";");

                         HighlightingRule rule;

                         for (int i = 0; i < keywordPatterns.size(); i++)
                              if (! keywordPatterns.at(i).isEmpty())
                                 {
                                  rule.pattern = QRegularExpression (keywordPatterns.at(i).trimmed(), pattern_opts);
                                  rule.format = fmt;
                                  highlightingRules.append (rule);
                              
                                  if (! rule.pattern.isValid())
                                     qDebug() << "! valid " << rule.pattern.pattern();
                             }
                         }       
                      else
                      if (xml_format == 1)
                         {
                          HighlightingRule rule;
                          rule.pattern = QRegularExpression (xml.readElementText().trimmed().remove('\n'), pattern_opts);
                          rule.format = fmt;
                          highlightingRules.append(rule);
                        
                          if (! rule.pattern.isValid())
                             qDebug() << "! valid " << rule.pattern.pattern();
                         }
                             
                     } //keywords
                 else
                    if (attr_type == "item")
                       {
                        QString color = hash_get_val (global_palette, xml.attributes().value ("color").toString(), "darkBlue");
                        QTextCharFormat fmt = tformat_from_style (xml.attributes().value ("fontstyle").toString(), color, darker_val);

                        HighlightingRule rule;
                        rule.pattern = QRegularExpression (xml.readElementText().trimmed(), pattern_opts);
                        rule.format = fmt;
                        highlightingRules.append(rule);
                        
                        if (! rule.pattern.isValid())
                           qDebug() << "! valid " << rule.pattern.pattern();

                       }
                    else
                        if (attr_type == "mcomment-start")
                           {
                            QString color = hash_get_val (global_palette, xml.attributes().value ("color").toString(), "gray");
                            QTextCharFormat fmt = tformat_from_style (xml.attributes().value ("color").toString(), color, darker_val);

                            multiLineCommentFormat = fmt;
                            commentStartExpression = QRegularExpression (xml.readElementText().trimmed(), pattern_opts);
                           }
                    else
                        if (attr_type == "mcomment-end")
                           {
                            commentEndExpression = QRegularExpression (xml.readElementText().trimmed(), pattern_opts);
                           }
                    else
                        if (attr_type == "comment")
                           {
                            if (xml.attributes().value ("name").toString() == "cm_mult")
                               cm_mult = xml.readElementText().trimmed();
                            else
                            if (xml.attributes().value ("name").toString() == "cm_single")
                               cm_single = xml.readElementText().trimmed();
                          }

                     }//item

       }//is start

  if (xml.hasError())
     qDebug() << "xml parse error";

  } //cycle
}


void CSyntaxHighlighterQRegularExpression::highlightBlock (const QString &text)
{
  foreach (HighlightingRule rule, highlightingRules)
          {
           QRegularExpressionMatch m = rule.pattern.match (text);
           
           int index = m.capturedStart();
  
           while (index >= 0)
                 {
                  int length = m.capturedLength();
                  setFormat (index, length, rule.format);
                  m = rule.pattern.match (text, index + length);
                  index = m.capturedStart();
                 }
          }

  setCurrentBlockState (0);

  int startIndex = 0;

  if (! commentStartExpression.isValid() || ! commentEndExpression.isValid())
     return;

  QRegularExpressionMatch m_start = commentStartExpression.match (text);
  
  if (previousBlockState() != 1)
      startIndex = m_start.capturedStart();

  while (startIndex >= 0)
        {
         QRegularExpressionMatch m_end = commentEndExpression.match (text, startIndex);
         
         int endIndex = m_end.capturedStart();
         
         int commentLength;

         if (endIndex == -1)
            {
             setCurrentBlockState (1);
             commentLength = text.length() - startIndex;
            }
         else
             commentLength = endIndex - startIndex + m_end.capturedLength();

         setFormat (startIndex, commentLength, multiLineCommentFormat);

         m_start = commentStartExpression.match (text, startIndex + commentLength);
         startIndex = m_start.capturedStart(); 
        }

//   qDebug() << "2";
}

#endif

void CSyntaxHighlighterQRegExp::load_from_xml (const QString &fname)
{
  //qDebug() << "CSyntaxHighlighter::load_from_xml - start";

  int darker_val = settings->value ("darker_val", 100).toInt();
 
  exts = "default";
  langs = "default";

  cs = Qt::CaseSensitive;
  
  QString temp = qstring_load (fname);
  QXmlStreamReader xml (temp);

  while (! xml.atEnd())
        {
         xml.readNext();

         QString tag_name = xml.name().toString().toLower();


         if (xml.isStartElement())
            if (tag_name == "document")
                {
                 exts = xml.attributes().value ("exts").toString();
                 langs = xml.attributes().value ("langs").toString();
                }

         if (xml.isStartElement())
            {

             if (tag_name == "item")
                {
                 QString attr_type = xml.attributes().value ("type").toString();
                 QString attr_name = xml.attributes().value ("name").toString();

                 if (attr_name == "options")
                    {
                     QString s_wrap = xml.attributes().value ("wrap").toString();
                     if (! s_wrap.isEmpty())
                        if (s_wrap == "0" || s_wrap == "false")
                           wrap = false;

                     QString s_casecare = xml.attributes().value ("casecare").toString();
                     if (! s_casecare.isEmpty())
                        if (s_casecare == "0" || s_casecare == "false")
                           casecare = false;
                        
                     if (! casecare) 
                        cs = Qt::CaseInsensitive;
                     }

                 if (attr_type == "keywords")
                    {
                     QString color = hash_get_val (global_palette, xml.attributes().value ("color").toString(), "darkBlue");
                     QTextCharFormat fmt = tformat_from_style (xml.attributes().value ("fontstyle").toString(), color, darker_val);

                     if (xml_format == 0)
                        {
                         QStringList keywordPatterns = xml.readElementText().trimmed().split(";");

                         HighlightingRule rule;

                         for (int i = 0; i < keywordPatterns.size(); i++)
                             if (! keywordPatterns.at(i).isEmpty())
                                {
                                 rule.pattern = QRegExp (keywordPatterns.at(i).trimmed(), cs, QRegExp::RegExp);
                                 rule.format = fmt;
                                 highlightingRules.append (rule);
                                }
                          }
                     else
                         if (xml_format == 1)
                            {
                             HighlightingRule rule;
                             rule.pattern = QRegExp (xml.readElementText().trimmed().remove('\n'), cs);
                             rule.format = fmt;
                             highlightingRules.append(rule);
                        
                             if (! rule.pattern.isValid())
                                qDebug() << "! valid " << rule.pattern.pattern();
                            }
                            
                     } //keywords
                 else
                 if (attr_type == "item")
                    {
                     QString color = hash_get_val (global_palette, xml.attributes().value ("color").toString(), "darkBlue");
                     QTextCharFormat fmt = tformat_from_style (xml.attributes().value ("fontstyle").toString(), color, darker_val);

                     HighlightingRule rule;
                     rule.pattern = QRegExp (xml.readElementText().trimmed(), cs, QRegExp::RegExp);
                     rule.format = fmt;
                     highlightingRules.append(rule);
                    }
                 else
                 if (attr_type == "mcomment-start")
                    {
                     QString color = hash_get_val (global_palette, xml.attributes().value ("color").toString(), "gray");
                     QTextCharFormat fmt = tformat_from_style (xml.attributes().value ("color").toString(), color, darker_val);

                     multiLineCommentFormat = fmt;
                     commentStartExpression = QRegExp (xml.readElementText().trimmed(), cs, QRegExp::RegExp);
                    }
                 else
                 if (attr_type == "mcomment-end")
                    {
                     commentEndExpression = QRegExp (xml.readElementText().trimmed(), cs, QRegExp::RegExp);
                    }
                 else
                 if (attr_type == "comment")
                    {
                     if (xml.attributes().value ("name").toString() == "cm_mult")
                         cm_mult = xml.readElementText().trimmed();
                     else
                         if (xml.attributes().value ("name").toString() == "cm_single")
                            cm_single = xml.readElementText().trimmed();
                    }
                }//item

       }//is start

  if (xml.hasError())
     qDebug() << "xml parse error";

  } //cycle
}


void CSyntaxHighlighterQRegExp::highlightBlock (const QString &text)
{
 
  foreach (HighlightingRule rule, highlightingRules)
          {
           int index;

           index = text.indexOf (rule.pattern);

           while (index >= 0)
                 {
                  int length = rule.pattern.matchedLength();
                  setFormat (index, length, rule.format);
                  index = text.indexOf (rule.pattern, index + length);
                 }
           }

  setCurrentBlockState (0);

  int startIndex = 0;

  if (commentStartExpression.isEmpty() || commentEndExpression.isEmpty())
     return;

  if (previousBlockState() != 1)
     startIndex = text.indexOf (commentStartExpression);

  while (startIndex >= 0)
        {
         int endIndex = text.indexOf (commentEndExpression, startIndex);

         int commentLength;
         if (endIndex == -1)
            {
             setCurrentBlockState (1);
             commentLength = text.length() - startIndex;
            }
         else
             commentLength = endIndex - startIndex + commentEndExpression.matchedLength();

         setFormat (startIndex, commentLength, multiLineCommentFormat);
         startIndex = text.indexOf (commentStartExpression, startIndex + commentLength);
        }
}



void document_holder::save_to_session (const QString &fileName)
{
  if (list.size() < 0)
     return;

  fname_current_session = fileName;
  QString l;

  foreach (CDocument *d, list)
          {
           l += d->get_triplex();
           l += "\n";
          }
  
  qstring_save (fileName, l.trimmed());
}


void document_holder::load_from_session (const QString &fileName)
{
  if (! file_exists (fileName))
     return;

  QStringList l = qstring_load (fileName).split ("\n");
  int c = l.size();
  if (c < 0)
     return;

  foreach (QString t, l)  
          open_file_triplex (t);

  fname_current_session = fileName;
}

/*
QHash <QString, QString> document_holder::load_eclipse_theme_xml (const QString &fname)
{
  QHash <QString, QString> result;
 
  QString temp = qstring_load (fname);
  QXmlStreamReader xml (temp);

   while (! xml.atEnd())
        {
         xml.readNext();

         QString tag_name = xml.name().toString();
         
         if (xml.isStartElement())
            {

            if (tag_name == "colorTheme")
             {
                  
                 log->log (xml.attributes().value ("id").toString());
                 log->log (xml.attributes().value ("name").toString());
                 log->log (xml.attributes().value ("modified").toString());
                 log->log (xml.attributes().value ("author").toString());
                 log->log (xml.attributes().value ("website").toString());
                }

         
            if (tag_name == "singleLineComment")
               {
                QString t = xml.attributes().value ("color").toString();
                if (! t.isEmpty())
                     result.insert ("single comment", t);
               }
         

            if (tag_name == "class")
               {
                QString t = xml.attributes().value ("color").toString();
                if (! t.isEmpty())
                   {
                    result.insert ("class", t);
                    result.insert ("type", t);
                   }
               }

         
         if (tag_name == "operator")
            {
             QString t = xml.attributes().value ("color").toString();
             if (! t.isEmpty())
                     result.insert ("operator", t);
            }

         
         
         if (tag_name == "string")
            {
             QString t = xml.attributes().value ("color").toString();
             if (! t.isEmpty())
                 result.insert ("quotes", t);
            }

         
         
         if (tag_name == "multiLineComment")
            {
             QString t = xml.attributes().value ("color").toString();
             if (! t.isEmpty())
                 result.insert ("mcomment-start", t);
            }

         
         
         if (tag_name == "foreground")
            {
             QString t = xml.attributes().value ("color").toString();
             if (! t.isEmpty())
                 {
                  result.insert ("text", t);
                  result.insert ("functions", t);
                  result.insert ("modifiers", t);
                  result.insert ("margin_color", t);
                  result.insert ("digits", t);
                  result.insert ("digits-float", t);
                  result.insert ("label", t);
                  result.insert ("include", t);
                  result.insert ("preproc", t);
                 }
             }

         if (tag_name == "background")
            {
             QString t = xml.attributes().value ("color").toString();
             if (! t.isEmpty())
                { 
                 result.insert ("background", t);
                 result.insert ("linenums_bg", t);
                }
            }
         
         
         if (tag_name == "selectionForeground")
            {
             QString t = xml.attributes().value ("color").toString();
             if (! t.isEmpty())
                result.insert ("sel-text", t);
            }

         
         
         if (tag_name == "selectionBackground")
            {
             QString t = xml.attributes().value ("color").toString();
             if (! t.isEmpty())
               result.insert ("sel-background", t);
            }

         
         if (tag_name == "keyword")
            {
             QString t = xml.attributes().value ("color").toString();
             if (! t.isEmpty())
                {
                 result.insert ("keywords", t);
                result.insert ("tags", t);
                }
             }

        	 
        	 
       	 if (tag_name == "currentLine")
            {
             QString t = xml.attributes().value ("color").toString();
             if (! t.isEmpty())
                result.insert ("cur_line_color", t);
            }

        	 
         if (tag_name == "bracket")
            {
             QString t = xml.attributes().value ("color").toString();
             if (! t.isEmpty())
                 result.insert ("brackets", t);
             }
             
        }//is start

  if (xml.hasError())
     qDebug() << "xml parse error";

  } //cycle
  

   result.insert ("error", "red");
   
   return result; 
}
*/
/*
void document_holder::load_global_palette (const QString &fileName)
{
  if (! file_exists (fileName))
      return;

  global_palette.clear();
  
  if (file_get_ext (fileName) == "xml")
     global_palette = load_eclipse_theme_xml (fileName);
  else
      global_palette = hash_load_keyval (fileName);
}
*/

QString CDocument::get_selected_text() const
{
  return textEdit->textCursor().selectedText();
}


void CDocument::set_selected_text (const QString &value)
{
  textEdit->textCursor().insertText (value);
}


void CDocument::insert_image (const QString &full_path)
{
  textEdit->textCursor().insertText (get_insert_image (file_name, full_path, markup_mode));
}


void CTEAEdit::calc_auto_indent()
{
  QTextCursor cur = textCursor();
  cur.movePosition (QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
  int aindent = 0;
  
  QString s = cur.selectedText();
  int len = s.size();
  
  QChar t = ' '; //what to detect - space or tab?
  if (s.indexOf ('\t') != -1)
     t = '\t';
  
  if (t != '\t')
     {
      for (int i = 0; i < len; i++)
          if (! s.at(i).isSpace())
            {
             aindent = i; 
             break;
            }
     }
  else   
     {
      for (int i = 0; i < len; i++)
          if (s.at(i) != '\t')
            {
             aindent = i; 
             break;
            }
     }
           

  if (aindent != 0)
     //indent_val = indent_val.fill (' ', aindent);
    indent_val = indent_val.fill (t, aindent);
  else 
      indent_val.clear();
}


void CTEAEdit::indent()
{
  if (! textCursor().hasSelection())
     {
      QString fl;
      fl = fl.fill (' ', tab_sp_width);
         
      if (spaces_instead_of_tabs)
         textCursor().insertText (fl);
      else
          textCursor().insertText ("\t");
       
      return;
     }

  QStringList l = textCursor().selectedText().split (QChar::ParagraphSeparator);
       
  QString fl;
  fl = fl.fill (' ', tab_sp_width);
        
  QMutableListIterator <QString> i (l);

  while (i.hasNext())
        {
         QString s = i.next();
         if (spaces_instead_of_tabs)
            i.setValue (s.prepend (fl));
         else
             i.setValue (s.prepend ("\t"));
         }       
       
  textCursor().insertText (l.join ("\n"));
  
  QTextCursor cur = textCursor();
  cur.movePosition (QTextCursor::Up, QTextCursor::KeepAnchor, l.size() - 1);   
  cur.movePosition (QTextCursor::StartOfLine, QTextCursor::KeepAnchor);   
  
  setTextCursor (cur);
}


void CTEAEdit::un_indent()
{
  if (! textCursor().hasSelection())
     {
      QString t = textCursor().selectedText();
      if (! t.isEmpty() && (t.size() > 1) && t[0].isSpace()) 
         t = t.mid (1);

      return;
     }
   
  QStringList l = textCursor().selectedText().split (QChar::ParagraphSeparator);
   
  QMutableListIterator <QString> i (l);

  while (i.hasNext())
        {
         QString t = i.next();
         
         if (! t.isEmpty() && (t.size() > 1)) 
             if (t[0] == '\t' || t[0].isSpace())
                {
                 t = t.mid (1);
                 i.setValue (t);
                }              
         }       
   
  textCursor().insertText (l.join ("\n"));

  QTextCursor cur = textCursor();
  cur.movePosition (QTextCursor::Up, QTextCursor::KeepAnchor, l.size() - 1);   
  cur.movePosition (QTextCursor::StartOfLine, QTextCursor::KeepAnchor);   

  setTextCursor (cur);
}


#define SK_A 38
#define SK_D 40
#define SK_W 25
#define SK_S 39

#define SK_E 26
#define SK_C 54


void CTEAEdit::keyPressEvent (QKeyEvent *event)
{
 // qDebug() << event->nativeScanCode() ; 

 // qDebug() << int_to_binary (event->nativeModifiers());
  // std::bitset<32> btst (event->nativeModifiers());
   
   /*
   for (std::size_t i = 0; i < btst.size(); ++i) {
        qDebug() << "btst[" << i << "]: " << btst[i];
    }
   */
   //LSHIFT = 0
//LCTRL = 2
//LALT = 3
  //LWIN = 6
  if (settings->value ("wasd", "0").toInt())
     {
      std::bitset<32> btst (event->nativeModifiers());
      QTextCursor cr = textCursor();

      QTextCursor::MoveMode m = QTextCursor::MoveAnchor;

// if (btst[0] == 1)
         //    m = QTextCursor::KeepAnchor;
       
      if (btst[3] == 1 || btst[6] == 1) //LALT or LWIN
         {
          int visible_lines;

          if (btst[6] == 1)
             m = QTextCursor::KeepAnchor;

          switch (event->nativeScanCode())
                 {
                  case SK_W:
                                 cr.movePosition (QTextCursor::Up, m);
                                 setTextCursor (cr);
                                 return;

                  case SK_S:
                                 cr.movePosition (QTextCursor::Down, m);
                                 setTextCursor (cr);
                                 return;

                  case SK_A:
                                 cr.movePosition (QTextCursor::Left, m);
                                 setTextCursor (cr);
                                 return;

                  case SK_D:
                                 cr.movePosition (QTextCursor::Right, m);
                                 setTextCursor (cr);
                                 return;
                                 
                   case SK_E:
                                  visible_lines = height() / fontMetrics().height();
                                  cr.movePosition (QTextCursor::Down, m, visible_lines);
                                  setTextCursor (cr);
                                  return;               
                                  
                   case SK_C:
                                  visible_lines = height() / fontMetrics().height();
                                  cr.movePosition (QTextCursor::Up, m, visible_lines);
                                  setTextCursor (cr);
                                  return;               
                                     
                 }
      
       }
    } 

  if (auto_indent)
     if (event->key() == Qt::Key_Return)
        {
         calc_auto_indent();
         QPlainTextEdit::keyPressEvent (event);
         textCursor().insertText (indent_val);
         return;
        }
   
  if (event->key() == Qt::Key_Tab)
     {
      indent();
      event->accept();
      return;
     }
  else
  if (event->key() == Qt::Key_Backtab)
     {
      un_indent();
      event->accept();
      return;
     }
  else
  if (event->key() == Qt::Key_Insert)
     {
      setOverwriteMode (! overwriteMode());
      event->accept();
      return;
     }   
  
  QPlainTextEdit::keyPressEvent (event);
}


int CTEAEdit::lineNumberAreaWidth()
{
  if (! draw_linenums)
     return 0; 

  int digits = 1;
  int max = qMax (1, blockCount());
  
  while (max >= 10) 
        {
         max /= 10;
         ++digits;
        }

  int space = (brace_width * 2) + (brace_width * digits);

  return space;
}


void CTEAEdit::updateLineNumberAreaWidth (int newBlockCount)
{
  setViewportMargins (lineNumberAreaWidth(), 0, 0, 0);
}


void CTEAEdit::updateLineNumberArea (const QRect &rect, int dy)
{
  if (dy)
     lineNumberArea->scroll (0, dy);
  else
     lineNumberArea->update (0, rect.y(), lineNumberArea->width(), rect.height());

  if (rect.contains (viewport()->rect()))
     updateLineNumberAreaWidth (0);
}


void CTEAEdit::lineNumberAreaPaintEvent (QPaintEvent *event)
{
  if (! draw_linenums)
     return;

  QPainter painter (lineNumberArea);
  painter.fillRect (event->rect(), linenums_bg);
  painter.setPen (text_color);

  QTextBlock block = firstVisibleBlock();
  int blockNumber = block.blockNumber();
  int top = (int) blockBoundingGeometry (block).translated (contentOffset()).top();
  int bottom = top + (int) blockBoundingRect (block).height();

  int w = lineNumberArea->width();
  int h = fontMetrics().height();

  while (block.isValid() && top <= event->rect().bottom()) 
        {
         if (block.isVisible() && bottom >= event->rect().top()) 
            {
             QString number = QString::number (blockNumber + 1) + " ";
//             painter.setPen (text_color);
             painter.drawText (0, top, w, h, Qt::AlignRight, number);
             /*int index = bookMark.indexOf(number.toInt());
 
             if (index != -1) 
                {
                //painter.drawText(0, top, 30, fontMetrics().height(), Qt::AlignCenter, "+");
                 painter.setBrush(QBrush(Qt::yellow, Qt::SolidPattern));
                 painter.drawEllipse(5, top + (fontMetrics().height()/4), fontMetrics().height()/2, fontMetrics().height()/2);
                }*/
              }

         block = block.next();
         top = bottom;
         bottom = top + (int) blockBoundingRect(block).height();
         ++blockNumber;
        }
}


void CTEAEdit::resizeEvent (QResizeEvent *e)
{
  QPlainTextEdit::resizeEvent (e);
  QRect cr = contentsRect();
  lineNumberArea->setGeometry (QRect (cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}


void CTEAEdit::braceHighlight()
{
  //extraSelections.clear();
  //setExtraSelections (extraSelections);
  
  brace_selection.format.setBackground (brackets_color);

  QTextDocument *doc = document();
  QTextCursor cursor = textCursor();
  QTextCursor beforeCursor = cursor;

  cursor.movePosition (QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
  QString brace = cursor.selectedText();

  beforeCursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
  QString beforeBrace = beforeCursor.selectedText();

  if ((brace != "{") && 
      (brace != "}") &&
      (brace != "[") &&
      (brace != "]") && 
      (brace != "(") && 
      (brace != ")") && 
      (brace != "<") && 
      (brace != ">")) 
      {
       if ((beforeBrace == "{") 
           || (beforeBrace == "}") 
           || (beforeBrace == "[")
           || (beforeBrace == "]")
           || (beforeBrace == "(")
           || (beforeBrace == ")")
           || (beforeBrace == "<")
           || (beforeBrace == ">")) 
          {
           cursor = beforeCursor;
           brace = cursor.selectedText();
          }
        else 
            return;
       }

  QString openBrace;
  QString closeBrace;

  if ((brace == "{") || (brace == "}")) 
     {
      openBrace = "{";
      closeBrace = "}";
     }
  else
  if ((brace == "[") || (brace == "]")) 
     {
      openBrace = "[";
      closeBrace = "]";
     }
  else
  if ((brace == "(") || (brace == ")")) 
     {
      openBrace = "(";
      closeBrace = ")";
     }
  else
  if ((brace == "<") || (brace == ">")) 
     {
      openBrace = "<";
      closeBrace = ">";
     }


  if (brace == openBrace) 
     {
      QTextCursor cursor1 = doc->find (closeBrace, cursor);
      QTextCursor cursor2 = doc->find (openBrace, cursor);
      if (cursor2.isNull()) 
         {
          brace_selection.cursor = cursor;
          extraSelections.append (brace_selection);
          brace_selection.cursor = cursor1;
          extraSelections.append (brace_selection);
          setExtraSelections (extraSelections);
         } 
      else 
          {
           while (cursor1.position() > cursor2.position()) 
                 {
                  cursor1 = doc->find (closeBrace, cursor1);
                  cursor2 = doc->find (openBrace, cursor2);
                  if (cursor2.isNull()) 
                      break;
                 }
                
           brace_selection.cursor = cursor;
           extraSelections.append (brace_selection);
           brace_selection.cursor = cursor1;
           extraSelections.append (brace_selection);
           setExtraSelections (extraSelections);
          }
       } 
   else
       {
        if (brace == closeBrace) 
           {
            QTextCursor cursor1 = doc->find (openBrace, cursor, QTextDocument::FindBackward);
            QTextCursor cursor2 = doc->find (closeBrace, cursor, QTextDocument::FindBackward);
            if (cursor2.isNull()) 
               {
                brace_selection.cursor = cursor;
                extraSelections.append (brace_selection);
                brace_selection.cursor = cursor1;
                extraSelections.append (brace_selection);
                setExtraSelections (extraSelections);
               }
            else 
                {
                 while (cursor1.position() < cursor2.position()) 
                       {
                        cursor1 = doc->find (openBrace, cursor1, QTextDocument::FindBackward);
                        cursor2 = doc->find (closeBrace, cursor2, QTextDocument::FindBackward);
                        if (cursor2.isNull()) 
                           break;
                       }
                       
                 brace_selection.cursor = cursor;
                 extraSelections.append (brace_selection);
                 brace_selection.cursor = cursor1;
                 extraSelections.append (brace_selection);
                 setExtraSelections (extraSelections);
                }
             }
     }
}


bool CTEAEdit::has_rect_selection()
{
 if (rect_sel_start.y() == -1 || rect_sel_end.y() == -1)
     return false; 
     
 return true;    
}
 

void CTEAEdit::update_rect_sel()
{
 // qDebug() << "CTEAEdit::update_rect_sel()  -1";
 
  if (rect_sel_start.y() == -1 || rect_sel_end.y() == -1)
     return; 
 
  QTextEdit::ExtraSelection rect_selection;

  //extraSelections.clear();
    
  int y1 = std::min (rect_sel_start.y(), rect_sel_end.y());
  int y2 = std::max (rect_sel_start.y(), rect_sel_end.y());
  //int ydiff = y2 - y1;
   
  
  int x1 = std::min (rect_sel_start.x(), rect_sel_end.x());
  int x2 = std::max (rect_sel_start.x(), rect_sel_end.x());
  int xdiff = x2 - x1;
 
  int correction = 0;

  QTextCursor cursor = textCursor();
  
  cursor.movePosition (QTextCursor::Start, QTextCursor::MoveAnchor);
  cursor.movePosition (QTextCursor::NextBlock, QTextCursor::MoveAnchor, y1);
  
  for (int y = y1; y <= y2; y++)
      {
//       qDebug() << "y:" << y;
       
       QTextBlock b = document()->findBlockByNumber (y); 
       
   //    qDebug() << "b.text().length(): " << b.text().length();
     //  qDebug() << "x1: " << x1;
       
       if (b.text().length() == 0)
          {
           //cursor.movePosition (QTextCursor::NextBlock, QTextCursor::MoveAnchor);
           correction++;
           continue;
          } 
       
       int sel_len = xdiff;
       
       if ((b.text().length() - x1) < xdiff)
          sel_len = b.text().length() - x1;
   
       cursor.movePosition (QTextCursor::Right, QTextCursor::MoveAnchor, x1 + correction);

       cursor.movePosition (QTextCursor::Right, QTextCursor::KeepAnchor, sel_len);
              
       rect_selection.cursor = cursor;
      // rect_selection.format.setBackground (brackets_color);
      rect_selection.format.setBackground (sel_back_color);
      rect_selection.format.setForeground (sel_text_color);
      
      
      extraSelections.append (rect_selection);
  
      cursor.movePosition (QTextCursor::NextBlock, QTextCursor::MoveAnchor);
       
      if (b.text().length() != 0)
          correction = 0;
       
      }

  setExtraSelections (extraSelections);
 /*
  qDebug() << "ydiff: " << ydiff;
  qDebug() << "y1:" << y1;
  qDebug() << "y2:" << y2;
  qDebug() << "x1:" << x1;
  qDebug() << "x2:" << x2;
  */
 // qDebug() << get_rect_sel();
  
}


  
void CTEAEdit::rect_sel_replace (const QString &s, bool insert)
{
/*

1. Определить с какой строки начинаем вставку
2. Определить как вставляемый текст перекрывает старый. Если строк в старом меньше (или конец файла), 
добавляем в выходной буфер строки.
3. Составляем выходной буфер из строк старого и нового.
4. Заменяем старый текст на выходной буфер.

*/

 // int y = textCursor().block().blockNumber();
 // int x = textCursor().position() - textCursor().block().position();
 
  int y1 = std::min (rect_sel_start.y(), rect_sel_end.y());
  int y2 = std::max (rect_sel_start.y(), rect_sel_end.y());
  int ydiff = y2 - y1;
   
  
  int x1 = std::min (rect_sel_start.x(), rect_sel_end.x());
  int x2 = std::max (rect_sel_start.x(), rect_sel_end.x());
  int xdiff = x2 - x1;
  
  int how_many_copy_from_source = ydiff;
  
  int lines_to_end = blockCount() - y1;
  
  if (ydiff > lines_to_end)
     how_many_copy_from_source = lines_to_end;
     
  QStringList sl_source;
  
  for (int line = y1; line <= y2; line++)
      {
       QTextBlock b = document()->findBlockByNumber (line); 
       sl_source.append (b.text());   
      }  
  
  QStringList sl_insert = s.split ("\n");
  
  QStringList sl_dest;
  
  for (int line = 0; line < sl_insert.size(); line++)
      { 
       QString t;
       
       if (line >= sl_source.size())
          {
           t = sl_insert [line];
           sl_dest.append (t);
           continue;  
          }
  
       t = sl_source[line].left (x1);
       t += sl_insert [line];
       
       if (! insert)
          t += sl_source[line].mid (x2);
       else   
           t += sl_source[line].mid (x1);
  
       sl_dest.append (t);
      }

  QString new_text = sl_dest.join ("\n");
  //new_text += "\n";

//теперь выделить при помощи курсора всё от y1 до y2 и обычным способом заменить текст     
  
  QTextCursor cursor = textCursor();
  
  cursor.movePosition (QTextCursor::Start, QTextCursor::MoveAnchor);
  cursor.movePosition (QTextCursor::NextBlock, QTextCursor::MoveAnchor, y1);
  cursor.movePosition (QTextCursor::NextBlock, QTextCursor::KeepAnchor, ydiff);
  cursor.movePosition (QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
  
  cursor.removeSelectedText();
  
  textCursor().insertText (new_text);
  
//  qDebug() << new_text;
}
  
/*  
void CTEAEdit::rect_sel_insert (const QString &s)
{
 
  int y1 = std::min (rect_sel_start.y(), rect_sel_end.y());
  int y2 = std::max (rect_sel_start.y(), rect_sel_end.y());
  int ydiff = y2 - y1;
   
  
  int x1 = std::min (rect_sel_start.x(), rect_sel_end.x());
  int x2 = std::max (rect_sel_start.x(), rect_sel_end.x());
  int xdiff = x2 - x1;
  
  int how_many_copy_from_source = ydiff;
  
  int lines_to_end = blockCount() - y1;
  
  if (ydiff > lines_to_end)
     how_many_copy_from_source = lines_to_end;
     
  QStringList sl_source;
  
  for (int line = y1; line <= y2; line++)
      {
       QTextBlock b = document()->findBlockByNumber (line); 
       sl_source.append (b.text());   
      }  
  
  QStringList sl_insert = s.split ("\n");
  
  QStringList sl_dest;
  
  for (int line = 0; line < sl_insert.size(); line++)
      { 
       QString t;
       
       if (line >= sl_source.size())
          {
           t = sl_insert [line];
           sl_dest.append (t);
           continue;  
          }
  
       t = sl_source[line].left (x1);
       t += sl_insert [line];
       t += sl_source[line].mid (x1);
  
       sl_dest.append (t);
      }

  QString new_text = sl_dest.join ("\n");
  //new_text += "\n";

//теперь выделить при помощи курсора всё от y1 до y2 и обычным способом заменить текст     
  
  QTextCursor cursor = textCursor();
  
  cursor.movePosition (QTextCursor::Start, QTextCursor::MoveAnchor);
  cursor.movePosition (QTextCursor::NextBlock, QTextCursor::MoveAnchor, y1);
  cursor.movePosition (QTextCursor::NextBlock, QTextCursor::KeepAnchor, ydiff);
  cursor.movePosition (QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
  
  cursor.removeSelectedText();
  
  textCursor().insertText (new_text);
  
  qDebug() << new_text;
}
 */   

bool CTEAEdit::canInsertFromMimeData (const QMimeData *source)
{
//  if (source->hasFormat ("text/uri-list"))
  //    return true;
  //else
    return true;
}


QMimeData* CTEAEdit::createMimeDataFromSelection()
{
  qDebug() << "CTEAEdit::createMimeDataFromSelection()";
  if (has_rect_selection())
     {
      QMimeData *md = new QMimeData;
      md->setText (get_rect_sel());
      return md;
     }

  return QPlainTextEdit::createMimeDataFromSelection();   
} 


void CTEAEdit::insertFromMimeData (const QMimeData *source)
{
  QString fName;
  QFileInfo info;

  bool b_ins_plain_text = ! source->hasUrls();
  if (source->hasUrls() && source->urls().at(0).scheme() != "file")
      b_ins_plain_text = true;

  if (b_ins_plain_text)
    {
     QPlainTextEdit::insertFromMimeData (source);
     return;
    }

  foreach (QUrl u, source->urls())
         {
          fName = u.toLocalFile();
          info.setFile(fName);
          if (info.isFile())
             doc->holder->open_file (fName, "UTF-8");
         }
}


QStringList text_get_bookmarks (const QString &s, const QString &sstart, const QString &send)
{
  QStringList result;

  int c = s.size();
  int i = 0;

  while (i < c)
        {
         int start = s.indexOf (sstart, i, Qt::CaseInsensitive);

         if (start == -1)
             break;

         int end = s.indexOf (send, start + sstart.length());
         if (end == -1)
             break;

         result.prepend (s.mid (start + sstart.length(), (end - start) - send.length()));

         i = end + 1;
        }

  return result;
}


void CDocument::update_labels()
{
  labels.clear();
  labels = text_get_bookmarks (textEdit->toPlainText(),
                                        settings->value ("label_start", "[?").toString(),
                                        settings->value ("label_end", "?]").toString());
}

//////////////////////////////////////////
void document_holder::open_current()
{
  QAction *act = qobject_cast<QAction *>(sender());

  CDocument *d = get_document_by_fname (act->text());
  if (d)
      tab_widget->setCurrentIndex (tab_widget->indexOf (d->tab_page));
}


void document_holder::update_current_files_menu()
{
 QStringList current_files; 
 
  foreach (CDocument *d, list)
         current_files.prepend (d->file_name);
 
  current_files_menu->clear();
  create_menu_from_list (this, current_files_menu, current_files, SLOT(open_current()));
}


void CTEAEdit::rect_sel_reset()
{
  rect_sel_start.setX (-1);
  rect_sel_start.setY (-1);
  rect_sel_end.setX (-1);
  rect_sel_end.setY (-1);
}


QString CTEAEdit::get_rect_sel()
{
  QString result;

  int y1 = std::min (rect_sel_start.y(), rect_sel_end.y());
  int y2 = std::max (rect_sel_start.y(), rect_sel_end.y());
  int ydiff = y2 - y1;
  
  int x1 = std::min (rect_sel_start.x(), rect_sel_end.x());
  int x2 = std::max (rect_sel_start.x(), rect_sel_end.x());
  int xdiff = x2 - x1;
 

  for (int y = y1; y <= y2; y++)
      {
       //int sel_len = xdiff;
       
       QTextBlock b = document()->findBlockByNumber (y); 
       
       //if ((b.text().length() - x1) < xdiff)
         // sel_len = b.text().length() - x1;
          
       QString t = b.text();   

       result += t.mid (x1, xdiff);
       
       if (y != y2)
          result += '\n';   
      }

  return result;
}


void CTEAEdit::rect_sel_cut (bool just_del)
{
  int y1 = std::min (rect_sel_start.y(), rect_sel_end.y());
  int y2 = std::max (rect_sel_start.y(), rect_sel_end.y());
  int ydiff = y2 - y1;
   
  
  int x1 = std::min (rect_sel_start.x(), rect_sel_end.x());
  int x2 = std::max (rect_sel_start.x(), rect_sel_end.x());
  //int xdiff = x2 - x1;
  
  int how_many_copy_from_source = ydiff;
  
  int lines_to_end = blockCount() - y1;
  
  if (ydiff > lines_to_end)
     how_many_copy_from_source = lines_to_end;
     
  QStringList sl_source;
  
  for (int line = y1; line <= y2; line++)
      {
       QTextBlock b = document()->findBlockByNumber (line); 
       sl_source.append (b.text());   
      }  
  
  QStringList sl_dest;
  QStringList sl_copy;
  
  for (int line = 0; line < sl_source.size(); line++)
      { 
       QString t;

       t = sl_source[line].left (x1);
       t += sl_source[line].mid (x2);
       sl_dest.append (t);
       
       sl_copy.append (sl_source[line].mid (x1, x2 - x1));
      }

  QString new_text = sl_dest.join ("\n");

//теперь выделить при помощи курсора всё от y1 до y2 и обычным способом заменить текст     
  
  QTextCursor cursor = textCursor();
  
  cursor.movePosition (QTextCursor::Start, QTextCursor::MoveAnchor);
  cursor.movePosition (QTextCursor::NextBlock, QTextCursor::MoveAnchor, y1);
  cursor.movePosition (QTextCursor::NextBlock, QTextCursor::KeepAnchor, ydiff);
  cursor.movePosition (QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
  
  cursor.beginEditBlock();
  
  cursor.removeSelectedText();
 
  cursor.insertText (new_text);

  cursor.endEditBlock();
  
  if (! just_del)  
     QApplication::clipboard()->setText (sl_copy.join ("\n"));
}


void CTEAEdit::update_ext_selections()
{
  extraSelections.clear();
  setExtraSelections (extraSelections);
  update_rect_sel();
  braceHighlight();     
}
  

void CTEAEdit::slot_selectionChanged()
{
  QTextCursor cursor = this->textCursor();
  
  if (cursor.selectionStart() != cursor.selectionEnd())
     {
      rect_sel_start.setX (-1);
      rect_sel_end.setX (-1);
      update_ext_selections();
     } 
}


void CTEAEdit::text_replace (const QString &s)
{
  if (has_rect_selection())
     rect_sel_replace (s, false);
  else
      textCursor().insertText (s);
}



/*
void CTEAEdit::copy()
{
  qDebug() << "CTEAEdit::copy()";
  
  if (has_rect_selection())
     QApplication::clipboard()->setText (get_rect_sel());   
  else   
      QPlainTextEdit::copy();
}
*/

/*
 * 
 
 // new INDEX-IN VARIANT

void CSyntaxHighlighter::highlightBlock (const QString &text)
{
 qDebug() << "1";
 
  foreach (HighlightingRule rule, highlightingRules)
          {
           QRegExp expression (rule.pattern);

           int index;

           if (! casecare)
              index = expression.indexIn (text.toLower());
           else
               index = expression.indexIn (text);

           while (index >= 0)
                 {
                  int length = expression.matchedLength();
                  setFormat (index, length, rule.format);

                  if (! casecare)
                      index = expression.indexIn (text.toLower(), index + length);
                  else
                      index = expression.indexIn (text, index + length);
                 }
           }

  setCurrentBlockState (0);

  int startIndex = 0;

  if (commentStartExpression.isEmpty() || commentEndExpression.isEmpty())
     return;

  if (previousBlockState() != 1)
     {
      if (! casecare)
         startIndex = commentStartExpression.indexIn (text.toLower());
      else
         startIndex = commentStartExpression.indexIn (text);
     }

  while (startIndex >= 0)
        {
         int endIndex;
         if (! casecare)
            endIndex = commentEndExpression.indexIn (text.toLower(), startIndex);
         else
            endIndex = commentEndExpression.indexIn (text, startIndex);

         int commentLength;
         if (endIndex == -1)
            {
             setCurrentBlockState (1);
             commentLength = text.length() - startIndex;
            }
         else
             commentLength = endIndex - startIndex + commentEndExpression.matchedLength();

         setFormat (startIndex, commentLength, multiLineCommentFormat);

         if (! casecare)
            startIndex = commentStartExpression.indexIn (text.toLower(), startIndex + commentLength);
         else
            startIndex = commentStartExpression.indexIn (text, startIndex + commentLength);
        }
        
      
}

 
 
 * 
 */

