/***************************************************************************
 *   2000-2023 by Peter Semiletov                                          *
 *   peter.semiletov@gmail.com                                             *

C++/Qt branch started at 08 November 2007
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



#include "document.h"
#include <math.h>
#include <algorithm>
#include <iostream>
#include <stdlib.h>
#include <cstdlib>


#if QT_VERSION >= 0x050000
#include <QRegularExpression>
#else
#include <QRegExp>
#endif

#if QT_VERSION < 0x050500
#include <QRegExp>
#endif

#include <QElapsedTimer>
#include <QDockWidget>
#include <QFileSystemModel>
#include <QMimeData>
#include <QStyleFactory>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QToolBar>
#include <QClipboard>
#include <QFileDialog>
#include <QMenuBar>
#include <QGroupBox>
#include <QImageWriter>
#include <QColorDialog>
#include <QTextCodec>
#include <QMimeData>
#include <QScrollArea>
#include <QXmlStreamReader>
#include <QDebug>
#include <QPainter>
#include <QInputDialog>
#include <QSettings>
#include <QLibraryInfo>
#include <QFontDialog>
#include <QUrl>

#ifdef PRINTER_ENABLE
#include <QPrinter>
#include <QPrintDialog>
#include <QAbstractPrintDialog>
#endif

#include "tea.h"
#include "utils.h"
#include "gui_utils.h"
#include "libretta_calc.h"
#include "textproc.h"
#include "logmemo.h"
#include "tzipper.h"
#include "wavinfo.h"
#include "exif_reader.h"

#include "spellchecker.h"


//′
const int UQS = 8242;
//″
const int UQD = 8243;
//°
const int UQDG = 176;



bool MyProxyStyle::b_altmenu = false;
int MyProxyStyle::cursor_blink_time = 1;

extern QSettings *settings;
extern QMenu *menu_current_files;
extern QHash <QString, QString> global_palette;
extern bool b_recent_off;
extern bool b_destroying_all;
extern int recent_list_max_items;
extern bool boring;


CTEA *main_window;
CDox *documents;
//QVariantMap hs_path;

QString current_version_number;
std::vector <CTextListWnd*> text_window_list;
QHash <QString, QHash<QString, QString> > hash_markup;

enum {
      FM_ENTRY_MODE_NONE = 0,
      FM_ENTRY_MODE_OPEN,
      FM_ENTRY_MODE_SAVE
     };


/*
===========================
CDarkerWindow
===========================
*/


void CDarkerWindow::closeEvent (QCloseEvent *event)
{
  event->accept();
}


CDarkerWindow::CDarkerWindow()
{
  setAttribute (Qt::WA_DeleteOnClose);
  setWindowFlags (Qt::Tool);
  setWindowTitle (tr ("Darker palette"));

  slider = new QSlider (Qt::Horizontal);
  slider->setMinimum (0);
  slider->setMaximum (200);

  QVBoxLayout *v_box = new QVBoxLayout;
  setLayout (v_box);
  v_box->addWidget (slider);

  slider->setValue (settings->value ("darker_val", "100").toInt() - 100);

  connect (slider, SIGNAL(valueChanged(int)), this, SLOT(slot_valueChanged(int)));
}


void CDarkerWindow::slot_valueChanged (int value)
{
  settings->setValue ("darker_val", value + 100);

  documents->apply_settings();
  main_window->update_stylesheet (main_window->fname_stylesheet);
}


/*
===========================
CTextListWnd
===========================
*/


void CTextListWnd::closeEvent (QCloseEvent *event)
{
  event->accept();
}


CTextListWnd::~CTextListWnd()
{
  vector<CTextListWnd*>::iterator it = std::find (text_window_list.begin(), text_window_list.end(), this);
  text_window_list.erase (it);
}


CTextListWnd::CTextListWnd (const QString &title, const QString &label_text)
{
  setAttribute (Qt::WA_DeleteOnClose);
  QVBoxLayout *lt = new QVBoxLayout;

  QLabel *l = new QLabel (label_text);
  list = new QListWidget (this);

  lt->addWidget (l);
  lt->addWidget (list);

  setLayout (lt);
  setWindowTitle (title);

  text_window_list.push_back (this);
}


/*
===========================
About window
===========================
*/


void CAboutWindow::closeEvent (QCloseEvent *event)
{
  event->accept();
}


void CAboutWindow::update_image()
{
  QImage img (400, 100, QImage::Format_ARGB32);
  QPainter painter (&img);

  QFont f;
  f.setPixelSize (25);

  painter.setPen (Qt::gray);
  painter.setFont (f);

  for (int y = 1; y < 100; y += 25)
  for (int x = 1; x < 400; x += 25)
      {
       QColor color;

       int i = rand() % 5;

       switch (i)
              {
               case 0: color = 0xfff3f9ff;
                       break;

               case 1: color = 0xffbfffb0;
                       break;

               case 2: color = 0xffa5a5a6;
                       break;

               case 3: color = 0xffebffe9;
                       break;

               case 4: color = 0xffbbf6ff;
                       break;
              }

       painter.fillRect (x, y, 25, 25, QBrush (color));

       if (i == 0)
           painter.drawText (x, y + 25, "0");

       if (i == 1)
           painter.drawText (x, y + 25, "1");
     }

  QString txt = "TEA";

  QFont f2 ("Monospace");
  f2.setPixelSize (75);
  painter.setFont (f2);

  painter.setPen (Qt::black);
  painter.drawText (4, 80, txt);

  painter.setPen (Qt::red);
  painter.drawText (2, 76, txt);

  logo->setPixmap (QPixmap::fromImage (img));
}


CAboutWindow::CAboutWindow()
{
 // setAttribute (Qt::WA_DeleteOnClose);

  QStringList sl_t = qstring_load (":/AUTHORS").split ("##");

  logo = new QLabel;
  update_image();

  QTimer *timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(update_image()));
  timer->start (1000);

  QTabWidget *tw = new QTabWidget (this);

  QPlainTextEdit *page_code = new QPlainTextEdit();
  QPlainTextEdit *page_thanks = new QPlainTextEdit();
  QPlainTextEdit *page_translators = new QPlainTextEdit();

  if (sl_t.size() == 3)
     {
      page_code->setPlainText (sl_t[0].trimmed());
      page_thanks->setPlainText (sl_t[2].trimmed());
      page_translators->setPlainText (sl_t[1].trimmed());
     }

  tw->addTab (page_code, tr ("Code"));
  tw->addTab (page_thanks, tr ("Acknowledgements"));
  tw->addTab (page_translators, tr ("Translations"));

  QVBoxLayout *layout = new QVBoxLayout();

  layout->addWidget(logo);
  layout->addWidget(tw);

  setLayout (layout);
  setWindowTitle (tr ("About"));
}


/*
===========================
Local utility functions
===========================
*/


//for the further compatibility
QTabWidget::TabPosition int_to_tabpos (int i)
{
  QTabWidget::TabPosition p = QTabWidget::North;

  switch (i)
         {
          case 0:
                 p = QTabWidget::North;
                 break;
          case 1:
                 p = QTabWidget::South;
                 break;
          case 2:
                 p = QTabWidget::West;
                 break;
          case 3:
                 p = QTabWidget::East;
                 break;
         }

  return p;
}


QString toggle_fname_header_source (const QString &fileName)
{
  QFileInfo f (fileName);

  QString ext = f.suffix();

  if (ext == "c" || ext == "cpp" || ext == "cxx" || ext == "cc" || ext == "c++" ||
      ext == "m" || ext == "mm")
     {
      if (file_exists (f.absolutePath() + "/" + f.baseName () + ".h"))
         return f.absolutePath() + "/" + f.baseName () + ".h";
      else
      if (file_exists (f.absolutePath() + "/" + f.baseName () + ".hxx"))
         return f.absolutePath() + "/" + f.baseName () + ".hxx";
      else
      if (file_exists (f.absolutePath() + "/" + f.baseName () + ".h++"))
         return f.absolutePath() + "/" + f.baseName () + ".h++";
      else
      if (file_exists (f.absolutePath() + "/" + f.baseName () + ".hh"))
         return f.absolutePath() + "/" + f.baseName () + ".hh";
      else
      if (file_exists (f.absolutePath() + "/" + f.baseName () + ".hpp"))
         return f.absolutePath() + "/" + f.baseName () + ".hpp";
     }
  else
  if (ext == "h" || ext == "h++" || ext == "hxx" || ext == "hh" || ext == "hpp")
     {
      if (file_exists (f.absolutePath() + "/" + f.baseName () + ".c"))
         return f.absolutePath() + "/" + f.baseName () + ".c";
      else
      if (file_exists (f.absolutePath() + "/" + f.baseName () + ".cpp"))
         return f.absolutePath() + "/" + f.baseName () + ".cpp";
      else
      if (file_exists (f.absolutePath() + "/" + f.baseName () + ".cxx"))
         return f.absolutePath() + "/" + f.baseName () + ".cxx";
      else
      if (file_exists (f.absolutePath() + "/" + f.baseName () + ".c++"))
         return f.absolutePath() + "/" + f.baseName () + ".c++";
      else
      if (file_exists (f.absolutePath() + "/" + f.baseName () + ".cc"))
         return f.absolutePath() + "/" + f.baseName () + ".cc";
      else
      if (file_exists (f.absolutePath() + "/" + f.baseName () + ".m"))
         return f.absolutePath() + "/" + f.baseName () + ".m";
      else
      if (file_exists (f.absolutePath() + "/" + f.baseName () + ".mm"))
         return f.absolutePath() + "/" + f.baseName () + ".mm";
     }

  return fileName;
}


void CTEA::closeEvent (QCloseEvent *event)
{
  if (main_tab_widget->currentIndex() == idx_tab_tune)
     leaving_options();

  qstring_save (dir_config + "/last_used_charsets", sl_last_used_charsets.join ("\n").trimmed());

  if (settings->value("session_restore", false).toBool())
     {
      QString fname_session = dir_sessions + "/def-session-777";
      documents->save_to_session (fname_session);
     }

  if (settings->value("save_buffers", true).toBool())
      documents->save_buffers (fname_saved_buffers);


  write_search_options();
  write_settings();

  qstring_save (fname_fif, sl_fif_history.join ("\n"));

  hash_save_keyval (fname_autosaving_files, documents->autosave_files);


  delete documents;
  delete img_viewer;

  if (wnd_about)
     {
      wnd_about->close();
      delete wnd_about;
    }

#if defined (HUNSPELL_ENABLE) || defined (ASPELL_ENABLE)
  delete spellchecker;
#endif

  delete shortcuts;

  if (text_window_list.size() > 0)
      for (vector <size_t>::size_type i = 0; i < text_window_list.size(); i++)
          text_window_list[i]->close();

  event->accept();
  deleteLater();
}


void CTEA::dragEnterEvent (QDragEnterEvent *event)
{
  if (event->mimeData()->hasFormat ("text/uri-list"))
     event->acceptProposedAction();
}


void CTEA::dropEvent (QDropEvent *event)
{
  if (! event->mimeData()->hasUrls())
     return;

  QString fName;
  QFileInfo info;

  QList<QUrl> l = event->mimeData()->urls();

  for (QList <QUrl>::iterator u = l.begin(); u != l.end(); ++u)
      {
       fName = u->toLocalFile();

       info.setFile (fName);
       if (info.isFile())
          documents->open_file (fName, cb_fman_codecs->currentText());
      }

  event->accept();
}

/*
===========================
Main window slots
===========================
*/

void CTEA::pageChanged (int index)
{
  if (b_destroying_all || index == -1)
      return;

  CDocument *d = documents->items[index];
  if (! d)
     return;

  d->update_title (settings->value ("full_path_at_window_title", 1).toBool());
  d->update_status();

  if (file_exists (d->file_name))
      documents->dir_last = get_file_path (d->file_name);

  documents->update_project (d->file_name);

  update_labels_menu();
}


void CTEA::logmemo_double_click (const QString &txt)
{
  if (documents->hash_project.isEmpty() || documents->fname_current_project.isEmpty())
      return;

  QStringList parsed = txt.split (":");
  if (parsed.size() == 0)
     return;

  QString source_fname;
  QString source_line;
  QString source_col;

  source_fname = parsed[0];

  if (parsed.size() > 1)
     source_line = parsed[1];

  if (parsed.size() > 2)
     source_col = parsed[2];

  if (source_fname.startsWith(".."))
     source_fname.remove (0, 2);

  QFileInfo dir_source (documents->fname_current_project);
  QString source_dir = dir_source.absolutePath();

  source_fname = source_dir + "/" + source_fname;

  log->no_jump = true;
  CDocument *d = documents->open_file (source_fname, "UTF-8");
  log->no_jump = false;

  if (! d)
     return;

  QTextCursor cur = d->textCursor();
  if (cur.isNull())
     return;

  cur.movePosition (QTextCursor::Start);
  cur.movePosition (QTextCursor::Down, QTextCursor::MoveAnchor, source_line.toInt() - 1);
  cur.movePosition (QTextCursor::Right, QTextCursor::MoveAnchor, source_col.toInt() - 1);
  cur.select (QTextCursor::WordUnderCursor);
  d->setTextCursor (cur);
  d->setFocus();
}


void CTEA::receiveMessage (const QString &msg)
{
  if (! msg.isEmpty())
     documents->open_file (msg, "UTF-8");
}


void CTEA::receiveMessageShared (const QStringList &msg)
{
  for (int i = 0; i < msg.size(); i++)
      {
       QFileInfo f (msg.at(i));

       if (! f.isAbsolute())
          {
           QString fullname (QDir::currentPath());
           fullname.append ("/").append (msg.at(i));
           documents->open_file (fullname, charset);
          }
      else
          documents->open_file (msg.at(i), charset);
    }

  show();
  activateWindow();
  raise();
}


void CTEA::tb_stop_clicked()
{
  boring = true;
}


void CTEA::read_settings()
{
  MyProxyStyle::cursor_blink_time = settings->value ("cursor_blink_time", 0).toInt();
  qApp->setCursorFlashTime (MyProxyStyle::cursor_blink_time);
  MyProxyStyle::b_altmenu = settings->value ("b_altmenu", "0").toBool();

  recent_list_max_items = settings->value ("recent_list.max_items", 21).toInt();

  main_tab_widget->setTabPosition (int_to_tabpos (settings->value ("ui_tabs_align", "0").toInt()));
  tab_editor->setTabPosition (int_to_tabpos (settings->value ("docs_tabs_align", "0").toInt()));

  markup_mode = settings->value ("markup_mode", "HTML").toString();
  charset = settings->value ("charset", "UTF-8").toString();
  fname_def_palette = settings->value ("fname_def_palette", ":/palettes/TEA").toString();

  QPoint pos = settings->value ("pos", QPoint (1, 200)).toPoint();
  QSize size = settings->value ("size", QSize (800, 512)).toSize();

  if (mainSplitter)
      mainSplitter->restoreState (settings->value ("splitterSizes").toByteArray());

  resize (size);
  move (pos);

  pos = settings->value ("md_viewer_pos", QPoint (1, 200)).toPoint();
  size = settings->value ("md_viewer_size", QSize (800, 512)).toSize();
  md_viewer.resize (size);
  md_viewer.move (pos);
}


void CTEA::write_settings()
{
  if (mainSplitter)
     settings->setValue ("splitterSizes", mainSplitter->saveState());

  settings->setValue ("pos", pos());
  settings->setValue ("size", size());
  settings->setValue ("charset", charset);
  settings->setValue ("spl_fman", spl_fman->saveState());
  settings->setValue ("dir_last", documents->dir_last);
  settings->setValue ("fname_def_palette", fname_def_palette);
  settings->setValue ("markup_mode", markup_mode);
  settings->setValue ("VER_NUMBER", QString (current_version_number));
  settings->setValue ("state", saveState());
  settings->setValue ("word_wrap", cb_wordwrap->isChecked());
  settings->setValue ("show_linenums", cb_show_linenums->isChecked());
  settings->setValue ("fif_at_toolbar", cb_fif_at_toolbar->isChecked());
  settings->setValue ("save_buffers", cb_save_buffers->isChecked());
  settings->setValue ("md_viewer_pos", md_viewer.pos());
  settings->setValue ("md_viewer_size", md_viewer.size());

  delete settings;
}


void CTEA::read_search_options()
{
  menu_find_whole_words->setChecked (settings->value ("find_whole_words", "0").toBool());
  menu_find_case->setChecked (settings->value ("find_case", "0").toBool());
  menu_find_regexp->setChecked (settings->value ("find_regexp", "0").toBool());
  menu_find_fuzzy->setChecked (settings->value ("find_fuzzy", "0").toBool());
  menu_find_from_cursor->setChecked (settings->value ("find_from_cursor", "1").toBool());
}


void CTEA::write_search_options()
{
  settings->setValue ("find_whole_words", menu_find_whole_words->isChecked());
  settings->setValue ("find_case", menu_find_case->isChecked());
  settings->setValue ("find_regexp", menu_find_regexp->isChecked());
  settings->setValue ("find_fuzzy", menu_find_fuzzy->isChecked());
  settings->setValue ("find_from_cursor", menu_find_from_cursor->isChecked());
}


/*
===================
File manager slots
===================
*/


void CTEA::fman_drives_changed (const QString & path)
{
  if (! ui_update)
     fman->nav (path);
}


void CTEA::fman_current_file_changed (const QString &full_path, const QString &just_name)
{
  if (! is_dir (full_path))
     ed_fman_fname->setText (just_name);
  else
      ed_fman_fname->setText ("");

  if (b_preview && is_image (full_path))
     {
      if (! img_viewer->window_mini.isVisible())
         {
          img_viewer->window_mini.show();
          activateWindow();
          fman->setFocus();
         }

      img_viewer->set_image_mini (full_path);
     }
}


void CTEA::fman_file_activated (const QString &full_path)
{
  if (file_get_ext (full_path) == ("zip"))
     {
      CZipper z;
      QStringList sl = z.unzip_list (full_path);

      for (int i = 0; i < sl.size(); i++)
           sl[i] = sl[i] + "<br>";

      log->log (sl.join("\n"));
      return;
     }


  if (is_image (full_path))
     {
      CDocument *d = documents->get_current();
      if (! d)
         return;

      d->insert_image (full_path);
      main_tab_widget->setCurrentIndex (idx_tab_edit);
      return;
     }

  CDocument *d = documents->open_file (full_path, cb_fman_codecs->currentText());
  if (d)
      charset = d->charset;

  add_to_last_used_charsets (cb_fman_codecs->currentText());
  main_tab_widget->setCurrentIndex (idx_tab_edit);
}


void CTEA::fman_dir_changed (const QString &full_path)
{
  ui_update = true;
  ed_fman_path->setText (full_path);

#if defined(Q_OS_WIN) || defined(Q_OS_OS2)
  cb_fman_drives->setCurrentIndex (cb_fman_drives->findText (full_path.left(3).toUpper()));
#endif

  ui_update = false;
}


void CTEA::fman_fname_entry_confirm()
{
  if (fm_entry_mode == FM_ENTRY_MODE_OPEN)
     fman_open();

  if (fm_entry_mode == FM_ENTRY_MODE_SAVE)
     cb_button_saves_as();
}


void CTEA::fman_naventry_confirm()
{
  fman->nav (ed_fman_path->text());
}


void CTEA::fman_add_bmk()
{
  sl_places_bmx.prepend (ed_fman_path->text());
  qstring_save (fname_places_bookmarks, sl_places_bmx.join ("\n"));
  update_places_bookmarks();
}


void CTEA::fman_del_bmk()
{
  int i = lv_places->currentRow();
  if (i < 5) //TEA built-in bookmark, don't remove
     return;

  if (i > sl_places_bmx.size() + 4) // -- GTK places
     {
      qDebug() << "GTK bookmarks, i: " << i;
      return;
     }

  //else TEA places

  qDebug() << "TEA places, i: " << i;

  QString s = lv_places->item(i)->text();
  if (s.isEmpty())
     return;

  sl_places_bmx.removeAt (sl_places_bmx.indexOf (s)); // поменять на индекс i + 5
  //sl_places_bmx.removeAt (i);

  qstring_save (fname_places_bookmarks, sl_places_bmx.join ("\n"));
  update_places_bookmarks();
}


void CTEA::fman_open()
{
  QString f = ed_fman_fname->text().trimmed();
  QStringList li = fman->get_sel_fnames();

  if (path_is_abs (f)) //if file name entry is the full path and not empty
     {
      CDocument *d = documents->open_file (f, cb_fman_codecs->currentText());
      if (d)
         {
          charset = d->charset;
          add_to_last_used_charsets (cb_fman_codecs->currentText());
          main_tab_widget->setCurrentIndex (idx_tab_edit);
         }

      return;
     }

  //if file name entry == just filename
  if (li.size() == 0 && ! f.isEmpty())
     {
      QString fname (fman->dir.path());
      fname.append ("/").append (f);
      CDocument *d = documents->open_file (fname, cb_fman_codecs->currentText());
      if (d)
         {
          charset = d->charset;
          add_to_last_used_charsets (cb_fman_codecs->currentText());
          main_tab_widget->setCurrentIndex (idx_tab_edit);
         }

      return;
     }

  //if file[s] selected at files list

  bool opened = false;

  for (int i = 0; i < li.size(); i++)
      {
       CDocument *d = 0;
       d = documents->open_file (li.at(i), cb_fman_codecs->currentText());
       if (d)
           {
            charset = d->charset;
            opened = true;
           }
      }

  add_to_last_used_charsets (cb_fman_codecs->currentText());

  if (opened)
     main_tab_widget->setCurrentIndex (idx_tab_edit);
}


void CTEA::fman_places_itemActivated (QListWidgetItem *item)
{
  int i = lv_places->currentRow();
  QString s = item->text();

  vector <QString> v;

  v.push_back (dir_templates);
  v.push_back (dir_snippets);
  v.push_back (dir_scripts);
  v.push_back (dir_tables);
  v.push_back (dir_config);

  if (i < 5)
     s = v[i];

  //if (i > sl_places_bmx.size() + 4)
    //  s = sl_gtk_bookmarks[i - (sl_places_bmx.size() + 6)];

  fman->nav (s);
}


void CTEA::cb_button_saves_as()
{
  CDocument *d = documents->get_current();

  if (! d || ed_fman_fname->text().isEmpty())
     return;

  QString fn = ed_fman_fname->text();
  QString filename;

  if (path_is_abs (fn)) //if file name entry is the full path and not empty
     filename = fn;
  else
      {
       filename = fman->dir.path();
       filename.append ("/").append (ed_fman_fname->text());
      }

  if (path_is_dir (filename))
     return;

  if (file_exists (filename))
     if (QMessageBox::warning (this, "TEA",
                               tr ("%1 already exists\n"
                               "Do you want to overwrite?")
                               .arg (filename),
                               QMessageBox::Yes,
                               QMessageBox::Cancel) == QMessageBox::Cancel)
         return;


  d->file_save_with_name (filename, cb_fman_codecs->currentText());
  d->set_markup_mode();

  add_to_last_used_charsets (cb_fman_codecs->currentText());

  d->set_hl();

  QFileInfo f (d->file_name);
  documents->dir_last = f.path();
  update_dyn_menus();

  shortcuts->load_from_file (shortcuts->fname);

  fman->refresh();
  main_tab_widget->setCurrentIndex (idx_tab_edit);
}



/*
void CTEA::ide_ctags()
{
  if (documents->hash_project.isEmpty())
     return;

  if (documents->fname_current_project.isEmpty())
     return;

  QFileInfo source_dir (documents->fname_current_project);

  QString command_ctags = hash_get_val (documents->hash_project,
                                       "command_ctags", "ctags -R");

  QProcess *process  = new QProcess (this);
  process->setWorkingDirectory (source_dir.absolutePath());

  connect (process, SIGNAL(readyReadStandardOutput()), this, SLOT(process_readyReadStandardOutput()));

  process->setProcessChannelMode (QProcess::MergedChannels) ;
  process->start (command_ctags, QIODevice::ReadWrite);

  if (! process->waitForFinished())
     return;

  //else parse ctags file
}



void CTEA::ide_gtags()
{
  if (documents->hash_project.isEmpty())
     return;

  if (documents->fname_current_project.isEmpty())
     return;

  QFileInfo source_dir (documents->fname_current_project);

  QString command_gtags = hash_get_val (documents->hash_project,
                                       "command_gtags", "gtags");

  QProcess *process  = new QProcess (this);
  process->setWorkingDirectory (source_dir.absolutePath());

//  connect (process, SIGNAL(readyReadStandardOutput()), this, SLOT(process_readyReadStandardOutput()));

  if (file_exists (source_dir.absolutePath() + "/GTAGS"))
     command_gtags = "global -u";


  process->setProcessChannelMode (QProcess::MergedChannels) ;
  process->start (command_gtags, QIODevice::ReadWrite);

  if (! process->waitForStarted())
     return;

  if (! process->waitForFinished())
     return;

}

void CTEA::ide_global_definition()
{
  if (documents->hash_project.isEmpty())
     return;

  if (documents->fname_current_project.isEmpty())
     return;

  CDocument *d = documents->get_current();
  if (! d)
      return;

  QString sel_text = d->get();

  QFileInfo source_dir (documents->fname_current_project);

  QString command = hash_get_val (documents->hash_project,
                                  "command_global_definition", "global -a -x --result=grep");


  command = command + " " + sel_text;

  QProcess *process  = new QProcess (this);
  process->setWorkingDirectory (source_dir.absolutePath());

  process->setProcessChannelMode (QProcess::MergedChannels) ;
  process->start (command, QIODevice::ReadWrite);

  if (! process->waitForStarted())
     return;

  if (! process->waitForFinished())
     return;

  QByteArray a = process->readAll();
  if (a.isEmpty())
     return;

  QString s(a);
//value_t            28 lib/optional/libffi/ffi.cpp struct value_t {

  //if (s.indexOf ('\t') != -1)
    // qDebug() << "TAB!";



//qDebug() << "4";

  //parse output

  QStringList sl_output = s.split ("\n");
  if (sl_output.size() == -1)
     return;

///home/rox/devel/tea-qt/main.cpp:36:int main (int argc, char *argv[])


  foreach (QString str, sl_output)
          {
           if (! str.isEmpty())
              {
               int idx_path = str.indexOf (":");
               QString str_path = str.left (idx_path);

               int idx_line = str.indexOf (":", idx_path + 1);
               QString str_line = str.mid (idx_path + 1, idx_line - idx_path);

//               int idx_pattern = str.indexOf (":", idx_line + 1);
               QString str_pattern = str.right (str.size() - idx_line + 1);

               qDebug() << "path: " << str_path;
               qDebug() << "line: " << str_line;
               qDebug() << "pattern: " << str_pattern;


              }
          }


qDebug() << "3";
}


void CTEA::ide_global_references()
{


}
*/


/*
===================
File menu callbacks
===================
*/

void CTEA::test()
{
 /* CIconvCharsetConverter c;
  QByteArray ba = file_load ("/home/rox/devel/test/1251.txt");

  QString s = c.to_utf16 ("CP1251", ba.data(), ba.size());
 qDebug() << s;*/

//  QIconvCodec c;

}


void CTEA::file_new()
{
  last_action = sender();
  documents->create_new();
  main_tab_widget->setCurrentIndex (idx_tab_edit);
}


void CTEA::file_open()
{
  last_action = sender();

  if (! settings->value ("use_trad_dialogs", "1").toBool())
     {
      CDocument *d = documents->get_current();

      if (d)
         {
          if (file_exists (d->file_name))
              fman->nav (get_file_path (d->file_name));
         }
      else
          fman->nav (documents->dir_last);

      main_tab_widget->setCurrentIndex (idx_tab_fman);
      fm_entry_mode = FM_ENTRY_MODE_OPEN;

      return;
     }

  //ELSE use the traditional dialog

  QFileDialog dialog (this);
  QSize size = settings->value ("dialog_size", QSize (width(), height())).toSize();
  dialog.resize (size);

  dialog.setFilter (QDir::AllEntries | QDir::Hidden);
  dialog.setOption (QFileDialog::DontUseNativeDialog, true);

#if QT_VERSION >= 0x050200
  dialog.setOption (QFileDialog::DontUseCustomDirectoryIcons, true);
#endif


  QList<QUrl> sidebarUrls = dialog.sidebarUrls();
  QList<QUrl> sidebarUrls_old = dialog.sidebarUrls();

  sidebarUrls.append (QUrl::fromLocalFile (dir_templates));
  sidebarUrls.append (QUrl::fromLocalFile (dir_snippets));
  sidebarUrls.append (QUrl::fromLocalFile (dir_sessions));
  sidebarUrls.append (QUrl::fromLocalFile (dir_scripts));
  sidebarUrls.append (QUrl::fromLocalFile (dir_tables));

#ifdef Q_OS_UNIX

  QDir volDir ("/mnt");
  QStringList volumes (volDir.entryList (volDir.filter() | QDir::NoDotAndDotDot));

  QDir volDir2 ("/media");
  QStringList volumes2 (volDir2.entryList (volDir.filter() | QDir::NoDotAndDotDot));

  for (int i = 0; i < volumes.size(); i++)
      sidebarUrls.append (QUrl::fromLocalFile ("/mnt/" + volumes.at(i)));

  for (int i = 0; i < volumes2.size(); i++)
      sidebarUrls.append (QUrl::fromLocalFile ("/media/" + volumes2.at(i)));

#endif

  dialog.setSidebarUrls (sidebarUrls);

  dialog.setFileMode (QFileDialog::ExistingFiles);
  dialog.setAcceptMode (QFileDialog::AcceptOpen);

  CDocument *d = documents->get_current();
  if (d)
     {
      if (file_exists (d->file_name))
          dialog.setDirectory (get_file_path (d->file_name));
     }
  else
      dialog.setDirectory (documents->dir_last);

  dialog.setNameFilter (tr ("All (*);;Text files (*.txt);;Markup files (*.xml *.html *.xhtml *.htm *.md);;C/C++ (*.c *.h *.cpp *.hh *.c++ *.h++ *.cxx)"));

  QLabel *l = new QLabel (tr ("Charset"));
  QComboBox *cb_codecs = new QComboBox (&dialog);
  dialog.layout()->addWidget (l);
  dialog.layout()->addWidget (cb_codecs);

  if (sl_last_used_charsets.size () > 0)
     cb_codecs->addItems (sl_last_used_charsets + sl_charsets);
  else
     {
      cb_codecs->addItems (sl_charsets);
      cb_codecs->setCurrentIndex (sl_charsets.indexOf ("UTF-8"));
     }

  QStringList fileNames;

  if (dialog.exec())
     {
      dialog.setSidebarUrls (sidebarUrls_old);

      fileNames = dialog.selectedFiles();

      for (int i = 0; i < fileNames.size(); i++)
          {
           CDocument *dc = documents->open_file (fileNames.at(i), cb_codecs->currentText());
           if (dc)
               charset = dc->charset;

           add_to_last_used_charsets (cb_codecs->currentText());
          }
     }
  else
      dialog.setSidebarUrls (sidebarUrls_old);

  settings->setValue ("dialog_size", dialog.size());
  update_dyn_menus();
}


void CTEA::file_open_at_cursor()
{
  last_action = sender();

  if (main_tab_widget->currentIndex() == idx_tab_fman)
     {
      fman_preview_image();
      return;
     }

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QString fname = d->get_filename_at_cursor();

  if (fname.isEmpty())
     return;

  if (is_image (fname))
     {
      if (settings->value ("override_img_viewer", 0).toBool()) //external image viewer
         {
          QString command = settings->value ("img_viewer_override_command", "display %s").toString();
          command = command.replace ("%s", fname);
          //QProcess::startDetached (command, QStringList());

          system (command.toUtf8().data());

          return;
         }
      else
          {
           if (file_get_ext (fname) == "gif")
              {
               CGIFWindow *w = new CGIFWindow;
               w->load_image (fname);
               return;
              }
           else //not GIF
               {
                if (! img_viewer->window_full.isVisible())
                   {
                    img_viewer->window_full.show();
                    activateWindow();
                   }

                img_viewer->set_image_full (fname);
                return;
               }
          }
      }


  if (fname.startsWith ("#"))  //HTML inner label
     {
      QString t = fname;
      t.remove (0, 1);
      t.prepend ("name=\"");
      if (d->find (t))
         return;

      t = fname;
      t.remove (0, 1);
      t.prepend ("id=\"");
      d->find (t);

      return;
     }

  documents->open_file (fname, d->charset);
}


void CTEA::file_last_opened()
{
  last_action = sender();

  if (documents->recent_files.size() > 0)
     {
      documents->open_file_triplex (documents->recent_files[0]);
      documents->recent_files.removeAt (0);
      documents->update_recent_menu();
     }
}


void CTEA::file_crapbook()
{
  last_action = sender();

  if (! QFile::exists (fname_crapbook))
      qstring_save (fname_crapbook, tr ("you can put here notes, etc"));

  documents->open_file (fname_crapbook, "UTF-8");
}


void CTEA::file_notes()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  if (! file_exists  (d->file_name))
     return;

  QString fname = d->file_name + ".notes";

  if (! file_exists (fname))
      qstring_save (fname, tr ("put your notes (for this file) here and they will be saved automatically"));

  documents->open_file (fname, "UTF-8");
}


bool CTEA::file_save()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return false;

  if (d->isReadOnly())
     {
      log->log (tr ("This file is opened in the read-only mode. You can save it with another name using <b>Save as</b>"));
      return false;
     }

  if (file_exists (d->file_name))
     d->file_save_with_name (d->file_name, d->charset);
  else
      return file_save_as();

  if (d->file_name == fname_bookmarks)
     update_bookmarks();

  if (d->file_name == fname_programs)
     update_programs();

  return true;
}


bool CTEA::file_save_as()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return false;

  if (! settings->value ("use_trad_dialogs", "1").toBool())
     {
      main_tab_widget->setCurrentIndex (idx_tab_fman);
      fm_entry_mode = FM_ENTRY_MODE_SAVE;

      if (file_exists (d->file_name))
         fman->nav (get_file_path (d->file_name));
      else
          fman->nav (documents->dir_last);

      ed_fman_fname->setFocus();

      return true;
     }


  //ELSE standard dialog

  QFileDialog dialog (this);
  QSize size = settings->value ("dialog_size", QSize (width(), height())).toSize();
  dialog.resize (size);

  dialog.setFilter (QDir::AllEntries | QDir::Hidden);
  dialog.setOption (QFileDialog::DontUseNativeDialog, true);

  QList<QUrl> sidebarUrls = dialog.sidebarUrls();
  QList<QUrl> sidebarUrls_old = dialog.sidebarUrls();

  sidebarUrls.append (QUrl::fromLocalFile(dir_templates));
  sidebarUrls.append (QUrl::fromLocalFile(dir_snippets));
  sidebarUrls.append (QUrl::fromLocalFile(dir_sessions));
  sidebarUrls.append (QUrl::fromLocalFile(dir_scripts));
  sidebarUrls.append (QUrl::fromLocalFile(dir_tables));

#ifdef Q_OS_LINUX

  QDir volDir ("/mnt");
  QStringList volumes (volDir.entryList (volDir.filter() | QDir::NoDotAndDotDot));

  QDir volDir2 ("/media");
  QStringList volumes2 (volDir2.entryList (volDir2.filter() | QDir::NoDotAndDotDot));

  for (int i = 0; i < volumes.size(); i++)
      sidebarUrls.append (QUrl::fromLocalFile ("/mnt/" + volumes.at(i)));

  for (int i = 0; i < volumes2.size(); i++)
      sidebarUrls.append (QUrl::fromLocalFile ("/media/" + volumes2.at(i)));

#endif

  dialog.setSidebarUrls (sidebarUrls);

  dialog.setFileMode (QFileDialog::AnyFile);
  dialog.setAcceptMode (QFileDialog::AcceptSave);
  dialog.setDirectory (documents->dir_last);

  QLabel *l = new QLabel (tr ("Charset"));
  QComboBox *cb_codecs = new QComboBox (&dialog);
  dialog.layout()->addWidget (l);
  dialog.layout()->addWidget (cb_codecs);

  if (sl_last_used_charsets.size () > 0)
     cb_codecs->addItems (sl_last_used_charsets + sl_charsets);
  else
     {
      cb_codecs->addItems (sl_charsets);
      cb_codecs->setCurrentIndex (sl_charsets.indexOf ("UTF-8"));
     }

  if (dialog.exec())
     {
      dialog.setSidebarUrls (sidebarUrls_old);

      QString fileName = dialog.selectedFiles().at(0);

      if (file_exists (fileName))
         {
          int ret = QMessageBox::warning (this, "TEA",
                                          tr ("%1 already exists\n"
                                          "Do you want to overwrite?")
                                           .arg (fileName),
                                          QMessageBox::Yes,
                                          QMessageBox::Cancel);

          if (ret == QMessageBox::Cancel)
             return false;
         }

      d->file_save_with_name (fileName, cb_codecs->currentText());
      d->set_markup_mode();
      d->set_hl();

      add_to_last_used_charsets (cb_codecs->currentText());
      update_dyn_menus();

      QFileInfo f (d->file_name);
      documents->dir_last = f.path();
     }
   else
       dialog.setSidebarUrls (sidebarUrls_old);

  settings->setValue ("dialog_size", dialog.size());
  return true;
}


void CTEA::file_save_bak()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  if (! file_exists (d->file_name))
     return;

  QString fname  = d->file_name + ".bak";
  d->file_save_with_name_plain (fname);
  log->log (tr ("%1 is saved").arg (fname));
}


void CTEA::file_save_version()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  if (! file_exists (d->file_name))
     return;

  QDate date = QDate::currentDate();
  QFileInfo fi;
  fi.setFile (d->file_name);

  QString version_timestamp_fmt = settings->value ("version_timestamp_fmt", "yyyy-MM-dd").toString();
  QTime t = QTime::currentTime();

  QString fname = fi.absoluteDir().absolutePath() +
                  "/" +
                  fi.baseName() +
                  "-" +
                  date.toString (version_timestamp_fmt) +
                  "-" +
                  t.toString ("hh-mm-ss") +
                  "." +
                  fi.suffix();


  if (d->file_save_with_name_plain (fname))
     log->log (tr ("%1 - saved").arg (fname));
  else
     log->log (tr ("Cannot save %1").arg (fname));
}


void CTEA::file_session_save_as()
{
  last_action = sender();

  if (documents->items.size() == 0)
     return;

  bool ok;
  QString name = QInputDialog::getText (this, tr ("Enter the name"),
                                              tr ("Name:"), QLineEdit::Normal,
                                              tr ("new_session"), &ok);
  if (! ok || name.isEmpty())
     return;

  QString fname = dir_sessions + "/" + name;
  documents->save_to_session (fname);
  update_sessions();
}


void CTEA::file_reload()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
     d->reload (d->charset);
}


void CTEA::file_reload_enc_itemDoubleClicked (QListWidgetItem *item)
{
  CDocument *d = documents->get_current();
  if (d)
     d->reload (item->text());
}


void CTEA::file_reload_enc()
{
  last_action = sender();

  CTextListWnd *w = new CTextListWnd (tr ("Reload with encoding"), tr ("Charset"));

  if (sl_last_used_charsets.size () > 0)
     w->list->addItems (sl_last_used_charsets + sl_charsets);
  else
      w->list->addItems (sl_charsets);

  connect (w->list, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
           this, SLOT(file_reload_enc_itemDoubleClicked(QListWidgetItem*)));

  w->show();
}


void CTEA::file_set_eol_unix()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
     d->eol = "\n";
}


void CTEA::file_set_eol_win()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
     d->eol = "\r\n";
}


void CTEA::file_set_eol_mac()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
     d->eol = "\r";
}


void CTEA::file_set_autosaving_file()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  if (! file_exists (d->file_name))
     return;

  documents->autosave_files.insert (d->file_name, d->file_name);
}


void CTEA::file_unset_autosaving_file()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  if (! file_exists (d->file_name))
     return;

  documents->autosave_files.remove (d->file_name);
}


#ifdef PRINTER_ENABLE
void CTEA::file_print()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QPrintDialog *dialog = new QPrintDialog (&printer, this);

  dialog->setWindowTitle (tr ("Print document"));

  if (d->textCursor().hasSelection())
      dialog->addEnabledOption (QAbstractPrintDialog::PrintSelection);

  if (dialog->exec() != QDialog::Accepted)
      return;

  d->print (&printer);
}
#endif

void CTEA::file_add_to_bookmarks()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  if (! file_exists (d->file_name))
     return;

  bool found = false;
  QStringList l_bookmarks = qstring_load (fname_bookmarks).split("\n");

  for (int i = 0; i < l_bookmarks.size(); i++)
      {
       if (l_bookmarks.at(i).contains (d->file_name)) //update the bookmark
          {
           l_bookmarks[i] = d->get_triplex();
           found = true;
           break;
          }
      }

  if (! found) //else just add new bookbmark
      l_bookmarks.prepend (d->get_triplex());

  QString bookmarks = l_bookmarks.join ("\n").trimmed();

  qstring_save (fname_bookmarks, bookmarks);
  update_bookmarks();
}


void CTEA::file_find_obsolete_paths()
{
  QStringList l_bookmarks = qstring_load (fname_bookmarks).split ("\n");

  for (int i = 0; i < l_bookmarks.size(); i++)
      {
       QStringList t = l_bookmarks[i].split ("*");
       if (! file_exists (t[0]))
          l_bookmarks[i] = "#" + l_bookmarks[i]; //comment out the bookmark line
      }

  QString bookmarks = l_bookmarks.join ("\n").trimmed();

  qstring_save (fname_bookmarks, bookmarks);
  update_bookmarks();
}


void CTEA::file_open_bookmarks_file()
{
  last_action = sender();
  documents->open_file (fname_bookmarks, "UTF-8");
}


void CTEA::file_open_programs_file()
{
  last_action = sender();

#if defined(Q_OS_WIN) || defined(Q_OS_OS2)

  if (! file_exists (fname_programs))
     qstring_save (fname_programs, tr ("#external programs list. example:\nopera=\"C:\\Program Files\\Opera\\opera.exe \" \"%s\""));

#else

  if (! file_exists (fname_programs))
     qstring_save (fname_programs, tr ("#external programs list. example:\nff=firefox file:///%s"));

#endif

  documents->open_file (fname_programs, "UTF-8");
}


void CTEA::file_open_bookmark()
{
  last_action = sender();
  documents->open_file_triplex (qobject_cast<QAction *>(last_action)->data().toString());
  main_tab_widget->setCurrentIndex (idx_tab_edit);
}


void CTEA::file_use_template()
{
  last_action = sender();

  QAction *a = qobject_cast<QAction *>(sender());
  QString txt = qstring_load (a->data().toString());

  CDocument *d = documents->create_new();
  if (d)
     d->put (txt);
}


void CTEA::file_open_session()
{
  last_action = sender();

  QAction *a = qobject_cast<QAction *>(sender());
  documents->load_from_session (a->data().toString());
}


void CTEA::file_recent_off()
{
  last_action = sender();
  b_recent_off = ! b_recent_off;
}


void CTEA::file_close()
{
  last_action = sender();
  documents->close_current();
}

/*
===================
Edit menu callbacks
===================
*/


void CTEA::ed_copy()
{
  last_action = sender();

  if (main_tab_widget->currentIndex() == idx_tab_edit)
     {
      CDocument *d = documents->get_current();
      if (d)
          d->copy();
     }
  else
      if (main_tab_widget->currentIndex() == idx_tab_learn)
          man->copy();
}


void CTEA::ed_paste()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
      d->paste();
}


void CTEA::ed_cut()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
      d->cut();
}


void CTEA::ed_select_all()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
      d->selectAll();
}



void CTEA::ed_block_start()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
     d->rect_block_start();
}


void CTEA::ed_block_end()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
     d->rect_block_end();
}


void CTEA::ed_block_copy()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  if (! d->has_rect_selection())
     return;

  QApplication::clipboard()->setText (d->rect_sel_get());
}


void CTEA::ed_block_paste()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
      d->rect_sel_replace (QApplication::clipboard()->text());
}


void CTEA::ed_block_cut()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
      d->rect_sel_cut();
}


void CTEA::ed_copy_current_fname()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
     QApplication::clipboard()->setText (d->file_name);
}


void CTEA::ed_undo()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
      d->undo();
}


void CTEA::ed_redo()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
      d->redo();
}


void CTEA::ed_indent()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
     d->indent();
}


void CTEA::ed_unindent()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
     d->un_indent();
}


void CTEA::ed_indent_by_first_line()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QStringList sl = d->get().split (QChar::ParagraphSeparator);
  if (sl.size() == 0)
    return;

  QString x = sl[0];
  QChar c = x[0];
  int pos = 0;

  if (c == ' ' || c == '\t')
     for (int i = 0; i < x.size(); i++)
         if (x[i] != c)
            {
             pos = i;
             break;
            }

  QString fill_string;
  fill_string.fill (c, pos);

  for (int i = 0; i < sl.size(); i++)
      {
       QString s = sl[i].trimmed();
       s.prepend (fill_string);
       sl[i] = s;
      }

  QString t = sl.join ("\n");

  d->put (t);
}


void CTEA::ed_comment()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  if (! d->highlighter)
     return;

  if (d->highlighter->comment_mult.isEmpty() && d->highlighter->comment_single.isEmpty())
     return;

  QString t = d->get();
  QString result;

  bool is_multiline = true;

  int sep_pos = t.indexOf (QChar::ParagraphSeparator);
  if (sep_pos == -1 || sep_pos == t.size() - 1)
     is_multiline = false;

  if (is_multiline)
      result = d->highlighter->comment_mult;
  else
      result = d->highlighter->comment_single;

  if (is_multiline && result.isEmpty())
     {
      QStringList sl = t.split (QChar::ParagraphSeparator);
      for (int i = 0; i < sl.size(); i++)
          {
           QString x = d->highlighter->comment_single;
           sl[i] = x.replace ("%s", sl[i]);
          }

      QString z = sl.join("\n");
      d->put (z);

      return;
     }

  d->put (result.replace ("%s", t));
}


void CTEA::ed_set_as_storage_file()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
     fname_storage_file = d->file_name;
}


void CTEA::ed_copy_to_storage_file()
{
  last_action = sender();

  CDocument *dsource = documents->get_current();
  if (! dsource)
     return;

  CDocument *ddest = documents->get_document_by_fname (fname_storage_file);
  if (ddest)
     {
      QString t = dsource->get();
      ddest->put (t);
      ddest->put ("\n");
     }
  else
      log->log (tr ("The storage file is closed or not set."));
}


void CTEA::ed_capture_clipboard_to_storage_file()
{
  last_action = sender();
  capture_to_storage_file = qobject_cast<QAction *>(sender())->isChecked();
//was capture_to_storage_file = ! capture_to_storage_file;
}


/*
===================
Markup menu callbacks
===================
*/


void CTEA::mrkup_mode_choosed()
{
  last_action = sender();

  QAction *a = qobject_cast<QAction *>(sender());
  markup_mode = a->data().toString();
  documents->markup_mode = markup_mode;

  CDocument *d = documents->get_current();
  if (d)
     d->markup_mode = markup_mode;
}


void CTEA::mrkup_header()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QAction *a = qobject_cast<QAction *>(sender());

  QString r;

  if (documents->markup_mode == "Markdown")
     {
      QString t;
      int n = a->data().toString().toLower()[1].digitValue();
      t.fill ('#', n);
      r = t + " " + d->get();
     }
  else
      r = QString ("<%1>%2</%1>").arg (
                   a->data().toString().toLower()).arg (
                   d->get());

  d->put (r);
}


void CTEA::mrkup_align_center()
{
  last_action = sender();
  markup_text ("align_center");
}


void CTEA::mrkup_align_left()
{
  last_action = sender();
  markup_text ("align_left");
}


void CTEA::mrkup_align_right()
{
  last_action = sender();
  markup_text ("align_right");
}


void CTEA::mrkup_align_justify()
{
  last_action = sender();
  markup_text ("align_justify");
}


void CTEA::mrkup_bold()
{
  last_action = sender();
  markup_text ("bold");
}


void CTEA::mrkup_italic()
{
  last_action = sender();
  markup_text ("italic");
}


void CTEA::mrkup_underline()
{
  last_action = sender();
  markup_text ("underline");
}

void CTEA::mrkup_link()
{
  last_action = sender();
  markup_text ("link");
}


void CTEA::mrkup_para()
{
  last_action = sender();
  markup_text ("para");
}


void CTEA::mrkup_color()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QColor color = QColorDialog::getColor (Qt::green, this);
  if (! color.isValid())
     return;

  QString s;

  if (d->textCursor().hasSelection())
      s = QString ("<span style=\"color:%1;\">%2</span>")
                   .arg (color.name())
                   .arg (d->get());
  else
      s = color.name();

  d->put (s);
}


void CTEA::mrkup_br()
{
  last_action = sender();
  markup_text ("newline");
}


void CTEA::mrkup_nbsp()
{
  last_action = sender();
  CDocument *d = documents->get_current();
  if (d)
     d->put ("&nbsp;");
}


void CTEA::markup_ins_image()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  main_tab_widget->setCurrentIndex (idx_tab_fman);

  if (file_exists (d->file_name))
     fman->nav (get_file_path (d->file_name));
}


void CTEA::mrkup_text_to_html()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QStringList l;

  if (d->textCursor().hasSelection())
     l = d->get().split (QChar::ParagraphSeparator);
  else
      l = d->toPlainText().split("\n");

  QString result;

  if (d->markup_mode == "HTML")
     result += "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n";
  else
      result += "<!DOCTYPE html PUBLIC \"-//W3C//DTD  1.0 Transitional//EN\" \"http://www.w3.org/TR/1/DTD/1-transitional.dtd\">\n";

  result += "<html>\n"
            "<head>\n"
            "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n"
            "<style type=\"text/css\">\n"
            ".p1\n"
            "{\n"
            "margin: 0px 0px 0px 0px;\n"
            "padding: 0px 0px 0px 0px;\n"
            "text-indent: 1.5em;\n"
            "text-align: justify;\n"
            "}\n"
            "</style>\n"
            "<title></title>\n"
            "</head>\n"
            "<body>\n";

  for (int i = 0; i < l.size(); i++)
      {
       QString t = l.at(i).simplified();

       if (t.isEmpty())
          {
           if (d->markup_mode == "HTML")
               result += "<br>\n";
            else
                result += "<br />\n";
          }
       else
           result += "<p class=\"p1\">" + t + "</p>\n";
      }

  result += "</body>\n</html>";

  CDocument *doc = documents->create_new();

  if (doc)
     doc->put (result);
}


void CTEA::mrkup_tags_to_entities()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
     d->put (str_to_entities (d->get()));
}


void CTEA::mrkup_antispam_email()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QString s = d->get();
  QString result;

  for (int i = 0; i < s.size(); i++)
      result = result + "&#" + QString::number (s.at (i).unicode()) + ";";

  d->put (result);
}


void CTEA::mrkup_document_weight()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QString result;
  QStringList l = html_get_by_patt (d->toPlainText(), "src=\"");

  QFileInfo f (d->file_name);
  QUrl baseUrl (d->file_name);

  vector <pair <QString, qint64> > files;
  files.push_back(make_pair(d->file_name, f.size()));

  int size_total = 0;
  int files_total = 1;

  for (int i = 0; i < l.size(); i++)
      {
       QUrl relativeUrl (l.at(i));
       QString resolved = baseUrl.resolved (relativeUrl).toString();
       QFileInfo info (resolved);

       if (! info.exists())
         files.push_back(std::make_pair(tr ("%1 is not found<br>").arg (resolved), info.size()));
       else
           {
            files.push_back(std::make_pair(resolved, info.size()));
            size_total += info.size();
            ++files_total;
           }
       }

  std::sort (files.begin(), files.end());

  for (vector <pair <QString, qint64> >::iterator p = files.begin(); p != files.end(); ++p)
      {
       result += tr ("%1 kbytes %2 <br>").arg (QString::number (p->second / 1024)).arg (p->first);
      }

  result.prepend (tr ("Total size = %1 kbytes in %2 files<br>").arg (QString::number (size_total / 1024))
                  .arg (QString::number (files_total)));

  log->log (result);
}


void CTEA::mrkup_preview_color()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  if (! d->textCursor().hasSelection())
     return;

  QString color = d->get();

  if (QColor::colorNames().indexOf (color) == -1)
     {
      color = color.remove (";");
      if (! color.startsWith ("#"))
          color = "#" + color;
     }
  else
     {
      QColor c (color);
      color = c.name();
     }

  QString style = QString ("color:%1; font-weight:bold;").arg (color);
  log->log (tr ("<span style=\"%1\">COLOR SAMPLE</span>").arg (style));
}


void CTEA::mrkup_strip_html_tags()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QString text;

  if (d->textCursor().hasSelection())
     text = d->get();
  else
      text = d->toPlainText();

  if (d->textCursor().hasSelection())
     d->put (strip_html (text));
  else
      d->setPlainText (strip_html (text));
}


void CTEA::mrkup_rename_selected()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  if (! d->textCursor().hasSelection())
     {
      log->log (tr ("Select the file name first!"));
      return;
     }

  QString fname = d->get_filename_at_cursor();

  if (fname.isEmpty())
     return;

  if (documents->get_document_by_fname (fname))
     {
      log->log (tr("You are trying to rename the opened file, please close it first!"));
      return;
     }

  QString newname = fif_get_text();
  if (newname.isEmpty())
     return;

  QFileInfo fi (fname);
  if (! fi.exists() && ! fi.isWritable())
     return;

  QString newfpath = fi.path() + "/" + newname;
  QFile::rename (fname, newfpath);
  update_dyn_menus();
  fman->refresh();

  QDir dir (d->file_name);
  QString new_name = dir.relativeFilePath (newfpath);

  if (new_name.startsWith (".."))
     new_name = new_name.remove (0, 1);

  if (d->get().startsWith ("./") && ! new_name.startsWith ("./"))
     new_name = "./" + new_name;

  if (! d->get().startsWith ("./") && new_name.startsWith ("./"))
     new_name = new_name.remove (0, 2);

  if (d->textCursor().hasSelection())
     d->put (new_name.trimmed());
}


/*
===================
Search menu callbacks
===================
*/


void CTEA::search_find()
{
  last_action = sender();

  if (main_tab_widget->currentIndex() == idx_tab_edit)
     {
      CDocument *d = documents->get_current();
      if (! d)
        return;

      QTextCursor cr;

      int from = 0;

      if (settings->value ("find_from_cursor", "1").toBool())
          from = d->textCursor().position();

      d->text_to_search = fif_get_text();

#if QT_VERSION >= 0x050500
      if (menu_find_regexp->isChecked())
         cr = d->document()->find (QRegularExpression (d->text_to_search), from, get_search_options());
#else
      if (menu_find_regexp->isChecked())
         cr = d->document()->find (QRegExp (d->text_to_search), from, get_search_options());
#endif

/*
#if QT_VERSION < 0x050000
         cr = d->document()->find (QRegExp (d->text_to_search), from, get_search_options());
#else
         cr = d->document()->find (QRegularExpression (d->text_to_search), from, get_search_options());
#endif
*/
      if (menu_find_fuzzy->isChecked())
         {
          int pos = str_fuzzy_search (d->toPlainText(), d->text_to_search, from, settings->value ("fuzzy_q", "60").toInt());
          if (pos != -1)
             {
              from = pos + d->text_to_search.length() - 1;
              //set selection:
              cr = d->textCursor();
              cr.setPosition (from, QTextCursor::MoveAnchor);
              cr.movePosition (QTextCursor::Right, QTextCursor::KeepAnchor, d->text_to_search.length());

              if (! cr.isNull())
                  d->setTextCursor (cr);
             }
          return;
         }
      else //normal search
          cr = d->document()->find (d->text_to_search, from, get_search_options());

      if (! cr.isNull())
          d->setTextCursor (cr);
      else
          log->log(tr ("not found!"));
     }
  else
  if (main_tab_widget->currentIndex() == idx_tab_learn)
      man_find_find();
  else
  if (main_tab_widget->currentIndex() == idx_tab_tune)
     opt_shortcuts_find();
  else
  if (main_tab_widget->currentIndex() == idx_tab_fman)
     fman_find();
}


void CTEA::search_find_next()
{
  last_action = sender();

  if (main_tab_widget->currentIndex() == idx_tab_edit)
     {
      CDocument *d = documents->get_current();
      if (! d)
         return;

      QTextCursor cr;
      if (menu_find_regexp->isChecked())

#if QT_VERSION >= 0x050500
      if (menu_find_regexp->isChecked())
         cr = d->document()->find (QRegularExpression (d->text_to_search), d->textCursor().position(), get_search_options());
#else
//#if QT_VERSION < 0x050500
      if (menu_find_regexp->isChecked())
         cr = d->document()->find (QRegExp (d->text_to_search), d->textCursor().position(), get_search_options());
//#endif
#endif

      if (menu_find_fuzzy->isChecked())
         {
          int pos = str_fuzzy_search (d->toPlainText(), d->text_to_search, d->textCursor().position(), settings->value ("fuzzy_q", "60").toInt());
          if (pos != -1)
             {
              cr = d->textCursor();
              cr.setPosition (pos, QTextCursor::MoveAnchor);
              cr.movePosition (QTextCursor::Right, QTextCursor::KeepAnchor, d->text_to_search.length());

              if (! cr.isNull())
                  d->setTextCursor (cr);
             }
          return;
         }
      else
          cr = d->document()->find (d->text_to_search, d->textCursor().position(), get_search_options());

      if (! cr.isNull())
          d->setTextCursor (cr);
     }
   else
   if (main_tab_widget->currentIndex() == idx_tab_learn)
      man_find_next();
   else
   if (main_tab_widget->currentIndex() == idx_tab_tune)
      opt_shortcuts_find_next();
   else
   if (main_tab_widget->currentIndex() == idx_tab_fman)
      fman_find_next();
}


void CTEA::search_find_prev()
{
  last_action = sender();

  if (main_tab_widget->currentIndex() == idx_tab_edit)
     {
      CDocument *d = documents->get_current();
      if (! d)
         return;

      QTextCursor cr;

#if QT_VERSION >= 0x050500
        if (menu_find_regexp->isChecked())
           cr = d->document()->find (QRegularExpression (d->text_to_search), d->textCursor().position(), get_search_options() | QTextDocument::FindBackward);
#endif

#if QT_VERSION < 0x050500
       if (menu_find_regexp->isChecked())
          cr = d->document()->find (QRegExp (d->text_to_search), d->textCursor().position(), get_search_options() | QTextDocument::FindBackward);
#endif

     if (! menu_find_regexp->isChecked())
         cr = d->document()->find (d->text_to_search,
                                   d->textCursor(),
                                   get_search_options() | QTextDocument::FindBackward);


      if (! cr.isNull())
          d->setTextCursor (cr);
     }
  else
  if (main_tab_widget->currentIndex() == idx_tab_learn)
      man_find_prev();
  else
  if (main_tab_widget->currentIndex() == idx_tab_tune)
      opt_shortcuts_find_prev();
  else
  if (main_tab_widget->currentIndex() == idx_tab_fman)
      fman_find_prev();
}


void CTEA::search_mark_all()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  int darker_val = settings->value ("darker_val", 100).toInt();

  QString text_color = hash_get_val (global_palette, "text", "black");
  QString back_color = hash_get_val (global_palette, "background", "white");
  QString t_text_color = QColor (text_color).darker(darker_val).name();
  QString t_back_color = QColor (back_color).darker(darker_val).name();

  bool cont_search = true;

  int pos_save = d->textCursor().position();

  d->selectAll();

  QTextCharFormat f = d->currentCharFormat();
  f.setBackground (QColor (t_back_color));
  f.setForeground (QColor (t_text_color));
  d->mergeCurrentCharFormat (f);

  d->textCursor().clearSelection();

  int from;

  if (settings->value ("find_from_cursor", "1").toBool())
      from = d->textCursor().position();
  else
      from = 0;

  d->text_to_search = fif_get_text();
  QTextCursor cr;

  while (cont_search)
        {

#if (QT_VERSION_MAJOR < 5)
         if (menu_find_regexp->isChecked())
            cr = d->document()->find (QRegExp (d->text_to_search), from, get_search_options());
#else
         if (menu_find_regexp->isChecked())
            cr = d->document()->find (QRegularExpression (d->text_to_search), from, get_search_options());
#endif
         else
         if (menu_find_fuzzy->isChecked()) //fuzzy search
            {
             int pos = str_fuzzy_search (d->toPlainText(), d->text_to_search, from, settings->value ("fuzzy_q", "60").toInt());
             if (pos != -1)
                {
                 //set selection:
                 cr = d->textCursor();
                 cr.setPosition (pos, QTextCursor::MoveAnchor);
                 cr.movePosition (QTextCursor::Right, QTextCursor::KeepAnchor, d->text_to_search.length());

                 if (! cr.isNull())
                     d->setTextCursor (cr);
                }
             else
                 cont_search = false;
            }
         else //normal search
             cr = d->document()->find (d->text_to_search, from, get_search_options());

         if (! cr.isNull())
            {
             d->setTextCursor (cr);
             QTextCharFormat fm = cr.blockCharFormat();
             fm.setBackground (QColor (hash_get_val (global_palette, "backgroundmark", "red")));
             fm.setForeground (QColor (hash_get_val (global_palette, "foregroundmark", "blue")));

             cr.mergeCharFormat (fm);
             d->setTextCursor (cr);
            }
         else
             cont_search = false;

         from = d->textCursor().position();
        }

  d->document()->setModified (false);
  d->goto_pos (pos_save);
}


void CTEA::search_unmark()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  int darker_val = settings->value ("darker_val", 100).toInt();

  QString text_color = hash_get_val (global_palette, "text", "black");
  QString back_color = hash_get_val (global_palette, "background", "white");

  QString t_text_color = QColor (text_color).darker(darker_val).name();
  QString t_back_color = QColor (back_color).darker(darker_val).name();

  d->selectAll();

  QTextCharFormat f =  d->currentCharFormat();
  f.setBackground (QColor (t_back_color));
  f.setForeground (QColor (t_text_color));
  d->mergeCurrentCharFormat (f);
  d->textCursor().clearSelection();
}


void CTEA::search_in_files_results_dclicked (QListWidgetItem *item)
{
  documents->open_file_triplex (item->text());
  main_tab_widget->setCurrentIndex (idx_tab_edit);
}


void CTEA::search_in_files()
{
  last_action = sender();

  if (main_tab_widget->currentIndex() != idx_tab_fman)
     return;

  QString text_to_search = fif_get_text();

  if (text_to_search.isEmpty())
     return;

  QStringList lresult;
  QString charset = cb_fman_codecs->currentText();
  QString path = fman->dir.path();

  CFTypeChecker fc;

  progress_show();

  log->log (tr ("Getting files list..."));
  qApp->processEvents();

  CFilesList lf;
  lf.get (path);

  log->log (tr ("Searching..."));
  qApp->processEvents();

  //pb_status->show();

  progress_show();

  pb_status->setRange (0, lf.list.size());
  pb_status->setFormat (tr ("%p% completed"));
  pb_status->setTextVisible (true);

  for (int i = 0; i < lf.list.size(); i++)
      {
       if (i % 100 == 0)
           qApp->processEvents();

       if (boring)
           break;

       pb_status->setValue (i);

       QString fname = lf.list[i];

       if (! fc.check (fname))
          continue;

       log->log (fname);

       CTio *tio = documents->tio_handler.get_for_fname (fname);
       tio->charset = charset;

       if (! tio->load (fname))
           log->log (tr ("cannot open %1 because of: %2")
                         .arg (fname)
                         .arg (tio->error_string));

       Qt::CaseSensitivity cs = Qt::CaseInsensitive;
       if (menu_find_case->isChecked())
          cs = Qt::CaseSensitive;

       int index = tio->data.indexOf (text_to_search, 0, cs);
       if (index != -1)
          lresult.append (fname + "*" + charset + "*" + QString::number (index));
      }

  progress_hide();

  CTextListWnd *w = new CTextListWnd (tr ("Search results"), tr ("Files"));
  w->move (this->x(), this->y());

  w->list->addItems (lresult);

  connect (w->list, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
           this, SLOT(search_in_files_results_dclicked(QListWidgetItem*)));

  w->resize (width() - 10, (int) height() / 2);
  w->show();
}


void CTEA::search_whole_words_mode()
{
  menu_find_fuzzy->setChecked (false);
}


void CTEA::search_from_cursor_mode()
{
  settings->setValue ("find_from_cursor", menu_find_from_cursor->isChecked());
}


void CTEA::search_regexp_mode()
{
  menu_find_fuzzy->setChecked (false);
}


void CTEA::search_fuzzy_mode()
{
  menu_find_whole_words->setChecked (false);
  menu_find_regexp->setChecked (false);;
}


void CTEA::search_replace_with()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
     d->put (fif_get_text());
}


void CTEA::search_replace_all()
{
  last_action = sender();

  Qt::CaseSensitivity cs = Qt::CaseInsensitive;
  if (menu_find_case->isChecked())
     cs = Qt::CaseSensitive;

  QStringList l = fif_get_text().split ("~");
  if (l.size() < 2)
     return;

  if (main_tab_widget->currentIndex() == idx_tab_edit)
     {
      CDocument *d = documents->get_current();
      if (! d)
         return;

      QString s = d->get();

#if (QT_VERSION_MAJOR < 5)
      if (menu_find_regexp->isChecked())
         s = s.replace (QRegExp (l[0]), l[1]);
      else
          s = s.replace (l[0], l[1], cs);
#else

      if (menu_find_regexp->isChecked())
         s = s.replace (QRegularExpression (l[0]), l[1]);
      else
          s = s.replace (l[0], l[1], cs);

#endif

      d->put (s);
     }
  else
      if (main_tab_widget->currentIndex() == idx_tab_fman)
         {
          QStringList sl = fman->get_sel_fnames();

          if (sl.size() < 1)
             return;

          char *charset = cb_fman_codecs->currentText().toLatin1().data();

          for (QList <QString>::iterator fname = sl.begin(); fname != sl.end(); ++fname)
              {
               QString f = qstring_load ((*fname), charset);
               QString r;

#if (QT_VERSION_MAJOR < 5)

               if (menu_find_regexp->isChecked())
                  r = f.replace (QRegExp (l[0]), l[1]);
               else
                  r = f.replace (l[0], l[1], cs);

#else

               if (menu_find_regexp->isChecked())
                  r = f.replace (QRegularExpression (l[0]), l[1]);
               else
                  r = f.replace (l[0], l[1], cs);

#endif

               qstring_save ((*fname), r, charset);
               log->log (tr ("%1 is processed and saved").arg ((*fname)));
              }
        }
}


void CTEA::search_replace_all_at_ofiles()
{
  last_action = sender();

  QStringList l = fif_get_text().split ("~");
  if (l.size() < 2)
     return;

  int c = documents->items.size();
  if (c == 0)
     return;

  Qt::CaseSensitivity cs = Qt::CaseInsensitive;
  if (menu_find_case->isChecked())
     cs = Qt::CaseSensitive;

  for (vector <size_t>::size_type i = 0; i < documents->items.size(); i++)
      {
       CDocument *d = documents->items[i];
       QString s;

#if QT_VERSION < 0x050000

       if (menu_find_regexp->isChecked())
          s = d->toPlainText().replace (QRegExp (l[0]), l[1]);
       else
           s = d->toPlainText().replace (l[0], l[1], cs);

#else

       if (menu_find_regexp->isChecked())
          s = d->toPlainText().replace (QRegularExpression (l[0]), l[1]);
       else
           s = d->toPlainText().replace (l[0], l[1], cs);

#endif

       d->selectAll();
       d->put (s);
      }
}


/*
===================
Fn menu callbacks
===================
*/


void CTEA::fn_repeat()
{
  if (last_action)
     qobject_cast<QAction *>(last_action)->trigger();
}


void CTEA::fn_scale_image()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QString fname = d->get_filename_at_cursor();

  if (! is_image (fname))
     return;

  QImage source (fname);
  if (source.isNull())
     return;

  QString t = fif_get_text();
  if (t.indexOf ("~") == -1)
     return;

  QFileInfo fi (fname);

  QStringList params = t.split ("~");

  if (params.size() < 2)
     {
      log->log (tr("Incorrect parameters at FIF"));
      return;
     }

  QString fnameout = params[0].replace ("%filename", fi.fileName());
  fnameout = fnameout.replace ("%basename", fi.baseName());
  fnameout = fnameout.replace ("%s", fname);

  fnameout = fnameout.replace ("%ext", fi.suffix());
  fnameout = fi.absolutePath() + "/" + fnameout;

  bool scale_by_side = true;

  if (params[1].indexOf("%") != -1)
     scale_by_side = false;

  int side = 800;
  int percent = 100;

  if (scale_by_side)
     side = params[1].toInt();
  else
      {
       params[1].chop (1);
       percent = params[1].toInt();
      }

  Qt::TransformationMode transformMode = Qt::FastTransformation;
  if (settings->value ("img_filter", 0).toBool())
     transformMode = Qt::SmoothTransformation;

  int quality = settings->value ("img_quality", "-1").toInt();

  if (settings->value ("cb_exif_rotate", 1).toBool())
     {
      int exif_orientation = get_exif_orientation (fname);

      QTransform transform;
      qreal angle = 0;

      if (exif_orientation == 3)
         angle = 180;
      else
      if (exif_orientation == 6)
         angle = 90;
      else
      if (exif_orientation == 8)
         angle = 270;

      if (angle != 0)
         {
          transform.rotate (angle);
          source = source.transformed (transform);
         }
     }

  QImage dest;

  if (scale_by_side && side)
     dest = image_scale_by (source, true, side, transformMode);
  else
      if (percent)
         dest = image_scale_by (source, false, percent, transformMode);

  QString fmt (settings->value ("output_image_fmt", "jpg").toString());

  fnameout = change_file_ext (fnameout, fmt);

  if (! dest.save (fnameout, fmt.toLatin1().constData(), quality))
     log->log (tr("Cannot save: %1").arg (fnameout));
  else
      log->log (tr("Saved: %1").arg (fnameout));
}


void CTEA::fn_use_snippet()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QAction *a = qobject_cast<QAction *>(sender());
  QString s = qstring_load (a->data().toString());
  if (s.isEmpty())
     return;

  if (s.contains ("%s"))
     s = s.replace ("%s", d->get());

  d->put (s);
}


void CTEA::view_use_keyboard()
{
  last_action = sender();

  QAction *a = qobject_cast<QAction *>(sender());
  QString fname = a->data().toString();

  QWidget *w = create_keyboard (fname);
  if (w)
     w->show();
}


#if QT_VERSION >= 0x051400

void CTEA::view_preview_md()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  //qDebug() << d->toPlainText();

  md_viewer.setMarkdown (d->toPlainText());
  md_viewer.show();

}
#endif

void CTEA::fn_run_script()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QAction *a = qobject_cast<QAction *>(sender());

  QString fname = a->data().toString();
  QString ext = file_get_ext (fname);

  if (! d->textCursor().hasSelection())
     return;

  QString intrp;

  if (ext == "rb")
     intrp = "ruby";
  else
  if (ext == "py")
     intrp = "python";
  else
  if (ext == "pl")
     intrp = "perl";
  else
  if (ext == "sh")
     intrp = "sh";
  else
  if (ext == "lua")
     intrp = "lua";
  else
  if (ext == "bat" || ext == "btm"  || ext == "cmd")
     intrp = "cmd.exe";

  if (intrp.isEmpty())
      return;

  qstring_save (fname_tempfile, d->get());
  qstring_save (fname_tempparamfile, fif_get_text());

  QString command = QString ("%1 %2 %3 %4").arg (
                             intrp).arg (
                             fname).arg (
                             fname_tempfile).arg (
                             fname_tempparamfile);

  QProcess *process = new QProcess (this);
  connect(process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(cb_script_finished(int,QProcess::ExitStatus)));

  //process->start (command, QStringList());

#if QT_VERSION >= 0x060000
  process->startCommand (command);
#else
  process->start (command);
#endif

}


void CTEA::cb_script_finished (int exitCode, QProcess::ExitStatus exitStatus)
{
  CDocument *d = documents->get_current();
  if (! d)
     return;

  QString s = qstring_load (fname_tempfile);
  if (! s.isEmpty())
     d->put(s);

  QFile f (fname_tempfile);
  f.remove();
  f.setFileName (fname_tempparamfile);
  f.remove();
}


void CTEA::fn_use_table()
{
  last_action = sender();
  QAction *a = qobject_cast<QAction *>(sender());

  if (main_tab_widget->currentIndex() == idx_tab_edit)
     {
      CDocument *d = documents->get_current();
      if (! d)
         return;

      QString text;

      if (d->textCursor().hasSelection())
         text = d->get();
      else
          text = d->toPlainText();

      if (d->textCursor().hasSelection())
         d->put (apply_table (text, a->data().toString(), menu_find_regexp->isChecked()));
      else
         d->setPlainText (apply_table (text, a->data().toString(), menu_find_regexp->isChecked()));
     }
  else
      if (main_tab_widget->currentIndex() == idx_tab_fman)
         {
          QStringList sl = fman->get_sel_fnames();

          if (sl.size() < 1)
             return;

          char *charset = cb_fman_codecs->currentText().toLatin1().data();

          for (QList <QString>::const_iterator fname = sl.begin(); fname != sl.end(); ++fname)
              {
               QString f = qstring_load ((*fname), charset);
               QString r = apply_table (f, a->data().toString(), menu_find_regexp->isChecked());
               qstring_save ((*fname), r, charset);
               log->log (tr ("%1 is processed and saved").arg ((*fname)));
              }
         }
}


void CTEA::fn_insert_loremipsum()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
     d->put (qstring_load (":/text-data/lorem-ipsum"));
}


void CTEA::fn_insert_template_tea()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
     d->put (qstring_load (":/text-data/template-teaproject"));
}


void CTEA::fn_insert_template_html()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
     d->put (qstring_load (":/text-data/template-html"));
}


void CTEA::fn_insert_template_html5()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
     d->put (qstring_load (":/text-data/template-html5"));
}


void CTEA::fn_insert_cpp()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
     d->put (qstring_load (":/text-data/tpl_cpp.cpp"));
}


void CTEA::fn_insert_c()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
     d->put (qstring_load (":/text-data/tpl_c.c"));
}


void CTEA::fn_insert_date()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
     d->put (QDate::currentDate ().toString (settings->value("date_format", "dd/MM/yyyy").toString()));
}


void CTEA::fn_insert_time()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
     d->put (QTime::currentTime ().toString (settings->value("time_format", "hh:mm:ss").toString()));
}


void CTEA::fn_case_up()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
      d->put (d->get().toUpper());
}


void CTEA::fn_case_down()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
      d->put (d->get().toLower());
}


void CTEA::fn_case_inverse()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
      return;

  if (! d->has_selection())
     return;

  QString s = d->get();

  for (int i = 0; i < s.size(); i++)
      {
       if (s[i].isUpper())
          s[i] = s[i].toLower();
       else
           s[i] = s[i].toUpper();

      }

      d->put (s);
}


void CTEA::fn_case_cap_sentences()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (!d)
     return;

  QString t = d->toPlainText();

  bool cap = false;

  for (int i = 0; i < t.size(); i++)
      {
       QChar c = t.at(i);

       if (c.isDigit())
          {
           cap = false; //probably timecode
           continue;
          }

       if (c == '.' || c == '?' || c == '!')
          {
           cap = true;
           continue;
          }

       if (t.at(i).isLetter() && cap)
          {
           t[i] = t.at(i).toUpper();
           cap = false;
          }
      }


  d->put (t);
}



void CTEA::fn_sort_casecare()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
      d->put (qstringlist_process (d->get(), fif_get_text(), QSTRL_PROC_FLT_WITH_SORTCASECARE));
}


void CTEA::fn_sort_casecareless()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
     d->put (qstringlist_process (d->get(), fif_get_text(), QSTRL_PROC_FLT_WITH_SORTNOCASECARE));
}


void CTEA::fn_sort_casecare_sep()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
      d->put (qstringlist_process (d->get(), fif_get_text(), QSTRL_PROC_FLT_WITH_SORTCASECARE_SEP));
}


void CTEA::fn_sort_length()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
      d->put (qstringlist_process (d->get(), fif_get_text(), QSTRL_PROC_FLT_WITH_SORTLEN));
}


void CTEA::fn_flip_a_list()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
      d->put (qstringlist_process (d->get(), fif_get_text(), QSTRL_PROC_LIST_FLIP));
}


void CTEA::fn_flip_a_list_sep()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
     d->put (qstringlist_process (d->get(), fif_get_text(), QSTRL_PROC_LIST_FLIP_SEP));
}


int latex_table_sort_col;

bool latex_table_sort_fn (const QStringList &l1, const QStringList &l2)
{
  return l1.at (latex_table_sort_col) < l2.at (latex_table_sort_col);
}


void CTEA::fn_cells_latex_table_sort_by_col_abc()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QString t = d->get();

  if (t.isEmpty())
     return;

  QStringList fiftxt = fif_get_text().split("~");

  if (fiftxt.size() < 2)
     return;

  QString sep = fiftxt[0];

  latex_table_sort_col = fiftxt[1].toInt();

  if (t.indexOf (sep) == -1)
     return;

  QStringList sl_temp = t.split (QChar::ParagraphSeparator);

  QList <QStringList> output;

  for (QList <QString>::iterator s = sl_temp.begin(); s != sl_temp.end(); ++s)
      {
       if (! s->isEmpty())
          {
           QStringList sl_parsed = s->split (sep);
           if (latex_table_sort_col + 1 <= sl_parsed.size())
              output.append (sl_parsed);
          }
      }

  std::sort (output.begin(), output.end(), latex_table_sort_fn);

  sl_temp.clear();

  for (int i = 0; i < output.size(); i++)
      {
       sl_temp.append (output.at(i).join (sep));
      }

  t = sl_temp.join ("\n");

  d->put (t);
}


void CTEA::fn_cells_swap_cells()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
      return;

  QStringList fiftxt = fif_get_text().split("~");

  if (fiftxt.size() < 3)
     return;

  int col1 = fiftxt[1].toInt();
  int col2 = fiftxt[2].toInt();

  QString sep = fiftxt[0];

  QString t = d->get();

  if (t.isEmpty())
     return;

  if (t.indexOf (sep) == -1)
     return;

  int imax = int (fmax (col1, col2));

  QStringList sl_temp = t.split (QChar::ParagraphSeparator);

  QList <QStringList> output;

  for (QList <QString>::iterator v = sl_temp.begin(); v != sl_temp.end(); ++v)
      {
       if (! v->isEmpty())
          {
           QStringList sl_parsed = v->split (sep);
           if (imax + 1 <= sl_parsed.size())
              {
               strlist_swap (sl_parsed, col1, col2);
               output.append (sl_parsed);
              }
          }
      }

  sl_temp.clear();

  for (int i = 0; i < output.size(); i++)
       sl_temp.append (output.at(i).join (sep));

  t = sl_temp.join ("\n");

  d->put (t);
}


void CTEA::fn_cells_delete_by_col()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
      return;

  QStringList fiftxt = fif_get_text().split("~");

  if (fiftxt.size() < 2)
     return;

  int col1 = fiftxt[1].toInt();

  QString sep = fiftxt[0];

  QString t = d->get();

  if (t.isEmpty())
      return;

  if (t.indexOf (sep) == -1)
     return;

  QStringList sl_temp = t.split (QChar::ParagraphSeparator);

  QList <QStringList> output;

  for (QList <QString>::iterator v = sl_temp.begin(); v != sl_temp.end(); ++v)
      {
       if (! v->isEmpty())
          {
           QStringList sl_parsed = v->split (sep);
           if (col1 + 1 <= sl_parsed.size())
              {
               sl_parsed.removeAt (col1);
               output.append (sl_parsed);
              }
          }
      }

  sl_temp.clear();

  for (int i = 0; i < output.size(); i++)
       sl_temp.append (output.at(i).join (sep));

  t = sl_temp.join ("\n");

  d->put (t);
}


void CTEA::fn_cells_copy_by_col()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
      return;

  QStringList fiftxt = fif_get_text().split("~");

  if (fiftxt.size() < 2)
      return;

  QString sep = fiftxt[0];

  int col1 = fiftxt[1].toInt();
  int col2 = 0;

  if (fiftxt.size() == 3)
     col2 = fiftxt[2].toInt();

  QString t = d->get();

  if (t.isEmpty())
      return;

  if (t.indexOf (sep) == -1)
      return;

  QStringList sl_temp = t.split (QChar::ParagraphSeparator);

  QList <QStringList> output;

  if (col2 > 0)
  for (QList <QString>::iterator v = sl_temp.begin(); v != sl_temp.end(); ++v)
      {
       if (! v->isEmpty())
          {
           QStringList sl_parsed = v->split (sep);
           if (col2 + 1 <= sl_parsed.size())
              {
               QStringList tl = sl_parsed.mid (col1, col2 - col1 + 1);
               output.append (tl);
              }
          }
      }
  else
  for (QList <QString>::iterator v = sl_temp.begin(); v != sl_temp.end(); ++v)
      {
       if (! v->isEmpty())
          {
           QStringList sl_parsed = v->split (sep);
           if (col1 + 1 <= sl_parsed.size())
              {
               QStringList tl = sl_parsed.mid (col1, 1);
               output.append (tl);
              }
           }
      }

  sl_temp.clear();

  for (int i = 0; i < output.size(); i++)
       sl_temp.append (output.at(i).join (sep));

  t = sl_temp.join ("\n");

  QApplication::clipboard()->setText (t);
}


void CTEA::fn_filter_rm_duplicates()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
     d->put (qstringlist_process (d->get(), fif_get_text(), QSTRL_PROC_FLT_REMOVE_DUPS));
}


void CTEA::fn_filter_rm_empty()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
      d->put (qstringlist_process (d->get(), fif_get_text(), QSTRL_PROC_FLT_REMOVE_EMPTY));
}


void CTEA::fn_filter_rm_less_than()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
     d->put (qstringlist_process (d->get(), fif_get_text(), QSTRL_PROC_FLT_LESS));
}


void CTEA::fn_filter_rm_greater_than()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
     d->put (qstringlist_process (d->get(), fif_get_text(), QSTRL_PROC_FLT_GREATER));
}


void CTEA::fn_filter_delete_before_sep()
{
  last_action = sender();
  fn_filter_delete_by_sep (true);
}


void CTEA::fn_filter_delete_after_sep()
{
  last_action = sender();
  fn_filter_delete_by_sep (false);
}


void CTEA::fn_filter_with_regexp()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
      d->put (qstringlist_process (d->get(), fif_get_text(), QSTRL_PROC_FLT_WITH_REGEXP));
}


void CTEA::fn_filter_by_repetitions()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QString result;

  QString pattern = fif_get_text();

  vector <int> positions;

  for (int i = 0; i < pattern.size(); ++i)
      {
       if (pattern[i] == '1')
          positions.push_back (i);
      }

  QStringList words = d->get().split (QChar::ParagraphSeparator);

  for (int i = 0; i < words.size(); ++i)
      {
       QString wrd = words[i];

       if (pattern.size() > wrd.size())
          continue;

       QChar ch = wrd [positions[0]];

       size_t count = 0;

       for (size_t j = 0; j < positions.size(); ++j)
           {
            if (wrd[positions[j]] == ch)
                count++;
           }

       if (count == positions.size())
          {
           result += wrd;
           result += "\n";
          }
      }

      if (! result.isEmpty())
         d->put (result);
}


void CTEA::fn_math_evaluate()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QString s = d->get();
  std::string utf8_text = s.toUtf8().constData();
  double f = calculate (utf8_text);
  QString fs = s.setNum (f);

  log->log (fs);
}


void CTEA::fn_math_number_arabic_to_roman()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
     d->put (arabicToRoman (d->get().toUInt()));
}


void CTEA::fn_math_number_roman_to_arabic()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
     d->put (QString::number(get_arab_num (d->get().toUpper().toStdString())));
}


void CTEA::fn_math_number_dec_to_bin()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
     d->put (int_to_binary (d->get().toInt()));
}


void CTEA::fn_math_number_bin_to_dec()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
     d->put (QString::number (bin_to_decimal (d->get())));
}


void CTEA::fn_math_number_flip_bits()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (!d)
     return;

  QString s = d->get();
  for (int i = 0; i < s.size(); i++)
      {
       if (s[i] == '1')
          s[i] = '0';
       else
       if (s[i] == '0')
          s[i] = '1';
      }

  d->put (s);
}


void CTEA::fn_math_sum_by_last_col()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
      return;

  QString t = d->get();

  if (t.isEmpty())
      return;

  t = t.replace (",", ".");

  double sum = 0.0;

  QStringList l = t.split (QChar::ParagraphSeparator);

  for (int i = 0; i < l.size(); i++)
      {
       if (l[i].isNull())
          continue;

       if (l[i].startsWith ("//") ||  l[i].startsWith ("#") || l[i].startsWith (";"))
          continue;

       QStringList lt = l[i].split (" ");
       if (lt.size() > 0)
          {
           QString s = lt.at (lt.size() - 1);
           if (! s.isNull())
              {
               std::string utf8_text = s.toUtf8().constData();
               double f = calculate (utf8_text);
               sum += f;
              }
          }
      }

  log->log (tr ("sum: %1").arg (sum));
}


void CTEA::fn_math_enum()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QStringList source = d->get().split (QChar::ParagraphSeparator);

  int pad = 0;
  int end = source.size() - 1;
  int step = 1;
  QString result;
  QString prefix;

  QStringList params = fif_get_text().split ("~");

  if (params.size() > 0)
     step = params[0].toInt();

  if (params.size() > 1)
     pad = params[1].toInt();

  if (params.size() > 2)
     prefix = params[2];

  if (step == 0)
     step = 1;

  for (int c = 0; c <= end; c++)
      {
       QString n;
       n = n.setNum (((c + 1) * step));
       if (pad != 0)
          n = n.rightJustified (pad, '0');

       result = result + n + prefix +  source.at(c) + '\n';
      }

  d->put (result);
}


//UTF-16BE, UTF-32BE
//′
//#define UQS 8242
//″
//#define UQD 8243
//°
//#define UQDG 176

//degrees minutes seconds: 40° 26′ 46″ N 79° 58′ 56″ W
//to
//decimal degrees: 40.446° N 79.982° W
void CTEA::fn_math_number_dms2dc()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
      return;

  QString t = d->get();
  t = t.remove (" ");

  t = t.replace ('\'', QChar (UQS));
  t = t.replace ('"', QChar (UQD));

  QChar north_or_south = ('N');
  if (t.contains ('S'))
     north_or_south = 'S';

  QChar east_or_west = ('E');
  if (t.contains ('W'))
     east_or_west = 'W';

  QStringList l = t.split (north_or_south);

  QString latitude = l[0];
  QString longtitude = l[1];

//  qDebug() << "latitude " << latitude;
//  qDebug() << "longtitude " << longtitude;

  int iqdg = latitude.indexOf (QChar (UQDG));
  int iqs = latitude.indexOf (QChar (UQS));
  int iqd = latitude.indexOf (QChar (UQD));

  QString degrees1 = latitude.left (iqdg);
  QString minutes1 = latitude.mid (iqdg + 1, iqs - iqdg - 1);
  QString seconds1 = latitude.mid (iqs + 1, iqd - iqs - 1);

  double lat_decimal_degrees = degrees1.toDouble() + (double) (minutes1.toDouble() / 60) + (double) (seconds1.toDouble() / 3600);
  QString lat_decimal_degrees_N = QString::number (lat_decimal_degrees, 'f', 3) + QChar (UQDG) + north_or_south;

  iqdg = longtitude.indexOf (QChar (UQDG));
  iqs = longtitude.indexOf (QChar (UQS));
  iqd = longtitude.indexOf (QChar (UQD));

  degrees1 = longtitude.left (iqdg);
  minutes1 = longtitude.mid (iqdg + 1, iqs - iqdg - 1);
  seconds1 = longtitude.mid (iqs + 1, iqd - iqs - 1);

  double longt_decimal_degrees = degrees1.toDouble() + (double) (minutes1.toDouble() / 60) + (double) (seconds1.toDouble() / 3600);

  QString longt_decimal_degrees_N = QString::number (longt_decimal_degrees, 'f', 3) + QChar (UQDG) + east_or_west;

  log->log (lat_decimal_degrees_N + " " + longt_decimal_degrees_N);
//  qDebug() << "decimal_degrees " << decimal_degrees;
//  qDebug() << "decimal_degrees_N " << decimal_degrees_N;
}

/*
degrees = floor (decimal_degrees)
minutes = floor (60 * (decimal_degrees - degrees))
seconds = 3600 * (decimal_degrees - degrees) - 60 * minites

*/

//decimal degrees: 40.446° N 79.982° W
//to
//degrees minutes seconds: 40° 26′ 46″ N 79° 58′ 56″ W

void CTEA::fn_math_number_dd2dms()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
      return;

  QString t = d->get();
  t = t.remove (" ");
  t = t.remove (QChar (UQDG));

  QChar north_or_south = ('N');
  if (t.contains ('S'))
     north_or_south = 'S';

  QChar east_or_west = ('E');
  if (t.contains ('W'))
     east_or_west = 'W';

  QStringList l = t.split (north_or_south);

  QString latitude = l[0];

#if QT_VERSION < 0x050000
  QString longtitude = l[1].remove (QRegExp("[a-zA-Z\\s]"));
#else
  QString longtitude = l[1].remove (QRegularExpression("[a-zA-Z\\s]"));
#endif

  double degrees = floor (latitude.toDouble());
  double minutes = floor (60 * (latitude.toDouble() - degrees));
  double seconds = round (3600 * (latitude.toDouble() - degrees) - 60 * minutes);

  double degrees2 = floor (longtitude.toDouble());
  double minutes2 = floor (60 * (longtitude.toDouble() - degrees2));
  double seconds2 = round (3600 * (longtitude.toDouble() - degrees2) - 60 * minutes2);


  QString result = QString::number (degrees) + QChar (UQDG) + QString::number (minutes) + QChar (UQS) +
                   QString::number (seconds) + QChar (UQD) +
                   north_or_south + " " +
                   QString::number (degrees2) + QChar (UQDG) + QString::number (minutes2)
                    + QChar (UQS) + QString::number (seconds2) + QChar (UQD) +
                   east_or_west;

  log->log (result);
}


#if QT_VERSION >= 0x060000

void CTEA::fn_math_srt_shift()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QString s_msecs = fif_get_text();
  if (s_msecs.isEmpty())
     return;

  int msecs = s_msecs.toInt();

  QString text = d->get();
  if (text.isEmpty())
      return;

  QString output = text;

  QRegularExpression re ("\\d{1,}:\\d{1,}:\\d{1,}(\\,|\\.)\\d{1,}");

  QString format;

  if (d->file_name.endsWith (".sbv"))
     format = "h:mm:ss.zzz";
  else
      format = "hh:mm:ss,zzz";


  QRegularExpressionMatchIterator i = re.globalMatch (text);
  while (i.hasNext())
        {
         QRegularExpressionMatch match = i.next();
         QString t_in (match.captured());

         QTime tm = QTime::fromString (t_in, format);

         if (! tm.isValid())
            continue;

         tm = tm.addMSecs (msecs);

         QString t_out = tm.toString (format);

         output.replace (t_in, t_out);
        }

      d->put (output);
}
#endif


void CTEA::fn_morse_from_ru()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
     d->put (morse_from_lang (d->get().toUpper(), "ru"));
}


void CTEA::fn_morse_to_ru()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
     d->put (morse_to_lang (d->get(), "ru"));
}


void CTEA::fn_morse_from_en()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
     d->put (morse_from_lang (d->get().toUpper(), "en"));
}


void CTEA::fn_morse_to_en()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
     d->put (morse_to_lang (d->get(), "en"));
}


void CTEA::fn_analyze_text_stat()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  bool b_sel = d->textCursor().hasSelection();

  QString s;

  if (b_sel)
     s = d->get();
  else
      s = d->toPlainText();

  int c = s.length();
  int purechars = 0;
  int lines = 1;

  for (int i = 0; i < c; ++ i)
      {
       QChar ch = s.at (i);

       if (ch.isLetterOrNumber() || ch.isPunct())
          purechars++;

       if (! b_sel)
          {
           if (ch == '\n')
              lines++;
          }
       else
           if (ch == QChar::ParagraphSeparator)
              lines++;
      }


  QString result = tr ("chars: %1<br>chars without spaces: %2<br>lines: %3<br>author's sheets: %4")
                       .arg (QString::number (c))
                       .arg (QString::number (purechars))
                       .arg (QString::number (lines))
                       .arg (QString::number (c / 40000));

  log->log (result);
}


void CTEA::fn_analyze_extract_words()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QStringList w = d->get_words();

  CDocument *nd = documents->create_new();
  if (nd)
     nd->put (w.join("\n"));
}


void CTEA::fn_analyze_stat_words_lengths()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  unsigned long lengths[33] = { };

  QStringList w = d->get_words();

  for (int i =0; i < w.size(); i++)
      {
       int len = w.at(i).length();
       if (len <= 32)
          lengths[len]++;
      }

  QStringList l;

  QString col1 = tr ("Word length: ");
  QString col2 = tr ("Number:");

  l.append (col1 + col2);

  for (int i = 1; i <= 32; i++)
      {
       QString s = QString::number (i) + col1.fill ('_', col1.length()) + QString::number (lengths[i]);
       l.append (s);
      }

  CDocument *nd = documents->create_new();
  if (nd)
     nd->put (l.join("\n"));
}


void CTEA::fn_analyze_count()
{
  last_action = sender();
  count_substring (false);
}


void CTEA::fn_analyze_count_rx()
{
  last_action = sender();
  count_substring (true);
}


bool pr_bigger_than (const pair<const QString &,int> &a,
                     const pair<const QString &,int> &b)
{
  return (a.second > b.second);
}


bool pr_bigger_than_str (const pair<const QString &,int> &a,
                         const pair<const QString &,int> &b)
{
  return (a.first < b.first);
}


bool pr_bigger_than_str_len (const pair<const QString &,int> &a,
                             const pair<const QString &,int> &b)
{
  return (a.first.size() < b.first.size());
}


void CTEA::fn_analyze_get_words_count()
{
  last_action = sender();
  run_unitaz (0);
}


void CTEA::fn_analyze_unitaz_abc()
{
  last_action = sender();
  run_unitaz (1);
}


void CTEA::fn_analyze_unitaz_len()
{
  last_action = sender();
  run_unitaz (2);
}


void CTEA::fn_text_apply_to_each_line()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QStringList sl = d->get().split (QChar::ParagraphSeparator);
  QString t = fif_get_text();

  if (t.isEmpty())
     return;

  if (t.startsWith ("@@"))
     {
      QString fname = dir_snippets + QDir::separator() + t;

      if (! file_exists (fname))
         {
          log->log (tr ("snippet %1 is not exists").arg (fname));
          return;
         }

      t = t.remove (0, 2);
      t = qstring_load (fname);
     }

  for (QList <QString>::iterator i = sl.begin(); i != sl.end(); ++i)
      {
       QString ts (t);
       (*i) = ts.replace ("%s", (*i));
      }

  QString x = sl.join ("\n");

  d->put (x);
}


void CTEA::fn_text_reverse()
{
  last_action = sender();
  CDocument *d = documents->get_current();
  if (! d)
     return;

  QString s = d->get();

  if (! s.isEmpty())
      d->put (string_reverse (s));
}


void CTEA::fn_text_escape()
{
  last_action = sender();

  CDocument *d = documents->get_current();

#if (QT_VERSION_MAJOR < 5)
  if (d)
     d->put (QRegExp::escape (d->get()));
#else
  if (d)
     d->put (QRegularExpression::escape (d->get()));
#endif
}

void CTEA::fn_text_remove_formatting()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
     d->put (d->get().simplified());
}


void CTEA::fn_text_compress()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QString s = d->get();

  s = s.remove ('\n');
  s = s.remove ('\t');
  s = s.remove (' ');
  s = s.remove (QChar::ParagraphSeparator);
  d->put (s);
}


void CTEA::fn_text_compare_two_strings()
{
  last_action = sender();

  QStringList l = fif_get_text().split ("~");
  if (l.size() < 2)
     return;

  if (l[0].size() < l[1].size())
     return;

  QString s;

  for (int i = 0; i < l[0].size(); i++)
      {
       if (l[0][i] == l[1][i])
          s = QString::number (i + 1) + ": " + l[0][i] + " == " + l[1][i];
       else
           s = QString::number (i + 1) + ": " + l[0][i] + " != " + l[1][i];

       log->log (s);
      }
}


void CTEA::fn_text_remove_formatting_at_each_line()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
      d->put (qstringlist_process (d->get(), "", QSTRL_PROC_REMOVE_FORMATTING));
}


void CTEA::fn_text_remove_trailing_spaces()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QStringList sl = d->get().split (QChar::ParagraphSeparator);

  for (QList <QString>::iterator s = sl.begin(); s != sl.end(); ++s)
      {
       if (s->isEmpty())
          continue;

       if (s->at (s->size() - 1).isSpace())
          {
           int index = s->size() - 1;
           while (s->at (--index).isSpace())
                 ;

           s->truncate (index + 1);
          }
      }

  QString x = sl.join ("\n");

  d->put (x);
}


void CTEA::fn_text_anagram()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
      return;

  QString t = d->get();
  if (t.isEmpty())
     return;

  QString txt = anagram (t).join("\n");

  d = documents->create_new();
  if (d)
     d->put (txt);
}


void CTEA::fn_text_regexp_match_check()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
      return;

  QString t = d->get();
  if (t.isEmpty())
     return;

  QString fiftxt = fif_get_text();

#if QT_VERSION >= 0x050000
  QRegularExpression r (fiftxt);
  QRegularExpressionMatch match = r.match(t);
  if (match.hasMatch())
      log->log (tr ("matched"));
  else
      log->log (tr ("does not"));

#else
  QRegExp r (fiftxt);
  if (r.exactMatch(t))
     log->log (tr ("matched"));
  else
      log->log (tr ("does not"));

#endif
}



void CTEA::fn_quotes_to_angle()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QString source = d->get();
  if (! source.isEmpty())
     d->put (conv_quotes (source, "\u00AB", "\u00BB"));
}


void CTEA::fn_quotes_curly()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QString source = d->get();
  if (! source.isEmpty())
      d->put (conv_quotes (source, "\u201C", "\u201D"));
}


void CTEA::fn_quotes_tex_curly()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QString source = d->get();
  if (! source.isEmpty())
     d->put (conv_quotes (source, "``", "\'\'"));
}


void CTEA::fn_quotes_tex_angle_01()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QString source = d->get();
  if (! source.isEmpty())
     d->put (conv_quotes (source, "<<", ">>"));
}


void CTEA::fn_quotes_tex_angle_02()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QString source = d->get();
  if (! source.isEmpty())
     d->put (conv_quotes (source, "\\glqq", "\\grqq"));
}


#if defined (HUNSPELL_ENABLE) || defined (ASPELL_ENABLE)

void CTEA::fn_change_spell_lang()
{
  last_action = sender();

  QAction *a = qobject_cast<QAction *>(sender());
  settings->setValue ("spell_lang", a->data().toString());
  spellchecker->change_lang (a->data().toString());
  spellchecker->load_dict();

  fn_spell_check();
}


bool ends_with_badchar (const QString &s)
{
  if (s.endsWith ("\""))
     return true;

  if (s.endsWith ("»"))
     return true;

  if (s.endsWith ("\\"))
     return true;

  return false;
}


void CTEA::fn_spell_check()
{
  last_action = sender();

  if (spellchecker->modules_list.size() == 0)
     {
      QMessageBox::about (0, "!", QObject::tr ("Please set up spell checker dictionaries at\n Options - Functions page"));
      return;
     }

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QColor color_error = QColor (hash_get_val (global_palette, "error", "red"));

  QElapsedTimer time_start;
  time_start.start();

  pb_status->show();
  pb_status->setRange (0, d->toPlainText().size() - 1);
  pb_status->setFormat (tr ("%p% completed"));
  pb_status->setTextVisible (true);

  int i = 0;

  QTextCursor cr = d->textCursor();

  int pos = cr.position();
  int savepos = pos;

  QString text = d->toPlainText();
  int text_size = text.size();

//delete all underlines
  cr.setPosition (0);
  cr.movePosition (QTextCursor::End, QTextCursor::KeepAnchor);
  QTextCharFormat f = cr.blockCharFormat();
  f.setFontUnderline (false);
  cr.mergeCharFormat (f);

  cr.setPosition (0);
  cr.movePosition (QTextCursor::Start, QTextCursor::MoveAnchor);

  do
    {
     pos = cr.position();
     if (pos >= text_size)
        break;

     QChar c = text.at (pos);

     if (char_is_bad (c))
     while (char_is_bad (c))
           {
            cr.movePosition (QTextCursor::NextCharacter);

            pos = cr.position();

            if (pos < text_size)
               c = text.at (pos);
            else
                break;
           }

     cr.movePosition (QTextCursor::EndOfWord, QTextCursor::KeepAnchor);

     QString stext = cr.selectedText();
     if (! stext.isEmpty() && ends_with_badchar (stext)) //extend selection
        {
         cr.movePosition (QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
         stext = cr.selectedText();
        }

     if (! stext.isEmpty())
     if (! spellchecker->check (stext))
        {
         f = cr.blockCharFormat();
         f.setUnderlineStyle (QTextCharFormat::SpellCheckUnderline);
         f.setUnderlineColor (color_error);
         cr.mergeCharFormat (f);
        }

     i++;

     if (i % 512 == 0)
        pb_status->setValue (i);
    }
   while (cr.movePosition (QTextCursor::NextWord));

  cr.setPosition (savepos);
  d->document()->setModified (false);

  pb_status->hide();

  log->log (tr("elapsed milliseconds: %1").arg (time_start.elapsed()));
}


void CTEA::fn_spell_add_to_dict()
{
  last_action = sender();

  if (spellchecker->modules_list.size() == 0)
     {
      QMessageBox::about (0, "!", QObject::tr ("Please set up spell checker dictionaries at\n Options - Functions page"));
      return;
     }

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QTextCursor cr = d->textCursor();
  cr.select (QTextCursor::WordUnderCursor); //плохо работает
  spellchecker->add_to_user_dict (cr.selectedText());
}


void CTEA::fn_spell_remove_from_dict()
{
  last_action = qobject_cast<QAction *>(sender());

  if (spellchecker->modules_list.size() == 0)
     {
      QMessageBox::about (0, "!", QObject::tr ("Please set up spell checker dictionaries at\n Options - Functions page"));
      return;
     }


  CDocument *d = documents->get_current();
  if (! d)
     return;

  QTextCursor cr = d->textCursor();
  cr.select (QTextCursor::WordUnderCursor);
  spellchecker->remove_from_user_dict (cr.selectedText());
}


void CTEA::fn_spell_suggest_callback()
{
  last_action = sender();

  if (spellchecker->modules_list.size() == 0)
     {
      QMessageBox::about (0, "!", QObject::tr ("Please set up spell checker dictionaries at\n Options - Functions page"));
      return;
     }


  CDocument *d = documents->get_current();
  if (! d)
     return;

  QAction *a = qobject_cast<QAction *>(sender());
  QString new_text = a->data().toString();

  QTextCursor cr = d->textCursor();

  cr.select (QTextCursor::WordUnderCursor);
  QString s = cr.selectedText();
  if (s.isEmpty())
     return;

  if (s[0].isUpper())
     new_text[0] = new_text[0].toUpper();

  cr.insertText (new_text);
  d->setTextCursor (cr);
}


void CTEA::fn_spell_suggest()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QTextCursor cr = d->textCursor();
  cr.select (QTextCursor::WordUnderCursor);
  QString s = cr.selectedText();
  if (s.isEmpty())
     return;

  QStringList l = spellchecker->get_suggestions_list (s);

  QMenu *m = new QMenu (this);
  create_menu_from_list (this, m, l, SLOT (fn_spell_suggest_callback()));
  m->popup (mapToGlobal (d->cursorRect().topLeft()));
}

#endif


/*
====================
Cal menu
===================
*/

void CTEA::calendar_update()
{
  if (settings->value ("start_week_on_sunday", "0").toBool())
     calendar->setFirstDayOfWeek (Qt::Sunday);
  else
      calendar->setFirstDayOfWeek (Qt::Monday);

  int year = calendar->yearShown();
  int month = calendar->monthShown();

  QDate dbase (year, month, 1);

  QTextCharFormat format_past;
  QTextCharFormat format_future;
  QTextCharFormat format_normal;

  format_past.setFontStrikeOut (true);
  format_future.setFontUnderline (true);

  format_future.setForeground (Qt::red);
  format_future.setBackground (Qt::darkBlue);

  format_normal.setForeground (Qt::white);
  format_normal.setBackground (Qt::black);

  format_past.setForeground (Qt::white);
  format_past.setBackground (Qt::darkGray);


  int days_count = dbase.daysInMonth();

  for (int day = 1; day <= days_count; day++)
      {
       QDate date (year, month, day);
       QString sdate;

//      sdate = sdate.sprintf ("%02d-%02d-%02d", year, month, day);

       sdate += QString("%1").arg (year, 2, 10, QChar('0'));
       sdate += "-";
       sdate += QString("%1").arg (month, 2, 10, QChar('0'));
       sdate += "-";
       sdate += QString("%1").arg (day, 2, 10, QChar('0'));

       QString fname  = dir_days + "/" + sdate;

       if (file_exists (fname))
          {
           if (date < QDate::currentDate())
              calendar->setDateTextFormat (date, format_past);
           else
           if (date >= QDate::currentDate())
              calendar->setDateTextFormat (date, format_future);
          }
        else
            calendar->setDateTextFormat (date, format_normal);
      }
}

/*
void CTEA::create_moon_phase_algos()
{
  moon_phase_algos.insert (MOON_PHASE_TRIG2, tr ("Trigonometric 2"));
  moon_phase_algos.insert (MOON_PHASE_TRIG1, tr ("Trigonometric 1"));
  moon_phase_algos.insert (MOON_PHASE_CONWAY, tr ("Conway"));
  moon_phase_algos.insert (MOON_PHASE_LEUESHKANOV, tr ("Leueshkanov"));
}
*/

void CTEA::cal_moon_mode()
{
  calendar->moon_mode = ! calendar->moon_mode;
  calendar->do_update();
  settings->setValue ("moon_mode", calendar->moon_mode);
}


void CTEA::cal_set_date_a()
{
  date1 = calendar->selectedDate();
}


void CTEA::cal_set_date_b()
{
  date2 = calendar->selectedDate();
}


void CTEA::cal_add_days()
{
  QDate selected = calendar->selectedDate();
  selected = selected.addDays (fif_get_text().toInt());
  calendar->setSelectedDate (selected);
}


void CTEA::cal_add_months()
{
  QDate selected = calendar->selectedDate();
  selected = selected.addMonths (fif_get_text().toInt());
  calendar->setSelectedDate (selected);
}


void CTEA::cal_add_years()
{
  QDate selected = calendar->selectedDate();
  selected = selected.addYears (fif_get_text().toInt());
  calendar->setSelectedDate (selected);
}


void CTEA::cal_set_to_current()
{
  calendar->showToday();
  calendar->setSelectedDate (QDate::currentDate());
}


void CTEA::cal_gen_mooncal()
{
  int jdate1 = date1.toJulianDay();
  int jdate2 = date2.toJulianDay();

  QString s;

  QString date_format = settings->value("date_format", "dd/MM/yyyy").toString();

  for (int d = jdate1; d <= jdate2; d++)
      {
       QDate date = QDate::fromJulianDay (d);
       int moon_day = moon_phase_trig2 (date.year(), date.month(), date.day());

       s += date.toString (date_format);
       s += " = ";
       s += QString::number (moon_day);
       s += "\n";
      }

  CDocument *nd = documents->create_new();
  nd->put (s);
  main_tab_widget->setCurrentIndex (idx_tab_edit);
}


void CTEA::cal_diff_days()
{
  int days = date2.daysTo (date1);
  if (days < 0)
      days = ~ days;

  log->log (QString::number (days));
}


void CTEA::cal_remove()
{
  QString fname = dir_days + "/" + calendar->selectedDate().toString ("yyyy-MM-dd");
  QFile::remove (fname);
  calendar_update();
}


/*
===================
IDE menu callbacks
===================
*/


void CTEA::ide_run()
{
  last_action = sender();

  if (documents->hash_project.isEmpty())
     return;

  if (documents->fname_current_project.isEmpty())
     return;

  QFileInfo source_dir (documents->fname_current_project);

  QString dir_build = hash_get_val (documents->hash_project,
                                    "dir_build", source_dir.absolutePath());

  if (! path_is_abs (dir_build)) //dir is not absolute path
      dir_build = source_dir.absolutePath() + "/" + dir_build;

  QString command_run = hash_get_val (documents->hash_project, "command_run", "");

  QProcess *process  = new QProcess (this);
  process->setWorkingDirectory (dir_build);

  connect (process, SIGNAL(readyReadStandardOutput()), this, SLOT(process_readyReadStandardOutput()));
  process->setProcessChannelMode (QProcess::MergedChannels) ;


#if QT_VERSION >= 0x060000
  process->startCommand (command_run);
#else
  process->start (command_run);
#endif

  //process->start (command_run, QStringList());
}


void CTEA::ide_build()
{
  last_action = sender();

  if (documents->hash_project.isEmpty())
     return;

  if (documents->fname_current_project.isEmpty())
     return;

  QFileInfo source_dir (documents->fname_current_project);

  QString dir_build = hash_get_val (documents->hash_project, "dir_build", source_dir.absolutePath());

  if (! path_is_abs (dir_build)) //dir is not absolute path
      dir_build = source_dir.absolutePath() + "/" + dir_build;

  QString command_build = hash_get_val (documents->hash_project, "command_build", "make");

  QProcess *process  = new QProcess (this);
  process->setWorkingDirectory (dir_build);

  connect (process, SIGNAL(readyReadStandardOutput()), this, SLOT(process_readyReadStandardOutput()));

  process->setProcessChannelMode (QProcess::MergedChannels) ;
 // process->start (command_build, QStringList());

#if QT_VERSION >= 0x060000
  process->startCommand (command_build);
#else
  process->start (command_build);
#endif
}


void CTEA::ide_clean()
{
  last_action = sender();

  if (documents->hash_project.isEmpty())
      return;

  if (documents->fname_current_project.isEmpty())
      return;

  QFileInfo source_dir (documents->fname_current_project);

  QString dir_build = hash_get_val (documents->hash_project, "dir_build", source_dir.absolutePath());


  if (! path_is_abs (dir_build)) //dir is not absolute path
      dir_build = source_dir.absolutePath() + "/" + dir_build;

  QString command_clean = hash_get_val (documents->hash_project, "command_clean", "make");

  QProcess *process  = new QProcess (this);
  process->setWorkingDirectory (dir_build);

  connect (process, SIGNAL(readyReadStandardOutput()), this, SLOT(process_readyReadStandardOutput()));

  process->setProcessChannelMode (QProcess::MergedChannels) ;
 // process->start (command_clean, QStringList());

#if QT_VERSION >= 0x060000
  process->startCommand (command_clean);
#else
  process->start (command_clean);
#endif

}


void CTEA::ide_toggle_hs()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  if (file_exists (d->file_name))
      documents->open_file (toggle_fname_header_source (d->file_name), d->charset);
}


/*
===================
Nav menu callbacks
===================
*/


void CTEA::nav_save_pos()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
     d->position = d->textCursor().position();
}


void CTEA::nav_goto_pos()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QTextCursor cr = d->textCursor();
  cr.setPosition (d->position);
  d->setTextCursor (cr);
}


void CTEA::nav_goto_line()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QTextCursor cr = d->textCursor();
  cr.movePosition (QTextCursor::Start);
  cr.movePosition (QTextCursor::NextBlock, QTextCursor::MoveAnchor, fif_get_text().toInt() - 1);

  d->setTextCursor (cr);
  d->setFocus();
}


void CTEA::nav_goto_right_tab()
{
  last_action = sender();

  int i;

  if (tab_editor->currentIndex() == (tab_editor->count() - 1))
     i = 0;
  else
      i = tab_editor->currentIndex() + 1;

  if (tab_editor->count() > 0)
     tab_editor->setCurrentIndex (i);
}


void CTEA::nav_goto_left_tab()
{
  last_action = sender();

  int i;

  if (tab_editor->currentIndex() == 0)
     i = tab_editor->count() - 1;
  else
      i = tab_editor->currentIndex() - 1;

  if (tab_editor->count() > 0)
      tab_editor->setCurrentIndex (i);
}


void CTEA::nav_focus_to_fif()
{
  last_action = sender();
  fif->setFocus (Qt::OtherFocusReason);
}


void CTEA::nav_focus_to_editor()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
     d->setFocus (Qt::OtherFocusReason);
}


void CTEA::nav_labels_update_list()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  d->update_labels();
  update_labels_menu();
}


/*
===================
Fm menu callbacks
===================
*/


void CTEA::fman_multi_rename_zeropad()
{
  last_action = sender();

  QString fiftxt = fif_get_text();
  int finalsize = fiftxt.toInt();
  if (finalsize < 1)
     finalsize = 10;

  QStringList sl = fman->get_sel_fnames();

  if (sl.size() < 1)
     return;

  for (int i = 0; i < sl.size(); i++)
      {
       QString fname = sl[i];
       QFileInfo fi (fname);

       if (fi.exists() && fi.isWritable())
          {
           int zeroes_to_add = finalsize - fi.baseName().length();

           QString newname = fi.baseName();
           QString ext = file_get_ext (fname);

#if (QT_VERSION_MAJOR < 5)
           newname.remove(QRegExp("[a-zA-Z\\s]"));
#else
           newname.remove(QRegularExpression("[a-zA-Z\\s]"));
#endif

           if (newname.isEmpty())
              continue;

           QString pad = "0";
           pad = pad.repeated (zeroes_to_add);
           newname = pad + newname;

           QString newfpath (fi.path());
           newfpath.append ("/").append (newname);
           newfpath.append (".");
           newfpath.append (ext);

           QFile::rename (fname, newfpath);
          }
       }

  update_dyn_menus();
  fman->refresh();
}


void CTEA::fman_multi_rename_del_n_first_chars()
{
  last_action = sender();

  QString fiftxt = fif_get_text();
  int todel = fiftxt.toInt();
  if (todel < 1)
     todel = 1;

  QStringList sl = fman->get_sel_fnames();

  if (sl.size() < 1)
     return;

  for (int i = 0; i < sl.size(); i++)
      {
       QFileInfo fi (sl.at(i));

       if (fi.exists() && fi.isWritable())
          {
           QString newname = fi.fileName();
           newname = newname.mid (todel);

           QString newfpath (fi.path());
           newfpath.append ("/").append (newname);
           QFile::rename (sl.at(i), newfpath);
          }
       }

  update_dyn_menus();
  fman->refresh();
}


void CTEA::fman_multi_rename_replace()
{
  last_action = sender();

  QStringList l = fif_get_text().split ("~");
  if (l.size() < 2)
     return;

  QStringList sl = fman->get_sel_fnames();

  if (sl.size() < 1)
     return;

  for (int i = 0; i < sl.size(); i++)
      {
       QFileInfo fi (sl.at(i));

       if (fi.exists() && fi.isWritable())
          {
           QString newname = fi.fileName();
           newname = newname.replace (l[0], l[1]);

           QString newfpath (fi.path());
           newfpath.append ("/").append (newname);
           QFile::rename (sl.at(i), newfpath);
          }
      }

  update_dyn_menus();
  fman->refresh();
}


void CTEA::fman_multi_rename_apply_template()
{
  last_action = sender();

  QString fiftxt = fif_get_text();

  QStringList sl = fman->get_sel_fnames();

  if (sl.size() < 1)
     return;

  for (int i = 0; i < sl.size(); i++)
      {
       QFileInfo fi (sl.at(i));

       if (fi.exists() && fi.isWritable())
          {
           QString ext = file_get_ext (sl.at(i));
           QString newname = fiftxt;
           newname = newname.replace ("%filename", fi.fileName());
           newname = newname.replace ("%ext", ext);
           newname = newname.replace ("%basename", fi.baseName());

           QString newfpath (fi.path());
           newfpath.append ("/").append (newname);
           QFile::rename (sl.at(i), newfpath);
           }
      }

  update_dyn_menus();
  fman->refresh();
}


void CTEA::fman_fileop_create_dir()
{
  last_action = sender();

  bool ok;
  QString newdir = QInputDialog::getText (this, tr ("Enter the name"),
                                                tr ("Name:"), QLineEdit::Normal,
                                                tr ("new_directory"), &ok);
  if (! ok || newdir.isEmpty())
     return;

  QString dname = fman->dir.path() + "/" + newdir;

  QDir d;
  if (d.mkpath (dname))
     fman->nav (dname);
}


void CTEA::fman_fileop_rename()
{
  last_action = sender();

  QString fname = fman->get_sel_fname();
  if (fname.isEmpty())
     return;

  QFileInfo fi (fname);
  if (! fi.exists() && ! fi.isWritable())
     return;

  bool ok;
  QString newname = QInputDialog::getText (this, tr ("Enter the name"),
                                                 tr ("Name:"), QLineEdit::Normal,
                                                 tr ("new"), &ok);
  if (! ok || newname.isEmpty())
     return;

  QString newfpath = fi.path() + "/" + newname;
  QFile::rename (fname, newfpath);
  update_dyn_menus();
  fman->refresh();

  QModelIndex index = fman->index_from_name (newname);
  fman->selectionModel()->setCurrentIndex (index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
  fman->scrollTo (index, QAbstractItemView::PositionAtCenter);
}


void CTEA::fman_fileop_delete()
{
  last_action = sender();

  QString fname = fman->get_sel_fname();
  if (fname.isEmpty())
     return;

  int i = fman->get_sel_index(); //save the index

  QFileInfo fi (fname);
  if (! fi.exists() && ! fi.isWritable())
     return;

  if (QMessageBox::warning (this, "TEA",
                            tr ("Are you sure to delete\n"
                            "%1?").arg (fname),
                            QMessageBox::Yes,
                            QMessageBox::No) == QMessageBox::No)
      return;

  QFile::remove (fname);
  update_dyn_menus();
  fman->refresh();

  QModelIndex index = fman->index_from_idx (i);
  if (! index.isValid())
     index = fman->index_from_idx (0);

  fman->selectionModel()->setCurrentIndex (index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
  fman->scrollTo (index, QAbstractItemView::PositionAtCenter);
}


void CTEA::fm_fileinfo_info()
{
  last_action = sender();

  QString fname;

  if (main_tab_widget->currentIndex() == idx_tab_fman)
     fname = fman->get_sel_fname();
  else
      {
       CDocument *d = documents->get_current();
       if (d)
          fname = d->file_name;
      }

  QFileInfo fi (fname);
  if (! fi.exists())
     return;

  QStringList l;

//detect EOL

  QFile f (fname);

  if (f.open (QIODevice::ReadOnly))
     {
      QString n (tr("End of line: "));
      QByteArray barr = f.readAll();

      int nl = barr.count ('\n');
      int cr = barr.count ('\r');

      if (nl > 0 && cr == 0)
         n += "UNIX";

      if (nl > 0 && cr > 0)
         n += "Windows";

      if (nl == 0 && cr > 0)
         n += "Mac";

      l.append (n);
     }

  l.append (tr ("file name: %1").arg (fi.absoluteFilePath()));
  l.append (tr ("size: %1 kbytes").arg (QString::number (fi.size() / 1024)));
  l.append (tr ("last modified: %1").arg (fi.lastModified().toString ("yyyy-MM-dd@hh:mm:ss")));

  if (file_get_ext (fname) == "wav")
     {
      CWavReader wr;
      wr.get_info (fname);
      l.append (tr ("bits per sample: %1").arg (wr.wav_chunk_fmt.bits_per_sample));
      l.append (tr ("number of channels: %1").arg (wr.wav_chunk_fmt.num_channels));
      l.append (tr ("sample rate: %1").arg (wr.wav_chunk_fmt.sample_rate));
      if (wr.wav_chunk_fmt.bits_per_sample == 16)
         l.append (tr ("RMS for all channels: %1 dB").arg (wr.rms));
     }

  log->log (l.join ("<br>"));
}


void CTEA::fman_fileinfo_count_lines_in_selected_files()
{
  last_action = sender();

  QString ft = fif_get_text();
  if (ft.isEmpty())
      return;

  QStringList sl = fman->get_sel_fnames();

  if (sl.size() < 1)
     return;

  long int sum = 0;

  for (int i = 0; i < sl.size(); i++)
      {
       QByteArray f = file_load (sl.at(i));
       sum += f.count ('\n');
      }

  log->log (tr ("There are %1 lines at %2 files").arg (sum).arg (sl.size()));
}


void CTEA::fman_zip_create()
{
  last_action = sender();

  bool ok;

  QString name = QInputDialog::getText (this, tr ("Enter the archive name"),
                                              tr ("Name:"), QLineEdit::Normal,
                                              tr ("new_archive"), &ok);

  if (! ok)
     return;

  fman->zipper.files_list.clear();
  fman->zipper.archive_name = name;

  if (! name.endsWith (".zip"))
     name.append (".zip");

  fman->zipper.archive_fullpath = fman->dir.path() + "/" + name;
}


void CTEA::fman_zip_add()
{
  last_action = sender();

  QString f = ed_fman_fname->text().trimmed();
  QStringList li = fman->get_sel_fnames();

  if (! f.isEmpty())
  if (f[0] == '/')
     {
      fman->zipper.files_list.append (f);
      return;
     }

  if (li.size() == 0)
     {
      QString fname = fman->dir.path() + "/" + f;
      fman->zipper.files_list.append (fname);
      return;
     }

  for (int i = 0; i < li.size(); i++)
      fman->zipper.files_list.append (li.at(i));
}


void CTEA::fman_zip_save()
{
  last_action = sender();

  fman->zipper.pack_prepared();
  fman->refresh();
}


void CTEA::fman_zip_info()
{
  last_action = sender();

  QString fn = fman->get_sel_fname();
  if (fn.isEmpty())
     return;

  CZipper z;

  QStringList sl = z.unzip_list (fn);

  for (int i = 0; i < sl.size(); i++)
       sl[i] = sl[i].append ("<br>");

  log->log (sl.join("\n"));
}


void CTEA::fman_zip_unpack()
{
  last_action = sender();

  CZipper z;

  QString f = ed_fman_fname->text().trimmed();
  QStringList li = fman->get_sel_fnames();

  if (! f.isEmpty())
  if (f[0] == '/')
     {
      z.unzip (f, fman->dir.path());
      return;
     }

  if (li.size() == 0)
     {
      QString fname (fman->dir.path());
      fname.append ("/").append (f);
      z.unzip (fname, fman->dir.path());
      return;
     }

  for (QList <QString>::iterator fname = li.begin(); fname != li.end(); ++fname)
      {
       z.unzip ((*fname), fman->dir.path());
       log->log ((*fname) + tr (" is unpacked"));
      }
}


void CTEA::fman_img_conv_by_side()
{
  last_action = sender();

  int side = fif_get_text().toInt();
  if (side != 0)
     fman_convert_images (true, side);
}


void CTEA::fman_img_conv_by_percent()
{
  last_action = sender();

  int percent = fif_get_text().toInt();
  if (percent != 0)
     fman_convert_images (false, percent);
}


void CTEA::fman_img_make_gallery()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  if (! file_exists (d->file_name))
     return;

  int side = settings->value ("ed_side_size", 110).toInt();
  int thumbs_per_row = settings->value ("ed_thumbs_per_row", 4).toInt();

  QString link_options = settings->value ("ed_link_options", "target=\"_blank\"").toString();
  if (! link_options.startsWith (" "))
     link_options.prepend (" ");

  QString dir_out (fman->dir.absolutePath());

  QString table ("<table>\n\n");

  Qt::TransformationMode transformMode = Qt::FastTransformation;

  pb_status->show();
  pb_status->setFormat (tr ("%p% completed"));
  pb_status->setTextVisible (true);

  QStringList li = fman->get_sel_fnames();
  int quality = settings->value ("img_quality", "-1").toInt();

  pb_status->setRange (0, li.size() - 1 );

  int x = 0;
  int col = 0;

  for (int i = 0; i < li.size(); i++)
      {
       QString fname = li[i];
       if (is_image (fname))
          {
           QFileInfo fi (fname);

           if (fi.baseName().startsWith ("tmb_"))
              continue;

           QImage source (fname);
           if (! source.isNull())
              {
               qApp->processEvents();

               QImage dest = image_scale_by (source, true, side, transformMode);

               QString dest_fname (dir_out);
               dest_fname.append ("/");
               dest_fname.append ("tmb_");
               dest_fname.append (fi.fileName());
               dest_fname = change_file_ext (dest_fname, "jpg");

               dest.save (dest_fname, 0, quality);

               QFileInfo inf (d->file_name);
               QDir dir (inf.absolutePath());

               QString tmb = get_insert_image (d->file_name, dest_fname, d->markup_mode);
               QString cell = "<a href=\"%source\"" + link_options +">%thumb</a>";
               cell.replace ("%source", dir.relativeFilePath (fname));
               cell.replace ("%thumb", tmb);

               if (col == 0)
                  table += "<tr>\n\n";

               table += "<td>\n";

               table += cell;

               table += "</td>\n";

               col++;
               if (col == thumbs_per_row)
                  {
                   table += "</tr>\n\n";
                   col = 0;
                  }

               pb_status->setValue (x++);
              }
           }
          }

  pb_status->hide();
  fman->refresh();

  if (! table.endsWith ("</tr>\n\n"))
     table += "</tr>\n\n";

  table += "</table>\n";

  if (d)
     d->put (table);
}


void CTEA::fman_home()
{
  last_action = sender();

#if defined(Q_OS_WIN) || defined(Q_OS_OS2)
  fman->nav ("c:\\");
#else
  fman->nav (QDir::homePath());
#endif
}


void CTEA::fman_refresh()
{
  last_action = sender();

  fman->refresh();
}


void CTEA::fman_preview_image()
{
  last_action = sender();

  QString fname = fman->get_sel_fname();
  if (fname.isEmpty())
     return;

  if (is_image (fname))
     {
      if (file_get_ext (fname) == "gif")
         {
          CGIFWindow *w = new CGIFWindow;
          w->load_image (fname);
          return;
         }

      img_viewer->window_full.show();
      img_viewer->set_image_full (fname);
     }
}


void CTEA::fman_select_by_regexp()
{
  last_action = sender();
  fman_items_select_by_regexp (true);
}


void CTEA::fman_deselect_by_regexp()
{
  last_action = sender();
  fman_items_select_by_regexp (false);
}


/*
===================
View menu callbacks
===================
*/


void CTEA::view_use_theme()
{
  last_action = sender();
  QAction *a = qobject_cast<QAction *>(sender());

  QString css_fname = a->data().toString() + "/" + "stylesheet.css";

  if (! file_exists (css_fname))
     {
      log->log (tr ("There is no stylesheet file"));
      return;
    }

  update_stylesheet (css_fname);
  fname_stylesheet = css_fname;
  settings->setValue ("fname_stylesheet", fname_stylesheet);
}


void CTEA::view_use_palette()
{
  last_action = sender();

  QAction *a = qobject_cast<QAction *>(sender());

//  QString tf = qstring_clear (a->text());

  QString fname = dir_palettes + "/" + a->data().toString();

  if (! file_exists (fname))
     fname = ":/palettes/" + a->data().toString();

  fname_def_palette = fname;
  load_palette (fname);

  update_stylesheet (fname_stylesheet);
  documents->apply_settings();
}


void CTEA::view_use_profile()
{
  last_action = sender();
  QAction *a = qobject_cast<QAction *>(sender());

  QSettings s (a->data().toString(), QSettings::IniFormat);

  QPoint pos = s.value ("pos", QPoint (1, 200)).toPoint();
  QSize size = s.value ("size", QSize (600, 420)).toSize();

  if (mainSplitter/* && ! settings->value ("ui_mode", 0).toBool()*/)
     mainSplitter->restoreState (s.value ("splitterSizes").toByteArray());

  resize (size);
  move (pos);

  fname_def_palette = s.value ("fname_def_palette", ":/palettes/TEA").toString();
  load_palette (fname_def_palette);

  settings->setValue ("fname_def_palette", fname_def_palette);
  settings->setValue ("word_wrap", s.value ("word_wrap", "1").toBool());
  settings->setValue ("show_linenums", s.value ("show_linenums", "0").toBool());
  settings->setValue ("additional_hl", s.value ("additional_hl", "0").toBool());
  settings->setValue ("show_margin", s.value ("show_margin", "0").toBool());

  settings->setValue ("editor_font_name", s.value ("editor_font_name", "Serif").toString());
  settings->setValue ("editor_font_size", s.value ("editor_font_size", "14").toInt());
  settings->setValue ("logmemo_font", s.value ("logmemo_font", "Monospace").toString());
  settings->setValue ("logmemo_font_size", s.value ("logmemo_font_size", "14").toInt());
  settings->setValue ("app_font_name", s.value ("app_font_name", "Sans").toString());
  settings->setValue ("app_font_size", s.value ("app_font_size", "12").toInt());

  cb_wordwrap->setChecked (s.value ("word_wrap", "1").toBool());

  cb_show_linenums->setChecked (s.value ("show_linenums", "0").toBool());
  cb_hl_current_line->setChecked (s.value ("additional_hl", "0").toBool());
  cb_show_margin->setChecked (s.value ("show_margin", "0").toBool());

  update_stylesheet (fname_stylesheet);
  documents->apply_settings();
}


void CTEA::view_profile_save_as()
{
  last_action = sender();

  bool ok;
  QString name = QInputDialog::getText (this, tr ("Enter the name"),
                                              tr ("Name:"), QLineEdit::Normal,
                                              tr ("new_profile"), &ok);
  if (! ok || name.isEmpty())
     return;

  QString fname (dir_profiles);
  fname.append ("/").append (name);

  QSettings s (fname, QSettings::IniFormat);

  s.setValue ("fname_def_palette", fname_def_palette);

  s.setValue ("word_wrap", settings->value ("word_wrap", "1").toBool());
  s.setValue ("show_linenums", settings->value ("show_linenums", "0").toBool());
  s.setValue ("additional_hl", settings->value ("additional_hl", "0").toBool());

  s.setValue ("pos", pos());
  s.setValue ("size", size());

  if (mainSplitter /*&& ! settings->value ("ui_mode", 0).toBool()*/)
     s.setValue ("splitterSizes", mainSplitter->saveState());

  s.setValue ("editor_font_name", settings->value ("editor_font_name", "Monospace").toString());
  s.setValue ("editor_font_size", settings->value ("editor_font_size", "16").toInt());

  s.setValue ("logmemo_font", settings->value ("logmemo_font", "Monospace").toString());
  s.setValue ("logmemo_font_size", settings->value ("logmemo_font_size", "12").toInt());

  s.setValue ("app_font_name", settings->value ("app_font_name", "Sans").toString());
  s.setValue ("app_font_size", settings->value ("app_font_size", "12").toInt());

  s.sync();

  update_profiles();
  shortcuts->load_from_file (shortcuts->fname);
}


void CTEA::view_toggle_wrap()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (d)
     d->set_word_wrap (! d->get_word_wrap());
}


void CTEA::view_hide_error_marks()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QTextCursor cr = d->textCursor();

//delete all underlines
  cr.setPosition (0);
  cr.movePosition (QTextCursor::End, QTextCursor::KeepAnchor);
  QTextCharFormat f = cr.blockCharFormat();
  f.setFontUnderline (false);
  cr.mergeCharFormat (f);
  d->document()->setModified (false);
}


void CTEA::view_toggle_fs()
{
  last_action = sender();
  setWindowState(windowState() ^ Qt::WindowFullScreen);
}


void CTEA::view_stay_on_top()
{
  last_action = sender();

  Qt::WindowFlags flags = windowFlags();
  flags ^= Qt::WindowStaysOnTopHint;
  setWindowFlags (flags );
  show();
  activateWindow();
}


void CTEA::view_darker()
{
  last_action = sender();
  CDarkerWindow *wd = new CDarkerWindow;
  wd->show();
}


/*
===================
? menu callbacks
===================
*/


void CTEA::help_show_about()
{
  //CAboutWindow *a = new CAboutWindow();
  if (! wnd_about)
     wnd_about = new CAboutWindow();

  wnd_about->move (x() + 20, y() + 20);
  wnd_about->show();
}


void CTEA::help_show_news()
{
  QString fname = ":/NEWS";
  if (QLocale::system().name().left(2) == "ru")
     fname = ":/NEWS-RU";

  CDocument *d = documents->open_file (fname, "UTF-8");
  if (d)
     d->setReadOnly (true);
}


void CTEA::help_show_todo()
{
  CDocument *d = documents->open_file (":/TODO", "UTF-8");
  if (d)
     d->setReadOnly (true);
}


void CTEA::help_show_changelog()
{
  CDocument *d = documents->open_file (":/ChangeLog", "UTF-8");
  if (d)
     d->setReadOnly (true);
}


void CTEA::help_show_gpl()
{
  CDocument *d = documents->open_file (":/COPYING", "UTF-8");
  if (d)
     d->setReadOnly (true);
}


/*
====================================
Application stuff inits and updates
====================================
*/


CTEA::CTEA()
{
  wnd_about = 0;
  mainSplitter = 0;
  ui_update = true;
  boring = false;
  b_destroying_all = false;
  last_action = 0;
  b_recent_off = false;
  lv_menuitems = NULL;
  fm_entry_mode = FM_ENTRY_MODE_NONE;
  calendar = 0;
  capture_to_storage_file = false;

  date1 = QDate::currentDate();
  date2 = QDate::currentDate();

  idx_tab_edit = 0;
  idx_tab_tune = 0;
  idx_tab_fman = 0;
  idx_tab_learn = 0;
  idx_tab_calendar = 0;
  idx_tab_keyboard = 0;

  create_paths();

  QString sfilename = dir_config + "/tea.conf";
  settings = new QSettings (sfilename, QSettings::IniFormat);

  QString lng = settings->value ("lng", QLocale::system().name()).toString().left(2).toLower();

  if (! file_exists (":/translations/" + lng + ".qm"))
     lng = "en";

#if QT_VERSION >= 0x060000
  if (transl_app.load (QString ("qt_%1").arg (lng), QLibraryInfo::path (QLibraryInfo::TranslationsPath)))
     qApp->installTranslator (&transl_app);
#else
  if (transl_system.load (QString ("qt_%1").arg (lng), QLibraryInfo::location (QLibraryInfo::TranslationsPath)))
     qApp->installTranslator (&transl_system);
#endif

  if (transl_app.load (":/translations/" + lng))
      qApp->installTranslator (&transl_app);

  fname_stylesheet = settings->value ("fname_stylesheet", ":/themes/TEA").toString();
  theme_dir = get_file_path (fname_stylesheet) + "/";

  l_charset = new QLabel;
  l_status = new QLabel;

  pb_status = new QProgressBar;
  pb_status->setRange (0, 0);

  tb_stop = new QToolButton;
  tb_stop->setIcon (style()->standardIcon(QStyle::SP_MediaStop));
  connect (tb_stop, SIGNAL(clicked()), this, SLOT(tb_stop_clicked()));

  statusBar()->addWidget (l_status);
  statusBar()->addPermanentWidget (pb_status);
  statusBar()->addPermanentWidget (tb_stop);
  statusBar()->addPermanentWidget (l_charset);

//  pb_status->hide();
  progress_hide();

  create_actions();
  create_menus();
  create_toolbars();

  update_styles();
  update_bookmarks();
  update_templates();
  update_tables();
  update_snippets();
  update_sessions();
  update_scripts();
  update_programs();
  update_palettes();
  update_themes();
  update_keyboards();
  update_charsets();
  update_profiles();
  create_markup_hash();

  setMinimumSize (12, 12);

  if (! settings->value ("ui_mode", 0).toBool())
     create_main_widget_splitter();
  else
      create_main_widget_docked();

  idx_prev = 0;
  connect (main_tab_widget, SIGNAL(currentChanged(int)), this, SLOT(main_tab_page_changed(int)));

  read_settings();
  read_search_options();

  documents = new CDox();
  documents->parent_wnd = this;
  documents->tab_widget = tab_editor;
  documents->main_tab_widget = main_tab_widget;

  documents->menu_recent = menu_file_recent;
  documents->recent_list_fname = dir_config + "/recentfiles";
  documents->reload_recent_list();
  documents->update_recent_menu();
  documents->log = log;
  documents->markup_mode = markup_mode;
  documents->dir_config = dir_config;
  documents->todo.dir_days = dir_days;
  documents->fname_crapbook = fname_crapbook;

  documents->fname_saved_buffers = fname_saved_buffers;
  documents->l_status_bar = l_status;
  documents->l_charset = l_charset;

  documents->autosave_files = hash_load_keyval (fname_autosaving_files);


  load_palette (fname_def_palette);

  update_stylesheet (fname_stylesheet);
  documents->apply_settings();

  documents->todo.load_dayfile();

  update_hls();

#if defined (HUNSPELL_ENABLE) || defined (ASPELL_ENABLE)
  setup_spellcheckers();
#endif

  shortcuts = new CShortcuts (this);
  shortcuts->fname = dir_config + "/shortcuts";
  shortcuts->load_from_file (shortcuts->fname);

  sl_fif_history = qstring_load (fname_fif).split ("\n");
  cmb_fif->addItems (sl_fif_history);
  cmb_fif->clearEditText();

  create_fman();
  create_options();
  create_calendar();
  create_manual();

  update_fonts();

  documents->dir_last = settings->value ("dir_last", QDir::homePath()).toString();
  b_preview = settings->value ("b_preview", false).toBool();

  img_viewer = new CImgViewer;

  restoreState (settings->value ("state", QByteArray()).toByteArray());

  current_version_number = VERSION_NUMBER;
  if (current_version_number.indexOf ('\"') != -1)
     {
      current_version_number.remove (0, 1);
      current_version_number.remove (current_version_number.size() - 1, 1);
     }

  QString vn = settings->value ("VER_NUMBER", "").toString();
  if (vn.isEmpty() || vn != QString (current_version_number))
      help_show_news();

  if (settings->value ("session_restore", false).toBool())
     {
      QString fname_session (dir_sessions);
      fname_session.append ("/def-session-777");
      documents->load_from_session (fname_session);
     }

  if (settings->value ("save_buffers", true).toBool())
      documents->load_from_buffers (fname_saved_buffers);


  handle_args();
  ui_update = false;

  setIconSize (QSize (icon_size, icon_size));
  tb_fman_dir->setIconSize (QSize (icon_size, icon_size));

  QClipboard *clipboard = QApplication::clipboard();
  connect (clipboard , SIGNAL(dataChanged()), this, SLOT(clipboard_dataChanged()));

  setAcceptDrops (true);

  log->log (tr ("<b>TEA %1</b> by Peter Semiletov | tea.ourproject.org<br>Support TEA via PayPal: peter.semiletov@gmail.com<br>Git: github.com/psemiletov/tea-qt<br>AUR: aur.archlinux.org/packages/tea-qt-git").arg (QString (current_version_number)));

  QTextCursor cr = log->textCursor();
  cr.movePosition (QTextCursor::Start);
  log->setTextCursor (cr);

  QString icon_fname = ":/icons/tea-icon-v3-0" + settings->value ("icon_fname", "1").toString() + ".png";
  qApp->setWindowIcon (QIcon (icon_fname));
  idx_tab_edit_activate();
}


void CTEA::handle_args()
{
  QStringList l = qApp->arguments();
  int size = l.size();
  if (size < 2)
     return;

  QString charset = settings->value ("cmdline_default_charset", "UTF-8").toString();//"UTF-8";

  for (int i = 1; i < size; i++)
      {
       QString t = l.at(i);
       if (t.startsWith ("--charset"))
          {
           QStringList pair = t.split ("=");
           if (pair.size() > 1)
              charset = pair[1];
          }
       else
           {
            QFileInfo f (l.at(i));

            if (! f.isAbsolute())
               {
                QString fullname (QDir::currentPath());
                fullname.append ("/").append (l.at(i));
                documents->open_file (fullname, charset);
               }
            else
                documents->open_file (l.at(i), charset);
          }
      }
}


void CTEA::create_paths()
{
  portable_mode = false;

  QStringList l = qApp->arguments();
  if (l.contains ("--p"))
     portable_mode = true;

  QDir dr;
  if (! portable_mode)
     dir_config = dr.homePath();
  else
      dir_config = QCoreApplication::applicationDirPath();

#if defined(Q_OS_WIN) || defined(Q_OS_OS2)
  dir_config.append ("/tea");
#else
  dir_config.append ("/.config/tea");
#endif

//  hs_path["dir_config"] = dir_config;

  dr.setPath (dir_config);
  if (! dr.exists())
     dr.mkpath (dir_config);

  fname_crapbook = dir_config + "/crapbook.txt";
//  hs_path["fname_crapbook"] = fname_crapbook;

  fname_saved_buffers = dir_config + "/saved_buffers.txt";
  //hs_path["saved_buffers"] = fname_saved_buffers;

  fname_autosaving_files = dir_config + "/autosaving_files.txt";
  //hs_path["autosaving_files"] = fname_autosaving_files;

  fname_fif = dir_config + "/fif";
  //hs_path["fname_fif"] = fname_fif;

  QString old_bmx_file = dir_config + "/tea_bmx";
  fname_bookmarks = dir_config + "/bookmarks";
  //hs_path["fname_bookmarks"] = fname_bookmarks;

  if (file_exists (old_bmx_file))
     {
      QString file_content = qstring_load (old_bmx_file);
      file_content = file_content.replace (",", "*");
      qstring_save (fname_bookmarks, file_content);

      QFile f (old_bmx_file);
      f.rename (old_bmx_file + ".bak");
     }



  fname_programs = dir_config + "/programs";
  //hs_path["fname_programs"] = fname_programs;

  fname_places_bookmarks = dir_config + "/places_bookmarks";
  //hs_path["fname_places_bookmarks"] = fname_places_bookmarks;

  fname_tempfile = QDir::tempPath() + "/tea.tmp";
  //hs_path["fname_tempfile"] = fname_tempfile;

  fname_tempparamfile = QDir::tempPath() + "/teaparam.tmp";
  //hs_path["fname_tempparamfile"] = fname_tempparamfile;

  dir_tables = dir_config + "/tables";

  dr.setPath (dir_tables);
  if (! dr.exists())
     dr.mkpath (dir_tables);

  dir_user_dict = dir_config + "/dictionaries";

  dr.setPath (dir_user_dict);
  if (! dr.exists())
     dr.mkpath (dir_user_dict);

  dir_profiles = dir_config + "/profiles";

  dr.setPath (dir_profiles);
  if (! dr.exists())
     dr.mkpath (dir_profiles);

  dir_templates = dir_config + "/templates";

  dr.setPath (dir_templates);
  if (! dr.exists())
     dr.mkpath (dir_templates);

  dir_snippets = dir_config + "/snippets";

  dr.setPath (dir_snippets);
  if (! dr.exists())
     dr.mkpath (dir_snippets);

  dir_scripts = dir_config + "/scripts";

  dr.setPath (dir_scripts);
  if (! dr.exists())
     dr.mkpath (dir_scripts);

  dir_days = dir_config + "/days";

  dr.setPath (dir_days);
  if (! dr.exists())
     dr.mkpath (dir_days);

  dir_sessions = dir_config + "/sessions";

  dr.setPath (dir_sessions);
  if (! dr.exists())
     dr.mkpath (dir_sessions);


  dir_keyboards = dir_config + "/keyboards";

  dr.setPath (dir_keyboards);
  if (! dr.exists())
     dr.mkpath (dir_keyboards);


  dir_themes = dir_config + "/themes";

  dr.setPath (dir_themes);
  if (! dr.exists())
     dr.mkpath (dir_themes);

  dir_hls = dir_config + "/hls";

  dr.setPath (dir_hls);
  if (! dr.exists())
     dr.mkpath (dir_hls);

  dir_palettes = dir_config + "/palettes";

  dr.setPath (dir_palettes);
  if (! dr.exists())
     dr.mkpath (dir_palettes);
}



#if defined (HUNSPELL_ENABLE) || defined (ASPELL_ENABLE)

void CTEA::setup_spellcheckers()
{

#ifdef HUNSPELL_ENABLE
  spellcheckers.append ("Hunspell");
#endif

#ifdef ASPELL_ENABLE
  spellcheckers.append ("Aspell");
#endif

  cur_spellchecker = settings->value ("cur_spellchecker", "Hunspell").toString();

  if (spellcheckers.size() > 0)
     if (! spellcheckers.contains (cur_spellchecker))
         cur_spellchecker = spellcheckers[0];

#ifdef ASPELL_ENABLE
  if (cur_spellchecker == "Aspell")
     {
      QString lang = settings->value ("spell_lang", QLocale::system().name().left(2)).toString();
#if defined(Q_OS_WIN) || defined(Q_OS_OS2)
      QString win32_aspell_path = settings->value ("win32_aspell_path", aspell_default_dict_path()).toString();
      spellchecker = new CAspellchecker (lang, win32_aspell_path);
#else
      spellchecker = new CAspellchecker (lang);
#endif
     }

#endif


#ifdef HUNSPELL_ENABLE
   if (cur_spellchecker == "Hunspell")
      spellchecker = new CHunspellChecker (settings->value ("spell_lang", QLocale::system().name().left(2)).toString(), settings->value ("hunspell_dic_path", hunspell_default_dict_path()).toString(), dir_user_dict);
#endif

 create_spellcheck_menu();
}


void CTEA::create_spellcheck_menu()
{
  menu_spell_langs->clear();
  spellchecker->get_speller_modules_list();

  if (spellchecker->modules_list.size() > 0)
     create_menu_from_list (this, menu_spell_langs, spellchecker->modules_list, SLOT(fn_change_spell_lang()));
}

#endif


void CTEA::create_main_widget_splitter()
{
  QWidget *main_widget = new QWidget;
  QVBoxLayout *v_box = new QVBoxLayout;
  main_widget->setLayout (v_box);

  main_tab_widget = new QTabWidget;
  main_tab_widget->setObjectName ("main_tab_widget");
  main_tab_widget->setTabShape (QTabWidget::Triangular);

  tab_editor = new QTabWidget;
  tab_editor->setUsesScrollButtons (true);

#if (QT_VERSION_MAJOR >= 4 && QT_VERSION_MINOR >= 5)
  tab_editor->setMovable (true);
#endif

  tab_editor->setObjectName ("tab_editor");

  QPushButton *bt_close = new QPushButton ("X", this);
  connect (bt_close, SIGNAL(clicked()), this, SLOT(file_close()));
  tab_editor->setCornerWidget (bt_close);

  log = new CLogMemo;

  connect (log, SIGNAL(double_click(QString)), this, SLOT(logmemo_double_click(QString)));

  mainSplitter = new QSplitter (Qt::Vertical);
  v_box->addWidget (mainSplitter);

  main_tab_widget->setMinimumHeight (10);
  log->setMinimumHeight (10);

  mainSplitter->addWidget (main_tab_widget);
  mainSplitter->addWidget (log);

// FIF creation code

  if (! settings->value ("fif_at_toolbar", 0).toBool())
     {
      cmb_fif = new QComboBox;
      cmb_fif->setInsertPolicy (QComboBox::InsertAtTop);
      cmb_fif->setObjectName ("FIF");

      cmb_fif->setEditable (true);
      cmb_fif->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Fixed);

      fif = cmb_fif->lineEdit();
      fif->setStatusTip (tr ("The famous input field. Use for search/replace, function parameters"));

      connect (fif, SIGNAL(returnPressed()), this, SLOT(search_find()));

      QHBoxLayout *lt_fte = new QHBoxLayout;

      v_box->addLayout (lt_fte, 0);

      int tleft = 1;
      int tright = 1;
      int ttop = 1;
      int tbottom = 1;

      v_box->getContentsMargins(&tleft, &ttop, &tright, &tbottom);
      v_box->setContentsMargins(tleft, 1, tright, 1);

      QToolBar *tb_fif = new QToolBar;

      QAction *act_fif_find = tb_fif->addAction (style()->standardIcon(QStyle::SP_ArrowForward), "");
      act_fif_find->setToolTip (tr ("Find"));
      connect (act_fif_find, SIGNAL(triggered()), this, SLOT(search_find()));

      QAction *act_fif_find_prev = tb_fif->addAction (style()->standardIcon(QStyle::SP_ArrowUp), "");
      act_fif_find_prev->setToolTip (tr ("Find previous"));
      connect (act_fif_find_prev, SIGNAL(triggered()), this, SLOT(search_find_prev()));

      QAction *act_fif_find_next = tb_fif->addAction (style()->standardIcon(QStyle::SP_ArrowDown), "");
      act_fif_find_next->setToolTip (tr ("Find next"));
      connect (act_fif_find_next, SIGNAL(triggered()), this, SLOT(search_find_next()));

      QLabel *l_fif = new QLabel (tr ("FIF"));

      lt_fte->addWidget (l_fif, 0, Qt::AlignRight);
      lt_fte->addWidget (cmb_fif, 0);

      lt_fte->addWidget (tb_fif, 0);
     }

  mainSplitter->setStretchFactor (1, 1);

  idx_tab_edit = main_tab_widget->addTab (tab_editor, tr ("editor"));
  setCentralWidget (main_widget);

  connect (tab_editor, SIGNAL(currentChanged(int)), this, SLOT(pageChanged(int)));
}


void CTEA::create_main_widget_docked()
{
  setDockOptions (QMainWindow::AnimatedDocks | QMainWindow::AllowNestedDocks);

  QWidget *main_widget = new QWidget;
  QVBoxLayout *v_box = new QVBoxLayout;
  main_widget->setLayout (v_box);
  setCentralWidget (main_widget);

  main_tab_widget = new QTabWidget;
  main_tab_widget->setObjectName ("main_tab_widget");
  v_box->addWidget (main_tab_widget);

  main_tab_widget->setTabShape (QTabWidget::Triangular);

  tab_editor = new QTabWidget;
  tab_editor->setUsesScrollButtons (true);

//#if QT_VERSION >= 0x040500
#if (QT_VERSION_MAJOR >= 4 && QT_VERSION_MINOR >= 5)
  tab_editor->setMovable (true);
#endif

  tab_editor->setObjectName ("tab_editor");

  QPushButton *bt_close = new QPushButton ("X", this);
  connect (bt_close, SIGNAL(clicked()), this, SLOT(file_close()));
  tab_editor->setCornerWidget (bt_close);

  QDockWidget *dock_logmemo = new QDockWidget (tr ("logmemo"), this);
  dock_logmemo->setFeatures (QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
  dock_logmemo->setAllowedAreas (Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);

  log = new CLogMemo (dock_logmemo);

  connect (log, SIGNAL(double_click(QString)), this, SLOT(logmemo_double_click(QString)));

  dock_logmemo->setWidget (log);
  dock_logmemo->setObjectName ("dock_log");
  addDockWidget (Qt::BottomDockWidgetArea, dock_logmemo);


// FIF creation code

  if (! settings->value ("fif_at_toolbar", 0).toBool())
     {
      QDockWidget *dock_fif = new QDockWidget (tr ("famous input field"), this);
      dock_fif->setAllowedAreas (Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
      dock_fif->setObjectName ("dock_fif");
      dock_fif->setFeatures (QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

      QWidget *w_fif = new QWidget (dock_fif);
      w_fif->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Maximum);

      cmb_fif = new QComboBox;
      cmb_fif->setObjectName ("FIF");
      cmb_fif->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Fixed);

      cmb_fif->setEditable (true);
      fif = cmb_fif->lineEdit();
      fif->setStatusTip (tr ("The famous input field. Use for search/replace, function parameters"));

      connect (fif, SIGNAL(returnPressed()), this, SLOT(search_find()));

      QHBoxLayout *lt_fte = new QHBoxLayout;
      w_fif->setLayout (lt_fte);

      QToolBar *tb_fif = new QToolBar;

      QAction *act_fif_find = tb_fif->addAction (style()->standardIcon(QStyle::SP_ArrowForward), "");
      act_fif_find->setToolTip (tr ("Find"));
      connect (act_fif_find, SIGNAL(triggered()), this, SLOT(search_find()));

      QAction *act_fif_find_next = tb_fif->addAction (style()->standardIcon(QStyle::SP_ArrowDown), "");
      act_fif_find_next->setToolTip (tr ("Find next"));
      connect (act_fif_find_next, SIGNAL(triggered()), this, SLOT(search_find_next()));

      QAction *act_fif_find_prev = tb_fif->addAction (style()->standardIcon(QStyle::SP_ArrowUp), "");
      act_fif_find_prev->setToolTip (tr ("Find previous"));
      connect (act_fif_find_prev, SIGNAL(triggered()), this, SLOT(search_find_prev()));

      QLabel *l_fif = new QLabel (tr ("FIF"));

      lt_fte->addWidget (l_fif, 0, Qt::AlignRight);
      lt_fte->addWidget (cmb_fif, 0);
      lt_fte->addWidget (tb_fif, 0);

      dock_fif->setWidget (w_fif);
      addDockWidget (Qt::BottomDockWidgetArea, dock_fif);
     }

  idx_tab_edit = main_tab_widget->addTab (tab_editor, tr ("editor"));

  connect (tab_editor, SIGNAL(currentChanged(int)), this, SLOT(pageChanged(int)));
}


void CTEA::create_actions()
{
  icon_size = settings->value ("icon_size", "32").toInt();

  act_test = new QAction (get_theme_icon("file-save.png"), tr ("Test"), this);
  connect (act_test, SIGNAL(triggered()), this, SLOT(test()));

  filesAct = new QAction (get_theme_icon ("current-list.png"), tr ("Files"), this);

  act_labels = new QAction (get_theme_icon ("labels.png"), tr ("Labels"), this);
  connect (act_labels, SIGNAL(triggered()), this, SLOT(nav_labels_update_list()));

  newAct = new QAction (get_theme_icon ("file-new.png"), tr ("New"), this);

  newAct->setShortcut (QKeySequence ("Ctrl+N"));
  newAct->setStatusTip (tr ("Create a new file"));
  connect (newAct, SIGNAL(triggered()), this, SLOT(file_new()));

  QIcon ic_file_open = get_theme_icon ("file-open.png");
  ic_file_open.addFile (get_theme_icon_fname ("file-open-active.png"), QSize(), QIcon::Active);

  openAct = new QAction (ic_file_open, tr ("Open file"), this);

  openAct->setStatusTip (tr ("Open an existing file"));
  connect (openAct, SIGNAL(triggered()), this, SLOT(file_open()));

  QIcon ic_file_save = get_theme_icon ("file-save.png");
  ic_file_save.addFile (get_theme_icon_fname ("file-save-active.png"), QSize(), QIcon::Active);

  saveAct = new QAction (ic_file_save, tr ("Save"), this);
  saveAct->setShortcut (QKeySequence ("Ctrl+S"));
  saveAct->setStatusTip (tr ("Save the document to disk"));
  connect (saveAct, SIGNAL(triggered()), this, SLOT(file_save()));

  saveAsAct = new QAction (get_theme_icon ("file-save-as.png"), tr ("Save As"), this);
  saveAsAct->setStatusTip (tr ("Save the document under a new name"));
  connect (saveAsAct, SIGNAL(triggered()), this, SLOT(file_save_as()));

  exitAct = new QAction (tr ("Exit"), this);
  exitAct->setShortcut (QKeySequence ("Ctrl+Q"));
  exitAct->setStatusTip (tr ("Exit the application"));
  connect (exitAct, SIGNAL(triggered()), this, SLOT(close()));

  QIcon ic_edit_cut = get_theme_icon ("edit-cut.png");
  ic_edit_cut.addFile (get_theme_icon_fname ("edit-cut-active.png"), QSize(), QIcon::Active);

  cutAct = new QAction (ic_edit_cut, tr ("Cut"), this);
  cutAct->setShortcut (QKeySequence ("Ctrl+X"));
  cutAct->setStatusTip (tr ("Cut the current selection's contents to the clipboard"));
  connect (cutAct, SIGNAL(triggered()), this, SLOT(ed_cut()));

  QIcon ic_edit_copy = get_theme_icon ("edit-copy.png");
  ic_edit_copy.addFile (get_theme_icon_fname ("edit-copy-active.png"), QSize(), QIcon::Active);

  copyAct = new QAction (ic_edit_copy, tr("Copy"), this);
  copyAct->setShortcut (QKeySequence ("Ctrl+C"));
  copyAct->setStatusTip (tr ("Copy the current selection's contents to the clipboard"));
  connect (copyAct, SIGNAL(triggered()), this, SLOT(ed_copy()));

  QIcon ic_edit_paste = get_theme_icon ("edit-paste.png");
  ic_edit_paste.addFile (get_theme_icon_fname ("edit-paste-active.png"), QSize(), QIcon::Active);

  pasteAct = new QAction (ic_edit_paste, tr("Paste"), this);
  pasteAct->setShortcut (QKeySequence ("Ctrl+V"));
  pasteAct->setStatusTip (tr ("Paste the clipboard's contents into the current selection"));
  connect (pasteAct, SIGNAL(triggered()), this, SLOT(ed_paste()));

  undoAct = new QAction (tr ("Undo"), this);
  undoAct->setShortcut (QKeySequence ("Ctrl+Z"));
  connect (undoAct, SIGNAL(triggered()), this, SLOT(ed_undo()));

  redoAct = new QAction (tr ("Redo"), this);
  connect (redoAct, SIGNAL(triggered()), this, SLOT(ed_redo()));

  aboutAct = new QAction (tr ("About"), this);
  connect (aboutAct, SIGNAL(triggered()), this, SLOT(help_show_about()));

  aboutQtAct = new QAction (tr ("About Qt"), this);
  connect (aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}


void CTEA::create_menus()
{
/*
===================
File menu
===================
*/

#if QT_VERSION < 0x050000
  QApplication::instance()->setAttribute(Qt::AA_DontUseNativeMenuBar);
#endif

  menu_file = menuBar()->addMenu (tr ("File"));
  menu_file->setTearOffEnabled (true);

  //menu_file->addAction (act_test);

  menu_file->addAction (newAct);
  add_to_menu (menu_file, tr ("Open"), SLOT(file_open()), "Ctrl+O", get_theme_icon_fname ("file-open.png"));
  add_to_menu (menu_file, tr ("Last closed file"), SLOT(file_last_opened()));
  add_to_menu (menu_file, tr ("Open at cursor"), SLOT(file_open_at_cursor()), "F2");
  add_to_menu (menu_file, tr ("Crapbook"), SLOT(file_crapbook()));
  add_to_menu (menu_file, tr ("Notes"), SLOT(file_notes()));

  menu_file->addSeparator();

  menu_file->addAction (saveAct);
  menu_file->addAction (saveAsAct);

  QMenu *tm = menu_file->addMenu (tr ("Save as different"));
  tm->setTearOffEnabled (true);

  add_to_menu (tm, tr ("Save .bak"), SLOT(file_save_bak()), "Ctrl+B");
  add_to_menu (tm, tr ("Save timestamped version"), SLOT(file_save_version()));
  add_to_menu (tm, tr ("Save session"), SLOT(file_session_save_as()));

  menu_file->addSeparator();

  menu_file_actions = menu_file->addMenu (tr ("File actions"));
  add_to_menu (menu_file_actions, tr ("Reload"), SLOT(file_reload()));
  add_to_menu (menu_file_actions, tr ("Reload with encoding"), SLOT(file_reload_enc()));
  menu_file_actions->addSeparator();
  add_to_menu (menu_file_actions, tr ("Set UNIX end of line"), SLOT(file_set_eol_unix()));
  add_to_menu (menu_file_actions, tr ("Set Windows end of line"), SLOT(file_set_eol_win()));
  add_to_menu (menu_file_actions, tr ("Set old Mac end of line (CR)"), SLOT(file_set_eol_mac()));
  menu_file_actions->addSeparator();
  add_to_menu (menu_file_actions, tr ("Set as autosaving file"), SLOT(file_set_autosaving_file()));
  add_to_menu (menu_file_actions, tr ("Unset the autosaving file"), SLOT(file_unset_autosaving_file()));


  menu_file_recent = menu_file->addMenu (tr ("Recent files"));

  menu_file_bookmarks = menu_file->addMenu (tr ("Bookmarks"));

  menu_file_edit_bookmarks = menu_file->addMenu (tr ("Edit bookmarks"));
  add_to_menu (menu_file_edit_bookmarks, tr ("Add to bookmarks"), SLOT(file_add_to_bookmarks()));
  add_to_menu (menu_file_edit_bookmarks, tr ("Find obsolete paths"), SLOT(file_find_obsolete_paths()));

  menu_file_templates = menu_file->addMenu (tr ("Templates"));
  menu_file_sessions = menu_file->addMenu (tr ("Sessions"));

  menu_file_configs = menu_file->addMenu (tr ("Configs"));
  add_to_menu (menu_file_configs, tr ("Bookmarks list"), SLOT(file_open_bookmarks_file()));
  add_to_menu (menu_file_configs, tr ("Programs list"), SLOT(file_open_programs_file()));

  menu_file->addSeparator();

  add_to_menu (menu_file, tr ("Do not add to recent"), SLOT(file_recent_off()))->setCheckable (true);

#ifdef PRINTER_ENABLE
  add_to_menu (menu_file, tr ("Print"), SLOT(file_print()));
#endif

  add_to_menu (menu_file, tr ("Close current"), SLOT(file_close()), "Ctrl+W");

  menu_file->addAction (exitAct);

/*
===================
Edit menu
===================
*/

  menu_edit = menuBar()->addMenu (tr ("Edit"));
  menu_edit->setTearOffEnabled (true);

  menu_edit->addAction (cutAct);
  menu_edit->addAction (copyAct);
  menu_edit->addAction (pasteAct);

  menu_edit->addSeparator();

  add_to_menu (menu_edit, tr ("Select all"), SLOT(ed_select_all()));

  menu_edit->addSeparator();

  add_to_menu (menu_edit, tr ("Block start"), SLOT(ed_block_start()));
  add_to_menu (menu_edit, tr ("Block end"), SLOT(ed_block_end()));
  add_to_menu (menu_edit, tr ("Copy block"), SLOT(ed_block_copy()));
  add_to_menu (menu_edit, tr ("Paste block"), SLOT(ed_block_paste()));
  add_to_menu (menu_edit, tr ("Cut block"), SLOT(ed_block_cut()));

  menu_edit->addSeparator();

  add_to_menu (menu_edit, tr ("Copy current file name"), SLOT(ed_copy_current_fname()));

  menu_edit->addSeparator();

  menu_edit->addAction (undoAct);
  menu_edit->addAction (redoAct);

  menu_edit->addSeparator();

  add_to_menu (menu_edit, tr ("Indent (tab)"), SLOT(ed_indent()));
  add_to_menu (menu_edit, tr ("Un-indent (shift+tab)"), SLOT(ed_unindent()));
  add_to_menu (menu_edit, tr ("Indent by first line"), SLOT(ed_indent_by_first_line()));

  menu_edit->addSeparator();

  add_to_menu (menu_edit, tr ("Comment selection"), SLOT(ed_comment()));

  menu_edit->addSeparator();

  add_to_menu (menu_edit, tr ("Set as storage file"), SLOT(ed_set_as_storage_file()));
  add_to_menu (menu_edit, tr ("Copy to storage file"), SLOT(ed_copy_to_storage_file()));
  add_to_menu (menu_edit, tr ("Start/stop capture clipboard to storage file"), SLOT(ed_capture_clipboard_to_storage_file()))->setCheckable (true);


/*
===================
Markup menu
===================
*/

  menu_markup = menuBar()->addMenu (tr ("Markup"));
  menu_markup->setTearOffEnabled (true);

  tm = menu_markup->addMenu (tr ("Mode"));
  tm->setTearOffEnabled (true);

  create_menu_from_list (this, tm,
                         QString ("HTML XHTML Docbook LaTeX Markdown Lout DokuWiki MediaWiki").split (" "),
                         SLOT (mrkup_mode_choosed()));

  tm = menu_markup->addMenu (tr ("Header"));
  tm->setTearOffEnabled (true);

  create_menu_from_list (this, tm,
                         QString ("H1 H2 H3 H4 H5 H6").split (" "),
                         SLOT (mrkup_header()));

  tm = menu_markup->addMenu (tr ("Align"));
  tm->setTearOffEnabled (true);

  add_to_menu (tm, tr ("Center"), SLOT(mrkup_align_center()));
  add_to_menu (tm, tr ("Left"), SLOT(mrkup_align_left()));
  add_to_menu (tm, tr ("Right"), SLOT(mrkup_align_right()));
  add_to_menu (tm, tr ("Justify"), SLOT(mrkup_align_justify()));

  add_to_menu (menu_markup, tr ("Bold"), SLOT(mrkup_bold()), "Alt+B");
  add_to_menu (menu_markup, tr ("Italic"), SLOT(mrkup_italic()), "Alt+I");
  add_to_menu (menu_markup, tr ("Underline"), SLOT(mrkup_underline()));

  add_to_menu (menu_markup, tr ("Link"), SLOT(mrkup_link()), "Alt+L");
  add_to_menu (menu_markup, tr ("Paragraph"), SLOT(mrkup_para()), "Alt+P");
  add_to_menu (menu_markup, tr ("Color"), SLOT(mrkup_color()));

  add_to_menu (menu_markup, tr ("Break line"), SLOT(mrkup_br()), "Ctrl+Return");
  add_to_menu (menu_markup, tr ("Non-breaking space"), SLOT(mrkup_nbsp()), "Ctrl+Space");
  add_to_menu (menu_markup, tr ("Insert image"), SLOT(markup_ins_image()));

  tm = menu_markup->addMenu (tr ("[X]HTML tools"));
  tm->setTearOffEnabled (true);

  add_to_menu (tm, tr ("Text to [X]HTML"), SLOT(mrkup_text_to_html()));
  add_to_menu (tm, tr ("Convert tags to entities"), SLOT(mrkup_tags_to_entities()));
  add_to_menu (tm, tr ("Antispam e-mail"), SLOT(mrkup_antispam_email()));
  add_to_menu (tm, tr ("Document weight"), SLOT(mrkup_document_weight()));
  add_to_menu (tm, tr ("Preview selected color"), SLOT(mrkup_preview_color()));
  add_to_menu (tm, tr ("Strip HTML tags"), SLOT(mrkup_strip_html_tags()));
  add_to_menu (tm, tr ("Rename selected file"), SLOT(mrkup_rename_selected()));

/*
===================
Search menu
===================
*/

  menu_search = menuBar()->addMenu (tr ("Search"));
  menu_search->setTearOffEnabled (true);

  add_to_menu (menu_search, tr ("Find"), SLOT(search_find()), "Ctrl+F");
  add_to_menu (menu_search, tr ("Find next"), SLOT(search_find_next()), "F3");
  add_to_menu (menu_search, tr ("Find previous"), SLOT(search_find_prev()),"Ctrl+F3");

  menu_search->addSeparator();

  add_to_menu (menu_search, tr ("Find in files"), SLOT(search_in_files()));

  menu_search->addSeparator();

  add_to_menu (menu_search, tr ("Replace with"), SLOT(search_replace_with()));
  add_to_menu (menu_search, tr ("Replace all"), SLOT(search_replace_all()));
  add_to_menu (menu_search, tr ("Replace all in opened files"), SLOT(search_replace_all_at_ofiles()));

  menu_search->addSeparator();

  add_to_menu (menu_search, tr ("Mark all found"), SLOT(search_mark_all()));
  add_to_menu (menu_search, tr ("Unmark"), SLOT(search_unmark()));

  menu_search->addSeparator();

  menu_find_case = menu_search->addAction (tr ("Case sensitive"));
  menu_find_case->setCheckable (true);

  menu_find_whole_words = menu_search->addAction (tr ("Whole words"));
  menu_find_whole_words->setCheckable (true);
  connect (menu_find_whole_words, SIGNAL(triggered()), this, SLOT(search_whole_words_mode()));

  menu_find_from_cursor = menu_search->addAction (tr ("From cursor"));
  menu_find_from_cursor->setCheckable (true);
  connect (menu_find_from_cursor, SIGNAL(triggered()), this, SLOT(search_from_cursor_mode()));

  menu_find_regexp = menu_search->addAction (tr ("Regexp mode"));
  menu_find_regexp->setCheckable (true);
  connect (menu_find_regexp, SIGNAL(triggered()), this, SLOT(search_regexp_mode()));

  menu_find_fuzzy = menu_search->addAction (tr ("Fuzzy mode"));
  menu_find_fuzzy->setCheckable (true);
  connect (menu_find_fuzzy, SIGNAL(triggered()), this, SLOT(search_fuzzy_mode()));


/*
===================
Functions menu
===================
*/


  menu_functions = menuBar()->addMenu (tr ("Functions"));
  menu_functions->setTearOffEnabled (true);

  add_to_menu (menu_functions, tr ("Repeat last"), SLOT(fn_repeat()));

  menu_instr = menu_functions->addMenu (tr ("Tools"));
  menu_instr->setTearOffEnabled (true);
  add_to_menu (menu_instr, tr ("Scale image"), SLOT(fn_scale_image()));

  menu_fn_snippets = menu_functions->addMenu (tr ("Snippets"));
  menu_fn_scripts = menu_functions->addMenu (tr ("Scripts"));
  menu_fn_tables = menu_functions->addMenu (tr ("Tables"));

  tm = menu_functions->addMenu (tr ("Place"));
  tm->setTearOffEnabled (true);

  add_to_menu (tm, "Lorem ipsum", SLOT(fn_insert_loremipsum()));
  add_to_menu (tm, tr ("TEA project template"), SLOT(fn_insert_template_tea()));
  add_to_menu (tm, tr ("HTML template"), SLOT(fn_insert_template_html()));
  add_to_menu (tm, tr ("HTML5 template"), SLOT(fn_insert_template_html5()));
  add_to_menu (tm, tr ("C++ template"), SLOT(fn_insert_cpp()));
  add_to_menu (tm, tr ("C template"), SLOT(fn_insert_c()));
  add_to_menu (tm, tr ("Date"), SLOT(fn_insert_date()));
  add_to_menu (tm, tr ("Time"), SLOT(fn_insert_time()));


  tm = menu_functions->addMenu (tr ("Case"));
  tm->setTearOffEnabled (true);

  add_to_menu (tm, tr ("UPCASE"), SLOT(fn_case_up()),"Ctrl+Up");
  add_to_menu (tm, tr ("lower case"), SLOT(fn_case_down()),"Ctrl+Down");
  add_to_menu (tm, tr ("Case inverse"), SLOT(fn_case_inverse()));

  add_to_menu (tm, tr ("Capitalize sentences"), SLOT(fn_case_cap_sentences()));


  tm = menu_functions->addMenu (tr ("Sort"));
  tm->setTearOffEnabled (true);

  add_to_menu (tm, tr ("Sort case sensitively"), SLOT(fn_sort_casecare()));
  add_to_menu (tm, tr ("Sort case insensitively"), SLOT(fn_sort_casecareless()));
  add_to_menu (tm, tr ("Sort case sensitively, with separator"), SLOT(fn_sort_casecare_sep()));
  add_to_menu (tm, tr ("Sort by length"), SLOT(fn_sort_length()));

  add_to_menu (tm, tr ("Flip a list"), SLOT(fn_flip_a_list()));
  add_to_menu (tm, tr ("Flip a list with separator"), SLOT(fn_flip_a_list_sep()));


  tm = menu_functions->addMenu (tr ("Cells"));
  tm->setTearOffEnabled (true);

  add_to_menu (tm, tr ("Sort table by column ABC"), SLOT(fn_cells_latex_table_sort_by_col_abc()));
  add_to_menu (tm, tr ("Swap cells"), SLOT(fn_cells_swap_cells()));
  add_to_menu (tm, tr ("Delete by column"), SLOT(fn_cells_delete_by_col()));
  add_to_menu (tm, tr ("Copy by column[s]"), SLOT(fn_cells_copy_by_col()));


  tm = menu_functions->addMenu (tr ("Filter"));
  tm->setTearOffEnabled (true);

  add_to_menu (tm, tr ("Remove duplicates"), SLOT(fn_filter_rm_duplicates()));
  add_to_menu (tm, tr ("Remove empty lines"), SLOT(fn_filter_rm_empty()));
  add_to_menu (tm, tr ("Remove lines < N size"), SLOT(fn_filter_rm_less_than()));
  add_to_menu (tm, tr ("Remove lines > N size"), SLOT(fn_filter_rm_greater_than()));
  add_to_menu (tm, tr ("Remove before delimiter at each line"), SLOT(fn_filter_delete_before_sep()));
  add_to_menu (tm, tr ("Remove after delimiter at each line"), SLOT(fn_filter_delete_after_sep()));
  add_to_menu (tm, tr ("Filter with regexp"), SLOT(fn_filter_with_regexp()));
  add_to_menu (tm, tr ("Filter by repetitions"), SLOT(fn_filter_by_repetitions()));


  tm = menu_functions->addMenu (tr ("Math"));
  tm->setTearOffEnabled (true);

  add_to_menu (tm, tr ("Evaluate"), SLOT(fn_math_evaluate()), "F4");
  add_to_menu (tm, tr ("Arabic to Roman"), SLOT(fn_math_number_arabic_to_roman()));
  add_to_menu (tm, tr ("Roman to Arabic"), SLOT(fn_math_number_roman_to_arabic()));
  add_to_menu (tm, tr ("Decimal to binary"), SLOT(fn_math_number_dec_to_bin()));
  add_to_menu (tm, tr ("Binary to decimal"), SLOT(fn_math_number_bin_to_dec()));
  add_to_menu (tm, tr ("Flip bits (bitwise complement)"), SLOT(fn_math_number_flip_bits()));
  add_to_menu (tm, tr ("Enumerate"), SLOT(fn_math_enum()));
  add_to_menu (tm, tr ("Sum by last column"), SLOT(fn_math_sum_by_last_col()));
  add_to_menu (tm, tr ("deg min sec > dec degrees"), SLOT(fn_math_number_dms2dc()));
  add_to_menu (tm, tr ("dec degrees > deg min sec"), SLOT(fn_math_number_dd2dms()));

#if QT_VERSION >= 0x060000

  add_to_menu (tm, tr ("Subtitles: shift timecodes by msecs"), SLOT(fn_math_srt_shift()));

#endif

  tm = menu_functions->addMenu (tr ("Morse code"));
  tm->setTearOffEnabled (true);

  add_to_menu (tm, tr ("From Russian to Morse"), SLOT(fn_morse_from_ru()));
  add_to_menu (tm, tr ("From Morse To Russian"), SLOT(fn_morse_to_ru()));
  add_to_menu (tm, tr ("From English to Morse"), SLOT(fn_morse_from_en()));
  add_to_menu (tm, tr ("From Morse To English"), SLOT(fn_morse_to_en()));


  tm = menu_functions->addMenu (tr ("Analyze"));
  tm->setTearOffEnabled (true);

  add_to_menu (tm, tr ("Text statistics"), SLOT(fn_analyze_text_stat()));
  add_to_menu (tm, tr ("Extract words"), SLOT(fn_analyze_extract_words()));
  add_to_menu (tm, tr ("Words lengths"), SLOT(fn_analyze_stat_words_lengths()));
  add_to_menu (tm, tr ("Count the substring"), SLOT(fn_analyze_count()));
  add_to_menu (tm, tr ("Count the substring (regexp)"), SLOT(fn_analyze_count_rx()));
  add_to_menu (tm, tr ("UNITAZ quantity sorting"), SLOT(fn_analyze_get_words_count()));
  add_to_menu (tm, tr ("UNITAZ sorting by alphabet"), SLOT(fn_analyze_unitaz_abc()));
  add_to_menu (tm, tr ("UNITAZ sorting by length"), SLOT(fn_analyze_unitaz_len()));


  tm = menu_functions->addMenu (tr ("Text"));
  tm->setTearOffEnabled (true);

  add_to_menu (tm, tr ("Apply to each line"), SLOT(fn_text_apply_to_each_line()),"Alt+E");
  add_to_menu (tm, tr ("Remove formatting"), SLOT(fn_text_remove_formatting()));
  add_to_menu (tm, tr ("Remove formatting at each line"), SLOT(fn_text_remove_formatting_at_each_line()));
  add_to_menu (tm, tr ("Remove trailing spaces"), SLOT(fn_text_remove_trailing_spaces()));
  add_to_menu (tm, tr ("Compress"), SLOT(fn_text_compress()));
  add_to_menu (tm, tr ("Anagram"), SLOT(fn_text_anagram()));
  add_to_menu (tm, tr ("Escape regexp"), SLOT(fn_text_escape()));
  add_to_menu (tm, tr ("Reverse"), SLOT(fn_text_reverse()));
  add_to_menu (tm, tr ("Compare two strings"), SLOT(fn_text_compare_two_strings()));
  add_to_menu (tm, tr ("Check regexp match"), SLOT(fn_text_regexp_match_check()));

  tm = menu_functions->addMenu (tr ("Quotes"));
  tm->setTearOffEnabled (true);

  add_to_menu (tm, tr ("Straight to double angle quotes"), SLOT(fn_quotes_to_angle()));
  add_to_menu (tm, tr ("Straight to curly double quotes"), SLOT(fn_quotes_curly()));
  add_to_menu (tm, tr ("LaTeX: Straight to curly double quotes"), SLOT(fn_quotes_tex_curly()));
  add_to_menu (tm, tr ("LaTeX: Straight to double angle quotes"), SLOT(fn_quotes_tex_angle_01()));
  add_to_menu (tm, tr ("LaTeX: Straight to double angle quotes v2"), SLOT(fn_quotes_tex_angle_02()));


#if defined (HUNSPELL_ENABLE) || defined (ASPELL_ENABLE)
  menu_functions->addSeparator();

  menu_spell_langs = menu_functions->addMenu (tr ("Spell-checker languages"));
  menu_spell_langs->setTearOffEnabled (true);

  add_to_menu (menu_functions, tr ("Spell check"), SLOT(fn_spell_check()), "", get_theme_icon_fname ("fn-spell-check.png"));
  add_to_menu (menu_functions, tr ("Suggest"), SLOT(fn_spell_suggest()));
  add_to_menu (menu_functions, tr ("Add to dictionary"), SLOT(fn_spell_add_to_dict()));
  add_to_menu (menu_functions, tr ("Remove from dictionary"), SLOT(fn_spell_remove_from_dict()));

  menu_functions->addSeparator();

#endif


/*
====================
Cal menu
===================
*/

  menu_cal = menuBar()->addMenu (tr ("Calendar"));
  menu_cal->setTearOffEnabled (true);

  add_to_menu (menu_cal, tr ("Moon mode on/off"), SLOT(cal_moon_mode()));
  add_to_menu (menu_cal, tr ("Mark first date"), SLOT(cal_set_date_a()));
  add_to_menu (menu_cal, tr ("Mark last date"), SLOT(cal_set_date_b()));

  menu_cal_add = menu_cal->addMenu (tr ("Add or subtract"));
  menu_cal_add->setTearOffEnabled (true);

  add_to_menu (menu_cal_add, tr ("Days"), SLOT(cal_add_days()));
  add_to_menu (menu_cal_add, tr ("Months"), SLOT(cal_add_months()));
  add_to_menu (menu_cal_add, tr ("Years"), SLOT(cal_add_years()));

  menu_cal->addSeparator();

  add_to_menu (menu_cal, tr ("Go to current date"), SLOT(cal_set_to_current()));
  add_to_menu (menu_cal, tr ("Calculate moon days between dates"), SLOT(cal_gen_mooncal()));
  add_to_menu (menu_cal, tr ("Number of days between two dates"), SLOT(cal_diff_days()));
  add_to_menu (menu_cal, tr ("Remove day record"), SLOT(cal_remove()));

/*
====================
Run menu
===================
*/

  menu_programs = menuBar()->addMenu (tr ("Run"));

/*
====================
IDE menu
===================
*/


  menu_ide = menuBar()->addMenu (tr ("IDE"));;
  menu_ide->setTearOffEnabled (true);

  add_to_menu (menu_ide, tr ("Run program"), SLOT(ide_run()));
  add_to_menu (menu_ide, tr ("Build program"), SLOT(ide_build()));
  add_to_menu (menu_ide, tr ("Clean program"), SLOT(ide_clean()));

  menu_ide->addSeparator();

  add_to_menu (menu_ide, tr ("Toggle header/source"), SLOT(ide_toggle_hs()));

/*
===================
Nav menu
===================
*/


  menu_nav = menuBar()->addMenu (tr ("Nav"));
  menu_nav->setTearOffEnabled (true);

  add_to_menu (menu_nav, tr ("Save position"), SLOT(nav_save_pos()));
  add_to_menu (menu_nav, tr ("Go to saved position"), SLOT(nav_goto_pos()));
  add_to_menu (menu_nav, tr ("Go to line"), SLOT(nav_goto_line()),"Alt+G");
  add_to_menu (menu_nav, tr ("Next tab"), SLOT(nav_goto_right_tab()));
  add_to_menu (menu_nav, tr ("Prev tab"), SLOT(nav_goto_left_tab()));
  add_to_menu (menu_nav, tr ("Focus the Famous input field"), SLOT(nav_focus_to_fif()), "Ctrl+F");
  add_to_menu (menu_nav, tr ("Focus the editor"), SLOT(nav_focus_to_editor()));

  menu_labels = menu_nav->addMenu (tr ("Labels"));
  add_to_menu (menu_nav, tr ("Refresh labels"), SLOT(nav_labels_update_list()));

  menu_current_files = menu_nav->addMenu (tr ("Current files"));

/*
===================
Fm menu callbacks
===================
*/

  menu_fm = menuBar()->addMenu (tr ("Fm"));
  menu_fm->setTearOffEnabled (true);

  menu_fm_multi_rename = menu_fm->addMenu (tr ("Multi-rename"));
  menu_fm_multi_rename->setTearOffEnabled (true);

  add_to_menu (menu_fm_multi_rename, tr ("Zero pad file names"), SLOT(fman_multi_rename_zeropad()));
  add_to_menu (menu_fm_multi_rename, tr ("Delete N first chars at file names"), SLOT(fman_multi_rename_del_n_first_chars()));
  add_to_menu (menu_fm_multi_rename, tr ("Replace in file names"), SLOT(fman_multi_rename_replace()));
  add_to_menu (menu_fm_multi_rename, tr ("Apply template"), SLOT(fman_multi_rename_apply_template()));

  menu_fm_file_ops = menu_fm->addMenu (tr ("File operations"));
  menu_fm_file_ops->setTearOffEnabled (true);

  add_to_menu (menu_fm_file_ops, tr ("Create new directory"), SLOT(fman_fileop_create_dir()));
  add_to_menu (menu_fm_file_ops, tr ("Rename"), SLOT(fman_fileop_rename()));
  add_to_menu (menu_fm_file_ops, tr ("Delete file"), SLOT(fman_fileop_delete()));


  menu_fm_file_infos = menu_fm->addMenu (tr ("File information"));
  menu_fm_file_infos->setTearOffEnabled (true);

  add_to_menu (menu_fm_file_infos, tr ("Count lines in selected files"), SLOT(fman_fileinfo_count_lines_in_selected_files()));
  add_to_menu (menu_fm_file_infos, tr ("Full info"), SLOT(fm_fileinfo_info()));

  menu_fm_zip = menu_fm->addMenu (tr ("ZIP"));
  menu_fm_zip->setTearOffEnabled (true);

  menu_fm_zip->addSeparator();

  add_to_menu (menu_fm_zip, tr ("Create new ZIP"), SLOT(fman_zip_create()));
  add_to_menu (menu_fm_zip, tr ("Add to ZIP"), SLOT(fman_zip_add()));
  add_to_menu (menu_fm_zip, tr ("Save ZIP"), SLOT(fman_zip_save()));

  menu_fm_zip->addSeparator();

  add_to_menu (menu_fm_zip, tr ("List ZIP content"), SLOT(fman_zip_info()));
  add_to_menu (menu_fm_zip, tr ("Unpack ZIP to current directory"), SLOT(fman_zip_unpack()));

  menu_fm_img_conv = menu_fm->addMenu (tr ("Images"));
  menu_fm_img_conv->setTearOffEnabled (true);

  add_to_menu (menu_fm_img_conv, tr ("Scale by side"), SLOT(fman_img_conv_by_side()));
  add_to_menu (menu_fm_img_conv, tr ("Scale by percentages"), SLOT(fman_img_conv_by_percent()));
  add_to_menu (menu_fm_img_conv, tr ("Create web gallery"), SLOT(fman_img_make_gallery()));

  add_to_menu (menu_fm, tr ("Go to home dir"), SLOT(fman_home()));
  add_to_menu (menu_fm, tr ("Refresh current dir"), SLOT(fman_refresh()));
  add_to_menu (menu_fm, tr ("Preview image"), SLOT(fman_preview_image()));
  add_to_menu (menu_fm, tr ("Select by regexp"), SLOT(fman_select_by_regexp()));
  add_to_menu (menu_fm, tr ("Deselect by regexp"), SLOT(fman_deselect_by_regexp()));

/*
===================
View menu
===================
*/

  menu_view = menuBar()->addMenu (tr ("View"));
  menu_view->setTearOffEnabled (true);

  menu_view_themes = menu_view->addMenu (tr ("Themes"));
  menu_view_themes->setTearOffEnabled (true);

  menu_view_palettes = menu_view->addMenu (tr ("Palettes"));
  menu_view_palettes->setTearOffEnabled (true);

  menu_view_keyboards = menu_view->addMenu (tr ("Keyboards"));
  menu_view_keyboards->setTearOffEnabled (true);


  //menu_view_hl = menu_view->addMenu (tr ("Highlighting mode"));
  //menu_view_hl->setTearOffEnabled (true);

  menu_view_profiles = menu_view->addMenu (tr ("Profiles"));
  menu_view_profiles->setTearOffEnabled (true);

#if QT_VERSION >= 0x051400
  add_to_menu (menu_view, tr ("Preview Markdown"), SLOT(view_preview_md()));
#endif


  add_to_menu (menu_view, tr ("Save profile"), SLOT(view_profile_save_as()));
  add_to_menu (menu_view, tr ("Toggle word wrap"), SLOT(view_toggle_wrap()));
  add_to_menu (menu_view, tr ("Hide error marks"), SLOT(view_hide_error_marks()));
  add_to_menu (menu_view, tr ("Toggle fullscreen"), SLOT(view_toggle_fs()));
  add_to_menu (menu_view, tr ("Stay on top"), SLOT(view_stay_on_top()));
  add_to_menu (menu_view, tr ("Darker"), SLOT(view_darker()));

/*
===================
? menu
===================
*/

  helpMenu = menuBar()->addMenu ("?");
  helpMenu->setTearOffEnabled (true);

  helpMenu->addAction (aboutAct);
  helpMenu->addAction (aboutQtAct);
  add_to_menu (helpMenu, tr ("NEWS"), SLOT(help_show_news()));
  add_to_menu (helpMenu, "TODO", SLOT(help_show_todo()));
  add_to_menu (helpMenu, "ChangeLog", SLOT(help_show_changelog()));
  add_to_menu (helpMenu, tr ("License"), SLOT(help_show_gpl()));
}


/*************************
----------------------
OPTIONS PAGE UI
----------------------
*************************/

void CTEA::create_options()
{
  tab_options = new QTabWidget;

  idx_tab_tune = main_tab_widget->addTab (tab_options, tr ("options"));

/*
----------------------
OPTIONS::INTERFACE
----------------------
*/

  QWidget *page_interface = new QWidget (tab_options);
  page_interface->setObjectName ("page_interface");

  QVBoxLayout *page_interface_layout = new QVBoxLayout;
  page_interface_layout->setAlignment (Qt::AlignTop);

  QStringList sl_ui_modes;
  sl_ui_modes << tr ("Classic") << tr ("Docked");

  cmb_ui_mode = new_combobox (page_interface_layout,
                              tr ("UI mode (TEA restart needed)"),
                              sl_ui_modes,
                              settings->value ("ui_mode", 0).toInt());


  QStringList sl_lngs = read_dir_entries (":/translations");

  for (QList <QString>::iterator i = sl_lngs.begin(); i != sl_lngs.end(); ++i)
      {
       (*i) = i->left(2);
      }

  sl_lngs.append ("en");

  QString lng = settings->value ("lng", QLocale::system().name()).toString().left(2).toLower();

  if (! file_exists (":/translations/" + lng + ".qm"))
     lng = "en";

  cmb_lng = new_combobox (page_interface_layout,
                          tr ("UI language (TEA restart needed)"),
                          sl_lngs,
                          settings->value ("lng", lng).toString());


  QString default_style = qApp->style()->objectName();
  if (default_style == "GTK+") //can be buggy, so disable it
     default_style = "Cleanlooks";

  cmb_styles = new_combobox (page_interface_layout,
                             tr ("UI style (TEA restart needed)"),
                             QStyleFactory::keys(),
                             settings->value ("ui_style", default_style).toString());


  connect (cmb_styles, SIGNAL(currentIndexChanged(int)),
           this, SLOT(slot_style_currentIndexChanged(int)));


  QPushButton *bt_font_interface = new QPushButton (tr ("Interface font"), this);
  connect (bt_font_interface, SIGNAL(clicked()), this, SLOT(slot_font_interface_select()));

  QPushButton *bt_font_editor = new QPushButton (tr ("Editor font"), this);
  connect (bt_font_editor, SIGNAL(clicked()), this, SLOT(slot_font_editor_select()));

  QPushButton *bt_font_logmemo = new QPushButton (tr ("Logmemo font"), this);
  connect (bt_font_logmemo, SIGNAL(clicked()), this, SLOT(slot_font_logmemo_select()));

  page_interface_layout->addWidget (bt_font_interface);
  page_interface_layout->addWidget (bt_font_editor);
  page_interface_layout->addWidget (bt_font_logmemo);

  QStringList sl_tabs_align;

  sl_tabs_align.append (tr ("Up"));
  sl_tabs_align.append (tr ("Bottom"));
  sl_tabs_align.append (tr ("Left"));
  sl_tabs_align.append (tr ("Right"));

  int ui_tab_align = settings->value ("ui_tabs_align", "3").toInt();
  main_tab_widget->setTabPosition (int_to_tabpos (ui_tab_align ));

  QComboBox *cmb_ui_tabs_align = new_combobox (page_interface_layout,
                                               tr ("GUI tabs align"),
                                               sl_tabs_align,
                                               ui_tab_align);

  connect (cmb_ui_tabs_align, SIGNAL(currentIndexChanged(int)),
           this, SLOT(cmb_ui_tabs_currentIndexChanged(int)));

  int docs_tab_align = settings->value ("docs_tabs_align", "0").toInt();
  tab_editor->setTabPosition (int_to_tabpos (docs_tab_align));


  QComboBox *cmb_docs_tabs_align = new_combobox (page_interface_layout,
                                                 tr ("Documents tabs align"),
                                                 sl_tabs_align,
                                                 docs_tab_align);

  connect (cmb_docs_tabs_align, SIGNAL(currentIndexChanged(int)),
           this, SLOT(cmb_docs_tabs_currentIndexChanged(int)));


  QStringList sl_icon_sizes;
  sl_icon_sizes << "16" << "24" << "32" << "48" << "64";

  cmb_icon_size = new_combobox (page_interface_layout,
                                tr ("Icons size"),
                                sl_icon_sizes,
                                settings->value ("icon_size", "32").toString());

  connect (cmb_icon_size, SIGNAL(currentIndexChanged(int)),
           this, SLOT(cmb_icon_sizes_currentIndexChanged(int)));

  QStringList sl_tea_icons;
  sl_tea_icons.append ("1");
  sl_tea_icons.append ("2");
  sl_tea_icons.append ("3");

  cmb_tea_icons = new_combobox (page_interface_layout,
                                tr ("TEA program icon"),
                                sl_tea_icons,
                                settings->value ("icon_fname", "1").toString());

  connect (cmb_tea_icons, SIGNAL(currentIndexChanged(int)),
           this, SLOT(cmb_tea_icons_currentIndexChanged(int)));

  cb_fif_at_toolbar = new QCheckBox (tr ("FIF at the top (restart needed)"), tab_options);
  cb_fif_at_toolbar->setChecked (settings->value ("fif_at_toolbar", "0").toBool());
  page_interface_layout->addWidget (cb_fif_at_toolbar);

  cb_show_linenums = new QCheckBox (tr ("Show line numbers"), tab_options);
  cb_show_linenums->setChecked (settings->value ("show_linenums", "0").toBool());
  page_interface_layout->addWidget (cb_show_linenums);

  cb_wordwrap = new QCheckBox (tr ("Word wrap"), tab_options);
  cb_wordwrap->setChecked (settings->value ("word_wrap", "1").toBool());
  page_interface_layout->addWidget (cb_wordwrap);

  cb_show_tabs_and_spaces = new QCheckBox (tr ("Show tabs and spaces"), tab_options);
  cb_show_tabs_and_spaces->setChecked (settings->value ("show_tabs_and_spaces", "0").toBool());
  page_interface_layout->addWidget (cb_show_tabs_and_spaces);


  cb_hl_enabled = new QCheckBox (tr ("Syntax highlighting enabled"), tab_options);
  cb_hl_enabled->setChecked (settings->value ("hl_enabled", "1").toBool());
  page_interface_layout->addWidget (cb_hl_enabled);

  cb_hl_current_line = new QCheckBox (tr ("Highlight current line"), tab_options);
  cb_hl_current_line->setChecked (settings->value ("additional_hl", "0").toBool());
  page_interface_layout->addWidget (cb_hl_current_line);

  cb_hl_brackets = new QCheckBox (tr ("Highlight paired brackets"), tab_options);
  cb_hl_brackets->setChecked (settings->value ("hl_brackets", "0").toBool());
  page_interface_layout->addWidget (cb_hl_brackets);

  cb_auto_indent = new QCheckBox (tr ("Automatic indent"), tab_options);
  cb_auto_indent->setChecked (settings->value ("auto_indent", "0").toBool());
  page_interface_layout->addWidget (cb_auto_indent);

  cb_spaces_instead_of_tabs = new QCheckBox (tr ("Use spaces instead of tabs"), tab_options);
  cb_spaces_instead_of_tabs->setChecked (settings->value ("spaces_instead_of_tabs", "1").toBool());
  page_interface_layout->addWidget (cb_spaces_instead_of_tabs);

  spb_tab_sp_width = new_spin_box (page_interface_layout,
                                   tr ("Tab width in spaces"), 1, 64,
                                   settings->value ("tab_sp_width", 8).toInt());

  cb_cursor_xy_visible = new QCheckBox (tr ("Show cursor position"), tab_options);
  cb_cursor_xy_visible->setChecked (settings->value ("cursor_xy_visible", "1").toBool());
  page_interface_layout->addWidget (cb_cursor_xy_visible);

  cb_center_on_cursor = new QCheckBox (tr ("Cursor center on scroll"), tab_options);
  cb_center_on_cursor->setChecked (settings->value ("center_on_scroll", "1").toBool());
  page_interface_layout->addWidget (cb_center_on_cursor);

  spb_cursor_blink_time = new_spin_box (page_interface_layout,
                                        tr ("Cursor blink time (msecs, zero is OFF)"), 0, 10000,
                                        settings->value ("cursor_blink_time", 0).toInt());


  spb_cursor_width = new_spin_box (page_interface_layout,
                                   tr ("Cursor width"), 1, 5,
                                   settings->value ("cursor_width", 2).toInt());



  cb_show_margin = new QCheckBox (tr ("Show margin at"), tab_options);
  cb_show_margin->setChecked (settings->value ("show_margin", "0").toBool());

  spb_margin_pos = new QSpinBox;
  spb_margin_pos->setValue (settings->value ("margin_pos", 72).toInt());

  QHBoxLayout *lt_margin = new QHBoxLayout;

  lt_margin->insertWidget (-1, cb_show_margin, 0, Qt::AlignLeft);
  lt_margin->insertWidget (-1, spb_margin_pos, 1, Qt::AlignLeft);

  page_interface_layout->addLayout (lt_margin);

  cb_full_path_at_window_title = new QCheckBox (tr ("Show full path at window title"), tab_options);
  cb_full_path_at_window_title->setChecked (settings->value ("full_path_at_window_title", "1").toBool());
  page_interface_layout->addWidget (cb_full_path_at_window_title);

  page_interface->setLayout (page_interface_layout);
  page_interface->show();


  QScrollArea *scra_interface = new QScrollArea;
  scra_interface->setWidgetResizable (true);
  scra_interface->setWidget (page_interface);

  tab_options->addTab (scra_interface, tr ("Interface"));

/*
----------------------
OPTIONS::COMMON
----------------------
*/

  QWidget *page_common = new QWidget (tab_options);
  QVBoxLayout *page_common_layout = new QVBoxLayout;
  page_common_layout->setAlignment (Qt::AlignTop);

  cb_altmenu = new QCheckBox (tr ("Use Alt key to access main menu"), tab_options);
  cb_altmenu->setChecked (MyProxyStyle::b_altmenu);

  connect (cb_altmenu, SIGNAL(stateChanged(int)),
           this, SLOT(cb_altmenu_stateChanged(int)));

  cb_wasd = new QCheckBox (tr ("Use Left Alt + WASD as additional cursor keys"), tab_options);
  cb_wasd->setChecked (settings->value ("wasd", "0").toBool());


#if defined(JOYSTICK_SUPPORTED)

  cb_use_joystick = new QCheckBox (tr ("Use joystick as cursor keys"), tab_options);
  cb_use_joystick->setChecked (settings->value ("use_joystick", "0").toBool());
  connect (cb_use_joystick, SIGNAL(stateChanged(int)), this, SLOT(cb_use_joystick_stateChanged(int)));
#endif

  cb_auto_img_preview = new QCheckBox (tr ("Automatic preview images at file manager"), tab_options);
  cb_auto_img_preview->setChecked (settings->value ("b_preview", "0").toBool());

  cb_session_restore = new QCheckBox (tr ("Restore the last session on start-up"), tab_options);
  cb_session_restore->setChecked (settings->value ("session_restore", "0").toBool());

  cb_use_enca_for_charset_detection = new QCheckBox (tr ("Use Enca for charset detection"), tab_options);
  cb_use_enca_for_charset_detection->setChecked (settings->value ("use_enca_for_charset_detection", 0).toBool());

  cb_override_img_viewer = new QCheckBox (tr ("Use external image viewer for F2"), tab_options);
  cb_override_img_viewer->setChecked (settings->value ("override_img_viewer", 0).toBool());

  ed_img_viewer_override = new QLineEdit (this);
  ed_img_viewer_override->setText (settings->value ("img_viewer_override_command", "display %s").toString());

  QHBoxLayout *hb_imgvovr = new QHBoxLayout;

  hb_imgvovr->addWidget (cb_override_img_viewer);
  hb_imgvovr->addWidget (ed_img_viewer_override);

  hb_imgvovr->insertWidget (-1, cb_override_img_viewer, 0, Qt::AlignLeft);
  hb_imgvovr->insertWidget (-1, ed_img_viewer_override, 1, Qt::AlignLeft);

  cb_use_trad_dialogs = new QCheckBox (tr ("Use traditional File Save/Open dialogs"), tab_options);
  cb_use_trad_dialogs->setChecked (settings->value ("use_trad_dialogs", "1").toBool());

  cb_start_on_sunday = new QCheckBox (tr ("Start week on Sunday"), tab_options);
  cb_start_on_sunday->setChecked (settings->value ("start_week_on_sunday", "0").toBool());

  cb_northern_hemisphere = new QCheckBox (tr ("Northern hemisphere"), this);
  cb_northern_hemisphere->setChecked (settings->value ("northern_hemisphere", "1").toBool());

  page_common_layout->addWidget (cb_start_on_sunday);
  page_common_layout->addWidget (cb_northern_hemisphere);

  QStringList sl_moon_algos;
  sl_moon_algos << "0" << "1" << "2" << "3";

/*
  cmb_moon_phase_algos = new_combobox (page_common_layout,
                                       tr ("Moon phase algorithm"),
                                       moon_phase_algos.values(),
                                       settings->value ("moon_phase_algo", MOON_PHASE_TRIG2).toInt());
*/


  cmb_moon_phase_algos = new_combobox (page_common_layout,
                                       tr ("Moon phase algorithm"),
                                       sl_moon_algos,
                                       settings->value ("moon_phase_algo", MOON_PHASE_TRIG2).toInt());

  cmb_moon_phase_algos->setToolTip (tr ("0 - Trigonometrical v2, 1 - Trigonometrical v1, 2 - Conway, 3 - Leueshkanov"));


  cmb_cmdline_default_charset = new_combobox (page_common_layout,
                                              tr ("Charset for file open from command line"),
                                              sl_charsets,
                                              sl_charsets.indexOf (settings->value ("cmdline_default_charset", "UTF-8").toString()));

  cmb_zip_charset_in = new_combobox (page_common_layout,
                                     tr ("ZIP unpacking: file names charset"),
                                     sl_charsets,
                                     sl_charsets.indexOf (settings->value ("zip_charset_in", "UTF-8").toString()));


  cmb_zip_charset_out = new_combobox (page_common_layout,
                                      tr ("ZIP packing: file names charset"),
                                      sl_charsets,
                                      sl_charsets.indexOf (settings->value ("zip_charset_out", "UTF-8").toString()));

  page_common_layout->addWidget (cb_altmenu);
  page_common_layout->addWidget (cb_wasd);

#if defined(JOYSTICK_SUPPORTED)
  page_common_layout->addWidget (cb_use_joystick);
#endif

  page_common_layout->addWidget (cb_auto_img_preview);
  page_common_layout->addWidget (cb_session_restore);
  page_common_layout->addWidget (cb_use_trad_dialogs);
  page_common_layout->addWidget (cb_use_enca_for_charset_detection);

  page_common_layout->addLayout (hb_imgvovr);



  QGroupBox *gb_common_autosave = new QGroupBox (tr ("Autosaving"));
  QVBoxLayout *vb_common_autosave = new QVBoxLayout;
  vb_common_autosave->setAlignment (Qt::AlignTop);
  gb_common_autosave->setLayout (vb_common_autosave);

  cb_save_buffers = new QCheckBox (tr ("Temporary save unsaved buffers on exit"), tab_options);
  cb_save_buffers->setChecked (settings->value ("save_buffers", "0").toBool());
  vb_common_autosave->addWidget (cb_save_buffers);

  cb_autosave = new QCheckBox (tr ("Autosave buffers and autosaving files (each N seconds)"), tab_options);
  cb_autosave->setChecked (settings->value ("autosave", "0").toBool());

  spb_autosave_period = new QSpinBox;
  spb_autosave_period->setMinimum (5);
  spb_autosave_period->setValue (settings->value ("autosave_period", 30).toInt());

  QHBoxLayout *lt_autosave = new QHBoxLayout;

  lt_autosave->insertWidget (-1, cb_autosave, 0, Qt::AlignLeft);
  lt_autosave->insertWidget (-1, spb_autosave_period, 1, Qt::AlignLeft);

  vb_common_autosave->addLayout (lt_autosave);



  page_common_layout->addWidget (gb_common_autosave);


  page_common->setLayout (page_common_layout);
  page_common->show();

  QScrollArea *scra_common = new QScrollArea;
  scra_common->setWidgetResizable (true);
  scra_common->setWidget (page_common);

  tab_options->addTab (scra_common, tr ("Common"));


/*
 ----------------------
OPTIONS::FUNCTIONS
----------------------
*/

  QWidget *page_functions = new QWidget (tab_options);
  QVBoxLayout *page_functions_layout = new QVBoxLayout;
  page_functions_layout->setAlignment (Qt::AlignTop);

  QGroupBox *gb_labels = new QGroupBox (tr ("Labels"));
  QVBoxLayout *vb_labels = new QVBoxLayout;
  gb_labels->setLayout (vb_labels);

  ed_label_start = new_line_edit (vb_labels, tr ("Label starts with: "), settings->value ("label_start", "[?").toString());
  ed_label_end = new_line_edit (vb_labels, tr ("Label ends with: "), settings->value ("label_end", "?]").toString());

  page_functions_layout->addWidget (gb_labels);

  QGroupBox *gb_datetime = new QGroupBox (tr ("Date and time"));
  QVBoxLayout *vb_datetime = new QVBoxLayout;
  gb_datetime->setLayout (vb_datetime);

  ed_date_format  = new_line_edit (vb_datetime, tr ("Date format"), settings->value ("date_format", "dd/MM/yyyy").toString());
  ed_time_format  = new_line_edit (vb_datetime, tr ("Time format"), settings->value ("time_format", "hh:mm:ss").toString());

  page_functions_layout->addWidget (gb_datetime);

  QLabel *l_t = 0;

#if defined (HUNSPELL_ENABLE) || defined (ASPELL_ENABLE)

  QGroupBox *gb_spell = new QGroupBox (tr ("Spell checking"));
  QVBoxLayout *vb_spell = new QVBoxLayout;
  gb_spell->setLayout(vb_spell);

  QHBoxLayout *hb_spellcheck_engine = new QHBoxLayout;

  cmb_spellcheckers = new_combobox (hb_spellcheck_engine,
                                    tr ("Spell checker engine"),
                                    spellcheckers,
                                    cur_spellchecker);

  vb_spell->addLayout (hb_spellcheck_engine);

#ifdef HUNSPELL_ENABLE

  QHBoxLayout *hb_spellcheck_path = new QHBoxLayout;
  l_t = new QLabel (tr ("Hunspell dictionaries directory"));

  ed_spellcheck_path = new QLineEdit (this);

  ed_spellcheck_path->setText (settings->value ("hunspell_dic_path", hunspell_default_dict_path()).toString());/*QDir::homePath ()).toString()*/
  ed_spellcheck_path->setReadOnly (true);

  QPushButton *pb_choose_path = new QPushButton (tr ("Select"), this);

  connect (pb_choose_path, SIGNAL(clicked()), this, SLOT(pb_choose_hunspell_path_clicked()));

  hb_spellcheck_path->addWidget (l_t);
  hb_spellcheck_path->addWidget (ed_spellcheck_path);
  hb_spellcheck_path->addWidget (pb_choose_path);

  vb_spell->addLayout (hb_spellcheck_path);

#endif


#ifdef ASPELL_ENABLE

#if defined(Q_OS_WIN) || defined(Q_OS_OS2)

  QHBoxLayout *hb_aspellcheck_path = new QHBoxLayout;
  l_t = new QLabel (tr ("Aspell directory"));

  ed_aspellcheck_path = new QLineEdit (this);
  ed_aspellcheck_path->setText (settings->value ("win32_aspell_path", aspell_default_dict_path()).toString());
  ed_aspellcheck_path->setReadOnly (true);

  QPushButton *pb_choose_path2 = new QPushButton (tr ("Select"), this);

  connect (pb_choose_path2, SIGNAL(clicked()), this, SLOT(pb_choose_aspell_path_clicked()));

  hb_aspellcheck_path->addWidget (l_t);
  hb_aspellcheck_path->addWidget (ed_aspellcheck_path);
  hb_aspellcheck_path->addWidget (pb_choose_path2);

  vb_spell->addLayout (hb_aspellcheck_path);

#endif

#endif

  connect (cmb_spellcheckers, SIGNAL(currentIndexChanged(int)),
           this, SLOT(cmb_spellchecker_currentIndexChanged(int)));

  page_functions_layout->addWidget (gb_spell);

#endif


  QGroupBox *gb_func_misc = new QGroupBox (tr ("Miscellaneous"));
  QVBoxLayout *vb_func_misc = new QVBoxLayout;
  vb_func_misc->setAlignment (Qt::AlignTop);
  gb_func_misc->setLayout (vb_func_misc);

  spb_fuzzy_q = new_spin_box (vb_func_misc, tr ("Fuzzy search factor"), 10, 100, settings->value ("fuzzy_q", "60").toInt());

  page_functions_layout->addWidget (gb_func_misc);

  cb_show_ebooks_fine = new QCheckBox (tr ("Show ebooks fine"), tab_options);
  cb_show_ebooks_fine->setChecked (settings->value ("show_ebooks_fine", "0").toBool());

  vb_func_misc->addWidget (cb_show_ebooks_fine);


  page_functions->setLayout (page_functions_layout);
  page_functions->show();



  QScrollArea *scra_functions = new QScrollArea;
  scra_functions->setWidgetResizable (true);
  scra_functions->setWidget (page_functions);

  tab_options->addTab (scra_functions, tr ("Functions"));

/*
----------------------
OPTIONS::IMAGES
----------------------
*/


  QWidget *page_images = new QWidget (tab_options);
  QVBoxLayout *page_images_layout = new QVBoxLayout;
  page_images_layout->setAlignment (Qt::AlignTop);

  QGroupBox *gb_images = new QGroupBox (tr ("Miscellaneous"));
  QVBoxLayout *vb_images = new QVBoxLayout;
  vb_images->setAlignment (Qt::AlignTop);

  gb_images->setLayout (vb_images);

  cmb_output_image_fmt = new_combobox (vb_images,
                                       tr ("Image conversion output format"),
                                       bytearray_to_stringlist (QImageWriter::supportedImageFormats()),
                                       settings->value ("output_image_fmt", "jpg").toString());

  cb_output_image_flt = new QCheckBox (tr ("Scale images with bilinear filtering"), this);
  cb_output_image_flt->setChecked (settings->value ("img_filter", 0).toBool());

  vb_images->addWidget (cb_output_image_flt);

  spb_img_quality = new_spin_box (vb_images, tr ("Output images quality"), -1, 100, settings->value ("img_quality", "-1").toInt());

  cb_exif_rotate = new QCheckBox (tr ("Apply hard rotation by EXIF data"), this);
  cb_exif_rotate->setChecked (settings->value ("cb_exif_rotate", 1).toBool());

  cb_output_image_flt = new QCheckBox (tr ("Scale images with bilinear filtering"), this);
  cb_output_image_flt->setChecked (settings->value ("img_filter", 0).toBool());

  vb_images->addWidget (cb_output_image_flt);

  cb_zip_after_scale = new QCheckBox (tr ("Zip directory with processed images"), this);
  cb_zip_after_scale->setChecked (settings->value ("img_post_proc", 0).toBool());

  vb_images->addWidget (cb_zip_after_scale);
  vb_images->addWidget (cb_exif_rotate);
  page_images_layout->addWidget (gb_images);

  QGroupBox *gb_webgallery = new QGroupBox (tr ("Web gallery options"));
  QVBoxLayout *vb_webgal = new QVBoxLayout;
  vb_webgal->setAlignment (Qt::AlignTop);

  ed_side_size = new_line_edit (vb_webgal, tr ("Size of the side"), settings->value ("ed_side_size", "110").toString());
  ed_link_options = new_line_edit (vb_webgal, tr ("Link options"), settings->value ("ed_link_options", "target=\"_blank\"").toString());
  ed_cols_per_row = new_line_edit (vb_webgal, tr ("Columns per row"), settings->value ("ed_cols_per_row", "4").toString());

  gb_webgallery->setLayout(vb_webgal);
  page_images_layout->addWidget (gb_webgallery);

  QGroupBox *gb_exif = new QGroupBox (tr ("EXIF"));
  QVBoxLayout *vb_exif = new QVBoxLayout;
  gb_exif->setLayout(vb_exif);
  page_images_layout->addWidget (gb_exif);

  cb_zor_use_exif= new QCheckBox (tr ("Use EXIF orientation at image viewer"), this);
  cb_zor_use_exif->setChecked (settings->value ("zor_use_exif_orientation", 0).toBool());
  vb_exif->addWidget (cb_zor_use_exif);


  page_images->setLayout (page_images_layout);

  QScrollArea *scra_images = new QScrollArea;
  scra_images->setWidgetResizable (true);
  scra_images->setWidget (page_images);

  tab_options->addTab (scra_images, tr ("Images"));


/*
----------------------
OPTIONS::KEYBOARD
----------------------
*/


  QWidget *page_keyboard = new QWidget (tab_options);

  QHBoxLayout *lt_h = new QHBoxLayout;
  QHBoxLayout *lt_shortcut = new QHBoxLayout;
  QVBoxLayout *lt_vkeys = new QVBoxLayout;
  QVBoxLayout *lt_vbuttons = new QVBoxLayout;

  lv_menuitems = new QListWidget;

  lt_vkeys->addWidget (lv_menuitems);

  connect (lv_menuitems, SIGNAL(currentItemChanged (QListWidgetItem*,QListWidgetItem*)),
           this, SLOT(slot_lv_menuitems_currentItemChanged(QListWidgetItem*,QListWidgetItem*)));

  ent_shtcut = new CShortcutEntry;
  QLabel *l_shortcut = new QLabel (tr ("Shortcut"));

  lt_shortcut->addWidget (l_shortcut);
  lt_shortcut->addWidget (ent_shtcut);

  lt_vbuttons->addLayout (lt_shortcut);

  QPushButton *pb_assign_hotkey = new QPushButton (tr ("Assign"), this);
  QPushButton *pb_remove_hotkey = new QPushButton (tr ("Remove"), this);

  connect (pb_assign_hotkey, SIGNAL(clicked()), this, SLOT(pb_assign_hotkey_clicked()));
  connect (pb_remove_hotkey, SIGNAL(clicked()), this, SLOT(pb_remove_hotkey_clicked()));

  lt_vbuttons->addWidget (pb_assign_hotkey);
  lt_vbuttons->addWidget (pb_remove_hotkey, 0, Qt::AlignTop);

  lt_h->addLayout (lt_vkeys);
  lt_h->addLayout (lt_vbuttons);

  page_keyboard->setLayout (lt_h);
  page_keyboard->show();

  idx_tab_keyboard = tab_options->addTab (page_keyboard, tr ("Keyboard"));
}


void CTEA::create_calendar()
{
  calendar = new CCalendarWidget (this, dir_days);

  calendar->moon_mode = settings->value ("moon_mode", "0").toBool();
  calendar->northern_hemisphere = settings->value ("northern_hemisphere", "1").toBool();
  calendar->moon_phase_algo = settings->value ("moon_phase_algo", MOON_PHASE_TRIG2).toInt();
  calendar->setGridVisible (true);
  calendar->setVerticalHeaderFormat (QCalendarWidget::NoVerticalHeader);

  if (settings->value ("start_on_sunday", "0").toBool())
     calendar->setFirstDayOfWeek (Qt::Sunday);
  else
      calendar->setFirstDayOfWeek (Qt::Monday);

  connect (calendar, SIGNAL(clicked(QDate)), this, SLOT(calendar_clicked(QDate)));
  connect (calendar, SIGNAL(activated(QDate)), this, SLOT(calendar_activated(QDate)));
  connect (calendar, SIGNAL(currentPageChanged(int,int)), this, SLOT(calendar_currentPageChanged(int,int)));

  idx_tab_calendar = main_tab_widget->addTab (calendar, tr ("dates"));
}


void CTEA::create_toolbars()
{
  openAct->setMenu (menu_file_recent);
  filesAct->setMenu (menu_current_files);
  act_labels->setMenu (menu_labels);

  fileToolBar = addToolBar (tr ("File"));
  fileToolBar->setObjectName ("fileToolBar");
  fileToolBar->addAction (newAct);
  fileToolBar->addAction (openAct);
  fileToolBar->addAction (saveAct);

  editToolBar = addToolBar (tr ("Edit"));
  editToolBar->setObjectName ("editToolBar");
  editToolBar->addAction (cutAct);
  editToolBar->addAction (copyAct);
  editToolBar->addAction (pasteAct);

  if (! settings->value ("fif_at_toolbar", 0).toBool())
     {
      editToolBar->addSeparator();
      editToolBar->addAction (act_labels);
     }

  filesToolBar = addToolBar (tr ("Files"));
  filesToolBar->setObjectName ("filesToolBar");

  filesToolBar->setIconSize (QSize (icon_size, icon_size));

  QToolButton *tb_current_list = new QToolButton();
  tb_current_list->setIcon (get_theme_icon ("current-list.png"));

  tb_current_list->setMenu (menu_current_files);
  tb_current_list->setPopupMode(QToolButton::InstantPopup);
  filesToolBar->addWidget (tb_current_list);

  if (settings->value ("fif_at_toolbar", 0).toBool())
     {
      fifToolBar = addToolBar (tr ("FIF"));
      fifToolBar->setObjectName ("fifToolBar");

      cmb_fif = new QComboBox;
      cmb_fif->setInsertPolicy (QComboBox::InsertAtTop);
      cmb_fif->setObjectName ("FIF");

      cmb_fif->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Fixed);

      cmb_fif->setEditable (true);
      fif = cmb_fif->lineEdit();
      connect (fif, SIGNAL(returnPressed()), this, SLOT(search_find()));

      fifToolBar->addWidget (cmb_fif);

      QAction *act_fif_find = fifToolBar->addAction (style()->standardIcon(QStyle::SP_ArrowForward), "");
      act_fif_find->setToolTip (tr ("Find"));
      connect (act_fif_find, SIGNAL(triggered()), this, SLOT(search_find()));

      QAction *act_fif_find_next = fifToolBar->addAction (style()->standardIcon(QStyle::SP_ArrowDown), "");
      act_fif_find_next->setToolTip (tr ("Find next"));
      connect (act_fif_find_next, SIGNAL(triggered()), this, SLOT(search_find_next()));

      QAction *act_fif_find_prev = fifToolBar->addAction (style()->standardIcon(QStyle::SP_ArrowUp), "");
      act_fif_find_prev->setToolTip (tr ("Find previous"));
      connect (act_fif_find_prev, SIGNAL(triggered()), this, SLOT(search_find_prev()));
     }
}


void CTEA::create_manual()
{
  QWidget *wd_man = new QWidget (this);

  QVBoxLayout *lv_t = new QVBoxLayout;

  QString loc = QLocale::system().name().left (2).toLower();
  QString ts = settings->value ("lng", loc).toString();

  QString filename (":/manuals/");
  filename = filename + ts + ".html";

  if (! file_exists (filename))
      filename = ":/manuals/en.html";

  man_search_value = "";

  QHBoxLayout *lh_controls = new QHBoxLayout();

  QPushButton *bt_back = new QPushButton ("<");
  QPushButton *bt_forw = new QPushButton (">");

  lh_controls->addWidget (bt_back);
  lh_controls->addWidget (bt_forw);

  man = new QTextBrowser;
  man->setOpenExternalLinks (true);
  man->setSource (QUrl ("qrc" + filename));

  connect (bt_back, SIGNAL(clicked()), man, SLOT(backward()));
  connect (bt_forw, SIGNAL(clicked()), man, SLOT(forward()));

  lv_t->addLayout (lh_controls);
  lv_t->addWidget (man);

  wd_man->setLayout (lv_t);

  idx_tab_learn = main_tab_widget->addTab (wd_man, tr ("manual"));
}


void CTEA::create_fman()
{
  QWidget *wd_fman = new QWidget (this);

  QVBoxLayout *lav_main = new QVBoxLayout;
  QVBoxLayout *lah_controls = new QVBoxLayout;
  QHBoxLayout *lah_topbar = new QHBoxLayout;

  QLabel *l_t = new QLabel (tr ("Name"));
  ed_fman_fname = new QLineEdit;
  connect (ed_fman_fname, SIGNAL(returnPressed()), this, SLOT(fman_fname_entry_confirm()));

  ed_fman_path = new QLineEdit;
  connect (ed_fman_path, SIGNAL(returnPressed()), this, SLOT(fman_naventry_confirm()));

  tb_fman_dir = new QToolBar;
  tb_fman_dir->setObjectName ("tb_fman_dir");

  QAction *act_fman_go = new QAction (style()->standardIcon(QStyle::SP_ArrowForward), tr ("Go"), this);
  connect (act_fman_go, SIGNAL(triggered()), this, SLOT(fman_naventry_confirm()));

  QAction *act_fman_home = new QAction (style()->standardIcon(QStyle::SP_DirHomeIcon), tr ("Home"), this);
  connect (act_fman_home, SIGNAL(triggered()), this, SLOT(fman_home()));

  QAction *act_fman_refresh = new QAction (style()->standardIcon(QStyle::SP_BrowserReload), tr ("Refresh"), this);
  QAction *act_fman_ops = new QAction (style()->standardIcon(QStyle::SP_DriveHDIcon), tr ("Actions"), this);
  act_fman_ops->setMenu (menu_fm_file_ops);

  tb_fman_dir->addAction (act_fman_go);
  tb_fman_dir->addAction (act_fman_home);
  tb_fman_dir->addAction (act_fman_refresh);
  tb_fman_dir->addAction (act_fman_ops);

#if defined(Q_OS_WIN) || defined(Q_OS_OS2)

  cb_fman_drives = new QComboBox;
  lah_topbar->addWidget (cb_fman_drives);

  QFileInfoList l_drives = QDir::drives();
  for (QList <QFileInfo>::iterator fi = l_drives.begin(); fi != l_drives.end(); ++fi)
       cb_fman_drives->addItem (fi->path());

#endif

  lah_topbar->addWidget (ed_fman_path);
  lah_topbar->addWidget (tb_fman_dir);

  lah_controls->addWidget (l_t);
  lah_controls->addWidget (ed_fman_fname);

  l_t = new QLabel (tr ("Charset"));

  QPushButton *bt_magicenc = new QPushButton ("?", this);
  bt_magicenc->setToolTip (tr ("Guess encoding!"));
  connect (bt_magicenc, SIGNAL(clicked()), this, SLOT(guess_enc()));

  /*
#if QT_VERSION >= 0x051100
  bt_magicenc->setMaximumWidth (QApplication::fontMetrics().horizontalAdvance("???"));
#else
  bt_magicenc->setMaximumWidth (QApplication::fontMetrics().width ("???"));
#endif
*/

  cb_fman_codecs = new QComboBox;

  if (sl_last_used_charsets.size () > 0)
     cb_fman_codecs->addItems (sl_last_used_charsets + sl_charsets);
  else
     {
      cb_fman_codecs->addItems (sl_charsets);
      cb_fman_codecs->setCurrentIndex (sl_charsets.indexOf ("UTF-8"));
     }

  QPushButton *bt_fman_open = new QPushButton (tr ("Open"), this);
  connect (bt_fman_open, SIGNAL(clicked()), this, SLOT(fman_open()));
  bt_fman_open->setToolTip (tr ("Open a file from the file name provided above"));

  QPushButton *bt_fman_save_as = new QPushButton (tr ("Save as"), this);
  connect (bt_fman_save_as, SIGNAL(clicked()), this, SLOT(cb_button_saves_as()));
  bt_fman_save_as->setToolTip (tr ("Save the current opened file with the name provided above"));

  lah_controls->addWidget (l_t);

  QHBoxLayout *lt_hb = new QHBoxLayout;

  lt_hb->addWidget (cb_fman_codecs);
  lt_hb->addWidget (bt_magicenc);

  lah_controls->addLayout (lt_hb);

  lah_controls->addWidget (bt_fman_open);
  lah_controls->addWidget (bt_fman_save_as);

  fman = new CFMan;

  connect (fman, SIGNAL(file_activated(QString)), this, SLOT(fman_file_activated(QString)));
  connect (fman, SIGNAL(dir_changed(QString)), this, SLOT(fman_dir_changed(QString)));
  connect (fman, SIGNAL(current_file_changed(QString,QString)), this, SLOT(fman_current_file_changed(QString,QString)));

  connect (act_fman_refresh, SIGNAL(triggered()), fman, SLOT(refresh()));

#if defined(Q_OS_WIN) || defined(Q_OS_OS2)
  connect (cb_fman_drives, SIGNAL(currentIndexChanged (QString)),
          this, SLOT(fman_drives_changed(QString)));
#endif

  w_right = new QWidget (this);

  w_right->setMinimumWidth (10);

  QVBoxLayout *lw_right = new QVBoxLayout;
  w_right->setLayout (lw_right);
  lw_right->addLayout (lah_controls);

  QFrame *vline = new QFrame;
  vline->setFrameStyle (QFrame::HLine);
  lw_right->addWidget (vline);

  QLabel *l_bookmarks = new QLabel (tr ("<b>Bookmarks</b>"));
  lw_right->addWidget (l_bookmarks);

  QHBoxLayout *lah_places_bar = new QHBoxLayout;
  QPushButton *bt_add_bmk = new QPushButton ("+");
  QPushButton *bt_del_bmk = new QPushButton ("-");
  lah_places_bar->addWidget (bt_add_bmk);
  lah_places_bar->addWidget (bt_del_bmk);

  connect (bt_add_bmk, SIGNAL(clicked()), this, SLOT(fman_add_bmk()));
  connect (bt_del_bmk, SIGNAL(clicked()), this, SLOT(fman_del_bmk()));

  lv_places = new QListWidget;
  //lv_places->setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOn);

  update_places_bookmarks();
  connect (lv_places, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(fman_places_itemActivated(QListWidgetItem*)));

  QVBoxLayout *vbox = new QVBoxLayout;
  vbox->addLayout (lah_places_bar);
  vbox->addWidget (lv_places);

  lw_right->addLayout (vbox);

//commented out with Qt6
 // fman->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);

  spl_fman = new QSplitter (this);
  spl_fman->setChildrenCollapsible (true);

  spl_fman->addWidget (fman);
  spl_fman->addWidget (w_right);

  spl_fman->restoreState (settings->value ("spl_fman").toByteArray());

  lav_main->addLayout (lah_topbar);
  lav_main->addWidget (spl_fman);

  wd_fman->setLayout (lav_main);

  fman_home();

  idx_tab_fman = main_tab_widget->addTab (wd_fman, tr ("files"));
}


void CTEA::create_markup_hash()
{
  QHash<QString, QString> h1;

  h1["Docbook"] = "<emphasis role=\"bold\">%s</emphasis>";
  h1["LaTeX"] = "\\textbf{%s}";
  h1["HTML"] = "<b>%s</b>";
  h1["XHTML"] = "<b>%s</b>";
  h1["Lout"] = "@B{%s}";
  h1["MediaWiki"] = "'''%s'''";
  h1["DokuWiki"] = "**%s**";
  h1["Markdown"] = "**%s**";

  hash_markup.insert ("bold", h1);

  QHash<QString, QString> h2;

  h2["Docbook"] = "<emphasis role=\"italic\">%s</emphasis>";
  h2["LaTeX"] = "\\textit{%s}";
  h2["HTML"] = "<i>%s</i>";
  h2["XHTML"] = "<i>%s</i>";
  h2["Lout"] = "@I{%s}";
  h2["MediaWiki"] = "''%s''";
  h2["DokuWiki"] = "//%s//";
  h2["Markdown"] = "*%s*";

  hash_markup.insert ("italic", h2);

  QHash<QString, QString> h3;

  h3["HTML"] = "<p style=\"text-align:justify;\">%s</p>";
  h3["XHTML"] = "<p style=\"text-align:justify;\">%s</p>";

  hash_markup.insert ("align_justify", h3);

  QHash<QString, QString> h4;

  h4["LaTeX"] = "\\begin{center}%s\\end{center}";
  h4["HTML"] = "<p style=\"text-align:center;\">%s</p>";
  h4["XHTML"] = "<p style=\"text-align:center;\">%s</p>";

  hash_markup.insert ("align_center", h4);

  QHash<QString, QString> h5;

  h5["LaTeX"] = "\\begin{flushleft}%s\\end{flushleft}";
  h5["HTML"] = "<p style=\"text-align:left;\">%s</p>";
  h5["XHTML"] = "<p style=\"text-align:left;\">%s</p>";

  hash_markup.insert ("align_left", h5);

  QHash<QString, QString> h6;

  h6["LaTeX"] = "\\begin{flushright}%s\\end{flushright}";
  h6["HTML"] = "<p style=\"text-align:right;\">%s</p>";
  h6["XHTML"] = "<p style=\"text-align:right;\">%s</p>";

  hash_markup.insert ("align_right", h6);

  QHash<QString, QString> h7;

  h7["Docbook"] = "<emphasis role=\"underline\">%s</emphasis>";
  h7["LaTeX"] = "\\underline{%s}";
  h7["HTML"] = "<u>%s</u>";
  h7["XHTML"] = "<u>%s</u>";
  h7["Lout"] = "@Underline{%s}";
  h7["MediaWiki"] = "<u>%s</u>";
  h7["DokuWiki"] = "__%s__";

  hash_markup.insert ("underline", h7);

  QHash<QString, QString> h8;

  h8["Docbook"] = "<para>%s</para>";
  h8["HTML"] = "<p>%s</p>";
  h8["XHTML"] = "<p>%s</p>";
  h8["Lout"] = "@PP%s";

  hash_markup.insert ("para", h8);

  QHash<QString, QString> h9;

  h9["Docbook"] = "<ulink url=\"\">%s</ulink>";
  h9["HTML"] = "<a href=\"\">%s</a>";
  h9["XHTML"] = "<a href=\"\">%s</a>";
  h9["LaTeX"] = "\\href{}{%s}";
  h9["Markdown"] = "[%s]()";

  hash_markup.insert ("link", h9);

  QHash<QString, QString> h10;

  h10["LaTeX"] = "\\newline";
  h10["HTML"] = "<br>";
  h10["XHTML"] = "<br />";
  h10["Lout"] = "@LLP";
  h10["MediaWiki"] = "<br />";
  h10["DokuWiki"] = "\\\\ ";
  h10["Markdown"] = " \n";

  hash_markup.insert ("newline", h10);
}


void CTEA::update_stylesheet (const QString &f)
{
//Update paletted stuff

  int darker_val = settings->value ("darker_val", 100).toInt();

  QFontInfo fi = QFontInfo (qApp->font());

  QString fontsize = "font-size:" + settings->value ("app_font_size", fi.pointSize()).toString() + "pt;";
  QString fontfamily = "font-family:" + settings->value ("app_font_name", qApp->font().family()).toString() + ";";

  QString edfontsize = "font-size:" + settings->value ("editor_font_size", "16").toString() + "pt;";
  QString edfontfamily = "font-family:" + settings->value ("editor_font_name", "Serif").toString() + ";";

  QString logmemo_fontsize = "font-size:" + settings->value ("logmemo_font_size", "12").toString() + "pt;";
  QString logmemo_font = "font-family:" + settings->value ("logmemo_font", "Monospace").toString() + ";";

  QString stylesheet;

  stylesheet = "QWidget, QWidget * {" + fontfamily + fontsize + "}\n";
  stylesheet += "QPlainTextEdit, QPlainTextEdit * {" + edfontfamily + edfontsize + "}\n";
  stylesheet += "QTextEdit {" + edfontfamily + edfontsize + "}\n";
  stylesheet += "CLogMemo {" + logmemo_font + logmemo_fontsize + "}\n";
  stylesheet += "CLineNumberArea {" + edfontfamily + edfontsize + "}\n";

  QString text_color = hash_get_val (global_palette, "text", "black");
  QString t_text_color = QColor (text_color).darker(darker_val).name();

  QString back_color = hash_get_val (global_palette, "background", "white");
  QString t_back_color = QColor (back_color).darker(darker_val).name();

  QString sel_back_color = hash_get_val (global_palette, "sel-background", "black");
  QString sel_text_color = hash_get_val (global_palette, "sel-text", "white");

  QString t_sel_text_color = QColor (sel_text_color).darker(darker_val).name();
  QString t_sel_back_color = QColor (sel_back_color).darker(darker_val).name();

  QString css_plain_text_edit = QString ("QPlainTextEdit {color: %1; background-color: %2; selection-color: %3; selection-background-color: %4;}\n").arg (
                                         t_text_color).arg (
                                         t_back_color).arg (
                                         t_sel_text_color).arg (
                                         t_sel_back_color);

  stylesheet += css_plain_text_edit;

  QString css_tea_edit = QString ("CTEAEdit {color: %1; background-color: %2; selection-color: %3; selection-background-color: %4;}\n").arg (
                                  t_text_color).arg (
                                  t_back_color).arg (
                                  t_sel_text_color).arg (
                                  t_sel_back_color);

  stylesheet += css_tea_edit;

  QString css_tea_man = QString ("QTextBrowser {color: %1; background-color: %2; selection-color: %3; selection-background-color: %4;}\n").arg (
                                  t_text_color).arg (
                                  t_back_color).arg (
                                  t_sel_text_color).arg (
                                  t_sel_back_color);

  stylesheet += css_tea_man;

  QString css_fif = QString ("QComboBox#FIF { color: %1; background-color: %2; selection-color: %3; selection-background-color: %4;}\n").arg (
                             t_text_color).arg (
                             t_back_color).arg (
                             t_sel_text_color).arg (
                             t_sel_back_color);

  stylesheet += css_fif;

//Update themed stuff

  QString cssfile = qstring_load (f);
  QString css_path = get_file_path (f) + "/";

  cssfile = cssfile.replace ("./", css_path);
  cssfile += stylesheet;

  qApp->setStyleSheet ("");
  qApp->setStyleSheet (cssfile);
}


void CTEA::update_styles()
{
#if QT_VERSION >= 0x050000
  QString default_style = qApp->style()->objectName();

  if (default_style == "GTK+") //can be buggy
     default_style = "Fusion";

#else

  QString default_style = qApp->style()->objectName();

  if (default_style == "GTK+") //can be buggy
     default_style = "Cleanlooks";

#endif

  fname_stylesheet = settings->value ("fname_stylesheet", ":/themes/TEA").toString();

  MyProxyStyle *ps = new MyProxyStyle (QStyleFactory::create (settings->value ("ui_style", default_style).toString()));

  QApplication::setStyle (ps);
}


void CTEA::update_dyn_menus()
{
  update_templates();
  update_snippets();
  update_keyboards();
  update_scripts();
  update_palettes();
  update_themes();
  update_tables();
  update_profiles();
  update_labels_menu();
}


void CTEA::update_fonts()
{
  documents->apply_settings();
}


void CTEA::update_bookmarks()
{
  if (! file_exists (fname_bookmarks))
     return;

  QString bookmarks = qstring_load (fname_bookmarks);
  if (bookmarks.isEmpty())
     return;

  menu_file_bookmarks->clear();
  create_menu_from_list (this, menu_file_bookmarks,
                         bookmarks.split ("\n"),
                         SLOT (file_open_bookmark()));
}


void CTEA::update_templates()
{
  menu_file_templates->clear();

  create_menu_from_dir (this,
                        menu_file_templates,
                        dir_templates,
                        SLOT (file_use_template())
                       );
}


void CTEA::update_snippets()
{
   menu_fn_snippets->clear();
   create_menu_from_dir (this,
                         menu_fn_snippets,
                         dir_snippets,
                         SLOT (fn_use_snippet())
                        );
}


void CTEA::update_keyboards()
{
 // qDebug() << "update_keyboards()";
//  qDebug() << dir_keyboards;

   menu_view_keyboards->clear();
   create_menu_from_dir (this,
                         menu_view_keyboards,
                         dir_keyboards,
                         SLOT (view_use_keyboard())
                        );
}


void CTEA::update_sessions()
{
  menu_file_sessions->clear();
  create_menu_from_dir (this,
                        menu_file_sessions,
                        dir_sessions,
                        SLOT (file_open_session())
                       );
}


void CTEA::update_palettes()
{
  menu_view_palettes->clear();

  QStringList l1 = read_dir_entries (dir_palettes);
  QStringList l2 = read_dir_entries (":/palettes");
  l1 += l2;

  create_menu_from_list (this, menu_view_palettes,
                         l1,
                         SLOT (view_use_palette()));
}


void CTEA::update_labels_menu()
{
  menu_labels->clear();

  CDocument *d = documents->get_current();
  if (d)
     create_menu_from_list (this, menu_labels, d->labels, SLOT(select_label()));
}


void CTEA::update_themes()
{
  menu_view_themes->clear();

  create_menu_from_themes (this,
                           menu_view_themes,
                           ":/themes",
                           SLOT (view_use_theme())
                          );

  create_menu_from_themes (this,
                           menu_view_themes,
                           dir_themes,
                           SLOT (view_use_theme())
                           );
}


void CTEA::update_hls()
{

  CFilesList lf;
  lf.get (":/hls");

  QStringList l (lf.list);

  lf.get (dir_hls);
  l << lf.list;

  for (int i = 0; i < l.size(); i++)
      {
       QString fname = l[i];

       QString buffer = qstring_load_first_line (fname);
       QString rgxp = string_between (buffer, "pattern=\"", "\"");

       if (! rgxp.isEmpty())
          {

#if QT_VERSION >= 0x050000

           QRegularExpression re (rgxp, QRegularExpression::CaseInsensitiveOption);
           if (re.isValid())
              documents->hl_files.push_back (std::make_pair (re, fname));

#else

           QRegExp re (rgxp, Qt::CaseInsensitive, QRegExp::RegExp2);
           if (re.isValid())
              documents->hl_files.push_back(std::make_pair (re, fname));

#endif
          }
      }
}


void CTEA::update_tables()
{
  menu_fn_tables->clear();

  create_menu_from_dir (this,
                        menu_fn_tables,
                        dir_tables,
                        SLOT (fn_use_table())
                        );
}

void CTEA::update_scripts()
{
  menu_fn_scripts->clear();

  create_menu_from_dir (this,
                        menu_fn_scripts,
                        dir_scripts,
                        SLOT (fn_run_script())
                        );
}

//KDE: $HOME/.local/share/user-places.xbel GNOME  $HOME/.config/gtk-3.0/bookmarks

void CTEA::update_places_bookmarks()
{
  lv_places->clear();

  QStringList sl_items;
  sl_items << tr ("templates");
  sl_items << tr ("snippets");
  sl_items << tr ("scripts");
  sl_items << tr ("tables");
  sl_items << tr ("configs");

  lv_places->addItems (sl_items);

  if (! file_exists (fname_places_bookmarks))
     return;

  sl_places_bmx = qstring_load (fname_places_bookmarks).split ("\n");

  sl_places_bmx.removeAll (QString (""));

  if (sl_places_bmx.size() != 0)
     lv_places->addItems (sl_places_bmx);

  QString fname_gtk_bookmarks = QDir::homePath() + "/" + ".gtk-bookmarks";
  if (! file_exists (fname_gtk_bookmarks))
     return;

  lv_places->addItem (tr ("GTK Bookmarks:"));

  QStringList sl_gtk_bookmarks = qstring_load (fname_gtk_bookmarks).split ("\n");

  QStringList sl_filtered;

  int sz = sl_gtk_bookmarks.size();

  for (int i = 0; i < sz; i++)
      {
       QString s = sl_gtk_bookmarks.at(i);
       //int pos = s.lastIndexOf ('/');
       //if (pos == -1)
         // continue;

       //QString t = s.right (s.size() - ++pos);

       s = s.remove ("file://");

       if (s.contains ("%"))
          s =  QUrl::fromPercentEncoding (s.toLatin1().constData());

       sl_filtered.append (s);
      }

  if (sl_filtered.size() > 0)
     lv_places->addItems (sl_filtered);
}


void CTEA::update_programs()
{
  if (! file_exists (fname_programs))
     return;

  programs = hash_load_keyval (fname_programs);
  if (programs.count() < 0)
     return;

  menu_programs->clear();

  QStringList sl = programs.keys();
  sl.sort();

  create_menu_from_list (this, menu_programs,
                         sl,
                         SLOT (run_program()));
}


void CTEA::update_logmemo_palette()
{
  int darker_val = settings->value ("darker_val", 100).toInt();

  QString text_color = hash_get_val (global_palette, "text", "black");
  QString back_color = hash_get_val (global_palette, "background", "white");
  QString sel_back_color = hash_get_val (global_palette, "sel-background", "black");
  QString sel_text_color = hash_get_val (global_palette, "sel-text", "white");

  QString t_text_color = QColor (text_color).darker(darker_val).name();
  QString t_back_color = QColor (back_color).darker(darker_val).name();
  QString t_sel_text_color = QColor (sel_text_color).darker(darker_val).name();
  QString t_sel_back_color = QColor (sel_back_color).darker(darker_val).name();

  QString sheet = QString ("QPlainTextEdit { color: %1; background-color: %2; selection-color: %3; selection-background-color: %4;}").arg (
                           t_text_color).arg (
                           t_back_color).arg (
                           t_sel_text_color).arg (
                           t_sel_back_color);

  log->setStyleSheet (sheet);

  sheet = QString ("QLineEdit { color: %1; background-color: %2; selection-color: %3; selection-background-color: %4;}").arg (
                   t_text_color).arg (
                   t_back_color).arg (
                   t_sel_text_color).arg (
                   t_sel_back_color);

  fif->setStyleSheet (sheet);
}


void CTEA::update_charsets()
{
  QString fname  = dir_config + "/last_used_charsets";

  if (! file_exists (fname))
     qstring_save (fname, "UTF-8");

  sl_last_used_charsets = qstring_load (fname).split ("\n");

  QList<QByteArray> acodecs = QTextCodec::availableCodecs();

  for (QList <QByteArray>::iterator codec = acodecs.begin(); codec != acodecs.end(); ++codec)
      sl_charsets.prepend ((*codec));

  sl_charsets.sort();
}


void CTEA::update_profiles()
{
  menu_view_profiles->clear();
  create_menu_from_dir (this,
                        menu_view_profiles,
                        dir_profiles,
                        SLOT (view_use_profile())
                       );
}


void CTEA::opt_update_keyb()
{
  if (! lv_menuitems)
     return;

  lv_menuitems->clear();
  shortcuts->captions_iterate();
  lv_menuitems->addItems (shortcuts->captions);
}


/*
=====================
Misc callbacks
=====================
*/


void CTEA::select_label()
{
  last_action = sender();
  QAction *a = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QTextCursor cr;

  QString text_to_find = settings->value ("label_start", "[?").toString()
                         + a->data().toString()
                         + settings->value ("label_end", "?]").toString();

  cr = d->document()->find (text_to_find);

  if (! cr.isNull())
     d->setTextCursor (cr);
}


void CTEA::run_program()
{
  last_action = sender();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QAction *a = qobject_cast<QAction *>(sender());
  QString command = programs.value (a->data().toString());
  if (command.isEmpty())
     return;

  if (main_tab_widget->currentIndex() == idx_tab_edit)
     {
      if (! file_exists (d->file_name))
         {
          QMessageBox::critical (this, "!", tr ("Save the file first!"), QMessageBox::Ok, QMessageBox::Ok);
          return;
         }

      QFileInfo fi (d->file_name);

      command = command.replace ("%s", d->file_name);
      command = command.replace ("%basename", fi.baseName());
      command = command.replace ("%filename", fi.fileName());
      command = command.replace ("%ext", fi.suffix());
      command = command.replace ("%dir", fi.canonicalPath());

      QString fname = d->get_filename_at_cursor();
      if (! fname.isEmpty())
          command = command.replace ("%i", fname);
     }
  else
  if (main_tab_widget->currentIndex() == idx_tab_fman)
     command = command.replace ("%s", fman->get_sel_fname());

  QProcess *process  = new QProcess (this);

  connect (process, SIGNAL(readyReadStandardOutput()), this, SLOT(process_readyReadStandardOutput()));
  process->setProcessChannelMode (QProcess::MergedChannels) ;


#if QT_VERSION >= 0x060000
  process->startCommand (command);
#else
  process->start (command);
#endif

//  process->start (command, QStringList());


//system (command.toUtf8().data());
}

void CTEA::guess_enc()
{
  QString enc;
  QString fn = fman->get_sel_fname();

  if (! file_exists (fn))
     return;

  if (settings->value ("use_enca_for_charset_detection", 0).toBool())
      {
       enc = guess_enc_for_file (fn);
       if (enc == "err")
          {
           log->log (tr ("Enca is not installed, falling back to the built-in detection"));
           CCharsetMagic cm;
           enc = cm.guess_for_file (fn);
          }
      }
  else
      {
       CCharsetMagic cm;
       enc = cm.guess_for_file (fn);
      }

  if (! enc.isEmpty())
     cb_fman_codecs->setCurrentIndex (cb_fman_codecs->findText (enc, Qt::MatchFixedString));
}


void CTEA::clipboard_dataChanged()
{
  if (! capture_to_storage_file)
     return;

  CDocument *ddest = documents->get_document_by_fname (fname_storage_file);
  if (ddest)
     {
      QString t = QApplication::clipboard()->text();

      QString tpl = "%s\n";

      QString ftemplate = dir_config + "/cliptpl.txt";
      if (file_exists (ftemplate))
         tpl = qstring_load (ftemplate);

      tpl = tpl.replace ("%time", QTime::currentTime().toString (settings->value("time_format", "hh:mm:ss").toString()));
      tpl = tpl.replace ("%date", QDate::currentDate().toString (settings->value("date_format", "dd/MM/yyyy").toString()));

      QString text_to_insert = tpl.replace ("%s", t);

      ddest->put (text_to_insert);
     }
}


void CTEA::main_tab_page_changed (int index)
{
  if (idx_prev == idx_tab_fman)
     if (img_viewer && img_viewer->window_mini.isVisible())
         img_viewer->window_mini.close();

  if (idx_prev == idx_tab_tune)
      leaving_options();

  idx_prev = index;

  if (index == idx_tab_fman)
     {
      fman->setFocus();
      fm_entry_mode = FM_ENTRY_MODE_NONE;
      idx_tab_fman_activate();
     }

  if (index == idx_tab_calendar)
     {
      calendar_update();
      idx_tab_calendar_activate();
     }

  if (index == idx_tab_edit)
     idx_tab_edit_activate();

  if (index == idx_tab_tune)
     idx_tab_tune_activate();

  if (index == idx_tab_learn)
     idx_tab_learn_activate();
}


void CTEA::calendar_clicked (const QDate &date)
{
  QString fname = dir_days + "/" + date.toString ("yyyy-MM-dd");

  if (file_exists (fname))
     {
      QString s = qstring_load (fname);
      log->log (s);
     }
}


void CTEA::calendar_activated (const QDate &date)
{
  QString fname = dir_days + "/" + date.toString ("yyyy-MM-dd");

  bool fresh = false;

  if (settings->value ("cal_run_1st", true).toBool())
     {
      if (! file_exists (fname))
         qstring_save (fname, tr ("Enter your daily notes here.\nTo use time-based reminders, specify the time signature in 24-hour format [hh:mm], i.e.:\n[06:00]good morning!\n[20:10]go to theatre"));

      settings->setValue ("cal_run_1st", false);
      fresh = true;
     }
  else
  if (! file_exists (fname))
     {
      qstring_save (fname, tr ("Enter your daily notes here."));
      fresh = true;
     }

  CDocument *d = documents->open_file (fname, "UTF-8");
  if (! d)
     return;

 // if (fresh)
   //  d->selectAll();

  main_tab_widget->setCurrentIndex (idx_tab_edit);
}


void CTEA::calendar_currentPageChanged (int year, int month)
{
  calendar_update();
}


void CTEA::process_readyReadStandardOutput()
{
  QProcess *p = qobject_cast<QProcess *>(sender());
  QByteArray a = p->readAllStandardOutput()/*.data()*/;
  QTextCodec *c = QTextCodec::codecForLocale();
  QString t = c->toUnicode (a);

  log->terminal_output = true;
  log->log (t);
  log->terminal_output = false;
}


void CTEA::virt_keyb_clicked()
{
  CDocument *d = documents->get_current();
  if (! d)
     return;

  QPushButton* bsender = qobject_cast<QPushButton*>(sender());
  QString t = bsender->text();

  d->insertPlainText (t);
}


/*
===========================
Application misc. methods
===========================
*/


QHash <QString, QString> CTEA::load_eclipse_theme_xml (const QString &fname)
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


void CTEA::load_palette (const QString &fileName)
{
  if (! file_exists (fileName))
      return;

  global_palette.clear();

  if (file_get_ext (fileName) == "xml")
     global_palette = load_eclipse_theme_xml (fileName);
  else
      global_palette = hash_load_keyval (fileName);
}


void CTEA::fman_convert_images (bool by_side, int value)
{
  srand (QTime::currentTime().msec());

  QString dir_out ("images-out-");

  dir_out.append (QString::number (rand() % 777));
  dir_out.prepend ("/");
  dir_out.prepend (fman->dir.absolutePath());

  if (! fman->dir.mkpath (dir_out))
     return;

  Qt::TransformationMode transformMode = Qt::FastTransformation;
  if (settings->value ("img_filter", 0).toBool())
     transformMode = Qt::SmoothTransformation;

  pb_status->show();
  pb_status->setFormat (tr ("%p% completed"));
  pb_status->setTextVisible (true);

  QStringList li = fman->get_sel_fnames();

  int quality = settings->value ("img_quality", "-1").toInt();

  pb_status->setRange (0, li.size() - 1 );
  int i = 0;

  for (QList <QString>::iterator fname = li.begin(); fname != li.end(); ++fname)
      {
       if (! is_image ((*fname)))
           continue;

       QImage source ((*fname));

       if (source.isNull())
          continue;

       qApp->processEvents();

       if (settings->value ("cb_exif_rotate", 1).toBool())
          {
           int exif_orientation = get_exif_orientation ((*fname));

           QTransform transform;
           qreal angle = 0;

           if (exif_orientation == 3)
              angle = 180;
           else
           if (exif_orientation == 6)
              angle = 90;
           else
           if (exif_orientation == 8)
              angle = 270;
           if (angle != 0)
              {
               transform.rotate (angle);
               source = source.transformed (transform);
              }
          }


          QImage dest = image_scale_by (source, by_side, value, transformMode);
          QString fmt (settings->value ("output_image_fmt", "jpg").toString());
          QFileInfo fi ((*fname));

          QString dest_fname (dir_out);
          dest_fname.append ("/");
          dest_fname.append (fi.fileName());
          dest_fname = change_file_ext (dest_fname, fmt);

          if (! dest.save (dest_fname, fmt.toLatin1().constData(), quality))
             qDebug() << "Cannot save " << dest_fname;

          pb_status->setValue (i++);
         }

  pb_status->hide();

  if (settings->value ("img_post_proc", 0).toBool())
     {
      CZipper zipper;
      zipper.zip_directory (fman->dir.absolutePath(), dir_out);
     }

  fman->refresh();
}


QTextDocument::FindFlags CTEA::get_search_options()
{
  QTextDocument::FindFlags flags; //= 0;

  if (menu_find_whole_words->isChecked())
     flags = flags | QTextDocument::FindWholeWords;

  if (menu_find_case->isChecked())
     flags = flags | QTextDocument::FindCaseSensitively;

  return flags;
}


QString CTEA::fif_get_text()
{
  QString t = fif->text();

  int i = sl_fif_history.indexOf (t);

  if (i != -1)
     {
      sl_fif_history.removeAt (i);
      sl_fif_history.prepend (t);
     }
  else
      sl_fif_history.prepend (t);

  if (sl_fif_history.count() > 77)
     sl_fif_history.removeLast();

  return t;
}


QWidget* CTEA::create_keyboard (const QString &fname)
{
  if (! file_exists (fname))
     return 0;

  QStringList l = qstring_load (fname).split ("\n");
  if (l.size() == 0)
     return 0;

  QWidget *w = new QWidget;

  QFileInfo fi (fname);

  w->setWindowTitle (fi.fileName());

  Qt::WindowFlags flags;
  flags = Qt::Window | Qt::Tool | Qt::WindowStaysOnTopHint;

  w->setWindowFlags (flags);

  QVBoxLayout *lv = new QVBoxLayout;
  w->setLayout (lv);

  for (int y = 0; y< l.size(); y++)
      {
       QStringList row = l[y].split ("|");
       QHBoxLayout *lh = new QHBoxLayout;
       lv->addLayout (lh);

       for (int x = 0; x < row.size(); x++)
           {
            QPushButton *b = new QPushButton;
            connect (b, SIGNAL(clicked()), this, SLOT(virt_keyb_clicked()));
            b->setText (row[x]);
            lh->addWidget (b);
           }
      }

  return w;
}



QAction* CTEA::add_to_menu (QMenu *menu,
                            const QString &caption,
                            const char *method,
                            const QString &shortkt,
                            const QString &iconpath
                           )
{
  QAction *act = new QAction (caption, this);

  if (! shortkt.isEmpty())
     act->setShortcut (shortkt);

  if (! iconpath.isEmpty())
     act->setIcon (QIcon (iconpath));

  connect (act, SIGNAL(triggered()), this, method);
  menu->addAction (act);
  return act;
}


QIcon CTEA::get_theme_icon (const QString &name)
{
  QString fname = theme_dir + "icons/" + name;

  if (file_exists (fname))
     return QIcon (fname);

  return QIcon (":/icons/" + name);
}


QString CTEA::get_theme_icon_fname (const QString &name)
{
  QString fname = theme_dir + "icons/" + name;

  if (file_exists (fname))
     return fname;

  return ":/icons/" + name;
}


void CTEA::leaving_options()
{
  settings->setValue ("date_format", ed_date_format->text());
  settings->setValue ("time_format", ed_time_format->text());
  settings->setValue ("img_viewer_override_command", ed_img_viewer_override->text());
  settings->setValue ("wasd", cb_wasd->isChecked());

  settings->setValue ("ui_mode", cmb_ui_mode->currentIndex());

#if defined(JOYSTICK_SUPPORTED)
  settings->setValue ("use_joystick", cb_use_joystick->isChecked());
#endif

  settings->setValue ("full_path_at_window_title", cb_full_path_at_window_title->isChecked());
  settings->setValue ("word_wrap", cb_wordwrap->isChecked());

#if QT_VERSION >= 0x050000
//  settings->setValue ("qregexpsyntaxhl", cb_use_qregexpsyntaxhl->isChecked());
#endif

  settings->setValue ("additional_hl", cb_hl_current_line->isChecked());
  settings->setValue ("session_restore", cb_session_restore->isChecked());
  settings->setValue ("show_linenums", cb_show_linenums->isChecked());
  settings->setValue ("hl_enabled", cb_hl_enabled->isChecked());
  settings->setValue ("hl_brackets", cb_hl_brackets->isChecked());
  settings->setValue ("auto_indent", cb_auto_indent->isChecked());
  settings->setValue ("spaces_instead_of_tabs", cb_spaces_instead_of_tabs->isChecked());
  settings->setValue ("show_tabs_and_spaces", cb_show_tabs_and_spaces->isChecked());
  settings->setValue ("cursor_xy_visible", cb_cursor_xy_visible->isChecked());
  settings->setValue ("tab_sp_width", spb_tab_sp_width->value());
  settings->setValue ("center_on_scroll", cb_center_on_cursor->isChecked());
  settings->setValue ("show_margin", cb_show_margin->isChecked());
  settings->setValue ("margin_pos", spb_margin_pos->value());
  settings->setValue ("b_preview", cb_auto_img_preview->isChecked());
  settings->setValue ("cursor_blink_time", spb_cursor_blink_time->value());
  settings->setValue ("autosave_period", spb_autosave_period->value());
  settings->setValue ("autosave", cb_autosave->isChecked());

  documents->timer_autosave.stop();
  documents->timer_autosave.setInterval (spb_autosave_period->value() * 1000);
  documents->timer_autosave.start();


  MyProxyStyle::cursor_blink_time = spb_cursor_blink_time->value();

  qApp->setCursorFlashTime (spb_cursor_blink_time->value());

  settings->setValue ("cursor_width", spb_cursor_width->value());
  settings->setValue ("override_img_viewer", cb_override_img_viewer->isChecked());
  settings->setValue ("use_enca_for_charset_detection", cb_use_enca_for_charset_detection->isChecked());
  settings->setValue ("use_trad_dialogs", cb_use_trad_dialogs->isChecked());
  settings->setValue ("start_week_on_sunday", cb_start_on_sunday->isChecked());
  settings->setValue ("northern_hemisphere", cb_northern_hemisphere->isChecked());

  calendar->northern_hemisphere = bool (cb_northern_hemisphere->isChecked());

  /*
  int i = moon_phase_algos.key (cmb_moon_phase_algos->currentText());
  settings->setValue ("moon_phase_algo", i);
  calendar->moon_phase_algo = i;
*/


  int i = cmb_moon_phase_algos->currentText().toInt();
  settings->setValue ("moon_phase_algo", i);
  calendar->moon_phase_algo = i;

  settings->setValue ("lng", cmb_lng->currentText());

  settings->setValue ("zip_charset_in", cmb_zip_charset_in->currentText());
  settings->setValue ("zip_charset_out", cmb_zip_charset_out->currentText());
  settings->setValue ("cmdline_default_charset", cmb_cmdline_default_charset->currentText());
  settings->setValue ("label_end", ed_label_end->text());
  settings->setValue ("label_start", ed_label_start->text());
  settings->setValue ("output_image_fmt", cmb_output_image_fmt->currentText());
  settings->setValue ("img_filter", cb_output_image_flt->isChecked());
  settings->setValue ("fuzzy_q", spb_fuzzy_q->value());

  settings->setValue ("show_ebooks_fine", cb_show_ebooks_fine->isChecked());

  settings->setValue ("img_quality", spb_img_quality->value());
  settings->setValue ("img_post_proc", cb_zip_after_scale->isChecked());
  settings->setValue ("cb_exif_rotate", cb_exif_rotate->isChecked());
  settings->setValue ("zor_use_exif_orientation", cb_zor_use_exif->isChecked());
  settings->setValue ("ed_side_size", ed_side_size->text());
  settings->setValue ("ed_link_options", ed_link_options->text());
  settings->setValue ("ed_cols_per_row", ed_cols_per_row->text());

  b_preview = settings->value ("b_preview", false).toBool();

  calendar->do_update();
  documents->apply_settings();
}


void CTEA::add_to_last_used_charsets (const QString &s)
{
  int i = sl_last_used_charsets.indexOf (s);
  if (i == -1)
     sl_last_used_charsets.prepend (s);
  else
      sl_last_used_charsets.move (i, 0);

  if (sl_last_used_charsets.size() > 3)
     sl_last_used_charsets.removeLast();
}


void CTEA::count_substring (bool use_regexp)
{
  CDocument *d = documents->get_current();
  if (! d)
     return;

  QString text;

  if (d->textCursor().hasSelection())
     text = d->get();
  else
      text = d->toPlainText();

  int count = 0;
  Qt::CaseSensitivity cs = Qt::CaseInsensitive;

  if (menu_find_case->isChecked())
     cs = Qt::CaseSensitive;

#if (QT_VERSION_MAJOR < 5)

  if (use_regexp)
     count = text.count (QRegExp (fif_get_text()));
  else
      count = text.count (fif_get_text(), cs);

#else

  if (use_regexp)
     count = text.count (QRegularExpression (fif_get_text()));
  else
      count = text.count (fif_get_text(), cs);

#endif

  log->log (tr ("%1 number of occurrences of %2 is found").arg (count).arg (fif->text()));
}


void CTEA::run_unitaz (int mode)
{
  CDocument *d = documents->get_current();
  if (! d)
     return;

  //pb_status->show();

  progress_show();

  pb_status->setFormat (tr ("%p% completed"));
  pb_status->setTextVisible (true);

  QElapsedTimer time_start;
  time_start.start();

  int c = 0;

  QStringList total = d->get_words();
  QHash <QString, int> h;

  pb_status->setRange (0, total.size() - 1);

  for (int j = 0; j < total.size(); j++)
      {
       if (c % 100 == 0)
          qApp->processEvents();

       if (boring)
           break;

       QHash<QString, int>::iterator i = h.find (total.at(j).toLower());
       if (i != h.end())
           i.value() += 1;
       else
           h.insert(total.at(j).toLower(), 1);

       pb_status->setValue (c++);
      }

  vector <pair <QString, int> > uwords;

  QList <QString> keys = h.keys();
  int sz = keys.size();

  uwords.reserve (sz);

  for (int i = 0; i < sz; i++)
      uwords.push_back (make_pair(keys.at(i),h.value (keys.at(i))));

  if (mode == 0)
    std::sort (uwords.begin(), uwords.end(), pr_bigger_than);
  if (mode == 1)
     std::sort (uwords.begin(), uwords.end(), pr_bigger_than_str);
  if (mode == 2)
     std::sort (uwords.begin(), uwords.end(), pr_bigger_than_str_len);

  QStringList outp;

  for (size_t i = 0; i < uwords.size(); i++)
      outp.append (uwords.at(i).first + " = " + QString::number (uwords.at(i).second));

  double diff = static_cast <double> (total.size()) / static_cast <double> (uwords.size());
  double diff_per_cent = get_percent (static_cast <double> (total.size()), static_cast <double> (uwords.size()));

  outp.prepend (tr ("total to unique per cent diff: %1").arg (diff_per_cent, 0, 'f', 6));
  outp.prepend (tr ("total / unique: %1").arg (diff, 0, 'f', 6));
  outp.prepend (tr ("words unique: %1").arg (uwords.size()));
  outp.prepend (tr ("words total: %1").arg (total.size()));
  outp.prepend (tr ("text analysis of: %1").arg (d->file_name));
  outp.prepend (tr ("UNITAZ: UNIverlsal Text AnalyZer"));

  log->log (tr("elapsed milliseconds: %1").arg (time_start.elapsed()));

  CDocument *nd = documents->create_new();
  nd->put (outp.join ("\n"));

  //pb_status->hide();
  progress_hide();

}


void CTEA::markup_text (const QString &mode)
{
  CDocument *d = documents->get_current();
  if (! d)
     return;

  QString t = hash_markup[mode][d->markup_mode];

  if (! t.isEmpty())
      d->put (t.replace ("%s", d->get()));
}


void CTEA::fman_items_select_by_regexp (bool mode)
{
  QString ft = fif_get_text();
  if (ft.isEmpty())
      return;

#if QT_VERSION >= 0x050F00
  l_fman_find = fman->mymodel->findItems (ft, Qt::MatchRegularExpression);
#else
  l_fman_find = fman->mymodel->findItems (ft, Qt::MatchRegExp);
#endif

  if (l_fman_find.size() < 1)
     return;

  QItemSelectionModel *m = fman->selectionModel();
  for (int i = 0; i < l_fman_find.size(); i++)
      if (mode)
         m->select (fman->mymodel->indexFromItem (l_fman_find[i]), QItemSelectionModel::Select | QItemSelectionModel::Rows);
      else
          m->select (fman->mymodel->indexFromItem (l_fman_find[i]), QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
}


void CTEA::fn_filter_delete_by_sep (bool mode)
{
  CDocument *d = documents->get_current();
  if (! d)
     return;

  QStringList sl = d->get().split (QChar::ParagraphSeparator);

  QString t = fif_get_text();

  for (int i = 0; i < sl.size(); i++)
      {
       int n = sl[i].indexOf (t);
       if (n != -1)
          {
           QString s = sl[i];
           if (mode)
               s = s.right (s.size() - n);
           else
               s = s.left (n);

           sl[i] = s;
          }
      }

  QString x = sl.join ("\n");

  d->put (x);
}


void CTEA::fman_find()
{
  QString ft = fif_get_text();
  if (ft.isEmpty())
      return;

  l_fman_find = fman->mymodel->findItems (ft, Qt::MatchStartsWith);

  if (l_fman_find.size() < 1)
     return;

  fman_find_idx = 0;
  fman->setCurrentIndex (fman->mymodel->indexFromItem (l_fman_find[fman_find_idx]));
}


void CTEA::fman_find_next()
{
  QString ft = fif_get_text();
  if (ft.isEmpty())
      return;

  if (l_fman_find.size() < 1)
     return;

  if (fman_find_idx < (l_fman_find.size() - 1))
     fman_find_idx++;

  fman->setCurrentIndex (fman->mymodel->indexFromItem (l_fman_find[fman_find_idx]));
}


void CTEA::fman_find_prev()
{
  QString ft = fif_get_text();
  if (ft.isEmpty())
      return;

  if (l_fman_find.size() < 1)
     return;

  if (fman_find_idx != 0)
     fman_find_idx--;

  fman->setCurrentIndex (fman->mymodel->indexFromItem (l_fman_find[fman_find_idx]));
}


void CTEA::opt_shortcuts_find()
{
  int from = 0;

  QString fiftxt = fif_get_text();

  if (opt_shortcuts_string_to_find == fiftxt)
      from = lv_menuitems->currentRow();

  opt_shortcuts_string_to_find = fiftxt;

  if (from == -1)
      from = 0;

#if QT_VERSION < 0x050000
  int index = shortcuts->captions.indexOf (QRegExp (opt_shortcuts_string_to_find + ".*",
                                                    Qt::CaseInsensitive), from);
#else
  int index = shortcuts->captions.indexOf (QRegularExpression (opt_shortcuts_string_to_find + ".*",
                                                               QRegularExpression::CaseInsensitiveOption), from);
#endif

  if (index != -1)
     lv_menuitems->setCurrentRow (index);
}


void CTEA::opt_shortcuts_find_next()
{
  int from = lv_menuitems->currentRow();
  if (from == -1)
     from = 0;

#if QT_VERSION < 0x050000
  int index = shortcuts->captions.indexOf (QRegExp (opt_shortcuts_string_to_find + ".*",
                                                    Qt::CaseInsensitive), from + 1);
#else
  int index = shortcuts->captions.indexOf (QRegularExpression (opt_shortcuts_string_to_find + ".*",
                                                               QRegularExpression::CaseInsensitiveOption), from + 1);

#endif

  if (index != -1)
    lv_menuitems->setCurrentRow (index);
}


void CTEA::opt_shortcuts_find_prev()
{
  int from = lv_menuitems->currentRow();
  if (from == -1)
     from = 0;

#if QT_VERSION < 0x050000
  int index = shortcuts->captions.lastIndexOf (QRegExp (opt_shortcuts_string_to_find + ".*",
                                                        Qt::CaseInsensitive), from - 1);
#else

  int index = shortcuts->captions.lastIndexOf (QRegularExpression (opt_shortcuts_string_to_find + ".*",
                                                                   QRegularExpression::CaseInsensitiveOption), from - 1);
#endif

  if (index != -1)
     lv_menuitems->setCurrentRow (index);
}


void CTEA::idx_tab_edit_activate()
{
  menu_file->menuAction()->setVisible (true);
  menu_edit->menuAction()->setVisible (true);
  menu_programs->menuAction()->setVisible (true);
  menu_cal->menuAction()->setVisible (false);
  menu_markup->menuAction()->setVisible (true);
  menu_functions->menuAction()->setVisible (true);
  menu_search->menuAction()->setVisible (true);
  menu_nav->menuAction()->setVisible (true);
  menu_fm->menuAction()->setVisible (false);
  menu_view->menuAction()->setVisible (true);
  helpMenu->menuAction()->setVisible (true);
}


void CTEA::idx_tab_calendar_activate()
{
  menu_file->menuAction()->setVisible (true);
  menu_edit->menuAction()->setVisible (false);
  menu_cal->menuAction()->setVisible (true);
  menu_programs->menuAction()->setVisible (true);
  menu_markup->menuAction()->setVisible (false);
  menu_functions->menuAction()->setVisible (false);
  menu_search->menuAction()->setVisible (false);
  menu_nav->menuAction()->setVisible (false);
  menu_fm->menuAction()->setVisible (false);
  menu_view->menuAction()->setVisible (true);
  helpMenu->menuAction()->setVisible (true);
}


void CTEA::idx_tab_tune_activate()
{
  opt_update_keyb();

  menu_file->menuAction()->setVisible (true);
  menu_edit->menuAction()->setVisible (false);
  menu_programs->menuAction()->setVisible (true);
  menu_markup->menuAction()->setVisible (false);
  menu_functions->menuAction()->setVisible (true);
  menu_search->menuAction()->setVisible (false);
  menu_nav->menuAction()->setVisible (false);
  menu_fm->menuAction()->setVisible (false);
  menu_view->menuAction()->setVisible (true);
  helpMenu->menuAction()->setVisible (true);
  menu_cal->menuAction()->setVisible (false);
}


void CTEA::idx_tab_fman_activate()
{
  menu_file->menuAction()->setVisible (true);
  menu_edit->menuAction()->setVisible (false);
  menu_programs->menuAction()->setVisible (true);
  menu_markup->menuAction()->setVisible (false);
  menu_functions->menuAction()->setVisible (true);
  menu_search->menuAction()->setVisible (true);
  menu_nav->menuAction()->setVisible (false);
  menu_fm->menuAction()->setVisible (true);
  menu_view->menuAction()->setVisible (true);
  helpMenu->menuAction()->setVisible (true);
  menu_cal->menuAction()->setVisible (false);
}


void CTEA::idx_tab_learn_activate()
{
  menu_file->menuAction()->setVisible (true);
  menu_edit->menuAction()->setVisible (false);
  menu_programs->menuAction()->setVisible (true);
  menu_markup->menuAction()->setVisible (false);
  menu_functions->menuAction()->setVisible (false);
  menu_search->menuAction()->setVisible (true);
  menu_nav->menuAction()->setVisible (false);
  menu_fm->menuAction()->setVisible (false);
  menu_view->menuAction()->setVisible (true);
  helpMenu->menuAction()->setVisible (true);
  menu_cal->menuAction()->setVisible (false);
}


void CTEA::man_find_find()
{
  QString fiftxt = fif_get_text();
  man->find (fiftxt, get_search_options());
  man_search_value = fiftxt;
}


void CTEA::man_find_next()
{
  man->find (man_search_value, get_search_options());
}


void CTEA::man_find_prev()
{
  man->find (man_search_value, get_search_options() | QTextDocument::FindBackward);
}


void CTEA::progress_show()
{
  pb_status->show();
  tb_stop->show();
  boring = false;
}


void CTEA::progress_hide()
{
  pb_status->hide();
  tb_stop->hide();
  boring = false;
}


/*
====================
Tune page callbacks
====================
*/

#if defined(JOYSTICK_SUPPORTED)

void CTEA::cb_use_joystick_stateChanged (int state)
{
  bool b;
  if (state == Qt::Unchecked)
      b = false;
  else
      b = true;

  settings->setValue ("use_joystick", b);

  if (b)
     documents->timer_joystick.start (100);
  else
      documents->timer_joystick.stop();
}

#endif

void CTEA::cb_altmenu_stateChanged (int state)
{
  if (state == Qt::Unchecked)
      MyProxyStyle::b_altmenu = false;
  else
      MyProxyStyle::b_altmenu = true;

  settings->setValue ("b_altmenu", MyProxyStyle::b_altmenu);
}


void CTEA::cmb_ui_tabs_currentIndexChanged (int i)
{
  main_tab_widget->setTabPosition (int_to_tabpos (i));
  settings->setValue ("ui_tabs_align", i);
}


void CTEA::cmb_docs_tabs_currentIndexChanged (int i)
{
  tab_editor->setTabPosition (int_to_tabpos (i));

  settings->setValue ("docs_tabs_align", i);
}


void CTEA::cmb_icon_sizes_currentIndexChanged (int index)
{

  QComboBox *cmb = qobject_cast<QComboBox*>(sender());

  QString text = cmb->currentText();

  settings->setValue ("icon_size", text);

  icon_size = settings->value ("icon_size", "32").toInt();

  setIconSize (QSize (text.toInt(), text.toInt()));
  tb_fman_dir->setIconSize (QSize (text.toInt(), text.toInt()));
  filesToolBar->setIconSize (QSize (text.toInt(), text.toInt()));
}


void CTEA::cmb_tea_icons_currentIndexChanged (int)
{
  QComboBox *cmb = qobject_cast<QComboBox*>(sender());
  QString text = cmb->currentText();

  settings->setValue ("icon_fname", text);

  QString icon_fname = ":/icons/tea-icon-v3-0" + text + ".png";

  qApp->setWindowIcon (QIcon (icon_fname));
}


void CTEA::pb_assign_hotkey_clicked()
{
  if (! lv_menuitems->currentItem())
     return;

  if (ent_shtcut->text().isEmpty())
     return;

  shortcuts->set_new_shortcut (lv_menuitems->currentItem()->text(), ent_shtcut->text());
  shortcuts->save_to_file (shortcuts->fname);
}


void CTEA::pb_remove_hotkey_clicked()
{
  if (! lv_menuitems->currentItem())
     return;

  shortcuts->set_new_shortcut (lv_menuitems->currentItem()->text(), "");
  ent_shtcut->setText ("");
  shortcuts->save_to_file (shortcuts->fname);
}


void CTEA::slot_lv_menuitems_currentItemChanged (QListWidgetItem *current, QListWidgetItem *previous)
{
  if (! current)
     return;

  QAction *a = shortcuts->find_by_caption (current->text());
  if (a)
     ent_shtcut->setText (a->shortcut().toString());
}


void CTEA::slot_font_logmemo_select()
{
  bool ok;
  QFont font = QFontDialog::getFont(&ok, QFont(settings->value ("logmemo_font", "Monospace").toString(), settings->value ("logmemo_font_size", "12").toInt()), this);
  if (! ok)
     return;

  settings->setValue ("logmemo_font", font.family());
  settings->setValue ("logmemo_font_size", font.pointSize());
  update_stylesheet (fname_stylesheet);
}


void CTEA::slot_font_interface_select()
{
  bool ok;
  QFontInfo fi = QFontInfo (qApp->font());
  QFont font = QFontDialog::getFont (&ok, QFont (settings->value ("app_font_name", "Sans").toString(), settings->value ("app_font_size", fi.pointSize()).toInt()), this);
  if (! ok)
     return;

  settings->setValue ("app_font_name", font.family());
  settings->setValue ("app_font_size", font.pointSize());
  update_stylesheet (fname_stylesheet);
}


void CTEA::slot_font_editor_select()
{
  bool ok;
  QFont font = QFontDialog::getFont(&ok, QFont(settings->value ("editor_font_name", "Serif").toString(), settings->value ("editor_font_size", "16").toInt()), this);
  if (! ok)
     return;

  settings->setValue ("editor_font_name", font.family());
  settings->setValue ("editor_font_size", font.pointSize());
  update_stylesheet (fname_stylesheet);
}


void CTEA::slot_style_currentIndexChanged (int)
{
 QComboBox *cmb = qobject_cast<QComboBox*>(sender());
 QString text = cmb->currentText();

   if (text == "GTK+") //because it is buggy with some Qt versions. sorry!
     return;

  QStyle *style = QStyleFactory::create (text);
  if (style == 0)
     return;

  settings->setValue ("ui_style", text);
}


#if defined (HUNSPELL_ENABLE) || defined (ASPELL_ENABLE)
void CTEA::cmb_spellchecker_currentIndexChanged (int)
{
  QComboBox *cmb = qobject_cast<QComboBox*>(sender());
  QString text = cmb->currentText();

  cur_spellchecker = text;

  settings->setValue ("cur_spellchecker", cur_spellchecker);

  delete spellchecker;

  if (! spellcheckers.contains (cur_spellchecker) && spellcheckers.size() > 0)
     cur_spellchecker = spellcheckers[0];

#ifdef ASPELL_ENABLE
  if (cur_spellchecker == "Aspell")
     spellchecker = new CAspellchecker (settings->value ("spell_lang", QLocale::system().name().left(2)).toString());
#endif

#ifdef HUNSPELL_ENABLE
   if (cur_spellchecker == "Hunspell")
      spellchecker = new CHunspellChecker (settings->value ("spell_lang", QLocale::system().name().left(2)).toString(), hunspell_default_dict_path());
#endif

  settings->setValue ("spell_lang", "");

  create_spellcheck_menu();
}
#endif


#ifdef HUNSPELL_ENABLE
void CTEA::pb_choose_hunspell_path_clicked()
{
  QString path = QFileDialog::getExistingDirectory (this, tr ("Open Directory"), hunspell_default_dict_path(),
                                                    QFileDialog::ShowDirsOnly |
                                                    QFileDialog::DontResolveSymlinks);
  if (path.isEmpty())
      return;

  settings->setValue ("hunspell_dic_path", path);
  ed_spellcheck_path->setText (path);

  if (spellchecker)
      delete spellchecker;

  setup_spellcheckers();
  create_spellcheck_menu();
}
#endif


#ifdef ASPELL_ENABLE

void CTEA::pb_choose_aspell_path_clicked()
{
  QString path = QFileDialog::getExistingDirectory (this, tr ("Open Directory"), "/",
                                                    QFileDialog::ShowDirsOnly |
                                                    QFileDialog::DontResolveSymlinks);
  if (path.isEmpty())
     return;

  settings->setValue ("win32_aspell_path", path);
  ed_aspellcheck_path->setText (path);

  if (spellchecker)
     delete spellchecker;

  setup_spellcheckers();
  create_spellcheck_menu();
}
#endif
