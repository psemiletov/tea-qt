/***************************************************************************
 *   2007-2017 by Peter Semiletov                                          *
 *   peter.semiletov@gmail.com                                             *

started at 08 November 2007
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




#include "rvln.h"

#include "utils.h"
#include "gui_utils.h"
#include "libretta_calc.h"

#include "textproc.h"
#include "logmemo.h"
#include "tzipper.h"
#include "wavinfo.h"
#include "exif.h"

#include <math.h>


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
#include <QDesktopServices>
#include <QColorDialog>
#include <QTextCodec>
#include <QMimeData>

#include <QScrollArea>

#include <QXmlStreamReader>
//#include <QUrl>

//#include <QtSvg>


//#include <QWidgetAction>

#include <QDebug>


#include <QPainter>
#include <QInputDialog>
#include <QSettings>
#include <QLibraryInfo>
#include <QCryptographicHash>

#include <QProxyStyle>

//QML stuff

#if QT_VERSION >= 0x050000

#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickItem>

#include <QQuickView>

#endif

//end

#ifdef PRINTER_ENABLE

#include <QPrinter>
#include <QPrintDialog>
#include <QAbstractPrintDialog>

#endif



#ifdef SPELLCHECKER_ENABLE

#include "spellchecker.h"

#endif


#include "fontbox.h"


#if QT_VERSION >= 0x050000

class CPluginListItem: public QObject
{
public:

  QString id;
  CQQuickWindow *window;
 
  CPluginListItem (const QString &plid, CQQuickWindow *wnd);
};

#endif


bool b_altmenu;

int cursor_blink_time;

#if QT_VERSION >= 0x050000

class QStyleHints
{
public:
	int cursorFlashTime() const; 
};


int QStyleHints::cursorFlashTime() const
{
  return cursor_blink_time;	
}


#endif

class MyProxyStyle: public QProxyStyle
{
public:
     int styleHint(StyleHint hint, const QStyleOption *option = 0,
                   const QWidget *widget = 0, QStyleHintReturn *returnData = 0) const {
         
             if (hint == QStyle::SH_ItemView_ActivateItemOnSingleClick)
                 return 0;
         
             if (! b_altmenu && hint == QStyle::SH_MenuBar_AltKeyNavigation)
                 return 0;
                  
         return QProxyStyle::styleHint(hint, option, widget, returnData);
     }
     
   MyProxyStyle (QStyle * style = 0);
     
};


class CFSizeFName: public QObject
{
public:

  qint64 size;
  QString fname;

  CFSizeFName (qint64 sz, const QString &fn):
               size (sz),
               fname (fn)
               {
               }
};


extern QSettings *settings;
extern QMenu *current_files_menu;
extern QHash <QString, QString> global_palette;

extern bool b_recent_off;
extern bool b_destroying_all;
extern int recent_list_max_items;


rvln *mainWindow;  


QVariantMap hs_path;


#if QT_VERSION >= 0x050000

QList <CPluginListItem *> plugins_list;

#endif

//QFontDatabase *font_database;
document_holder *documents;


enum {
      FM_ENTRY_MODE_NONE = 0,
      FM_ENTRY_MODE_OPEN,
      FM_ENTRY_MODE_SAVE
     };




//for the further compat.
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


void rvln::create_paths()
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

  hs_path["dir_config"] = dir_config;
  
  dr.setPath (dir_config);
  if (! dr.exists())
     dr.mkpath (dir_config);

 // fname_userfonts.append (dir_config).append ("/userfonts.txt");
  //hs_path["fname_userfonts"] = fname_userfonts;
  
  fname_crapbook.append (dir_config).append ("/crapbook.txt");
  hs_path["fname_crapbook"] = fname_crapbook;
  
  fname_hls_cache.append (dir_config).append ("/hls_cache");
  hs_path["fname_hls_cache"] = fname_hls_cache;
  
  fname_fif.append (dir_config).append ("/fif");
  hs_path["fname_fif"] = fname_fif;
  
  fname_bookmarks.append (dir_config).append ("/tea_bmx");
  hs_path["fname_bookmarks"] = fname_bookmarks;
  
  fname_programs.append (dir_config).append ("/programs");
  hs_path["fname_programs"] = fname_programs;
  
  fname_places_bookmarks.append (dir_config).append ("/places_bookmarks");
  hs_path["fname_places_bookmarks"] = fname_places_bookmarks;
  
  fname_tempfile.append (QDir::tempPath()).append ("/tea.tmp");
  hs_path["fname_tempfile"] = fname_tempfile;
  
  fname_tempparamfile.append (QDir::tempPath()).append ("/teaparam.tmp");
  hs_path["fname_tempparamfile"] = fname_tempparamfile;
  
  dir_tables.append (dir_config).append ("/tables");

  dr.setPath (dir_tables);
  if (! dr.exists())
     dr.mkpath (dir_tables);

  dir_user_dict.append (dir_config).append ("/dictionaries");

  dr.setPath (dir_user_dict);
  if (! dr.exists())
     dr.mkpath (dir_user_dict);

  dir_plugins.append (dir_config).append ("/plugins");

  dr.setPath (dir_plugins);
  if (! dr.exists())
     dr.mkpath (dir_plugins);
  
  dir_profiles.append (dir_config).append ("/profiles");

  dr.setPath (dir_profiles);
  if (! dr.exists())
     dr.mkpath (dir_profiles);

  dir_templates.append (dir_config).append ("/templates");

  dr.setPath (dir_templates);
  if (! dr.exists())
     dr.mkpath (dir_templates);

  dir_snippets.append (dir_config).append ("/snippets");

  dr.setPath (dir_snippets);
  if (! dr.exists())
     dr.mkpath (dir_snippets);

  dir_scripts.append (dir_config).append ("/scripts");

  dr.setPath (dir_scripts);
  if (! dr.exists())
     dr.mkpath (dir_scripts);

  dir_days.append (dir_config).append ("/days");

  dr.setPath (dir_days);
  if (! dr.exists())
     dr.mkpath (dir_days);

  dir_sessions.append (dir_config).append ("/sessions");

  dr.setPath (dir_sessions);
  if (! dr.exists())
     dr.mkpath (dir_sessions);

  dir_themes.append (dir_config).append ("/themes");

  dr.setPath (dir_themes);
  if (! dr.exists())
     dr.mkpath (dir_themes);


  dir_hls.append (dir_config).append ("/hls");

  dr.setPath (dir_hls);
  if (! dr.exists())
     dr.mkpath (dir_hls);

  dir_palettes.append (dir_config).append ("/palettes");

  dr.setPath (dir_palettes);
  if (! dr.exists())
     dr.mkpath (dir_palettes);
}


void rvln::update_bookmarks()
{
  if (! file_exists (fname_bookmarks))
     return;

  bookmarks = qstring_load (fname_bookmarks);
  if (bookmarks.isEmpty())
     return;

  menu_file_bookmarks->clear();
  create_menu_from_list (this, menu_file_bookmarks,
                         bookmarks.split ("\n"),
                         SLOT (file_open_bookmark()));
}


void rvln::readSettings()
{
  cursor_blink_time = settings->value ("cursor_blink_time", 0).toInt();

  qApp->setCursorFlashTime (cursor_blink_time);
  
  recent_list_max_items = settings->value ("recent_list.max_items", 21).toInt();

  
  b_altmenu = settings->value ("b_altmenu", "0").toBool(); 
   
  int ui_tab_align = settings->value ("ui_tabs_align", "0").toInt();
  main_tab_widget->setTabPosition (int_to_tabpos (ui_tab_align));

  int docs_tab_align = settings->value ("docs_tabs_align", "0").toInt();
  tab_widget->setTabPosition (int_to_tabpos (docs_tab_align));

  markup_mode = settings->value ("markup_mode", "HTML").toString();
  charset = settings->value ("charset", "UTF-8").toString();
  fname_def_palette = settings->value ("fname_def_palette", ":/palettes/TEA").toString();
  QPoint pos = settings->value ("pos", QPoint (1, 200)).toPoint();
  QSize size = settings->value ("size", QSize (600, 420)).toSize();
  mainSplitter->restoreState (settings->value ("splitterSizes").toByteArray());

  resize (size);
  move (pos);
}


void rvln::writeSettings()
{
  settings->setValue ("pos", pos());
  settings->setValue ("size", size());
  settings->setValue ("charset", charset);
  settings->setValue ("splitterSizes", mainSplitter->saveState());
  settings->setValue ("spl_fman", spl_fman->saveState());

  settings->setValue ("dir_last", dir_last);
  settings->setValue ("fname_def_palette", fname_def_palette);
  settings->setValue ("markup_mode", markup_mode);
  settings->setValue ("VERSION_NUMBER", QString (VERSION_NUMBER));
  settings->setValue ("state", saveState());

  delete settings;
}


void rvln::create_main_widget()
{
  QWidget *main_widget = new QWidget;
  QVBoxLayout *v_box = new QVBoxLayout;
  main_widget->setLayout (v_box);

  main_tab_widget = new QTabWidget;
  main_tab_widget->setObjectName ("main_tab_widget");
  
  main_tab_widget->setTabShape (QTabWidget::Triangular);

  tab_widget = new QTabWidget;
  
  
#if QT_VERSION >= 0x040500
  tab_widget->setMovable (true);
#endif
  
  tab_widget->setObjectName ("tab_widget");
  
  
 //QString tab_widget_style = "QTabWidget::pane { border-top: 0px solid rgb(100,100,100, 80); margin:0;  background: rgba(50,50,50, 100);  border-radius: 1px;   padding:0;}";


//  QString tab_widget_style = "QTabWidget::pane { border-top: 0px solid grey; background-color: grey;}";
  //main_tab_widget->setStyleSheet (tab_widget_style); 
//  tab_widget->setStyleSheet (tab_widget_style); 
 

  QPushButton *bt_close = new QPushButton ("X", this);
  connect (bt_close, SIGNAL(clicked()), this, SLOT(close_current()));
  tab_widget->setCornerWidget (bt_close);

  log = new CLogMemo;


  mainSplitter = new QSplitter (Qt::Vertical);
  v_box->addWidget (mainSplitter);

  cmb_fif = new QComboBox;
  cmb_fif->setInsertPolicy (QComboBox::InsertAtTop); 
  cmb_fif->setObjectName ("FIF");
  

  cmb_fif->setEditable (true);
  fif = cmb_fif->lineEdit();
  connect (fif, SIGNAL(returnPressed()), this, SLOT(search_find()));

  main_tab_widget->setMinimumHeight (10);
  log->setMinimumHeight (10);

  
  mainSplitter->addWidget (main_tab_widget);
  mainSplitter->addWidget (log);

  
  QHBoxLayout *lt_fte = new QHBoxLayout;
  v_box->addLayout (lt_fte);

  

  QToolButton *bt_find = new QToolButton (this);
  QToolButton *bt_prev = new QToolButton (this);
  QToolButton *bt_next = new QToolButton (this);
  bt_next->setArrowType (Qt::RightArrow);
  bt_prev->setArrowType (Qt::LeftArrow);

  connect (bt_find, SIGNAL(clicked()), this, SLOT(search_find()));
  connect (bt_next, SIGNAL(clicked()), this, SLOT(search_find_next()));
  connect (bt_prev, SIGNAL(clicked()), this, SLOT(search_find_prev()));

  bt_find->setIcon (get_theme_icon ("search_find.png"));



  QLabel *l_fif = new QLabel (tr ("FIF"));
 
  lt_fte->addWidget (l_fif, 0, Qt::AlignRight);
  lt_fte->addWidget (cmb_fif, 1);
  
  lt_fte->addWidget (bt_find);
  lt_fte->addWidget (bt_prev);
  lt_fte->addWidget (bt_next);

  mainSplitter->setStretchFactor (1, 1);

  idx_tab_edit = main_tab_widget->addTab (tab_widget, tr ("editor"));
  setCentralWidget (main_widget);

  //tab_widget->resize (tab_widget->width(), width() - 100);
  connect (tab_widget, SIGNAL(currentChanged(int)), this, SLOT(pageChanged(int)));
}


#ifdef SPELLCHECK_ENABLE  

void rvln::setup_spellcheckers()
{
 
#ifdef ASPELL_ENABLE
  spellcheckers.append ("Aspell"); 
#endif

  
#ifdef HUNSPELL_ENABLE
  spellcheckers.append ("Hunspell"); 
#endif

  cur_spellchecker = settings->value ("cur_spellchecker", "Hunspell").toString();
  if (! spellcheckers.contains (cur_spellchecker))
     cur_spellchecker = spellcheckers[0]; 

  
#ifdef ASPELL_ENABLE
  if (cur_spellchecker == "Aspell")
     {
      QString lang = settings->value ("spell_lang", QLocale::system().name().left(2)).toString();

#if defined(Q_OS_WIN) || defined(Q_OS_OS2)

      QString win32_aspell_path = settings->value ("win32_aspell_path", "C:\\Program Files\\Aspell").toString();
      spellchecker = new CSpellchecker (lang, win32_aspell_path);

#else

      spellchecker = new CSpellchecker (lang);

#endif

     }

#endif

  
#ifdef HUNSPELL_ENABLE
   if (cur_spellchecker == "Hunspell")
      spellchecker = new CHunspellChecker (settings->value ("spell_lang", QLocale::system().name().left(2)).toString(), settings->value ("hunspell_dic_path", QDir::homePath()).toString(), dir_user_dict);  
#endif

  
 create_spellcheck_menu();
}

#endif  


void rvln::init_styles()
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

 // qDebug() << "fname_stylesheet = " << fname_stylesheet;

//  qApp->setStyleSheet("QMenu { background-color: blue }");
  
  //QApplication::setStyle (QStyleFactory::create (settings->value ("ui_style", default_style).toString()));
//  QApplication::setStyle (new MyProxyStyle);

   
  MyProxyStyle *ps = new MyProxyStyle (QStyleFactory::create (settings->value ("ui_style", default_style).toString()));
  
  QApplication::setStyle (ps);

//  update_stylesheet (fname_stylesheet);

}


rvln::rvln()
{
  ui_update = true;
  
  b_destroying_all = false;
  
  last_action = 0;

  b_recent_off = false;

  lv_menuitems = NULL;
  fm_entry_mode = FM_ENTRY_MODE_NONE;
  
  date1 = QDate::currentDate();
  date2 = QDate::currentDate();
  
  idx_tab_edit = 0;
  idx_tab_tune = 0;
  idx_tab_fman = 0;
  idx_tab_learn = 0;
  idx_tab_calendar = 0;
  idx_tab_keyboard = 0;

  calendar = 0;

  capture_to_storage_file = false;

 // font_database = new QFontDatabase();

  create_paths();
 // load_userfonts();
  
  QString sfilename = dir_config + "/tea.conf";
  settings = new QSettings (sfilename, QSettings::IniFormat);

  if (settings->value ("override_locale", 0).toBool())
     {
      QString ts = settings->value ("override_locale_val", "en").toString();
      if (ts.length() != 2)
         ts = "en";
      
      qtTranslator.load (QString ("qt_%1").arg (ts),
                         QLibraryInfo::location (QLibraryInfo::TranslationsPath));
      
      qApp->installTranslator (&qtTranslator);

      myappTranslator.load (":/translations/tea_" + ts);
      qApp->installTranslator (&myappTranslator);
     }
  else
      {
       qtTranslator.load (QString ("qt_%1").arg (QLocale::system().name()),
                          QLibraryInfo::location (QLibraryInfo::TranslationsPath));
       
       qApp->installTranslator (&qtTranslator);

       myappTranslator.load (":/translations/tea_" + QLocale::system().name());
       qApp->installTranslator (&myappTranslator);
      }

  QString fname_stylesheet = settings->value ("fname_stylesheet", ":/themes/TEA").toString();
  
  theme_dir = get_file_path (fname_stylesheet) + "/"; 
    
    
  createActions();
  createMenus();
  createToolBars();
  createStatusBar();
  
  init_styles();
    
  update_bookmarks();
  update_templates();
  update_tables();
  update_snippets();
  update_sessions();
  update_scripts();
 
#if QT_VERSION >= 0x050000

  update_plugins();
 
#endif
  
  update_programs();
  update_palettes();
  update_themes();
  update_charsets();
  update_profiles();
  create_markup_hash();
  
  create_moon_phase_algos();
   
  setMinimumSize (12, 12);
  
  create_main_widget();

  idx_prev = 0;
  connect (main_tab_widget, SIGNAL(currentChanged(int)), this, SLOT(main_tab_page_changed(int)));

  readSettings();
    
  
  read_search_options();
  
  documents = new document_holder();
  documents->parent_wnd = this;
  documents->tab_widget = tab_widget;
  documents->recent_menu = menu_file_recent;
  documents->recent_list_fname.append (dir_config).append ("/tea_recent");
  documents->reload_recent_list();
  documents->update_recent_menu();
  documents->log = log;
  documents->status_bar = statusBar();
  documents->markup_mode = markup_mode;
  documents->dir_config = dir_config;
  documents->todo.dir_days = dir_days;
  documents->fname_crapbook = fname_crapbook;
 
  load_palette (fname_def_palette);
 
  update_stylesheet (fname_stylesheet);
  documents->apply_settings();

 
  documents->todo.load_dayfile();

  update_hls();
  update_view_hls();

#ifdef SPELLCHECK_ENABLE
  setup_spellcheckers();
#endif  
  
  shortcuts = new CShortcuts (this);
  shortcuts->fname.append (dir_config).append ("/shortcuts");
  shortcuts->load_from_file (shortcuts->fname);

  sl_fif_history = qstring_load (fname_fif).split ("\n");
  cmb_fif->addItems (sl_fif_history);
  cmb_fif->clearEditText(); 

  createFman();
  createOptions();
  createCalendar();
  createManual();
  
  updateFonts();
 

  //update_stylesheet (fname_stylesheet);
  

  dir_last = settings->value ("dir_last", QDir::homePath()).toString();
  b_preview = settings->value ("b_preview", false).toBool(); 
  
  l_status = new QLabel;
  pb_status = new QProgressBar;
  pb_status->setRange (0, 0);

  statusBar()->insertPermanentWidget (0, pb_status);
  statusBar()->insertPermanentWidget (1, l_status);

  pb_status->hide();
  documents->l_status_bar = l_status;

  img_viewer = new CImgViewer;

  restoreState (settings->value ("state", QByteArray()).toByteArray());

  QString vn = settings->value ("VERSION_NUMBER", "").toString();
  if (vn.isEmpty() || vn != QString (VERSION_NUMBER))
     {
      update_hls (true);
      help_show_news();
     }

  if (settings->value ("session_restore", false).toBool())
     {
      QString fname_session (dir_sessions);
      fname_session.append ("/def-session-777");
      documents->load_from_session (fname_session);
     }
  
  handle_args();
  ui_update = false;
  
  setIconSize (QSize (icon_size, icon_size));
  tb_fman_dir->setIconSize (QSize (icon_size, icon_size));

  QClipboard *clipboard = QApplication::clipboard();
  connect (clipboard , SIGNAL(dataChanged()), this, SLOT(clipboard_dataChanged()));

  
#if QT_VERSION >= 0x050000
  
  plugins_init();

#endif
  
  setAcceptDrops (true);
  
  log->log (tr ("<b>TEA %1</b> by Peter Semiletov, tea@list.ru<br>sites: semiletov.org/tea and tea.ourproject.org<br>development: github.com/psemiletov/tea-qt<br>VK: vk.com/teaeditor<br>read the Manual under the <i>Manual</i> tab!").arg (QString (VERSION_NUMBER)));

  
  QString icon_fname = ":/icons/tea-icon-v3-0" + settings->value ("icon_fname", "1").toString() + ".png";
  
  qApp->setWindowIcon (QIcon (icon_fname));

  
  //tray_icon.setIcon (QIcon(":/icons/tea_icon_v2.png"));
  //tray_icon.show();
  
  idx_tab_edit_activate();
}


void rvln::closeEvent (QCloseEvent *event)
{
  if (main_tab_widget->currentIndex() == idx_tab_tune)
     leaving_tune();

  QString fname (dir_config);
  fname.append ("/last_used_charsets");

  qstring_save (fname, sl_last_used_charsets.join ("\n").trimmed());

  if (settings->value("session_restore", false).toBool())
     {
      QString fname_session (dir_sessions);
      fname_session.append ("/def-session-777");
      documents->save_to_session (fname_session);
     }

  write_search_options();
  writeSettings();

  qstring_save (fname_fif, sl_fif_history.join ("\n"));

  delete documents;
  delete img_viewer;
  
#if QT_VERSION >= 0x050000
  
  plugins_done();
  
#endif  

#ifdef SPELLCHECK_ENABLE

  delete spellchecker;

#endif

  delete shortcuts;
  
  QList<CMarkupPair *> l = hs_markup.values();
  
  foreach (CMarkupPair *p, l)
         {
          p->deleteLater();
         } 

  event->accept();
}


void rvln::newFile()
{
  last_action = qobject_cast<QAction *>(sender());
  documents->create_new();
  main_tab_widget->setCurrentIndex (idx_tab_edit);
}


void rvln::open()
{
  last_action = qobject_cast<QAction *>(sender());

  if (! settings->value ("use_trad_dialogs", "0").toBool())
     {
      CDocument *d = documents->get_current();
      if (d)
         {
          if (file_exists (d->file_name))
              fman->nav (get_file_path (d->file_name));
          else
              fman->nav (dir_last);
         }
      else
          fman->nav (dir_last);

      main_tab_widget->setCurrentIndex (idx_tab_fman);
      fm_entry_mode = FM_ENTRY_MODE_OPEN;

      return;
     }

  QFileDialog dialog (this);
  QSize size = settings->value ("dialog_size", QSize (width(), height())).toSize();
  dialog.resize (size);

  dialog.setFilter (QDir::AllEntries | QDir::Hidden);
  dialog.setOption (QFileDialog::DontUseNativeDialog, true);

  QList<QUrl> sidebarUrls = dialog.sidebarUrls();
  QList<QUrl> sidebarUrls_old = dialog.sidebarUrls();

  sidebarUrls.append (QUrl::fromLocalFile (dir_templates));
  sidebarUrls.append (QUrl::fromLocalFile (dir_snippets));
  sidebarUrls.append (QUrl::fromLocalFile (dir_sessions));
  sidebarUrls.append (QUrl::fromLocalFile (dir_scripts));
  sidebarUrls.append (QUrl::fromLocalFile (dir_tables));
  //sidebarUrls.append (QUrl::fromLocalFile (dir_variants));

#ifdef Q_OS_UNIX

  QDir volDir ("/mnt");
  QStringList volumes (volDir.entryList (volDir.filter() | QDir::NoDotAndDotDot));

  QDir volDir2 ("/media");
  QStringList volumes2 (volDir2.entryList (volDir.filter() | QDir::NoDotAndDotDot));

  foreach (QString v, volumes)  
          sidebarUrls.append (QUrl::fromLocalFile ("/mnt/" + v));

  foreach (QString v, volumes2)
          sidebarUrls.append (QUrl::fromLocalFile ("/media/" + v));


#endif

  dialog.setSidebarUrls (sidebarUrls);

  dialog.setFileMode (QFileDialog::ExistingFiles);
  dialog.setAcceptMode (QFileDialog::AcceptOpen);


  CDocument *d = documents->get_current();
  if (d)
     {
      if (file_exists (d->file_name))
          dialog.setDirectory (get_file_path (d->file_name));
      else
          dialog.setDirectory (dir_last);
     }
  else
      dialog.setDirectory (dir_last);

  dialog.setNameFilter (tr ("All (*);;Text files (*.txt);;Markup files (*.xml *.html *.htm *.);;C/C++ (*.c *.h *.cpp *.hh *.c++ *.h++ *.cxx)"));

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
      
      foreach (QString fn, fileNames)
              {
               CDocument *d = documents->open_file (fn, cb_codecs->currentText());
               if (d)
                  {
                   dir_last = get_file_path (d->file_name);
                   charset = d->charset;
                  }

               add_to_last_used_charsets (cb_codecs->currentText());
              }
     }
  else
      dialog.setSidebarUrls (sidebarUrls_old);

  settings->setValue ("dialog_size", dialog.size());
  update_dyn_menus();
}


bool rvln::save()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return false;

  if (d->textEdit->isReadOnly())
     {
      log->log (tr ("This file is open in the read-only mode. You can save it with another name using <b>Save as</b>"));
      return false;
     }

  if (file_exists (d->file_name))
     d->save_with_name (d->file_name, d->charset);
  else
      return saveAs();

  if (d->file_name == fname_bookmarks)
     update_bookmarks();

  if (d->file_name == fname_programs)
     update_programs();

  return true;
}


bool rvln::saveAs()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return false;

  if (! settings->value ("use_trad_dialogs", "0").toBool())
     {
      main_tab_widget->setCurrentIndex (idx_tab_fman);
      fm_entry_mode = FM_ENTRY_MODE_SAVE;

      if (file_exists (d->file_name))
         fman->nav (get_file_path (d->file_name));
          else
              fman->nav (dir_last);

      ed_fman_fname->setFocus();   
          
      return true;
     }

  QFileDialog dialog (this);
  QSize size = settings->value ("dialog_size", QSize (width(), height())).toSize();
  dialog.resize (size);

  dialog.setFilter(QDir::AllEntries | QDir::Hidden);
  dialog.setOption (QFileDialog::DontUseNativeDialog, true);

  QList<QUrl> sidebarUrls = dialog.sidebarUrls();
  QList<QUrl> sidebarUrls_old = dialog.sidebarUrls();

  sidebarUrls.append(QUrl::fromLocalFile(dir_templates));
  sidebarUrls.append(QUrl::fromLocalFile(dir_snippets));
  sidebarUrls.append(QUrl::fromLocalFile(dir_sessions));
  sidebarUrls.append(QUrl::fromLocalFile(dir_scripts));
  sidebarUrls.append(QUrl::fromLocalFile(dir_tables));
  //sidebarUrls.append(QUrl::fromLocalFile(dir_variants));

#ifdef Q_OS_LINUX

  QDir volDir ("/mnt");
  QStringList volumes (volDir.entryList (volDir.filter() | QDir::NoDotAndDotDot));
  
  foreach (QString v, volumes)  
          sidebarUrls.append (QUrl::fromLocalFile ("/mnt/" + v));


  QDir volDir2 ("/media");
  QStringList volumes2 (volDir2.entryList (volDir2.filter() | QDir::NoDotAndDotDot));

  foreach (QString v, volumes2)
          sidebarUrls.append (QUrl::fromLocalFile ("/media/" + v));


#endif

  dialog.setSidebarUrls (sidebarUrls);

  dialog.setFileMode (QFileDialog::AnyFile);
  dialog.setAcceptMode (QFileDialog::AcceptSave);
  dialog.setConfirmOverwrite (false);
  dialog.setDirectory (dir_last);

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
                                          QMessageBox::Yes | QMessageBox::Default,
                                          QMessageBox::Cancel | QMessageBox::Escape);

          if (ret == QMessageBox::Cancel)
             return false;
         }

      d->save_with_name (fileName, cb_codecs->currentText());
      d->set_markup_mode();
      d->set_hl();

      add_to_last_used_charsets (cb_codecs->currentText());
      update_dyn_menus();

      QFileInfo f (d->file_name);
      dir_last = f.path();
     }
   else
       dialog.setSidebarUrls (sidebarUrls_old);

  settings->setValue ("dialog_size", dialog.size());
  return true;
}


void rvln::about()
{
  last_action = qobject_cast<QAction *>(sender());

  CAboutWindow *a = new CAboutWindow();
  a->move (x() + 20, y() + 20);
  a->show();
}


void rvln::createActions()
{
  icon_size = settings->value ("icon_size", "32").toInt();
  //act_test = new QAction (QIcon (":/icons/file-save.png"), tr ("Test"), this);
  act_test = new QAction (get_theme_icon("file-save.png"), tr ("Test"), this);



  connect (act_test, SIGNAL(triggered()), this, SLOT(test()));

  filesAct = new QAction (get_theme_icon ("current-list.png"), tr ("Files"), this);
  
  
  act_labels = new QAction (get_theme_icon ("labels.png"), tr ("Labels"), this);
  connect (act_labels, SIGNAL(triggered()), this, SLOT(update_labels_list()));

  newAct = new QAction (get_theme_icon ("file-new.png"), tr ("New"), this);

  newAct->setShortcut (QKeySequence ("Ctrl+N"));
  newAct->setStatusTip (tr ("Create a new file"));
  connect (newAct, SIGNAL(triggered()), this, SLOT(newFile()));

  QIcon ic_file_open = get_theme_icon ("file-open.png");
  ic_file_open.addFile (get_theme_icon_fname ("file-open-active.png"), QSize(), QIcon::Active); 
  
  openAct = new QAction (ic_file_open, tr ("Open file"), this);
    
  openAct->setStatusTip (tr ("Open an existing file"));
  connect (openAct, SIGNAL(triggered()), this, SLOT(open()));

  QIcon ic_file_save = get_theme_icon ("file-save.png");
  ic_file_save.addFile (get_theme_icon_fname ("file-save-active.png"), QSize(), QIcon::Active); 
  
  saveAct = new QAction (ic_file_save, tr ("Save"), this);
  saveAct->setShortcut (QKeySequence ("Ctrl+S"));
  saveAct->setStatusTip (tr ("Save the document to disk"));
  connect (saveAct, SIGNAL(triggered()), this, SLOT(save()));

  saveAsAct = new QAction (get_theme_icon ("file-save-as.png"), tr ("Save As"), this);
  saveAsAct->setStatusTip (tr ("Save the document under a new name"));
  connect (saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

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
  connect (aboutAct, SIGNAL(triggered()), this, SLOT(about()));

  aboutQtAct = new QAction (tr ("About Qt"), this);
  connect (aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}


void rvln::createMenus()
{
  fileMenu = menuBar()->addMenu (tr ("File"));
  fileMenu->setTearOffEnabled (true);

  //fileMenu->addAction (act_test);

  fileMenu->addAction (newAct);
  add_to_menu (fileMenu, tr ("Open"), SLOT(open()), "Ctrl+O", get_theme_icon_fname ("file-open.png"));
  add_to_menu (fileMenu, tr ("Last closed file"), SLOT(file_last_opened()));
  add_to_menu (fileMenu, tr ("Open at cursor"), SLOT(open_at_cursor()), "F2");
  add_to_menu (fileMenu, tr ("Crapbook"), SLOT(file_crapbook()), "Alt+M");
  add_to_menu (fileMenu, tr ("Notes"), SLOT(file_notes()));

  fileMenu->addSeparator();

  fileMenu->addAction (saveAct);
  fileMenu->addAction (saveAsAct);

  QMenu *tm = fileMenu->addMenu (tr ("Save as different"));
  tm->setTearOffEnabled (true);

  add_to_menu (tm, tr ("Save .bak"), SLOT(file_save_bak()), "Ctrl+B");
  add_to_menu (tm, tr ("Save timestamped version"), SLOT(file_save_version()));
  add_to_menu (tm, tr ("Save session"), SLOT(session_save_as()));

  fileMenu->addSeparator();

  menu_file_actions = fileMenu->addMenu (tr ("File actions"));
  add_to_menu (menu_file_actions, tr ("Reload"), SLOT(file_reload()));
  add_to_menu (menu_file_actions, tr ("Reload with encoding"), SLOT(file_reload_enc()));
  menu_file_actions->addSeparator();
  add_to_menu (menu_file_actions, tr ("Set UNIX end of line"), SLOT(set_eol_unix()));
  add_to_menu (menu_file_actions, tr ("Set Windows end of line"), SLOT(set_eol_win()));
  add_to_menu (menu_file_actions, tr ("Set old Mac end of line (CR)"), SLOT(set_eol_mac()));
  
      
  menu_file_recent = fileMenu->addMenu (tr ("Recent files"));

  menu_file_bookmarks = fileMenu->addMenu (tr ("Bookmarks"));

  menu_file_edit_bookmarks = fileMenu->addMenu (tr ("Edit bookmarks"));
  add_to_menu (menu_file_edit_bookmarks, tr ("Add to bookmarks"), SLOT(file_add_to_bookmarks()));
  add_to_menu (menu_file_edit_bookmarks, tr ("Find obsolete paths"), SLOT(file_find_obsolete_paths()));

  menu_file_templates = fileMenu->addMenu (tr ("Templates"));
  menu_file_sessions = fileMenu->addMenu (tr ("Sessions"));

  menu_file_configs = fileMenu->addMenu (tr ("Configs"));
  add_to_menu (menu_file_configs, tr ("Bookmarks list"), SLOT(file_open_bookmarks_file()));
  add_to_menu (menu_file_configs, tr ("Programs list"), SLOT(file_open_programs_file()));

  fileMenu->addSeparator();

  menu_recent_off = add_to_menu (fileMenu, tr ("Do not add to recent"), SLOT(recentoff()));
  menu_recent_off->setCheckable (true);

  add_to_menu (fileMenu, tr ("Print"), SLOT(file_print()));
  add_to_menu (fileMenu, tr ("Close current"), SLOT(close_current()), "Ctrl+W");

  fileMenu->addAction (exitAct);


  editMenu = menuBar()->addMenu (tr ("Edit"));
  editMenu->setTearOffEnabled (true);

  editMenu->addAction (cutAct);
  editMenu->addAction (copyAct);
  editMenu->addAction (pasteAct);

 // add_to_menu (editMenu, tr ("Paste from charset"), SLOT(ed_paste_from_charset()));



  editMenu->addSeparator();


  add_to_menu (editMenu, tr ("Block start"), SLOT(ed_block_start()));
  add_to_menu (editMenu, tr ("Block end"), SLOT(ed_block_end()));
  add_to_menu (editMenu, tr ("Copy block"), SLOT(ed_block_copy()));
  add_to_menu (editMenu, tr ("Paste block"), SLOT(ed_block_paste()));
  add_to_menu (editMenu, tr ("Cut block"), SLOT(ed_block_cut()));
  
  

  editMenu->addSeparator();

  add_to_menu (editMenu, tr ("Copy current file name"), SLOT(edit_copy_current_fname()));

  editMenu->addSeparator();

  editMenu->addAction (undoAct);
  editMenu->addAction (redoAct);

  editMenu->addSeparator();
  
  add_to_menu (editMenu, tr ("Indent (tab)"), SLOT(ed_indent()));
  add_to_menu (editMenu, tr ("Un-indent (shift+tab)"), SLOT(ed_unindent()));
  add_to_menu (editMenu, tr ("Indent by first line"), SLOT(indent_by_first_line()));

  editMenu->addSeparator();

  add_to_menu (editMenu, tr ("Comment selection"), SLOT(ed_comment()));
  //add_to_menu (editMenu, tr ("Variants"), SLOT(ed_variants()));

  editMenu->addSeparator();

  add_to_menu (editMenu, tr ("Set as storage file"), SLOT(set_as_storage_file()));
  add_to_menu (editMenu, tr ("Copy to storage file"), SLOT(copy_to_storage_file()));
  add_to_menu (editMenu, tr ("Start/stop capture clipboard to storage file"), SLOT(capture_clipboard_to_storage_file()));



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
/*
  create_menu_from_list (this, tm,
                         QString ("Center Left Right Justify").split (" "),
                         SLOT (mrkup_align()));

  */
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
  add_to_menu (tm, tr ("Antispam e-mail"), SLOT(fn_antispam_email()));
  add_to_menu (tm, tr ("Document weight"), SLOT(mrkup_document_weight()));
  add_to_menu (tm, tr ("Preview selected color"), SLOT(mrkup_preview_color()));
  add_to_menu (tm, tr ("Strip HTML tags"), SLOT(fn_strip_html_tags()));
  add_to_menu (tm, tr ("Rename selected file"), SLOT(rename_selected()));

   
  
  menu_search = menuBar()->addMenu (tr ("Search"));
  menu_search->setTearOffEnabled (true);

  add_to_menu (menu_search, tr ("Find"), SLOT(search_find()));
  add_to_menu (menu_search, tr ("Find next"), SLOT(search_find_next()),"F3");
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
  
  
  menu_find_case = new QAction (tr ("Case sensitive"), this);
  menu_search->addAction (menu_find_case);
  menu_find_case->setCheckable (true);

  menu_find_whole_words = new QAction (tr ("Whole words"), this);
  menu_search->addAction (menu_find_whole_words);
  menu_find_whole_words->setCheckable (true);
  connect (menu_find_whole_words, SIGNAL(triggered()), this, SLOT(search_whole_words_mode()));

  menu_find_from_cursor = new QAction (tr ("From cursor"), this);
  menu_search->addAction (menu_find_from_cursor);
  menu_find_from_cursor->setCheckable (true);
  connect (menu_find_from_cursor, SIGNAL(triggered()), this, SLOT(search_from_cursor_mode()));

  
  menu_find_regexp = new QAction (tr ("Regexp mode"), this);
  menu_search->addAction (menu_find_regexp);
  menu_find_regexp->setCheckable (true);
  connect (menu_find_regexp, SIGNAL(triggered()), this, SLOT(search_regexp_mode()));


  menu_find_fuzzy = new QAction (tr ("Fuzzy mode"), this);
  menu_search->addAction (menu_find_fuzzy);
  menu_find_fuzzy->setCheckable (true);
  connect (menu_find_fuzzy, SIGNAL(triggered()), this, SLOT(search_fuzzy_mode()));

  menu_functions = menuBar()->addMenu (tr ("Functions"));
  menu_functions->setTearOffEnabled (true);

  add_to_menu (menu_functions, tr ("Repeat last"), SLOT(repeat()));
  
  menu_instr = menu_functions->addMenu (tr ("Tools"));
  menu_instr->setTearOffEnabled (true);
  add_to_menu (menu_instr, tr ("Font gallery"), SLOT(instr_font_gallery()));
  add_to_menu (menu_instr, tr ("Scale image"), SLOT(scale_image()));

  

#if QT_VERSION >= 0x050000

  menu_fn_plugins = menu_functions->addMenu (tr ("Plugins"));
  
#endif  
  
  menu_fn_snippets = menu_functions->addMenu (tr ("Snippets"));
  menu_fn_scripts = menu_functions->addMenu (tr ("Scripts"));
  menu_fn_tables = menu_functions->addMenu (tr ("Tables"));

  tm = menu_functions->addMenu (tr ("Place"));
  tm->setTearOffEnabled (true);

  add_to_menu (tm, "Lorem ipsum", SLOT(fn_insert_loremipsum()));
  add_to_menu (tm, tr ("HTML template"), SLOT(fn_insert_template_html()));
  add_to_menu (tm, tr ("HTML5 template"), SLOT(fn_insert_template_html5()));

  add_to_menu (tm, tr ("C++ template"), SLOT(fn_insert_cpp()));
  add_to_menu (tm, tr ("C template"), SLOT(fn_insert_c()));


  add_to_menu (tm, tr ("Date"), SLOT(fn_insert_date()));
  add_to_menu (tm, tr ("Time"), SLOT(fn_insert_time()));


  tm = menu_functions->addMenu (tr ("Case"));
  tm->setTearOffEnabled (true);

  add_to_menu (tm, tr ("UPCASE"), SLOT(upCase()),"Ctrl+Up");
  add_to_menu (tm, tr ("lower case"), SLOT(dnCase()),"Ctrl+Down");


  tm = menu_functions->addMenu (tr ("Sort"));
  tm->setTearOffEnabled (true);

  add_to_menu (tm, tr ("Sort case sensitively"), SLOT(fn_sort_casecare()));
  add_to_menu (tm, tr ("Sort case insensitively"), SLOT(fn_sort_casecareless()));

  add_to_menu (tm, tr ("Sort case sensitively, with separator"), SLOT(fn_sort_casecare_sep()));
  
  
  
  add_to_menu (tm, tr ("Flip a list"), SLOT(fn_flip_a_list()));
  add_to_menu (tm, tr ("Flip a list with separator"), SLOT(fn_flip_a_list_sep()));
  

  tm = menu_functions->addMenu (tr ("Cells"));
  tm->setTearOffEnabled (true);

  add_to_menu (tm, tr ("Sort table by column ABC"), SLOT(fn_sort_latex_table_by_col_abc()));
  add_to_menu (tm, tr ("Swap cells"), SLOT(fn_table_swap_cells()));
  add_to_menu (tm, tr ("Delete by column"), SLOT(fn_table_delete_cells()));
  add_to_menu (tm, tr ("Copy by column[s]"), SLOT(fn_table_copy_cells()));
       
   

  tm = menu_functions->addMenu (tr ("Filter"));
  tm->setTearOffEnabled (true);

  add_to_menu (tm, tr ("Remove duplicates"), SLOT(fn_filter_rm_duplicates()));
  add_to_menu (tm, tr ("Remove empty lines"), SLOT(fn_filter_rm_empty()));
  add_to_menu (tm, tr ("Remove lines < N size"), SLOT(fn_filter_rm_less_than()));
  add_to_menu (tm, tr ("Remove lines > N size"), SLOT(fn_filter_rm_greater_than()));
  add_to_menu (tm, tr ("Remove before delimiter at each line"), SLOT(fn_filter_delete_before_sep()));
  add_to_menu (tm, tr ("Remove after delimiter at each line"), SLOT(fn_filter_delete_after_sep()));

  add_to_menu (tm, tr ("Filter with regexp"), SLOT(fn_filter_with_regexp()));



  tm = menu_functions->addMenu (tr ("Math"));
  tm->setTearOffEnabled (true);

  add_to_menu (tm, tr ("Evaluate"), SLOT(fn_evaluate()), "F4");
  add_to_menu (tm, tr ("Arabic to Roman"), SLOT(fn_number_arabic_to_roman()));
  add_to_menu (tm, tr ("Roman to Arabic"), SLOT(fn_number_roman_to_arabic()));
  add_to_menu (tm, tr ("Decimal to binary"), SLOT(fn_number_decimal_to_binary()));
  add_to_menu (tm, tr ("Binary to decimal"), SLOT(fn_binary_to_decimal()));
  add_to_menu (tm, tr ("Flip bits (bitwise complement)"), SLOT(fn_number_flip_bits()));
  add_to_menu (tm, tr ("Enumerate"), SLOT(fn_enum()));
  add_to_menu (tm, tr ("Sum by last column"), SLOT(fn_sum_by_last_col()));

  add_to_menu (tm, tr ("deg min sec > dec degrees"), SLOT(fn_number_dms2dc()));
  add_to_menu (tm, tr ("dec degrees > deg min sec"), SLOT(fn_number_dd2dms()));





  tm = menu_functions->addMenu (tr ("Morse code"));
  tm->setTearOffEnabled (true);

  add_to_menu (tm, tr ("From Russian to Morse"), SLOT(fn_morse_from_ru()));
  add_to_menu (tm, tr ("From Morse To Russian"), SLOT(fn_morse_to_ru()));

  add_to_menu (tm, tr ("From English to Morse"), SLOT(fn_morse_from_en()));
  add_to_menu (tm, tr ("From Morse To English"), SLOT(fn_morse_to_en()));


  tm = menu_functions->addMenu (tr ("Analyze"));
  tm->setTearOffEnabled (true);

  add_to_menu (tm, tr ("Text statistics"), SLOT(fn_text_stat()));
  add_to_menu (tm, tr ("Extract words"), SLOT(fn_extract_words()));
  add_to_menu (tm, tr ("Words lengths"), SLOT(fn_stat_words_lengths()));
  
  add_to_menu (tm, tr ("UNITAZ quantity sorting"), SLOT(fn_get_words_count()));
  add_to_menu (tm, tr ("UNITAZ sorting alphabet"), SLOT(fn_unitaz_abc()));

  add_to_menu (tm, tr ("Count the substring"), SLOT(fn_count()));
  add_to_menu (tm, tr ("Count the substring (regexp)"), SLOT(fn_count_rx()));

  
  tm = menu_functions->addMenu (tr ("Text"));
  tm->setTearOffEnabled (true);

  add_to_menu (tm, tr ("Apply to each line"), SLOT(fn_apply_to_each_line()),"Alt+E");
  add_to_menu (tm, tr ("Remove formatting"), SLOT(fn_rm_formatting()));
  add_to_menu (tm, tr ("Remove formatting at each line"), SLOT(fn_rm_formatting_at_each_line()));
  add_to_menu (tm, tr ("Compress"), SLOT(fn_rm_compress()));
   
  
  add_to_menu (tm, tr ("Remove trailing spaces"), SLOT(fn_rm_trailing_spaces()));

  add_to_menu (tm, tr ("Escape regexp"), SLOT(fn_escape()));

  add_to_menu (tm, tr ("Reverse"), SLOT(fn_reverse()));
  
  tm = menu_functions->addMenu (tr ("Quotes"));
  tm->setTearOffEnabled (true);

  add_to_menu (tm, tr ("Straight to double angle quotes"), SLOT(fn_convert_quotes_angle()));
  add_to_menu (tm, tr ("Straight to curly double quotes"), SLOT(fn_convert_quotes_curly()));
  
  add_to_menu (tm, tr ("LaTeX: Straight to curly double quotes"), SLOT(fn_convert_quotes_tex_curly()));
  add_to_menu (tm, tr ("LaTeX: Straight to double angle quotes"), SLOT(fn_convert_quotes_tex_angle_01()));
  add_to_menu (tm, tr ("LaTeX: Straight to double angle quotes v2"), SLOT(fn_convert_quotes_tex_angle_02()));

  
#ifdef SPELLCHECK_ENABLE

  menu_functions->addSeparator();

  menu_spell_langs = menu_functions->addMenu (tr ("Spell-checker languages"));
  menu_spell_langs->setTearOffEnabled (true);

  add_to_menu (menu_functions, tr ("Spell check"), SLOT(fn_spell_check()), "", get_theme_icon_fname ("fn-spell-check.png"));
  add_to_menu (menu_functions, tr ("Suggest"), SLOT(fn_spell_suggest()));
  add_to_menu (menu_functions, tr ("Add to dictionary"), SLOT(fn_spell_add_to_dict()));
  add_to_menu (menu_functions, tr ("Remove from dictionary"), SLOT(fn_remove_from_dict()));

  menu_functions->addSeparator();

#endif


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
  
 
  
  menu_programs = menuBar()->addMenu (tr ("Run"));

  menu_nav = menuBar()->addMenu (tr ("Nav"));
  menu_nav->setTearOffEnabled (true);

  add_to_menu (menu_nav, tr ("Save position"), SLOT(nav_save_pos()));
  add_to_menu (menu_nav, tr ("Go to saved position"), SLOT(nav_goto_pos()));
  add_to_menu (menu_nav, tr ("Go to line"), SLOT(nav_goto_line()),"Alt+G");
  add_to_menu (menu_nav, tr ("Next tab"), SLOT(nav_goto_right_tab()));
  add_to_menu (menu_nav, tr ("Prev tab"), SLOT(nav_goto_left_tab()));
  add_to_menu (menu_nav, tr ("Toggle header/source"), SLOT(nav_toggle_hs()));
  add_to_menu (menu_nav, tr ("Focus the Famous input field"), SLOT(nav_focus_to_fif()), "Ctrl+F");
  add_to_menu (menu_nav, tr ("Focus the editor"), SLOT(nav_focus_to_editor()));

  menu_labels = menu_nav->addMenu (tr ("Labels"));
  add_to_menu (menu_nav, tr ("Refresh labels"), SLOT(update_labels_list()));
  
  current_files_menu = menu_nav->addMenu (tr ("Current files"));


  menu_fm = menuBar()->addMenu (tr ("Fm"));
  menu_fm->setTearOffEnabled (true);

  menu_fm_multi_rename = menu_fm->addMenu (tr ("Multi-rename"));
  menu_fm_multi_rename->setTearOffEnabled (true);

  add_to_menu (menu_fm_multi_rename, tr ("Zero pad file names"), SLOT(fman_zeropad()));
  add_to_menu (menu_fm_multi_rename, tr ("Delete N first chars at file names"), SLOT(fman_del_n_first_chars()));
  add_to_menu (menu_fm_multi_rename, tr ("Replace in file names"), SLOT(fman_multreplace()));
  add_to_menu (menu_fm_multi_rename, tr ("Apply template"), SLOT(fman_apply_template()));

  menu_fm_file_ops = menu_fm->addMenu (tr ("File operations"));
  menu_fm_file_ops->setTearOffEnabled (true);

  add_to_menu (menu_fm_file_ops, tr ("Create new directory"), SLOT(fman_create_dir()));
  add_to_menu (menu_fm_file_ops, tr ("Rename"), SLOT(fman_rename()));
  add_to_menu (menu_fm_file_ops, tr ("Delete file"), SLOT(fman_delete()));


  menu_fm_file_infos = menu_fm->addMenu (tr ("File information"));
  menu_fm_file_infos->setTearOffEnabled (true);


  add_to_menu (menu_fm_file_infos, tr ("Count lines in selected files"), SLOT(fman_count_lines_in_selected_files()));
  add_to_menu (menu_fm_file_infos, tr ("Full info"), SLOT(fm_full_info()));

  add_to_menu (menu_fm_file_infos, tr ("MD5 checksum"), SLOT(fm_hashsum_md5()));
  add_to_menu (menu_fm_file_infos, tr ("MD4 checksum"), SLOT(fm_hashsum_md4()));
  add_to_menu (menu_fm_file_infos, tr ("SHA1 checksum"), SLOT(fm_hashsum_sha1()));

  menu_fm_zip = menu_fm->addMenu (tr ("ZIP"));
  menu_fm_zip->setTearOffEnabled (true);

  add_to_menu (menu_fm_zip, tr ("Create new ZIP"), SLOT(fman_create_zip()));
  add_to_menu (menu_fm_zip, tr ("Add to ZIP"), SLOT(fman_add_to_zip()));
  add_to_menu (menu_fm_zip, tr ("Save ZIP"), SLOT(fman_save_zip()));


  add_to_menu (menu_fm_zip, tr ("List ZIP content"), SLOT(fman_zip_info()));
  add_to_menu (menu_fm_zip, tr ("Unpack ZIP to current directory"), SLOT(fman_unpack_zip()));

  
  menu_fm_img_conv = menu_fm->addMenu (tr ("Images"));
  menu_fm_img_conv->setTearOffEnabled (true);

  add_to_menu (menu_fm_img_conv, tr ("Scale by side"), SLOT(fman_img_conv_by_side()));
  add_to_menu (menu_fm_img_conv, tr ("Scale by percentages"), SLOT(fman_img_conv_by_percent()));
  add_to_menu (menu_fm_img_conv, tr ("Create web gallery"), SLOT(fman_mk_gallery()));

  
  add_to_menu (menu_fm, tr ("Go to home dir"), SLOT(fman_home()));
  add_to_menu (menu_fm, tr ("Refresh current dir"), SLOT(fman_refresh()));

  add_to_menu (menu_fm, tr ("Preview image"), SLOT(fman_preview_image()));

  add_to_menu (menu_fm, tr ("Select by regexp"), SLOT(fman_select_by_regexp()));
  add_to_menu (menu_fm, tr ("Deselect by regexp"), SLOT(fman_deselect_by_regexp()));

//  add_to_menu (menu_fm, tr ("Find in files"), SLOT(fman_find_in_files()));

  
  menu_view = menuBar()->addMenu (tr ("View"));
  menu_view->setTearOffEnabled (true);



  menu_view_themes = menu_view->addMenu (tr ("Themes"));
  menu_view_themes->setTearOffEnabled (true);


  menu_view_palettes = menu_view->addMenu (tr ("Palettes"));
  menu_view_palettes->setTearOffEnabled (true);

  menu_view_hl = menu_view->addMenu (tr ("Highlighting mode"));
  menu_view_hl->setTearOffEnabled (true);

  menu_view_profiles = menu_view->addMenu (tr ("Profiles"));
  menu_view_profiles->setTearOffEnabled (true);

  add_to_menu (menu_view, tr ("Save profile"), SLOT(profile_save_as()));
  
  add_to_menu (menu_view, tr ("Toggle word wrap"), SLOT(toggle_wrap()));
  add_to_menu (menu_view, tr ("Hide error marks"), SLOT(view_hide_error_marks()));
  add_to_menu (menu_view, tr ("Toggle fullscreen"), SLOT(view_toggle_fs()));
  add_to_menu (menu_view, tr ("Stay on top"), SLOT(view_stay_on_top()));
  add_to_menu (menu_view, tr ("Darker"), SLOT(darker()));
  add_to_menu (menu_view, tr ("Preview with default browser"), SLOT(view_preview_in_bro()));

  helpMenu = menuBar()->addMenu ("?");
  helpMenu->setTearOffEnabled (true);

  helpMenu->addAction (aboutAct);
  helpMenu->addAction (aboutQtAct);
  add_to_menu (helpMenu, tr ("NEWS"), SLOT(help_show_news()));
  add_to_menu (helpMenu, "TODO", SLOT(help_show_todo()));
  add_to_menu (helpMenu, "ChangeLog", SLOT(help_show_changelog()));
  add_to_menu (helpMenu, tr ("License"), SLOT(help_show_gpl()));
}


void rvln::createToolBars()
{
  openAct->setMenu (menu_file_recent);
  filesAct->setMenu (current_files_menu);
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

  editToolBar->addSeparator();

  editToolBar->addAction (act_labels);
  
  filesToolBar = addToolBar (tr ("Files"));
  filesToolBar->setObjectName("filesToolBar");

  filesToolBar->setIconSize (QSize (icon_size, icon_size));
      
  QToolButton *tb_current_list = new QToolButton();
  tb_current_list->setIcon (get_theme_icon ("current-list.png"));
    
  tb_current_list->setMenu (current_files_menu);
  tb_current_list->setPopupMode(QToolButton::InstantPopup);
  filesToolBar->addWidget (tb_current_list);
}


void rvln::createStatusBar()
{
  statusBar()->showMessage (tr ("Ready"));
}


rvln::~rvln()
{
  qDebug() << "~rvln()";
}


void rvln::pageChanged (int index)
{
  if (b_destroying_all)
      return;

  if (index == -1)
      return;  

  CDocument *d = documents->list[index];
  if (! d)
     return;

  d->update_title (settings->value ("full_path_at_window_title", 1).toBool());
  d->update_status();

  update_labels_menu();
}


void rvln::close_current()
{
  last_action = qobject_cast<QAction *>(sender());

  documents->close_current();
}


void rvln::ed_copy()
{
  last_action = qobject_cast<QAction *>(sender());

  if (main_tab_widget->currentIndex() == idx_tab_edit)
     {
      CDocument *d = documents->get_current();
      if (d)
         d->textEdit->copy();
     }
  else
      if (main_tab_widget->currentIndex() == idx_tab_learn)      
         man->copy();
}


void rvln::ed_cut()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (d)
     d->textEdit->cut();
}


void rvln::ed_paste()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (d)
     d->textEdit->paste();
}


void rvln::ed_undo()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (d)
     d->textEdit->undo();
}


void rvln::ed_redo()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (d)
     d->textEdit->redo();
}


void rvln::ed_clear()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (d)
     d->textEdit->clear();
}


void rvln::upCase()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (d)
      d->textEdit->textCursor().insertText (d->textEdit->textCursor().selectedText().toUpper());
      //d->textEdit->text_replace (d->textEdit->textCursor().selectedText().toUpper());
     
     
 //void CTEAEdit::text_replace (const QString &s)    
}


void rvln::dnCase()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (d)
      d->textEdit->textCursor().insertText (d->textEdit->textCursor().selectedText().toLower());
}


void rvln::remove_formatting()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (d)
     d->textEdit->textCursor().insertText (d->textEdit->textCursor().selectedText().simplified());
}


void rvln::markup_text (const QString &mode)
{
  CDocument *d = documents->get_current();
  if (! d)
     return;
 
  CMarkupPair *p = hs_markup[mode];
  
  if (! p)
     return;
  
  QString t = p->pattern[d->markup_mode];
 
  if (t.isEmpty())
     return;
 
  
  d->textEdit->textCursor().insertText (t.replace ("%s", d->textEdit->textCursor().selectedText()));
}
  
  
void rvln::mrkup_align_center()
{
  last_action = qobject_cast<QAction *>(sender());
  markup_text ("align_center");
}
  

void rvln::mrkup_align_left()
{
  last_action = qobject_cast<QAction *>(sender());
  markup_text ("align_left");
}


void rvln::mrkup_align_right()
{
  last_action = qobject_cast<QAction *>(sender());
  markup_text ("align_right");
}


void rvln::mrkup_align_justify()
{
  last_action = qobject_cast<QAction *>(sender());
  markup_text ("align_justify");
}

  
void rvln::mrkup_bold()
{
  last_action = qobject_cast<QAction *>(sender());

  markup_text ("bold");
}


void rvln::mrkup_italic()
{
  last_action = qobject_cast<QAction *>(sender());
  markup_text ("italic");
}


void rvln::mrkup_underline()
{
  last_action = qobject_cast<QAction *>(sender());
  markup_text ("underline");
}


void rvln::mrkup_para()
{
  last_action = qobject_cast<QAction *>(sender());
  markup_text ("para");
}


void rvln::mrkup_link()
{
  last_action = qobject_cast<QAction *>(sender());
  markup_text ("link");
}


void rvln::mrkup_br()
{
  last_action = qobject_cast<QAction *>(sender());
  markup_text ("newline");
}


void rvln::mrkup_nbsp()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (d)
     d->textEdit->textCursor().insertText ("&nbsp;");
}


QTextDocument::FindFlags rvln::get_search_options()
{
  QTextDocument::FindFlags flags = 0;

  if (menu_find_whole_words->isChecked())
     flags = flags | QTextDocument::FindWholeWords;

  if (menu_find_case->isChecked())
     flags = flags | QTextDocument::FindCaseSensitively;
  
  return flags;
}


void rvln::search_find()
{
  last_action = qobject_cast<QAction *>(sender());

  if (main_tab_widget->currentIndex() == idx_tab_edit)
     {
      CDocument *d = documents->get_current();
      if (! d)
        return;

      QTextCursor cr;
      int from = 0;
      QString fiftxt = fif_get_text();
            
      if (settings->value ("find_from_cursor", "1").toBool())
          from = d->textEdit->textCursor().position();
      else
          from = 0;
         
      d->text_to_search = fiftxt;

      if (menu_find_regexp->isChecked())
         cr = d->textEdit->document()->find (QRegExp (d->text_to_search), from, get_search_options());
      else
          if (menu_find_fuzzy->isChecked())
             {
              int pos = str_fuzzy_search (d->textEdit->toPlainText(), d->text_to_search, from, settings->value ("fuzzy_q", "60").toInt());
              if (pos != -1)
                 {
                  from = pos + d->text_to_search.length() - 1;
                  //set selection:
                  cr = d->textEdit->textCursor();
                  cr.setPosition (pos, QTextCursor::MoveAnchor);
                  cr.movePosition (QTextCursor::Right, QTextCursor::KeepAnchor, d->text_to_search.length());

                  if (! cr.isNull())
                      d->textEdit->setTextCursor (cr);
                 }
              return;
             }
      else //normal search
          cr = d->textEdit->document()->find (d->text_to_search, from, get_search_options());

      if (! cr.isNull())  
          d->textEdit->setTextCursor (cr);
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


void rvln::fman_find()
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


void rvln::fman_find_next()
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


void rvln::fman_find_prev()
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


void rvln::search_find_next()
{
//  last_action = qobject_cast<QAction *>(sender());

  if (main_tab_widget->currentIndex() == idx_tab_edit)
     {
      CDocument *d = documents->get_current();
      if (! d)
         return;

      QTextCursor cr;
      if (menu_find_regexp->isChecked())
         cr = d->textEdit->document()->find (QRegExp (d->text_to_search), d->textEdit->textCursor().position(), get_search_options());
      if (menu_find_fuzzy->isChecked())
         {
          int pos = str_fuzzy_search (d->textEdit->toPlainText(), d->text_to_search, d->textEdit->textCursor().position(), settings->value ("fuzzy_q", "60").toInt());
          if (pos != -1)
             {
              cr = d->textEdit->textCursor();
              cr.setPosition (pos, QTextCursor::MoveAnchor);
              cr.movePosition (QTextCursor::Right, QTextCursor::KeepAnchor, d->text_to_search.length());

              if (! cr.isNull())
                  d->textEdit->setTextCursor (cr);
             }
          return;
         }
      else
          cr = d->textEdit->document()->find (d->text_to_search, d->textEdit->textCursor().position(), get_search_options());
          
      if (! cr.isNull())  
          d->textEdit->setTextCursor (cr);
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


void rvln::search_find_prev()
{
  if (main_tab_widget->currentIndex() == idx_tab_edit)
     {
      CDocument *d = documents->get_current();
      if (! d)
         return;
      
      QTextCursor cr;
      
      if (menu_find_regexp->isChecked())
         cr = d->textEdit->document()->find (QRegExp (d->text_to_search),
                                             d->textEdit->textCursor(),
                                             get_search_options() | QTextDocument::FindBackward);
      else
          cr = d->textEdit->document()->find (d->text_to_search,
                                              d->textEdit->textCursor(),
                                              get_search_options() | QTextDocument::FindBackward);
      
      if (! cr.isNull())  
          d->textEdit->setTextCursor (cr);
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


void rvln::opt_shortcuts_find()
{
  int from = 0;

  QString fiftxt = fif_get_text();

  if (opt_shortcuts_string_to_find == fiftxt)
      from = lv_menuitems->currentRow();

  opt_shortcuts_string_to_find = fiftxt;
    
  if (from == -1)
      from = 0;

  int index = shortcuts->captions.indexOf (QRegExp (opt_shortcuts_string_to_find + ".*",
                                                    Qt::CaseInsensitive), from);
  if (index != -1) 
     lv_menuitems->setCurrentRow (index);
}


void rvln::opt_shortcuts_find_next()
{
  int from = lv_menuitems->currentRow();
  if (from == -1)
     from = 0;
  
  int index = shortcuts->captions.indexOf (QRegExp (opt_shortcuts_string_to_find + ".*",
                                                    Qt::CaseInsensitive), from + 1);
  if (index != -1) 
    lv_menuitems->setCurrentRow (index);
}


void rvln::opt_shortcuts_find_prev()
{
  int from = lv_menuitems->currentRow();
  if (from == -1)
     from = 0;
  
  int index = shortcuts->captions.lastIndexOf (QRegExp (opt_shortcuts_string_to_find + ".*",
                                                        Qt::CaseInsensitive), from - 1);
  if (index != -1) 
     lv_menuitems->setCurrentRow (index); 
}


void rvln::file_save_bak()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  if (! file_exists(d->file_name))
     return;
  
  QString fname (d->file_name);
  fname.append (".bak");
  d->save_with_name_plain (fname);
  log->log (tr ("%1 is saved").arg (fname));
}


void rvln::markup_ins_image()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  main_tab_widget->setCurrentIndex (idx_tab_fman);

  if (file_exists (d->file_name))
     fman->nav (get_file_path (d->file_name));
}


void rvln::slot_lv_menuitems_currentItemChanged (QListWidgetItem *current, QListWidgetItem *previous)
{
  if (! current)
     return;

  QAction *a = shortcuts->find_by_caption (current->text());
  if (a)
     ent_shtcut->setText (a->shortcut().toString());
}


void rvln::pb_assign_hotkey_clicked()
{
  if (! lv_menuitems->currentItem())
     return;

  if (ent_shtcut->text().isEmpty())
     return;

  shortcuts->set_new_shortcut (lv_menuitems->currentItem()->text(), ent_shtcut->text());
  shortcuts->save_to_file (shortcuts->fname);
}


void rvln::pb_remove_hotkey_clicked()
{
  if (! lv_menuitems->currentItem())
     return;

  shortcuts->set_new_shortcut (lv_menuitems->currentItem()->text(), "");
  ent_shtcut->setText ("");
  shortcuts->save_to_file (shortcuts->fname);
}


void rvln::cb_altmenu_stateChanged (int state)
{
  if (state == Qt::Unchecked)
      b_altmenu = false;
  else 
       b_altmenu = true;
  
  settings->setValue ("b_altmenu", b_altmenu);
}


void rvln::createOptions()
{
  tab_options = new QTabWidget;

  idx_tab_tune = main_tab_widget->addTab (tab_options, tr ("options"));
  
  QWidget *page_interface = new QWidget (tab_options);
  page_interface->setObjectName ("page_interface");

  QVBoxLayout *page_interface_layout = new QVBoxLayout;
  page_interface_layout->setAlignment (Qt::AlignTop);


  QString default_style = qApp->style()->objectName();
  if (default_style == "GTK+") //can be buggy
     default_style = "Cleanlooks";


  cmb_styles = new_combobox (page_interface_layout,
                             tr ("UI style"),
                             QStyleFactory::keys(),
                             settings->value ("ui_style", default_style).toString());


  connect (cmb_styles, SIGNAL(currentIndexChanged (const QString &)),
           this, SLOT(slot_style_currentIndexChanged (const QString &)));


  QHBoxLayout *lt_h = new QHBoxLayout;

  QLabel *l_font = new QLabel (tr ("Editor font"));

  cmb_font_name = new QFontComboBox (page_interface);
  cmb_font_name->setCurrentFont (QFont (settings->value ("editor_font_name", "Monospace").toString()));


  spb_font_size = new QSpinBox (page_interface);
  spb_font_size->setRange (6, 64);
  spb_font_size->setValue (settings->value ("editor_font_size", "16").toInt());


  connect (cmb_font_name, SIGNAL(currentIndexChanged (const QString &)),
           this, SLOT(slot_editor_fontname_changed(const QString &)));

  connect (spb_font_size, SIGNAL(valueChanged (int)), this, SLOT(slot_font_size_changed (int )));


  QLabel *l_app_font = new QLabel (tr ("Interface font"));

  cmb_app_font_name = new QFontComboBox (page_interface);
  cmb_app_font_name->setCurrentFont (QFont (settings->value ("app_font_name", qApp->font().family()).toString()));
  spb_app_font_size = new QSpinBox (page_interface);
  spb_app_font_size->setRange (6, 64);
  QFontInfo fi = QFontInfo (qApp->font());

  spb_app_font_size->setValue (settings->value ("app_font_size", fi.pointSize()).toInt());
  connect (spb_app_font_size, SIGNAL(valueChanged (int)), this, SLOT(slot_app_font_size_changed (int )));

  connect (cmb_app_font_name, SIGNAL(currentIndexChanged ( const QString & )),
           this, SLOT(slot_app_fontname_changed(const QString & )));

 // QPushButton *bt_add_user_font = new QPushButton (tr ("Add user font"), this);
  //connect (bt_add_user_font, SIGNAL(clicked()), this, SLOT(add_user_font()));


  lt_h->addWidget (l_font);
  lt_h->addWidget (cmb_font_name);
  lt_h->addWidget (spb_font_size);

  page_interface_layout->addLayout (lt_h, 1);

  lt_h = new QHBoxLayout;

  lt_h->addWidget (l_app_font);
  lt_h->addWidget (cmb_app_font_name);
  lt_h->addWidget (spb_app_font_size);

  page_interface_layout->addLayout (lt_h);
//  page_interface_layout->addWidget (bt_add_user_font);


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

  connect (cmb_ui_tabs_align, SIGNAL(currentIndexChanged (int)),
           this, SLOT(cmb_ui_tabs_currentIndexChanged (int)));


  int docs_tab_align = settings->value ("docs_tabs_align", "0").toInt();
  tab_widget->setTabPosition (int_to_tabpos (docs_tab_align));
  

  QComboBox *cmb_docs_tabs_align = new_combobox (page_interface_layout,
                             tr ("Documents tabs align"),
                             sl_tabs_align,
                             docs_tab_align);
                             
  connect (cmb_docs_tabs_align, SIGNAL(currentIndexChanged (int)),
           this, SLOT(cmb_docs_tabs_currentIndexChanged (int)));                             


  QStringList sl_icon_sizes;
  sl_icon_sizes << "16" << "24" << "32" << "48" << "64";

  cmb_icon_size = new_combobox (page_interface_layout,
                                tr ("Icons size"),
                                sl_icon_sizes,
                                settings->value ("icon_size", "32").toString());

  connect (cmb_icon_size, SIGNAL(currentIndexChanged (const QString &)),
           this, SLOT(cmb_icon_sizes_currentIndexChanged (const QString &)));

  
  QStringList sl_tea_icons;
  sl_tea_icons.append ("1");
  sl_tea_icons.append ("2");
  sl_tea_icons.append ("3");
  
  cmb_tea_icons = new_combobox (page_interface_layout,
                                tr ("TEA program icon"),
                                sl_tea_icons,
                                settings->value ("icon_fname", "1").toString());
 
  connect (cmb_tea_icons, SIGNAL(currentIndexChanged (const QString &)),
           this, SLOT(cmb_tea_icons_currentIndexChanged (const QString &)));

  
  
  cb_show_linenums = new QCheckBox (tr ("Show line numbers"), tab_options);
  cb_show_linenums->setCheckState (Qt::CheckState (settings->value ("show_linenums", "0").toInt()));
  page_interface_layout->addWidget (cb_show_linenums);


  cb_wordwrap = new QCheckBox (tr ("Word wrap"), tab_options);
  cb_wordwrap->setCheckState (Qt::CheckState (settings->value ("word_wrap", "2").toInt()));
  page_interface_layout->addWidget (cb_wordwrap);

  /*
  cb_right_to_left = new QCheckBox (tr ("Right to left alignment"), tab_options);
  cb_right_to_left->setCheckState (Qt::CheckState (settings->value ("right_to_left", "0").toInt()));
  page_interface_layout->addWidget (cb_right_to_left);
*/

  cb_use_hl_wrap = new QCheckBox (tr ("Use wrap setting from highlighting module"), tab_options);
  cb_use_hl_wrap->setCheckState (Qt::CheckState (settings->value ("use_hl_wrap", "0").toInt()));
  page_interface_layout->addWidget (cb_use_hl_wrap);


  cb_hl_current_line = new QCheckBox (tr ("Highlight current line"), tab_options);
  cb_hl_current_line->setCheckState (Qt::CheckState (settings->value ("additional_hl", "0").toInt()));
  page_interface_layout->addWidget (cb_hl_current_line);


  cb_hl_brackets = new QCheckBox (tr ("Highlight paired brackets"), tab_options);
  cb_hl_brackets->setCheckState (Qt::CheckState (settings->value ("hl_brackets", "0").toInt()));
  page_interface_layout->addWidget (cb_hl_brackets);


  cb_auto_indent = new QCheckBox (tr ("Automatic indent"), tab_options);
  cb_auto_indent->setCheckState (Qt::CheckState (settings->value ("auto_indent", "0").toInt()));
  page_interface_layout->addWidget (cb_auto_indent);


  cb_spaces_instead_of_tabs = new QCheckBox (tr ("Use spaces instead of tabs"), tab_options);
  cb_spaces_instead_of_tabs->setCheckState (Qt::CheckState (settings->value ("spaces_instead_of_tabs", "2").toInt()));
  page_interface_layout->addWidget (cb_spaces_instead_of_tabs);


  cb_cursor_xy_visible = new QCheckBox (tr ("Show cursor position"), tab_options);
  cb_cursor_xy_visible->setCheckState (Qt::CheckState (settings->value ("cursor_xy_visible", "2").toInt()));
  page_interface_layout->addWidget (cb_cursor_xy_visible);



  spb_tab_sp_width = new_spin_box (page_interface_layout,
                                   tr ("Tab width in spaces"), 1, 64,
                                   settings->value ("tab_sp_width", 8).toInt());
  
  cb_center_on_cursor = new QCheckBox (tr ("Cursor center on scroll"), tab_options);
  cb_center_on_cursor->setCheckState (Qt::CheckState (settings->value ("center_on_scroll", "2").toInt()));
  page_interface_layout->addWidget (cb_center_on_cursor);


  spb_cursor_blink_time = new_spin_box (page_interface_layout,
                                   tr ("Cursor blink time (msecs, zero is OFF)"), 0, 10000,
                                   settings->value ("cursor_blink_time", 0).toInt());


  spb_cursor_width = new_spin_box (page_interface_layout,
                                   tr ("Cursor width"), 1, 5,
                                   settings->value ("cursor_width", 2).toInt());



  cb_show_margin = new QCheckBox (tr ("Show margin at"), tab_options);
  cb_show_margin->setCheckState (Qt::CheckState (settings->value ("show_margin", "0").toInt()));

  spb_margin_pos = new QSpinBox;
  spb_margin_pos->setValue (settings->value ("margin_pos", 72).toInt());
   
  QHBoxLayout *lt_margin = new QHBoxLayout;
    
  lt_margin->insertWidget (-1, cb_show_margin, 0, Qt::AlignLeft);
  lt_margin->insertWidget (-1, spb_margin_pos, 1, Qt::AlignLeft);

  page_interface_layout->addLayout (lt_margin);

  cb_full_path_at_window_title = new QCheckBox (tr ("Show full path at window title"), tab_options);
  cb_full_path_at_window_title->setCheckState (Qt::CheckState (settings->value ("full_path_at_window_title", "2").toInt()));
  page_interface_layout->addWidget (cb_full_path_at_window_title);
  
  page_interface->setLayout (page_interface_layout);
  page_interface->show();


  QScrollArea *scra_interface = new QScrollArea;
  scra_interface->setWidgetResizable (true);
  scra_interface->setWidget (page_interface);

  tab_options->addTab (scra_interface, tr ("Interface"));


  //tab_options->addTab (page_interface, tr ("Interface"));

  //////////
  
  QWidget *page_common = new QWidget (tab_options);
  QVBoxLayout *page_common_layout = new QVBoxLayout;
  page_common_layout->setAlignment (Qt::AlignTop);

  cb_altmenu = new QCheckBox (tr ("Use Alt key to access main menu"), tab_options);
  if (b_altmenu)
    cb_altmenu->setCheckState (Qt::Checked);
  else
      cb_altmenu->setCheckState (Qt::Unchecked);
  
  connect (cb_altmenu, SIGNAL(stateChanged (int)),
           this, SLOT(cb_altmenu_stateChanged (int)));
   
  
  cb_wasd = new QCheckBox (tr ("Use Left Alt + WASD as additional cursor keys"), tab_options);
  cb_wasd->setCheckState (Qt::CheckState (settings->value ("wasd", "0").toInt()));
 
  
#if QT_VERSION >= 0x050000
    
  cb_use_qregexpsyntaxhl = new QCheckBox (tr ("Old syntax hl engine (restart TEA to apply)"), tab_options);
  cb_use_qregexpsyntaxhl->setCheckState (Qt::CheckState (settings->value ("qregexpsyntaxhl", 0).toInt()));
  
#endif

  cb_auto_img_preview = new QCheckBox (tr ("Automatic preview images at file manager"), tab_options);
  cb_auto_img_preview->setCheckState (Qt::CheckState (settings->value ("b_preview", "0").toInt()));

  cb_session_restore = new QCheckBox (tr ("Restore the last session on start-up"), tab_options);
  cb_session_restore->setCheckState (Qt::CheckState (settings->value ("session_restore", "0").toInt()));
  
  cb_override_locale = new QCheckBox (tr ("Override locale"), tab_options);
  cb_override_locale->setCheckState (Qt::CheckState (settings->value ("override_locale", 0).toInt()));

  ed_locale_override = new QLineEdit (this);
  ed_locale_override->setText (settings->value ("override_locale_val", "en").toString());

  QHBoxLayout *hb_locovr = new QHBoxLayout;
    
  //hb_locovr->addWidget (cb_override_locale);
  //hb_locovr->addWidget (ed_locale_override);

  hb_locovr->insertWidget (-1, cb_override_locale, 0, Qt::AlignLeft);
  hb_locovr->insertWidget (-1, ed_locale_override, 1, Qt::AlignLeft);

  
  cb_override_img_viewer = new QCheckBox (tr ("Use external image viewer for F2"), tab_options);
  cb_override_img_viewer->setCheckState (Qt::CheckState (settings->value ("override_img_viewer", 0).toInt()));

  ed_img_viewer_override = new QLineEdit (this);
  ed_img_viewer_override->setText (settings->value ("img_viewer_override_command", "display %s").toString());

  QHBoxLayout *hb_imgvovr = new QHBoxLayout;
    
  hb_imgvovr->addWidget (cb_override_img_viewer);
  hb_imgvovr->addWidget (ed_img_viewer_override);

  hb_imgvovr->insertWidget (-1, cb_override_img_viewer, 0, Qt::AlignLeft);
  hb_imgvovr->insertWidget (-1, ed_img_viewer_override, 1, Qt::AlignLeft);

  
  
/*
  QStringList sl_ui_langs;
  sl_ui_langs << "en" << "ru" << "de" << "fr";

  cmb_ui_langs = new_combobox (page_common_layout,
                                tr ("UI language"),
                                sl_ui_langs,
                                settings->value ("ui_lang", QLocale::system().name()).toString());

  connect (cmb_ui_langs, SIGNAL(currentIndexChanged (const QString &)),
           this, SLOT(cmb_ui_langs_currentIndexChanged (const QString &)));

*/

  
  cb_use_trad_dialogs = new QCheckBox (tr ("Use traditional File Save/Open dialogs"), tab_options);
  cb_use_trad_dialogs->setCheckState (Qt::CheckState (settings->value ("use_trad_dialogs", "0").toInt()));

  cb_start_on_sunday = new QCheckBox (tr ("Start week on Sunday"), tab_options);
  cb_start_on_sunday->setCheckState (Qt::CheckState (settings->value ("start_week_on_sunday", "0").toInt()));


  cb_northern_hemisphere = new QCheckBox (tr ("Northern hemisphere"), this);
  cb_northern_hemisphere->setCheckState (Qt::CheckState (settings->value ("northern_hemisphere", "2").toInt()));


  page_common_layout->addWidget (cb_start_on_sunday);
  page_common_layout->addWidget (cb_northern_hemisphere);


  cmb_moon_phase_algos = new_combobox (page_common_layout,
                             tr ("Moon phase algorithm"),
                             moon_phase_algos.values(),
                             settings->value ("moon_phase_algo", MOON_PHASE_TRIG2).toInt());
  



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
  
  
  
  
#if QT_VERSION >= 0x050000
  
  page_common_layout->addWidget (cb_use_qregexpsyntaxhl);

#endif  

  page_common_layout->addWidget (cb_auto_img_preview);
  page_common_layout->addWidget (cb_session_restore);
  page_common_layout->addWidget (cb_use_trad_dialogs);
  
 // page_common_layout->addLayout (hb_moon_phase_algo);


  page_common_layout->addLayout (hb_locovr);
  page_common_layout->addLayout (hb_imgvovr);
    

  page_common->setLayout (page_common_layout);
  page_common->show();

  QScrollArea *scra_common = new QScrollArea;
  scra_common->setWidgetResizable (true);
  scra_common->setWidget (page_common);

  tab_options->addTab (scra_common, tr ("Common"));

  //tab_options->addTab (page_common, tr ("Common"));


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

#ifdef SPELLCHECK_ENABLE

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
  ed_spellcheck_path->setText (settings->value ("hunspell_dic_path", QDir::homePath ()).toString());
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
  ed_aspellcheck_path->setText (settings->value ("win32_aspell_path", "C:\\Program Files\\Aspell").toString());
  ed_aspellcheck_path->setReadOnly (true);

  QPushButton *pb_choose_path2 = new QPushButton (tr ("Select"), this);

  connect (pb_choose_path2, SIGNAL(clicked()), this, SLOT(pb_choose_aspell_path_clicked()));

  hb_aspellcheck_path->addWidget (l_t);
  hb_aspellcheck_path->addWidget (ed_aspellcheck_path);
  hb_aspellcheck_path->addWidget (pb_choose_path2);

  vb_spell->addLayout (hb_aspellcheck_path);

#endif

#endif

  connect (cmb_spellcheckers, SIGNAL(currentIndexChanged (const QString&)),
           this, SLOT(cmb_spellchecker_currentIndexChanged (const QString &)));


  page_functions_layout->addWidget (gb_spell);


#endif


  QGroupBox *gb_func_misc = new QGroupBox (tr ("Miscellaneous"));
  QVBoxLayout *vb_func_misc = new QVBoxLayout;
  vb_func_misc->setAlignment (Qt::AlignTop);
  
  gb_func_misc->setLayout (vb_func_misc);
  

  spb_fuzzy_q = new_spin_box (vb_func_misc, tr ("Fuzzy search factor"), 10, 100, settings->value ("fuzzy_q", "60").toInt());

  page_functions_layout->addWidget (gb_func_misc);


  page_functions->setLayout (page_functions_layout);
  page_functions->show();

  QScrollArea *scra_functions = new QScrollArea;
  scra_functions->setWidgetResizable (true);
  scra_functions->setWidget (page_functions);

  tab_options->addTab (scra_functions, tr ("Functions"));

  //tab_options->addTab (page_functions, tr ("Functions"));

/////////////


  QWidget *page_images = new QWidget (tab_options);
  QVBoxLayout *page_images_layout = new QVBoxLayout;
  page_images_layout->setAlignment (Qt::AlignTop);

  QGroupBox *gb_images = new QGroupBox (tr("Miscellaneous"));
  QVBoxLayout *vb_images = new QVBoxLayout;
  vb_images->setAlignment (Qt::AlignTop);

  gb_images->setLayout (vb_images);


  cmb_output_image_fmt = new_combobox (vb_images,
                                       tr ("Image conversion output format"),
                                       bytearray_to_stringlist (QImageWriter::supportedImageFormats()),
                                       settings->value ("output_image_fmt", "jpg").toString());

  cb_output_image_flt = new QCheckBox (tr ("Scale images with bilinear filtering"), this);
  cb_output_image_flt->setCheckState (Qt::CheckState (settings->value ("img_filter", 0).toInt()));

  vb_images->addWidget (cb_output_image_flt);
  
  spb_img_quality = new_spin_box (vb_images, tr ("Output images quality"), -1, 100, settings->value ("img_quality", "-1").toInt());


  cb_exif_rotate = new QCheckBox (tr ("Apply hard rotation by EXIF data"), this);
  cb_exif_rotate->setCheckState (Qt::CheckState (settings->value ("cb_exif_rotate", 0).toInt()));



  cb_output_image_flt = new QCheckBox (tr ("Scale images with bilinear filtering"), this);
  cb_output_image_flt->setCheckState (Qt::CheckState (settings->value ("img_filter", 0).toInt()));

  vb_images->addWidget (cb_output_image_flt);

  cb_zip_after_scale = new QCheckBox (tr ("Zip directory with processed images"), this);
  cb_zip_after_scale->setCheckState (Qt::CheckState (settings->value ("img_post_proc", 0).toInt()));

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
  cb_zor_use_exif->setCheckState (Qt::CheckState (settings->value ("zor_use_exif_orientation", 0).toInt()));
  vb_exif->addWidget (cb_zor_use_exif);
      
   
  page_images->setLayout (page_images_layout);
  //page_images->show();

    
  QScrollArea *scra_images = new QScrollArea;
  scra_images->setWidgetResizable (true);
  scra_images->setWidget (page_images);

  tab_options->addTab (scra_images, tr ("Images"));
 
  
//  tab_options->addTab (page_images, tr ("Images"));
////////////////////////////

  QWidget *page_keyboard = new QWidget (tab_options);

  lt_h = new QHBoxLayout;

  QHBoxLayout *lt_shortcut = new QHBoxLayout;

  QVBoxLayout *lt_vkeys = new QVBoxLayout;
  QVBoxLayout *lt_vbuttons = new QVBoxLayout;

  lv_menuitems = new QListWidget;
  
  lt_vkeys->addWidget (lv_menuitems);

  connect (lv_menuitems, SIGNAL(currentItemChanged (QListWidgetItem *, QListWidgetItem *)),
           this, SLOT(slot_lv_menuitems_currentItemChanged (QListWidgetItem *, QListWidgetItem *)));

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


  connect (tab_options, SIGNAL(currentChanged(int)), this, SLOT(tab_options_pageChanged(int)));
}


void rvln::opt_update_keyb()
{
//  qDebug() << "opt_update_keyb()";

  if (! lv_menuitems)
     return;

  lv_menuitems->clear();

  shortcuts->captions_iterate();

  lv_menuitems->addItems (shortcuts->captions);
}


void rvln::slot_style_currentIndexChanged (const QString &text)
{
  if (text == "GTK+") //because it is buggy with some Qt versions. sorry!
     return;
/*
  QApplication::setStyle (QStyleFactory::create (text));
  
  QApplication::setStyle (new MyProxyStyle);
  */
  
  MyProxyStyle *ps = new MyProxyStyle (QStyleFactory::create (text));
  
  QApplication::setStyle (ps);

  settings->setValue ("ui_style", text);
}


void rvln::open_at_cursor()
{
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
      if (settings->value ("override_img_viewer", 0).toBool())
         {
          QString command = settings->value ("img_viewer_override_command", "display %s").toString();
          command = command.replace ("%s", fname);
          QProcess::startDetached (command);
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
           else
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

     /*
  if (fname.startsWith ("#"))
     {
      fname.remove (0, 1);
      fname.prepend ("name=\"");
      d->textEdit->find (fname);
      return;
     }
*/
     
  if (fname.startsWith ("#"))
     {
      QString t = fname;  
      t.remove (0, 1);
      t.prepend ("name=\"");
      if (d->textEdit->find (t))
         return;

      t = fname;  
      t.remove (0, 1);
      t.prepend ("id=\"");
      d->textEdit->find (t);
      
      return;
     }

     
  documents->open_file (fname, d->charset);
}


void rvln::toggle_wrap()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  if (d->textEdit->lineWrapMode() == QPlainTextEdit::NoWrap)
     d->textEdit->setLineWrapMode (QPlainTextEdit::WidgetWidth);
  else
      d->textEdit->setLineWrapMode (QPlainTextEdit::NoWrap);
}


void rvln::view_preview_in_bro()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QString cm ("file:///");
  cm.append (d->file_name);
  QDesktopServices::openUrl (cm);
}


void rvln::nav_save_pos()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (d)
     d->position = d->textEdit->textCursor().position();
}


void rvln::nav_goto_pos()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QTextCursor cr = d->textEdit->textCursor();
  cr.setPosition (d->position);
  d->textEdit->setTextCursor (cr);
}


void rvln::slot_editor_fontname_changed (const QString &text)
{
  settings->setValue ("editor_font_name", text);
  update_stylesheet (fname_stylesheet);
}


void rvln::slot_app_fontname_changed (const QString &text)
{
  settings->setValue ("app_font_name", text);
  update_stylesheet (fname_stylesheet);
}


void rvln::slot_app_font_size_changed (int i)
{
  settings->setValue("app_font_size", i);
  update_stylesheet (fname_stylesheet);
}


void rvln::slot_font_size_changed (int i)
{
  settings->setValue("editor_font_size", i);
  update_stylesheet (fname_stylesheet);
}


void rvln::mrkup_color()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QColor color = QColorDialog::getColor (Qt::green, this);
  if (! color.isValid())
     return;

  QString s;

  if (d->textEdit->textCursor().hasSelection())
      s = QString ("<span style=\"color:%1;\">%2</span>")
                   .arg (color.name())
                   .arg (d->textEdit->textCursor().selectedText());
  else
      s = color.name();

  d->textEdit->textCursor().insertText (s);
}


void rvln::nav_goto_line()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QTextCursor cr = d->textEdit->textCursor();
  cr.movePosition (QTextCursor::Start);
  //cr.movePosition (QTextCursor::Down, QTextCursor::MoveAnchor, fif_get_text().toInt());
  
  cr.movePosition (QTextCursor::NextBlock, QTextCursor::MoveAnchor, fif_get_text().toInt() - 1);
  
  d->textEdit->setTextCursor (cr);
  d->textEdit->setFocus();
}


void rvln::updateFonts()
{
  documents->apply_settings();
/*
  QFont f;
  f.fromString (settings->value ("editor_font_name", "Monospace").toString());
  f.setPointSize (settings->value ("editor_font_size", "16").toInt());
  man->setFont (f);

  QFont fapp;
  QFontInfo fi = QFontInfo (qApp->font());

  fapp.setPointSize (settings->value ("app_font_size", fi.pointSize()).toInt());
  fapp.fromString (settings->value ("app_font_name", qApp->font().family()).toString());
  qApp->setFont (fapp);

  */
/*
  
  QFontInfo fi = QFontInfo (qApp->font());


  QString fontsize = "font-size:" + settings->value ("app_font_size", fi.pointSize()).toString() + "pt;";  
  QString fontfamily = "font-family:" + settings->value ("app_font_name", qApp->font().family()).toString() + ";";  
  QString edfontsize = "font-size:" + settings->value ("editor_font_size", "16").toString() + "pt;";  
  QString edfontfamily = "font-family:" + settings->value ("editor_font_name", "Monospace").toString() + ";";  
 
  
  stylesheet = "QWidget {" + fontfamily + fontsize + "}\n";
  
  stylesheet += "QPlainTextEdit {" + edfontfamily + edfontsize + "}";
  
  stylesheet += "QTextEdit {" + edfontfamily + edfontsize + "}";

  stylesheet += "CLogMemo {" + fontsize + "}";

  stylesheet += "CLineNumberArea {" + edfontfamily + edfontsize + "}";
  
    
  qApp->setStyleSheet (stylesheet);
*/
}


void rvln::man_find_find()
{
  QString fiftxt = fif_get_text();

  if (man_search_value == fiftxt)
      man->find (fiftxt);
   else
      man->find (fiftxt, 0);

  man_search_value = fiftxt;
}


void rvln::man_find_next()
{
  man->find (man_search_value, get_search_options());
}


void rvln::man_find_prev()
{
  man->find (man_search_value, get_search_options() | QTextDocument::FindBackward);
}


void rvln::createManual()
{
  QWidget *wd_man = new QWidget (this);

  QVBoxLayout *lv_t = new QVBoxLayout;

  QString loc = QLocale::system().name().left (2);

  if (settings->value ("override_locale", 0).toBool())
     {
      QString ts = settings->value ("override_locale_val", "en").toString();
      if (ts.length() != 2)
          ts = "en";
      loc = ts;
     }

  QString filename (":/manuals/");
  filename.append (loc).append (".html");
  
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


void rvln::file_crapbook()
{
  last_action = qobject_cast<QAction *>(sender());

  if (! QFile::exists (fname_crapbook))
      qstring_save (fname_crapbook, tr ("you can put here notes, etc"));

  documents->open_file (fname_crapbook, "UTF-8");
}


void rvln::fn_apply_to_each_line()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QStringList sl = d->textEdit->textCursor().selectedText().split (QChar::ParagraphSeparator);
  QString t = fif_get_text();

  if (t.startsWith ("@@"))
     {
      QString fname = dir_snippets + QDir::separator() + t;

      if (file_exists (fname))
         {
          log->log (tr("snippet %1 is not exists").arg (fname));
          return;
         }

      t = t.remove (0, 2);
      t = qstring_load (fname);
     }


  QMutableListIterator <QString> i (sl);

  while (i.hasNext())
        {
         QString ts (t);
         QString s = i.next();
         i.setValue (ts.replace ("%s", s));
        }

   QString x = sl.join ("\n");

   d->textEdit->textCursor().insertText (x);
}


void rvln::fn_filter_with_regexp()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;
  
  d->textEdit->textCursor().insertText (qstringlist_process (
                                        d->textEdit->textCursor().selectedText(),
                                        fif_get_text(), 
                                        QSTRL_PROC_FLT_WITH_REGEXP));
}


void rvln::fn_reverse()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QString s = d->textEdit->textCursor().selectedText();
  if (s.isEmpty())
     return;
  
  d->textEdit->textCursor().insertText (string_reverse (s));
}


void rvln::file_print()
{
  last_action = qobject_cast<QAction *>(sender());

#ifdef PRINTER_ENABLE

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QPrintDialog *dialog = new QPrintDialog (&printer, this);

  dialog->setWindowTitle (tr ("Print document"));

  if (d->textEdit->textCursor().hasSelection())
      dialog->addEnabledOption (QAbstractPrintDialog::PrintSelection);

  if (dialog->exec() != QDialog::Accepted)
      return;

  d->textEdit->print (&printer);

#endif
}


void rvln::file_last_opened()
{
  last_action = qobject_cast<QAction *>(sender());

  if (documents->recent_files.size() > 0)
     {
      documents->open_file_triplex (documents->recent_files[0]);
      documents->recent_files.removeAt (0);
      documents->update_recent_menu();
      main_tab_widget->setCurrentIndex (idx_tab_edit); 
     }
}


#ifdef SPELLCHECK_ENABLE

void rvln::fn_change_spell_lang()
{
  last_action = qobject_cast<QAction *>(sender());

  QAction *Act = qobject_cast<QAction *>(sender());
  settings->setValue ("spell_lang", Act->text());
  spellchecker->change_lang (Act->text());
  fn_spell_check();
}


void rvln::create_spellcheck_menu()
{
  menu_spell_langs->clear();
  create_menu_from_list (this, menu_spell_langs, spellchecker->get_speller_modules_list(), SLOT(fn_change_spell_lang()));
}


bool ends_with_evilchar (const QString &s)
{
  if (s.endsWith ("\""))
     return true;

  if (s.endsWith ("»"))
     return true;

  if (s.endsWith ("\\"))
     return true;
  
  return false;
}


void rvln::fn_spell_check()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QTime time_start;
  time_start.start();

  pb_status->show();
  pb_status->setRange (0, d->textEdit->toPlainText().size() - 1);
  pb_status->setFormat (tr ("%p% completed"));
  pb_status->setTextVisible (true);

  int i = 0;

  QTextCursor cr = d->textEdit->textCursor();
  int pos = cr.position();

  QString text = d->textEdit->toPlainText();

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
//     if (i % 100 == 0)
  //      qApp->processEvents();
        
     QChar c = text.at (cr.position());
     if (char_is_shit (c))
        while (char_is_shit (c))
              {
               cr.movePosition (QTextCursor::NextCharacter);
               c = text.at (cr.position());
              }

     cr.movePosition (QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
          
     QString stext = cr.selectedText();
     if (! stext.isEmpty() && ends_with_evilchar (stext))
        {
         cr.movePosition (QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
         stext = cr.selectedText();
        }
           
     if (! stext.isEmpty())
     if (! spellchecker->check (cr.selectedText()))
        {
         f = cr.blockCharFormat();
         
#if QT_VERSION >= 0x050000
        
         f.setUnderlineStyle (QTextCharFormat::UnderlineStyle(QApplication::style()->styleHint(QStyle::SH_SpellCheckUnderlineStyle)));
         f.setUnderlineColor (QColor (hash_get_val (global_palette, "error", "red")));
  
#else

         f.setUnderlineStyle (QTextCharFormat::SpellCheckUnderline);
         f.setUnderlineColor (QColor (hash_get_val (global_palette, "error", "red")));

#endif         
         
         cr.mergeCharFormat (f);
        }

      pb_status->setValue (i++);
     }
  while (cr.movePosition (QTextCursor::NextWord));
  
  cr.setPosition (pos);
  d->textEdit->setTextCursor (cr);
  d->textEdit->document()->setModified (false);

  pb_status->hide();
  
  log->log (tr("elapsed milliseconds: %1").arg (time_start.elapsed()));
}


void rvln::fn_spell_add_to_dict()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QTextCursor cr = d->textEdit->textCursor();
  cr.select (QTextCursor::WordUnderCursor); //плохо работает
  QString s = cr.selectedText();

  if (! s.isEmpty())
     spellchecker->add_to_user_dict (s);
}


void rvln::fn_remove_from_dict()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QTextCursor cr = d->textEdit->textCursor();
  cr.select (QTextCursor::WordUnderCursor);
  QString s = cr.selectedText();

  if (! s.isEmpty())
     spellchecker->remove_from_user_dict (s);
}


void rvln::fn_spell_suggest_callback()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QAction *Act = qobject_cast<QAction *>(sender());
  QString new_text = Act->text();

  QTextCursor cr = d->textEdit->textCursor();

  cr.select (QTextCursor::WordUnderCursor);
  QString s = cr.selectedText();
  if (s.isEmpty())
     return; 
  
  if (s[0].isUpper())
     new_text[0] = new_text[0].toUpper();

  cr.insertText (new_text);
  d->textEdit->setTextCursor (cr);
}


void rvln::fn_spell_suggest()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QTextCursor cr = d->textEdit->textCursor();
  cr.select (QTextCursor::WordUnderCursor);
  QString s = cr.selectedText();
  if (s.isEmpty())
     return;

  QStringList l = spellchecker->get_suggestions_list (s);

  QMenu *m = new QMenu (this);
  create_menu_from_list (this, m, l, SLOT (fn_spell_suggest_callback()));
  m->popup (mapToGlobal(d->textEdit->cursorRect().topLeft()));
}

#endif


void rvln::file_open_bookmark()
{
  last_action = qobject_cast<QAction *>(sender());

  QAction *a = qobject_cast<QAction *>(sender());
  documents->open_file_triplex (a->text());
  main_tab_widget->setCurrentIndex (idx_tab_edit); 
}


void rvln::file_add_to_bookmarks()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  if (! file_exists (d->file_name))
     return;

  bool found = false;
  QStringList l_bookmarks = qstring_load (fname_bookmarks).split("\n");

  for (int i = 0; i < l_bookmarks.size(); i++)
      {
       if (l_bookmarks[i].contains (d->file_name))
          {
           l_bookmarks[i] = d->get_triplex();
           found = true;
           break;
          }
      }

  if (! found)
     l_bookmarks.prepend (d->get_triplex());

  bookmarks = l_bookmarks.join ("\n").trimmed();

  qstring_save (fname_bookmarks, bookmarks);
  update_bookmarks();
}


void rvln::file_use_template()
{
  last_action = qobject_cast<QAction *>(sender());

  QAction *a = qobject_cast<QAction *>(sender());

  QString txt = qstring_load (a->data().toString());

  CDocument *d = documents->create_new();
  if (d)
     d->textEdit->textCursor().insertText (txt);
}


void rvln::file_use_snippet()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QAction *a = qobject_cast<QAction *>(sender());
  QString s = qstring_load (a->data().toString());

  if (s.contains ("%s"))
     s = s.replace ("%s", d->textEdit->textCursor().selectedText());

  d->textEdit->textCursor().insertText (s);
}


void rvln::update_templates()
{
  menu_file_templates->clear();

  create_menu_from_dir (this,
                        menu_file_templates,
                        dir_templates,
                        SLOT (file_use_template())
                        );
}


void rvln::update_snippets()
{
   menu_fn_snippets->clear();
   create_menu_from_dir (this,
                        menu_fn_snippets,
                        dir_snippets,
                        SLOT (file_use_snippet())
                        );
}


void rvln::file_save_version()
{
  last_action = qobject_cast<QAction *>(sender());

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


  if (d->save_with_name_plain (fname))
     log->log (tr ("%1 - saved").arg (fname));
  else
     log->log (tr ("Cannot save %1").arg (fname));
}


void rvln::dragEnterEvent (QDragEnterEvent *event)
{
  if (event->mimeData()->hasFormat ("text/uri-list"))
     event->acceptProposedAction();
}


void rvln::dropEvent (QDropEvent *event)
{
  QString fName;
  QFileInfo info;
  
  if (! event->mimeData()->hasUrls())
     return;
     
  foreach (QUrl u, event->mimeData()->urls())     
          {
           fName = u.toLocalFile();
           info.setFile (fName);
           if (info.isFile())
               documents->open_file (fName, cb_fman_codecs->currentText());
           }
              
 event->accept();
}


void rvln::fn_evaluate()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QString s = d->textEdit->textCursor().selectedText();

  std::string utf8_text = s.toUtf8().constData();

  double f = calculate (utf8_text);

  QString fs = s.setNum (f);

  log->log (fs);
}


void rvln::fn_sort_casecare()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  d->textEdit->textCursor().insertText (qstringlist_process (
                                        d->textEdit->textCursor().selectedText(),
                                        fif_get_text(), QSTRL_PROC_FLT_WITH_SORTCASECARE)); 
}


void rvln::fn_sort_casecare_sep()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;
  
  d->textEdit->textCursor().insertText (qstringlist_process (
                                        d->textEdit->textCursor().selectedText(),
                                        fif_get_text(), QSTRL_PROC_FLT_WITH_SORTCASECARE_SEP)); 
}



void rvln::mrkup_text_to_html()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QStringList l;

  if (d->textEdit->textCursor().hasSelection())
     l = d->textEdit->textCursor().selectedText().split (QChar::ParagraphSeparator);
  else
      l = d->textEdit->toPlainText().split("\n");

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
  
  foreach (QString s, l)
          {
           QString t = s.simplified();

           if (t.isEmpty())
              {
               if (d->markup_mode == "HTML")
                  result += "<br>\n";
               else
                   result += "<br />\n";
               }
           else
               result.append ("<p class=\"p1\">").append (s).append ("</p>\n");
          }

  result += "</body>\n</html>";

  CDocument *doc = documents->create_new();

  if (doc)
     doc->textEdit->textCursor().insertText (result);
}


void rvln::fn_text_stat()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QString s;

  bool b_sel = d->textEdit->textCursor().hasSelection();
  
  if (b_sel)
     s = d->textEdit->textCursor().selectedText();
  else
      s = d->textEdit->toPlainText();

  int c = s.length();
  int purechars = 0;
  int lines = 1;

  for (int i = 0; i < c; ++ i)
      {
       QChar ch = s.at(i);

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

  documents->log->log (result);
}


void rvln::fn_antispam_email()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QString s = d->textEdit->textCursor().selectedText();
  QString result;

  int c = s.size();
  for (int i = 0; i < c; i++)
      result.append ("&#").append (QString::number(s.at(i).unicode())).append(";");

  d->textEdit->textCursor().insertText (result);
}


void rvln::search_replace_with()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  d->textEdit->textCursor().insertText (fif_get_text());
}


void rvln::search_replace_all_at_ofiles()
{
  last_action = qobject_cast<QAction *>(sender());

  QStringList l = fif_get_text().split ("~");
  if (l.size() < 2)
     return;

  int c = documents->list.size();
  if (c < 0)
     return;

  Qt::CaseSensitivity cs = Qt::CaseInsensitive;
  if (menu_find_case->isChecked())
     cs = Qt::CaseSensitive; 

  foreach (CDocument *d, documents->list)
          {
           QString s; 
    
           if (menu_find_regexp->isChecked())  
              s = d->textEdit->toPlainText().replace (QRegExp (l[0]), l[1]);
           else
               s = d->textEdit->toPlainText().replace (l[0], l[1], cs);
  
           d->textEdit->selectAll();
           d->textEdit->textCursor().insertText (s);
          }
}


void rvln::search_replace_all()
{
  last_action = qobject_cast<QAction *>(sender());

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
  
      QString s = d->textEdit->textCursor().selectedText();
      
      if (menu_find_regexp->isChecked())
         s = s.replace (QRegExp (l[0]), l[1]);
      else
          s = s.replace (l[0], l[1], cs);
  
      //d->textEdit->selectAll();
      d->textEdit->textCursor().insertText (s);
     }
  else
      if (main_tab_widget->currentIndex() == idx_tab_fman)
        {
          QStringList sl = fman->get_sel_fnames();

          if (sl.size() < 1)
             return;

          char *charset = cb_fman_codecs->currentText().toLatin1().data();

          foreach (QString fname, sl)
                 {
                  QString f = qstring_load (fname, charset);
                  QString r;

                  if (menu_find_regexp->isChecked())
                     r = f.replace (QRegExp (l[0]), l[1]);
                  else
                      r = f.replace (l[0], l[1], cs);

                  qstring_save (fname, r, charset);
                  log->log (tr ("%1 is processed and saved").arg (fname));
                 }
        }
}

/*
void CApplication::saveState (QSessionManager &manager)
{
  manager.setRestartHint (QSessionManager::RestartIfRunning);
}
*/

void rvln::update_charsets()
{
  QString fname (dir_config);
  fname.append ("/last_used_charsets");

  if (! file_exists (fname))
     qstring_save (fname, "UTF-8");

  sl_last_used_charsets = qstring_load (fname).split ("\n");

  foreach (QByteArray codec, QTextCodec::availableCodecs())
          sl_charsets.prepend (codec);
  
  sl_charsets.sort();
}


void rvln::add_to_last_used_charsets (const QString &s)
{
  int i = sl_last_used_charsets.indexOf (s);
  if (i == -1)
     sl_last_used_charsets.prepend (s);
  else
      sl_last_used_charsets.move (i, 0);

  if (sl_last_used_charsets.size() > 3)
     sl_last_used_charsets.removeLast();
}


void rvln::fn_flip_a_list()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;
  
  d->textEdit->textCursor().insertText (qstringlist_process (
                                        d->textEdit->textCursor().selectedText(),
                                        fif_get_text(), 
                                        QSTRL_PROC_LIST_FLIP));
}


void rvln::fn_flip_a_list_sep()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;
  
  d->textEdit->textCursor().insertText (qstringlist_process (
                                        d->textEdit->textCursor().selectedText(),
                                        fif_get_text(), 
                                        QSTRL_PROC_LIST_FLIP_SEP));
}


QString str_to_entities (const QString &s)
{
  QString t = s;
  t = t.replace ("&", "&amp;");

  t = t.replace ("\"", "&quot;");
  t = t.replace ("'", "&apos;");

  t = t.replace ("<", "&lt;");
  t = t.replace (">", "&gt;");

  return t;
}


void rvln::mrkup_tags_to_entities()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (d)
     d->textEdit->textCursor().insertText (str_to_entities (d->textEdit->textCursor().selectedText()));

  //QT5 QString::toHtmlEscaped
}


void rvln::fn_insert_loremipsum()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (d)
     d->textEdit->textCursor().insertText (qstring_load (":/text-data/lorem-ipsum"));
}


void rvln::mrkup_mode_choosed()
{
  last_action = qobject_cast<QAction *>(sender());

  QAction *a = qobject_cast<QAction *>(sender());
  markup_mode = a->text();
  documents->markup_mode = markup_mode;

  CDocument *d = documents->get_current();
  if (d)
     d->markup_mode = markup_mode;
}


void rvln::mrkup_header()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QAction *a = qobject_cast<QAction *>(sender());

  QString r;
  
  if (documents->markup_mode == "Markdown")
     {
      QString t;
      int n = a->text().toLower()[1].digitValue();
      t.fill ('#', n);
      t = t + " " + d->textEdit->textCursor().selectedText();
     }
  else
  r = QString ("<%1>%2</%1>").arg (
               a->text().toLower()).arg (
               d->textEdit->textCursor().selectedText());
                       
  d->textEdit->textCursor().insertText (r);
}


void rvln::mrkup_align()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QAction *a = qobject_cast<QAction *>(sender());

  QString r;

  if (d->markup_mode == "LaTeX")
      r = QString ("\\begin{%1}\n%2\\end{%1}").arg (a->text().toLower()).arg (d->textEdit->textCursor().selectedText());
  //if (d->markup_mode == "MediaWiki")
    // {
//FIXME write code here
     //}
  else
       {
        r.append ("<p style=\"text-align:").append (
                  a->text().toLower()).append (
                  ";\">").append (
                  d->textEdit->textCursor().selectedText()).append (
                  "</p>");
       }

  d->textEdit->textCursor().insertText (r);
}


void rvln::nav_goto_right_tab()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  if (tab_widget->currentIndex() > (tab_widget->count() - 1))
    return;

  tab_widget->setCurrentIndex (tab_widget->currentIndex() + 1);
}


void rvln::nav_goto_left_tab()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  if (tab_widget->currentIndex() == 0)
     return;

  tab_widget->setCurrentIndex (tab_widget->currentIndex() -1);
}


void rvln::fn_filter_rm_less_than()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;
  
  d->textEdit->textCursor().insertText (qstringlist_process (
                                        d->textEdit->textCursor().selectedText(),
                                        fif_get_text(), 
                                        QSTRL_PROC_FLT_LESS));
}


void rvln::fn_filter_rm_greater_than()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  d->textEdit->textCursor().insertText (qstringlist_process (
                                        d->textEdit->textCursor().selectedText(),
                                        fif_get_text(), 
                                        QSTRL_PROC_FLT_GREATER));
}


void rvln::fn_filter_rm_duplicates()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  d->textEdit->textCursor().insertText (qstringlist_process (
                                        d->textEdit->textCursor().selectedText(),
                                        fif_get_text(), 
                                        QSTRL_PROC_FLT_REMOVE_DUPS));
}


void rvln::fn_filter_rm_empty()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  d->textEdit->textCursor().insertText (qstringlist_process (
                                        d->textEdit->textCursor().selectedText(),
                                        fif_get_text(), 
                                        QSTRL_PROC_FLT_REMOVE_EMPTY));
}


void rvln::fn_extract_words()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QStringList w = d->get_words();

  CDocument *nd = documents->create_new();
  if (nd)
     nd->textEdit->textCursor().insertText (w.join("\n"));
}


QString toggle_fname_header_source (const QString &fileName)
{
  QFileInfo f (fileName);

  QString ext = f.suffix();

  if (ext == "c" || ext == "cpp" || ext == "cxx" || ext == "cc" || ext == "c++")
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
     }

  return fileName;
}


void rvln::nav_toggle_hs()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  if (! file_exists (d->file_name))
      return;

  documents->open_file (toggle_fname_header_source (d->file_name), d->charset);
}


QString morse_from_lang (const QString &s, const QString &lang)
{
  QHash<QString, QString> h = hash_load_keyval (":/text-data/morse-" + lang);

  QString result;
  QString x = s.toUpper();

  int c = x.size();
  for (int i = 0; i < c; i++)
      {
       QString t = h.value (QString (x[i]));
       if (! t.isEmpty())
          result.append (t).append (" ");
      }

  return result;
}


QString morse_to_lang (const QString &s, const QString &lang)
{
  QHash<QString, QString> h = hash_load_keyval (":/text-data/morse-" + lang);

  QStringList sl = s.toUpper().split (" ");

  QString result;

  int c = sl.size();
  for (int i = 0; i < c; i++)
      {
       QString t = h.key (sl[i]);
       if (! t.isEmpty())
          result.append (t);
      }

  return result;
}


void rvln::fn_morse_from_en()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (d)
     d->textEdit->textCursor().insertText (morse_from_lang (d->textEdit->textCursor().selectedText().toUpper(), "en"));
}


void rvln::fn_morse_to_en()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (d)
     d->textEdit->textCursor().insertText (morse_to_lang (d->textEdit->textCursor().selectedText(), "en"));
}


void rvln::fn_morse_from_ru()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (d)
     d->textEdit->textCursor().insertText (morse_from_lang (d->textEdit->textCursor().selectedText().toUpper(), "ru"));
}


void rvln::fn_morse_to_ru()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (d)
     d->textEdit->textCursor().insertText (morse_to_lang (d->textEdit->textCursor().selectedText(), "ru"));
}


void rvln::nav_focus_to_fif()
{
  last_action = qobject_cast<QAction *>(sender());

  fif->setFocus (Qt::OtherFocusReason);
}


void rvln::nav_focus_to_editor()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (d)
     d->textEdit->setFocus (Qt::OtherFocusReason);
}


void rvln::edit_copy_current_fname()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (d)
     QApplication::clipboard()->setText (d->file_name);
}


void rvln::fn_insert_date()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (d)
     d->textEdit->textCursor().insertText (QDate::currentDate ().toString (settings->value("date_format", "dd/MM/yyyy").toString()));
}


void rvln::fn_insert_time()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (d)
     d->textEdit->textCursor().insertText (QTime::currentTime ().toString (settings->value("time_format", "hh:mm:ss").toString()));
}


void rvln::fn_rm_formatting_at_each_line()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  d->textEdit->textCursor().insertText (qstringlist_process (d->textEdit->textCursor().selectedText(), "", QSTRL_PROC_REMOVE_FORMATTING));
}


//from http://www.cyberforum.ru/cpp-beginners/thread125615.html
int get_arab_num(std::string rom_str)
{
    int res = 0;
    for(size_t i = 0; i < rom_str.length(); ++i)
    {
        switch(rom_str[i])
        {
        case 'M': 
            res += 1000;
            break;
        case 'D': 
            res += 500;
            break;
        case 'C':
            i + 1 < rom_str.length() 
                    &&  (rom_str[i + 1] == 'D' 
                         || rom_str[i + 1] == 'M') ? res -= 100 : res += 100;            
            break;
        case 'L': 
            res += 50;
            break;
        case 'X': 
            i + 1 < rom_str.length() 
                    &&  (rom_str[i + 1] == 'L' 
                         || rom_str[i + 1] == 'C') ? res -= 10 : res += 10;            
            break;
        case 'V': 
            res += 5;
            break;
        case 'I': 
            i + 1 < rom_str.length() 
                    &&  (rom_str[i + 1] == 'V' 
                         || rom_str[i + 1] == 'X') ? res -= 1 : res += 1;            
            break;
 
        }//switch
    }//for
    return res;
}

void rvln::fn_number_arabic_to_roman()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (d)
     d->textEdit->textCursor().insertText (arabicToRoman (d->textEdit->textCursor().selectedText().toUInt()));
}


void rvln::fn_number_roman_to_arabic()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (d)
     //d->textEdit->textCursor().insertText (QString::number(romanToDecimal (d->textEdit->textCursor().selectedText().toUpper().toUtf8().data())));
     d->textEdit->textCursor().insertText (QString::number(get_arab_num (d->textEdit->textCursor().selectedText().toUpper().toStdString())));
}


void rvln::help_show_gpl()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->open_file (":/COPYING", "UTF-8");
  if (d)
     d->textEdit->setReadOnly (true);
}


void rvln::update_dyn_menus()
{
  update_templates();
  update_snippets();
  update_scripts();
  update_palettes();
  update_themes();

  //opt_update_keyb();
  update_view_hls();
  update_tables();
  update_profiles();
  update_labels_menu();
}


void rvln::file_open_bookmarks_file()
{
  last_action = qobject_cast<QAction *>(sender());

  documents->open_file (fname_bookmarks, "UTF-8");
}


void rvln::file_open_programs_file()
{
  last_action = qobject_cast<QAction *>(sender());

#if defined(Q_OS_WIN) || defined(Q_OS_OS2)

  if (! file_exists (fname_programs))
     qstring_save (fname_programs, tr ("#external programs list. example:\nopera=\"C:\\Program Files\\Opera\\opera.exe \" \"%s\""));

#else

  if (! file_exists (fname_programs))
     qstring_save (fname_programs, tr ("#external programs list. example:\nopera=opera %s"));

#endif

  documents->open_file (fname_programs, "UTF-8");
}


void rvln::process_readyReadStandardOutput()
{
  QProcess *p = qobject_cast<QProcess *>(sender());
  QByteArray a = p->readAllStandardOutput().data();  
  QTextCodec *c = QTextCodec::codecForLocale(); 
  QString t = c->toUnicode (a); 
  
  log->log (t);
}


void rvln::file_open_program()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QAction *a = qobject_cast<QAction *>(sender());
  QString command = programs.value(a->text());
  if (command.isEmpty())
     return;

  if (main_tab_widget->currentIndex() == idx_tab_edit)
     {
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
  process->setProcessChannelMode(QProcess::MergedChannels) ;
  
  process->start (command, QIODevice::ReadWrite);
}


void rvln::update_programs()
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
                         SLOT (file_open_program()));
}


void rvln::fn_insert_template_html()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (d)
     d->textEdit->textCursor().insertText (qstring_load (":/text-data/template-html"));
}


void rvln::fn_insert_template_html5()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (d)
     d->textEdit->textCursor().insertText (qstring_load (":/text-data/template-html5"));
}


void rvln::view_hide_error_marks()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QTextCursor cr = d->textEdit->textCursor();

//delete all underlines
  cr.setPosition (0);
  cr.movePosition (QTextCursor::End, QTextCursor::KeepAnchor);
  QTextCharFormat f = cr.blockCharFormat();
  f.setFontUnderline (false);
  cr.mergeCharFormat (f);
  d->textEdit->document()->setModified (false);
}


void rvln::fn_rm_formatting()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (d)
     d->textEdit->textCursor().insertText (d->textEdit->textCursor().selectedText().simplified());
}


void rvln::fn_rm_compress()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;
   
  QString s = d->textEdit->textCursor().selectedText();
     
  s = s.remove ('\n');   
  s = s.remove ('\t');   
  s = s.remove (' ');   
  s = s.remove (QChar::ParagraphSeparator);   
  d->textEdit->textCursor().insertText (s);
}


void rvln::view_toggle_fs()
{
  last_action = qobject_cast<QAction *>(sender());

  setWindowState(windowState() ^ Qt::WindowFullScreen);
}


void rvln::help_show_news()
{
  last_action = qobject_cast<QAction *>(sender());

  QString fname = ":/NEWS";
  if (QLocale::system().name().left(2) == "ru")
     fname = ":/NEWS-RU";

  CDocument *d = documents->open_file (fname, "UTF-8");
  if (d)
     d->textEdit->setReadOnly (true);
}


void rvln::help_show_changelog()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->open_file (":/ChangeLog", "UTF-8");
  if (d)
     d->textEdit->setReadOnly (true);
}


void rvln::help_show_todo()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->open_file (":/TODO", "UTF-8");
  if (d)
     d->textEdit->setReadOnly (true);
}


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
       
       int i = qrand() % 5;
       
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
  setAttribute (Qt::WA_DeleteOnClose);

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
  QPlainTextEdit *page_maintainers = new QPlainTextEdit();

  if (sl_t.size() == 4)
     {
      page_code->setPlainText (sl_t[0].trimmed());
      page_thanks->setPlainText (sl_t[3].trimmed());
      page_translators->setPlainText (sl_t[1].trimmed());
      page_maintainers->setPlainText (sl_t[2].trimmed());
     }

  tw->addTab (page_code, tr ("Code"));
  tw->addTab (page_thanks, tr ("Acknowledgements"));
  tw->addTab (page_translators, tr ("Translations"));
  tw->addTab (page_maintainers, tr ("Packages"));

  QVBoxLayout *layout = new QVBoxLayout();

  layout->addWidget(logo);
  layout->addWidget(tw);

  setLayout (layout);
  setWindowTitle (tr ("About"));
}


void rvln::cb_script_finished (int exitCode, QProcess::ExitStatus exitStatus)
{
  CDocument *d = documents->get_current();
  if (! d)
     return;

  QString s = qstring_load (fname_tempfile);
  if (! s.isEmpty())
     d->textEdit->textCursor().insertText(s);

  QFile f (fname_tempfile);
  f.remove();
  f.setFileName (fname_tempparamfile);
  f.remove();
}


void rvln::update_scripts()
{
  menu_fn_scripts->clear();

  create_menu_from_dir (this,
                        menu_fn_scripts,
                        dir_scripts,
                        SLOT (fn_run_script())
                        );
}


void rvln::fn_run_script()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QAction *a = qobject_cast<QAction *>(sender());

  QString fname = a->data().toString();
  QString ext = file_get_ext (fname);

  if (! d->textEdit->textCursor().hasSelection())
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

  if (intrp.isEmpty())
      return;

  qstring_save (fname_tempfile, d->textEdit->textCursor().selectedText());
  qstring_save (fname_tempparamfile, fif_get_text());

  QString command = QString ("%1 %2 %3 %4").arg (
                             intrp).arg (
                             fname).arg (
                             fname_tempfile).arg (
                             fname_tempparamfile);

  QProcess *process = new QProcess (this);
  connect(process, SIGNAL(finished ( int, QProcess::ExitStatus )), this, SLOT(cb_script_finished (int, QProcess::ExitStatus )));

  process->start (command);
}


void rvln::cb_button_saves_as()
{
  CDocument *d = documents->get_current();
  if (! d)
     return;

  if (ed_fman_fname->text().isEmpty())
     return;

  QString filename (fman->dir.path());
  
  filename.append ("/").append (ed_fman_fname->text());

  if (file_exists (filename))
     if (QMessageBox::warning (this, "TEA",
                               tr ("%1 already exists\n"
                               "Do you want to overwrite?")
                               .arg (filename),
                               QMessageBox::Yes | QMessageBox::Default,
                               QMessageBox::Cancel | QMessageBox::Escape) == QMessageBox::Cancel)
         return;


   d->save_with_name (filename, cb_fman_codecs->currentText());
   d->set_markup_mode();

   add_to_last_used_charsets (cb_fman_codecs->currentText());

   d->set_hl();
   QFileInfo f (d->file_name);
   dir_last = f.path();
   update_dyn_menus();

   shortcuts->load_from_file (shortcuts->fname);

   fman->refresh();
   main_tab_widget->setCurrentIndex (idx_tab_edit);
}


void rvln::fman_home()
{
  last_action = qobject_cast<QAction *>(sender());

#if defined(Q_OS_WIN) || defined(Q_OS_OS2)

  fman->nav ("c:/");

#else

  fman->nav (QDir::homePath());

#endif
}


void rvln::fman_add_bmk()
{
  sl_places_bmx.prepend (ed_fman_path->text());
  qstring_save (fname_places_bookmarks, sl_places_bmx.join ("\n"));
  update_places_bookmarks();
}


void rvln::fman_del_bmk()
{
  int i = lv_places->currentRow();
  if (i < 5)
     return;

  QString s = lv_places->item(i)->text();
  if (s.isEmpty())
     return;

  i = sl_places_bmx.indexOf (s);
  sl_places_bmx.removeAt (i);
  qstring_save (fname_places_bookmarks, sl_places_bmx.join ("\n"));
  update_places_bookmarks();
}


void rvln::fman_naventry_confirm()
{
  fman->nav (ed_fman_path->text());
}


void rvln::fman_places_itemActivated (QListWidgetItem *item)
{
  int index = lv_places->currentRow();
  
  if (index == 0)
     {
      fman->nav (dir_templates);
      return;
     }
  else
  if (index == 1)
     {
      fman->nav (dir_snippets);
      return;
     }
  else
  if (index == 2)
     {
      fman->nav (dir_scripts);
      return;
     }
  else
  if (index == 3)
     {
      fman->nav (dir_tables);
      return;
     }
  else
  if (index == 4)
     {
      fman->nav (dir_config);
      return;
     }
  
  
  fman->nav (item->text());
}


void rvln::update_places_bookmarks()
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
  if (sl_places_bmx.size() != 0)
     lv_places->addItems (sl_places_bmx);
}


void rvln::fman_open()
{
  QString f = ed_fman_fname->text().trimmed();
  QStringList li = fman->get_sel_fnames();

  if (! f.isEmpty())
  if (f[0] == '/')
     {
      CDocument *d = documents->open_file (f, cb_fman_codecs->currentText());
      if (d)
         {
          dir_last = get_file_path (d->file_name);
          charset = d->charset;
          add_to_last_used_charsets (cb_fman_codecs->currentText());
         }

      main_tab_widget->setCurrentIndex (idx_tab_edit);
      return;
     }

  if (li.size() == 0)
     {
      QString fname (fman->dir.path());
      fname.append ("/").append (f);
      CDocument *d = documents->open_file (fname, cb_fman_codecs->currentText());
      if (d)
         {
          dir_last = get_file_path (d->file_name);
          charset = d->charset;
          add_to_last_used_charsets (cb_fman_codecs->currentText());
         }

      main_tab_widget->setCurrentIndex (idx_tab_edit);
      return;
     }

  foreach (QString fname, li)
          {
           CDocument *d = 0;
           d = documents->open_file (fname, cb_fman_codecs->currentText());
           if (d)
              {
               dir_last = get_file_path (d->file_name);
               charset = d->charset;
              }
          }

  add_to_last_used_charsets (cb_fman_codecs->currentText());
  main_tab_widget->setCurrentIndex (idx_tab_edit);
}


void rvln::fman_create_dir()
{
  bool ok;
  QString newdir = QInputDialog::getText (this, tr ("Enter the name"),
                                                tr ("Name:"), QLineEdit::Normal,
                                                tr ("new_directory"), &ok);
  if (! ok || newdir.isEmpty())
     return;

  QString dname = fman->dir.path() + "/" + newdir;

  QDir d;
  if (! d.mkpath (dname))
     return;

  fman->nav (dname);
}


void rvln::fn_convert_quotes_angle()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QString source = d->textEdit->textCursor().selectedText();
  if (source.isEmpty())
     return;

  QString dest = conv_quotes (source, "\u00AB", "\u00BB");
  
  d->textEdit->textCursor().insertText (dest);
}


void rvln::fn_convert_quotes_curly()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QString source = d->textEdit->textCursor().selectedText();
  if (source.isEmpty())
     return;

  QString dest = conv_quotes (source, "\u201C", "\u201D");
  
  d->textEdit->textCursor().insertText (dest);
}


void rvln::fn_convert_quotes_tex_curly()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QString source = d->textEdit->textCursor().selectedText();
  if (source.isEmpty())
     return;

  QString dest = conv_quotes (source, "``", "\'\'");

  d->textEdit->textCursor().insertText (dest);
}


void rvln::fn_convert_quotes_tex_angle_01()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QString source = d->textEdit->textCursor().selectedText();
  if (source.isEmpty())
     return;

  QString dest = conv_quotes (source, "<<", ">>");

  d->textEdit->textCursor().insertText (dest);
}


void rvln::fn_convert_quotes_tex_angle_02()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QString source = d->textEdit->textCursor().selectedText();
  if (source.isEmpty())
     return;

  QString dest = conv_quotes (source, "\\glqq", "\\grqq");

  d->textEdit->textCursor().insertText (dest);
}



void rvln::fn_enum()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QStringList source = d->textEdit->textCursor().selectedText().split (QChar::ParagraphSeparator);

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

  for (int c = 0; c <= end; c++)
      {
       QString n;
       n = n.setNum (((c + 1) * step));
       if (pad != 0)
          n = n.rightJustified (pad, '0');

       result.append (n).append (prefix).append (source.at(c)).append ('\n');
      }

  d->textEdit->textCursor().insertText (result);
}


void rvln::view_stay_on_top()
{
  last_action = qobject_cast<QAction *>(sender());

  Qt::WindowFlags flags = windowFlags();
  flags ^= Qt::WindowStaysOnTopHint;
  setWindowFlags (flags );
  show();
  activateWindow();
}


void rvln::update_sessions()
{
  menu_file_sessions->clear();
  create_menu_from_dir (this,
                        menu_file_sessions,
                        dir_sessions,
                        SLOT (file_open_session())
                       );
}


void rvln::file_open_session()
{
  last_action = qobject_cast<QAction *>(sender());

  QAction *a = qobject_cast<QAction *>(sender());
  documents->load_from_session (a->data().toString());
}


void rvln::session_save_as()
{
  last_action = qobject_cast<QAction *>(sender());

  if (documents->list.size() < 0)
     return;

  bool ok;
  QString name = QInputDialog::getText (this, tr ("Enter the name"),
                                              tr ("Name:"), QLineEdit::Normal,
                                              tr ("new_session"), &ok);
  if (! ok || name.isEmpty())
     return;

  QString fname (dir_sessions);
  fname.append ("/").append (name);
  documents->save_to_session (fname);
  update_sessions();
}


QHash <QString, QString> rvln::load_eclipse_theme_xml (const QString &fname)
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


void rvln::load_palette (const QString &fileName)
{
  if (! file_exists (fileName))
      return;

  global_palette.clear();
  
  if (file_get_ext (fileName) == "xml")
     global_palette = load_eclipse_theme_xml (fileName);
  else
      global_palette = hash_load_keyval (fileName);
}


void rvln::file_use_palette()
{
  last_action = qobject_cast<QAction *>(sender());

  QAction *a = qobject_cast<QAction *>(sender());
  QString fname (dir_palettes);
  fname.append ("/").append (a->text());

  if (! file_exists (fname))
     fname = ":/palettes/" + a->text();

  fname_def_palette = fname;
  load_palette (fname);
  
  update_stylesheet (fname_stylesheet);
  
  documents->apply_settings();

  //update_logmemo_palette();
}


void rvln::update_logmemo_palette()
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


void rvln::update_palettes()
{
  menu_view_palettes->clear();

  QStringList l1 = read_dir_entries (dir_palettes);
  QStringList l2 = read_dir_entries (":/palettes");
  l1 += l2;

  create_menu_from_list (this, menu_view_palettes,
                         l1,
                         SLOT (file_use_palette()));
}


void rvln::update_hls (bool force)
{
  documents->hls.clear();

  QStringList l1 = read_dir_entries (":/hls");
  l1 << read_dir_entries (dir_hls);
  QString newlist = l1.join("\n").trimmed();

  QString fname_hls_flist (dir_config);
  fname_hls_flist.append ("/fname_hls_flist");

  if (force)
     {
      QFile::remove (fname_hls_flist);
      qDebug() << "new version, hls cache is updated";
     }

  if (! file_exists (fname_hls_flist))
     {
      qstring_save (fname_hls_flist, l1.join ("\n").trimmed());
      QFile::remove (fname_hls_cache);
     }
  else
      {
       QString oldlist = qstring_load (fname_hls_flist);
       if (newlist != oldlist)
          {
           qstring_save (fname_hls_flist, newlist);
           QFile::remove (fname_hls_cache);
          }
      }

  
  if (! file_exists (fname_hls_cache))
     {
      for (int i = 0; i < l1.size(); i++)
          {
           QString fname = ":/hls/" + l1[i];
           if (! file_exists (fname))
              fname = dir_hls + "/" + l1[i];

           QString buffer = qstring_load (fname);
           QString exts = string_between (buffer, "exts=\"", "\"");
           if (! exts.isEmpty())
              {
               QStringList l = exts.split (";");
               for (int i = 0; i < l.size(); i++)
                   documents->hls.insert (l[i], fname);
              }
          }

      qstring_save (fname_hls_cache, hash_keyval_to_string (documents->hls));
     }
  else
      documents->hls = hash_load_keyval (fname_hls_cache);
}


void rvln::mrkup_preview_color()
{
  CDocument *d = documents->get_current();
  if (! d)
     return;

  if (! d->textEdit->textCursor().hasSelection())
     return;
  
  QString color = d->textEdit->textCursor().selectedText();

  if (QColor::colorNames().indexOf (color) == - 1)
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


void rvln::fman_drives_changed (const QString & path)
{
  if (! ui_update)
     fman->nav (path);
}


void rvln::createFman()
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

  QAction *act_fman_go = new QAction (get_theme_icon("go.png"), tr ("Go"), this);
  connect (act_fman_go, SIGNAL(triggered()), this, SLOT(fman_naventry_confirm()));

  QAction *act_fman_home = new QAction (get_theme_icon ("home.png"), tr ("Home"), this);
  connect (act_fman_home, SIGNAL(triggered()), this, SLOT(fman_home()));

  QAction *act_fman_refresh = new QAction (get_theme_icon ("refresh.png"), tr ("Refresh"), this);
  QAction *act_fman_ops = new QAction (get_theme_icon ("create-dir.png"), tr ("Operations"), this);
  act_fman_ops->setMenu (menu_fm_file_ops);

  tb_fman_dir->addAction (act_fman_go);
  tb_fman_dir->addAction (act_fman_home);
  tb_fman_dir->addAction (act_fman_refresh);
  tb_fman_dir->addAction (act_fman_ops);

//#if defined(Q_WS_WIN) || defined(Q_OS_OS2)
#if defined(Q_OS_WIN) || defined(Q_OS_OS2)

  cb_fman_drives = new QComboBox;
  lah_topbar->addWidget (cb_fman_drives);

  QFileInfoList l_drives = QDir::drives();
  foreach (QFileInfo fi, l_drives)
           cb_fman_drives->addItem (fi.path());
 
#endif

  lah_topbar->addWidget (ed_fman_path);
  lah_topbar->addWidget (tb_fman_dir);

  lah_controls->addWidget (l_t);
  lah_controls->addWidget (ed_fman_fname);

  l_t = new QLabel (tr ("Charset"));
  
  QPushButton *bt_magicenc = new QPushButton ("?", this);
        
  bt_magicenc->setMaximumWidth (QApplication::fontMetrics().width ("???"));
  connect (bt_magicenc, SIGNAL(clicked()), this, SLOT(guess_enc()));
  
    
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

  QPushButton *bt_fman_save_as = new QPushButton (tr ("Save as"), this);
  connect (bt_fman_save_as, SIGNAL(clicked()), this, SLOT(cb_button_saves_as()));

  lah_controls->addWidget (l_t);
    
  
  QHBoxLayout *lt_hb = new QHBoxLayout;
  
  lt_hb->addWidget (cb_fman_codecs);
  lt_hb->addWidget (bt_magicenc);
  
  lah_controls->addLayout (lt_hb); 

  lah_controls->addWidget (bt_fman_open);
  lah_controls->addWidget (bt_fman_save_as);

  fman = new CFMan;

  connect (fman, SIGNAL(file_activated (const QString &)), this, SLOT(fman_file_activated (const QString &)));
  connect (fman, SIGNAL(dir_changed  (const QString &)), this, SLOT(fman_dir_changed  (const QString &)));
  connect (fman, SIGNAL(current_file_changed  (const QString &, const QString &)), this, SLOT(fman_current_file_changed  (const QString &, const QString &)));

  connect (act_fman_refresh, SIGNAL(triggered()), fman, SLOT(refresh()));

#if defined(Q_OS_WIN) || defined(Q_OS_OS2)

  connect (cb_fman_drives, SIGNAL(currentIndexChanged ( const QString & )),
          this, SLOT(fman_drives_changed(const QString & )));

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
  connect (lv_places, SIGNAL(itemActivated (QListWidgetItem *)), this, SLOT(fman_places_itemActivated (QListWidgetItem *)));

  QVBoxLayout *vbox = new QVBoxLayout;
  vbox->addLayout (lah_places_bar);
  vbox->addWidget (lv_places);

  lw_right->addLayout (vbox);
  
  fman->setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);

  spl_fman = new QSplitter (this);
  spl_fman->setChildrenCollapsible (true);
  
//  spl_fman->setStretchFactor (1, 1);


  spl_fman->addWidget (fman);
  spl_fman->addWidget (w_right);
  
  spl_fman->restoreState (settings->value ("spl_fman").toByteArray());

  lav_main->addLayout (lah_topbar);
  lav_main->addWidget (spl_fman);

  wd_fman->setLayout (lav_main);

  fman_home();
  
  idx_tab_fman = main_tab_widget->addTab (wd_fman, tr ("files"));
}


void rvln::fman_file_activated (const QString &full_path)
{
 
  if (file_get_ext (full_path) == ("zip"))
     {
      //check if plugin:

 //      qDebug() << "full path = " << full_path;
      
//       qDebug() << "1";
       
      CZipper z;
      
      //QString fname = full_path;

      QStringList sl = z.unzip_list (full_path);
      
//      qDebug() << "sl.size = " << sl.size();
      
      
      bool is_plugin = false;
      
//      qDebug() << "2";
             
      for (int i = 0; i < sl.size(); i++)
          {
           if (sl[i].endsWith("main.qml"))
             {
              is_plugin = true;
              break;
             }
          }
      
  //    qDebug() << "3";
       
      
      if (is_plugin)
         {
          int ret = QMessageBox::warning (this, "TEA",
                                          tr ("It seems that %1 contains TEA plugin.\n Do you want to install it?")
                                           .arg (full_path),
                                          QMessageBox::Ok | QMessageBox::Default,
                                          QMessageBox::Cancel | QMessageBox::Escape);

          if (ret == QMessageBox::Cancel)
             return;
          
          if (! z.unzip (full_path, dir_plugins))
             qDebug() << "! unzip";


#if QT_VERSION >= 0x050000

           update_plugins();

#endif
          
         }
      else   
          //fman_zip_info();
      {
    //   qDebug() << "4";
         
       for (int i = 0; i < sl.size(); i++)
         sl[i] = sl[i].append ("<br>");
      
       log->log (sl.join("\n"));

       
      }
       
      
      //log->log (s);      
      
      //fman_zip_info();
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
     {
      dir_last = get_file_path (d->file_name);
      charset = d->charset;
     }

  add_to_last_used_charsets (cb_fman_codecs->currentText());
  main_tab_widget->setCurrentIndex (idx_tab_edit);
}


void rvln::fman_dir_changed (const QString &full_path)
{
  ui_update = true;
  ed_fman_path->setText (full_path);

//#if defined(Q_WS_WIN) || defined(Q_OS_OS2)
#if defined(Q_OS_WIN) || defined(Q_OS_OS2)


  cb_fman_drives->setCurrentIndex (cb_fman_drives->findText (full_path.left(3).toUpper()));

#endif

  ui_update = false;
}


void rvln::fman_current_file_changed (const QString &full_path, const QString &just_name)
{
  ed_fman_fname->setText (just_name);

  if (b_preview)
  if (is_image (full_path))
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


void rvln::fman_rename()
{
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
  
  QString newfpath (fi.path());
  newfpath.append ("/").append (newname);
  QFile::rename (fname, newfpath);
  update_dyn_menus();
  fman->refresh();

  QModelIndex index = fman->index_from_name (newname);
  fman->selectionModel()->setCurrentIndex (index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
  fman->scrollTo (index, QAbstractItemView::PositionAtCenter);
}


void rvln::fman_delete()
{
  QString fname = fman->get_sel_fname();
  if (fname.isEmpty())
     return;

  int i = fman->get_sel_index();

  QFileInfo fi (fname);
  if (! fi.exists() && ! fi.isWritable())
     return;
 
  if (QMessageBox::warning (this, "TEA",
                            tr ("Are you sure to delete\n"
                            "%1?").arg (fname),
                            QMessageBox::Yes | QMessageBox::Default,
                            QMessageBox::No | QMessageBox::Escape) == QMessageBox::No)
      return;
 
  QFile::remove (fname);
  update_dyn_menus();
  fman->refresh();

  if (i < fman->list.count())
     {
      QModelIndex index = fman->index_from_idx (i);
      fman->selectionModel()->setCurrentIndex (index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
      fman->scrollTo (index, QAbstractItemView::PositionAtCenter);
     }
}


void rvln::fm_hashsum_md5()
{
  QString filename = fman->get_sel_fname();
  if (! file_exists (filename))
      return;

  QCryptographicHash h (QCryptographicHash::Md5);

  h.addData (file_load (filename));
  QString sm = h.result().toHex();
  QString s = tr ("MD5 checksum for %1 is %2").arg (filename).arg (sm);

  log->log (s);
}


void rvln::fm_hashsum_md4()
{
  QCryptographicHash h (QCryptographicHash::Md4);
  QString filename = fman->get_sel_fname();

  if (! file_exists (filename))
      return;

  h.addData (file_load (filename));
  QString sm = h.result().toHex();
  QString s = tr ("MD4 checksum for %1 is %2").arg (filename).arg (sm);

  log->log (s);
}


void rvln::fm_hashsum_sha1()
{
  QString filename = fman->get_sel_fname();
  if (! file_exists (filename))
      return;

  QCryptographicHash h (QCryptographicHash::Sha1);

  h.addData (file_load (filename));
  QString sm = h.result().toHex();
  QString s = tr ("SHA1 checksum for %1 is %2").arg (filename).arg (sm);

  log->log (s);
}


void rvln::fman_refresh()
{
  fman->refresh();
}


void rvln::fm_full_info()
{
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
  l.append (tr ("created: %1").arg (fi.created().toString ("yyyy-MM-dd@hh:mm:ss")));
  l.append (tr ("modified: %1").arg (fi.lastModified().toString ("yyyy-MM-dd@hh:mm:ss")));

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


void rvln::file_reload()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  d->reload (d->charset);
}


void CTextListWnd::closeEvent (QCloseEvent *event)
{
  event->accept();
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
}


void rvln::file_reload_enc_itemDoubleClicked (QListWidgetItem *item)
{
  CDocument *d = documents->get_current();
  if (d)
     d->reload (item->text());
}


void rvln::file_reload_enc()
{
  last_action = qobject_cast<QAction *>(sender());

  CTextListWnd *w = new CTextListWnd (tr ("Reload with encoding"), tr ("Charset"));

  if (sl_last_used_charsets.size () > 0)
     w->list->addItems (sl_last_used_charsets + sl_charsets);
  else
      w->list->addItems (sl_charsets);
     
  connect (w->list, SIGNAL(itemDoubleClicked ( QListWidgetItem *)), 
           this, SLOT(file_reload_enc_itemDoubleClicked ( QListWidgetItem *)));

  w->show();
}


void rvln::handle_args()
{
  QStringList l = qApp->arguments();
  int size = l.size();
  if (size < 2)
     return;

  QString charset = settings->value ("cmdline_default_charset", "UTF-8").toString();//"UTF-8";
  
  for (int i = 1; i < size; i++)
      {
       QString t = l.at(i);
       if (t.startsWith("--charset"))
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


bool CStrIntPair_bigger_than (CStrIntPair *o1, CStrIntPair *o2)
{
   return o1->int_value > o2->int_value;
}


bool CStrIntPair_bigger_than_str (CStrIntPair *o1, CStrIntPair *o2)
{
   return o1->string_value < o2->string_value;
}


void rvln::run_unitaz (int mode)
{
  CDocument *d = documents->get_current();
  if (! d)
     return;

  pb_status->show();
  pb_status->setFormat (tr ("%p% completed"));
  pb_status->setTextVisible (true);

  int c = 0;

  QStringList total = d->get_words();
  QHash <QString, int> h;

  pb_status->setRange (0, total.size() - 1);

  foreach (QString w, total)
          {
           if (c % 100 == 0)
              qApp->processEvents();

           QHash<QString, int>::iterator i = h.find (w.toLower());
           if (i != h.end())
              i.value() += 1;
           else
                h.insert(w.toLower(), 1);

           pb_status->setValue (c++);
          }

//  QStringList l;
  QList <CStrIntPair*> uwords;

  foreach (QString s, h.keys())
           uwords.append (new CStrIntPair (s, h.value (s)));

  if (mode == 0)
     qSort (uwords.begin(), uwords.end(), CStrIntPair_bigger_than);
  if (mode == 1)
     qSort (uwords.begin(), uwords.end(), CStrIntPair_bigger_than_str);

  QStringList outp;

  foreach (CStrIntPair *sp, uwords)
          outp.append (sp->string_value + " = " + QString::number (sp->int_value));

  double diff = static_cast <double> (total.size()) / static_cast <double> (uwords.size());
  double diff_per_cent = get_percent (static_cast <double> (total.size()), static_cast <double> (uwords.size()));

  outp.prepend (tr ("total to unique per cent diff: %1").arg (diff_per_cent, 0, 'f', 6));
  outp.prepend (tr ("total / unique: %1").arg (diff, 0, 'f', 6));
  outp.prepend (tr ("words unique: %1").arg (uwords.size()));
  outp.prepend (tr ("words total: %1").arg (total.size()));
  outp.prepend (tr ("text analysis of: %1").arg (d->file_name));
  outp.prepend (tr ("UNITAZ: UNIverlsal Text AnalyZer"));

  CDocument *nd = documents->create_new();
  nd->textEdit->textCursor().insertText (outp.join ("\n"));

  while (! uwords.isEmpty())
        delete uwords.takeFirst();

  pb_status->hide();
}


void rvln::fn_get_words_count()
{
  last_action = qobject_cast<QAction *>(sender());

  run_unitaz (0);
}


void rvln::fn_unitaz_abc()
{
  last_action = qobject_cast<QAction *>(sender());

  run_unitaz (1);
}


CStrIntPair::CStrIntPair (const QString &s, int i): QObject()
{
   string_value = s;
   int_value = i;
}

void rvln::create_markup_hash()
{
  CMarkupPair *p = new CMarkupPair;

  p->pattern["Docbook"] = "<emphasis role=\"bold\">%s</emphasis>";
  p->pattern["LaTeX"] = "\\textbf{%s}";
  p->pattern["HTML"] = "<b>%s</b>";
  p->pattern["XHTML"] = "<b>%s</b>";
  p->pattern["Lout"] = "@B{%s}";
  p->pattern["MediaWiki"] = "'''%s'''";
  p->pattern["DokuWiki"] = "**%s**";
  p->pattern["Markdown"] = "**%s**";
  
  hs_markup.insert ("bold", p);

  p = new CMarkupPair;

  p->pattern["LaTeX"] = "\\begin{center}%s\\end{center}";
  p->pattern["HTML"] = "<p style=\"text-align:center;\">%s</p>";
  p->pattern["XHTML"] = "<p style=\"text-align:center;\">%s</p>";
   
  hs_markup.insert ("align_center", p);


  p->pattern["HTML"] = "<p style=\"text-align:justify;\">%s</p>";
  p->pattern["XHTML"] = "<p style=\"text-align:justify;\">%s</p>";
   
  hs_markup.insert ("align_justify", p);

  p = new CMarkupPair;

  p->pattern["LaTeX"] = "\\begin{flushleft}%s\\end{flushleft}";
  p->pattern["HTML"] = "<p style=\"text-align:left;\">%s</p>";
  p->pattern["XHTML"] = "<p style=\"text-align:left;\">%s</p>";
   
  hs_markup.insert ("align_left", p);
  
  p = new CMarkupPair;

  p->pattern["LaTeX"] = "\\begin{flushright}%s\\end{flushright}";
  p->pattern["HTML"] = "<p style=\"text-align:right;\">%s</p>";
  p->pattern["XHTML"] = "<p style=\"text-align:right;\">%s</p>";
   
  hs_markup.insert ("align_right", p);
  
  p = new CMarkupPair;

  p->pattern["Docbook"] = "<emphasis role=\"italic\">%s</emphasis>";
  p->pattern["LaTeX"] = "\\textit{%s}";
  p->pattern["HTML"] = "<i>%s</i>";
  p->pattern["XHTML"] = "<i>%s</i>";
  p->pattern["Lout"] = "@I{%s}";
  p->pattern["MediaWiki"] = "''%s''";
  p->pattern["DokuWiki"] = "//%s//";
  p->pattern["Markdown"] = "*%s*";
  
  hs_markup.insert ("italic", p);

  
  p = new CMarkupPair;

  p->pattern["Docbook"] = "<emphasis role=\"underline\">%s</emphasis>";
  p->pattern["LaTeX"] = "\\underline{%s}";
  p->pattern["HTML"] = "<u>%s</u>";
  p->pattern["XHTML"] = "<u>%s</u>";
  p->pattern["Lout"] = "@Underline{%s}";
  p->pattern["MediaWiki"] = "<u>%s</u>";
  p->pattern["DokuWiki"] = "__%s__";

  hs_markup.insert ("underline", p);


  p = new CMarkupPair;

  p->pattern["Docbook"] = "<para>%s</para>";
  p->pattern["HTML"] = "<p>%s</p>";
  p->pattern["XHTML"] = "<p>%s</p>";
  p->pattern["Lout"] = "@PP%s";
  
  hs_markup.insert ("para", p);


  p = new CMarkupPair;

  p->pattern["Docbook"] = "<ulink url=\"\">%s</ulink>";
  p->pattern["HTML"] = "<a href=\"\">%s</a>";
  p->pattern["XHTML"] = "<a href=\"\">%s</a>";
  p->pattern["LaTeX"] = "\\href{}{%s}";
  p->pattern["Markdown"] = "[](%s)";

  hs_markup.insert ("link", p);


  p = new CMarkupPair;

  p->pattern["LaTeX"] = "\\newline";
  p->pattern["HTML"] = "<br>";
  p->pattern["XHTML"] = "<br />";
  p->pattern["Lout"] = "@LLP";
  p->pattern["MediaWiki"] = "<br />";
  p->pattern["DokuWiki"] = "\\\\ ";
  p->pattern["DokuWiki"] = "  ";

  hs_markup.insert ("newline", p);
}


void rvln::count_substring (bool use_regexp)
{
  CDocument *d = documents->get_current();
  if (! d)
     return;

  QString text;

  if (d->textEdit->textCursor().hasSelection())
     text = d->textEdit->textCursor().selectedText();
  else
      text = d->textEdit->toPlainText();

  int count = 0;
  Qt::CaseSensitivity cs = Qt::CaseInsensitive;

  if (menu_find_case->isChecked())
     cs = Qt::CaseSensitive;

  if (use_regexp)
     count = text.count (QRegExp (fif_get_text()));
  else
      count = text.count (fif_get_text(), cs);

  log->log (tr ("%1 number of occurrences of %2 is found").arg (count).arg (fif->text()));
}


void rvln::fn_count()
{
  last_action = qobject_cast<QAction *>(sender());

  count_substring (false);
}


void rvln::fn_count_rx()
{
  last_action = qobject_cast<QAction *>(sender());

  count_substring (true);
}


void rvln::read_search_options()
{
  menu_find_whole_words->setChecked (settings->value ("find_whole_words", "0").toBool());
  menu_find_case->setChecked (settings->value ("find_case", "0").toBool());
  menu_find_regexp->setChecked (settings->value ("find_regexp", "0").toBool());
  menu_find_fuzzy->setChecked (settings->value ("find_fuzzy", "0").toBool());
  menu_find_from_cursor->setChecked (settings->value ("find_from_cursor", "1").toBool());
}


void rvln::write_search_options()
{
  settings->setValue ("find_whole_words", menu_find_whole_words->isChecked());
  settings->setValue ("find_case", menu_find_case->isChecked());
  settings->setValue ("find_regexp", menu_find_regexp->isChecked());
  settings->setValue ("find_fuzzy", menu_find_fuzzy->isChecked());
  settings->setValue ("find_from_cursor", menu_find_from_cursor->isChecked());
}


void rvln::main_tab_page_changed (int index)
{
  if (idx_prev == idx_tab_fman)
     if (img_viewer && img_viewer->window_mini.isVisible())
         img_viewer->window_mini.close();

  if (idx_prev == idx_tab_tune)
      leaving_tune();

  idx_prev = index;
      
  if (index == idx_tab_fman)
     {
      fman->setFocus();
      fm_entry_mode = FM_ENTRY_MODE_NONE;
      idx_tab_fman_activate();
     }
  else       
  if (index == idx_tab_calendar)
    {
     calendar_update();
     idx_tab_calendar_activate();
    }
  else
  if (index == idx_tab_edit)
     idx_tab_edit_activate();
  else   
  if (index == idx_tab_tune)
     idx_tab_tune_activate();
  else   
  if (index == idx_tab_learn)
     idx_tab_learn_activate();
}


QString rvln::fif_get_text()
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


void rvln::fn_rm_trailing_spaces()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QStringList sl = d->textEdit->textCursor().selectedText().split (QChar::ParagraphSeparator);

  QMutableListIterator <QString> i (sl);

  while (i.hasNext())
        {
         QString s = i.next();

         if (s.isEmpty())
            continue;

         if (s.at (s.size() - 1).isSpace())
            {
             int index = s.size() - 1;
             while (s.at (--index).isSpace())
                   ;

             s.truncate (index + 1);
             i.setValue (s);
            }
        }

  QString x = sl.join ("\n"); 
  
  d->textEdit->textCursor().insertText (x);
}


void rvln::fman_convert_images (bool by_side, int value)
{
  qsrand (QTime::currentTime().msec());

  QString dir_out ("images-out-");
  dir_out.append (QString::number (qrand() % 777));

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

  foreach (QString fname, li)
          if (is_image (fname))
             {
              QImage source (fname);

              if (! source.isNull())
                 {
                  qApp->processEvents();
                  
                 if (settings->value ("cb_exif_rotate", 0).toInt()) 
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

                  
                  QImage dest = image_scale_by (source, by_side, value, transformMode);


                  QString fmt (settings->value ("output_image_fmt", "jpg").toString());

                  QFileInfo fi (fname);
                  QString dest_fname (dir_out);
                  dest_fname.append ("/");
                  dest_fname.append (fi.fileName());
                  dest_fname = change_file_ext (dest_fname, fmt);

                  if (! dest.save (dest_fname, fmt.toLatin1().constData(), quality))
                      qDebug() << "Cannot save " << dest_fname;

                  pb_status->setValue (i++);
                 }
             }

  pb_status->hide();

  if (settings->value ("img_post_proc", 0).toBool())
     {
      CZipper zipper;
      zipper.zip_directory (fman->dir.absolutePath(), dir_out);
     }

  fman->refresh();
}


void rvln::fman_img_conv_by_side()
{
  last_action = qobject_cast<QAction *>(sender());

  int side = fif_get_text().toInt();
  if (side == 0)
     return;

  fman_convert_images (true, side);
}


void rvln::fman_img_conv_by_percent()
{
  last_action = qobject_cast<QAction *>(sender());

  int percent = fif_get_text().toInt();
  if (percent == 0)
     return;

  fman_convert_images (false, percent);
}


void rvln::fn_escape()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  d->textEdit->textCursor().insertText (
     QRegExp::escape (d->textEdit->textCursor().selectedText()));
}


void rvln::fman_add_to_zip()
{
  last_action = qobject_cast<QAction *>(sender());

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
      QString fname (fman->dir.path());
      fname.append ("/").append (f);
      fman->zipper.files_list.append (fname);
      return;
     }

  foreach (QString fname, li)
          fman->zipper.files_list.append (fname);
}


void rvln::fman_create_zip()
{
  last_action = qobject_cast<QAction *>(sender());

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


void rvln::fman_save_zip()
{
  fman->zipper.pack_prepared();
  fman->refresh();
}


void rvln::fman_preview_image()
{
  last_action = qobject_cast<QAction *>(sender());

  QString fname = fman->get_sel_fname();
  if (fname.isEmpty())
     return;
  
  if (is_image (fname))
     {
      if (file_get_ext (fname) == "gif")
              {
              qDebug() << "aaaa";
               CGIFWindow *w = new CGIFWindow;
               w->load_image (fname);
               return; 
              }
     
      img_viewer->window_full.show();
      img_viewer->set_image_full (fname);
     }
}


void rvln::ed_indent()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  d->textEdit->indent();
}  


void rvln::ed_unindent()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  d->textEdit->un_indent();
}  


void rvln::fn_sort_casecareless()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;
  
  d->textEdit->textCursor().insertText (qstringlist_process (
                                        d->textEdit->textCursor().selectedText(),
                                        fif_get_text(), 
                                        QSTRL_PROC_FLT_WITH_SORTNOCASECARE));
}


void rvln::fman_fname_entry_confirm()
{
  if (fm_entry_mode == FM_ENTRY_MODE_OPEN)
     fman_open();     
   
  if (fm_entry_mode == FM_ENTRY_MODE_SAVE)
     cb_button_saves_as();
}


void rvln::update_view_hls()
{
  menu_view_hl->clear();
  
  QStringList l = documents->hls.uniqueKeys();
  l.sort();

  create_menu_from_list (this, menu_view_hl,
                         l,
                         SLOT (file_use_hl()));
}


void rvln::file_use_hl()
{
  last_action = qobject_cast<QAction *>(sender());

  QAction *a = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;
  
  d->set_hl (false, a->text());
}


void rvln::fn_strip_html_tags()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QString text;

  if (d->textEdit->textCursor().hasSelection())
     text = d->textEdit->textCursor().selectedText();
  else
      text = d->textEdit->toPlainText();

  if (d->textEdit->textCursor().hasSelection())
     d->textEdit->textCursor().insertText (strip_html (text));
  else
      d->textEdit->setPlainText (strip_html (text));
}


void rvln::fn_number_decimal_to_binary()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (d)
     d->textEdit->textCursor().insertText (int_to_binary (d->textEdit->textCursor().selectedText().toInt()));
}


void rvln::fn_number_flip_bits()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (!d)
     return;
  
  QString s = d->textEdit->textCursor().selectedText();
  for (int i = 0; i < s.size(); i++)
      {
       if (s[i] == '1')
          s[i] = '0';
       else          
       if (s[i] == '0')
          s[i] = '1';
      }   
  
  d->textEdit->textCursor().insertText (s);
}


void rvln::fn_use_table()
{
  last_action = qobject_cast<QAction *>(sender());

  QAction *a = qobject_cast<QAction *>(sender());

  if (main_tab_widget->currentIndex() == idx_tab_edit)
    {
     CDocument *d = documents->get_current();
     if (! d)
        return;
  
     QString text;

     if (d->textEdit->textCursor().hasSelection())
         text = d->textEdit->textCursor().selectedText();
     else
         text = d->textEdit->toPlainText();
  
     if (d->textEdit->textCursor().hasSelection())
        d->textEdit->textCursor().insertText (apply_table (text, a->data().toString(), menu_find_regexp->isChecked()));
     else
         d->textEdit->setPlainText (apply_table (text, a->data().toString(), menu_find_regexp->isChecked()));
     }
 else
     if (main_tab_widget->currentIndex() == idx_tab_fman)
        {
         QStringList sl = fman->get_sel_fnames();

         if (sl.size() < 1)
            return;

         char *charset = cb_fman_codecs->currentText().toLatin1().data();

         foreach (QString fname, sl)
                 {
                  QString f = qstring_load (fname, charset);
                  QString r = apply_table (f, a->data().toString(), menu_find_regexp->isChecked());
                  qstring_save (fname, r, charset);
                  log->log (tr ("%1 is processed and saved").arg (fname));
                 }
        }
}


void rvln::update_tables()
{
  menu_fn_tables->clear();

  create_menu_from_dir (this,
                        menu_fn_tables,
                        dir_tables,
                        SLOT (fn_use_table())
                        );
}


void rvln::fn_binary_to_decimal()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (d)
     d->textEdit->textCursor().insertText (QString::number (bin_to_decimal (d->textEdit->textCursor().selectedText())));
}


#ifdef SPELLCHECK_ENABLE

void rvln::cmb_spellchecker_currentIndexChanged (const QString &text)
{
  cur_spellchecker = text;


  qDebug() << "cur_spellchecker = " << cur_spellchecker;

  settings->setValue ("cur_spellchecker", cur_spellchecker);

  delete spellchecker;

  if (! spellcheckers.contains (cur_spellchecker))
     cur_spellchecker = spellcheckers[0]; 

  
#ifdef ASPELL_ENABLE
  if (cur_spellchecker == "Aspell")
     spellchecker = new CSpellchecker (settings->value ("spell_lang", QLocale::system().name().left(2)).toString());
#endif

  
#ifdef HUNSPELL_ENABLE
   if (cur_spellchecker == "Hunspell")
      spellchecker = new CHunspellChecker (settings->value ("spell_lang", QLocale::system().name().left(2)).toString(), "/home/rox/devel/test/hunspell");  
#endif
  
 create_spellcheck_menu();
}

#ifdef HUNSPELL_ENABLE

void rvln::pb_choose_hunspell_path_clicked()
{
  QString path = QFileDialog::getExistingDirectory (this, tr ("Open Directory"), "/",
                                                    QFileDialog::ShowDirsOnly |
                                                QFileDialog::DontResolveSymlinks);
  if (! path.isEmpty())
  {
   settings->setValue ("hunspell_dic_path", path);
   ed_spellcheck_path->setText (path);

   if (spellchecker)
      delete spellchecker;

   setup_spellcheckers();
   create_spellcheck_menu();
  }
}

#endif


#ifdef ASPELL_ENABLE
#if defined(Q_OS_WIN) || defined(Q_OS_OS2)


void rvln::pb_choose_aspell_path_clicked()
{
  QString path = QFileDialog::getExistingDirectory (this, tr ("Open Directory"), "/",
                                                    QFileDialog::ShowDirsOnly |
                                                    QFileDialog::DontResolveSymlinks);
  if (! path.isEmpty())
    {
     settings->setValue ("win32_aspell_path", path);
     ed_aspellcheck_path->setText (path);

     if (spellchecker)
        delete spellchecker;
     
     setup_spellcheckers();
     create_spellcheck_menu();
    }

}
#endif
#endif

#endif


void rvln::file_find_obsolete_paths()
{
  QStringList l_bookmarks = qstring_load (fname_bookmarks).split ("\n");

  for (int i = 0; i < l_bookmarks.size(); i++)
      {
       QStringList t = l_bookmarks[i].split (",");
       if (! file_exists (t[0]))
          l_bookmarks[i] = "#" + l_bookmarks[i];
      }

  bookmarks = l_bookmarks.join ("\n").trimmed();

  qstring_save (fname_bookmarks, bookmarks);
  update_bookmarks();
}


void rvln::fn_filter_delete_by_sep (bool mode)
{
  CDocument *d = documents->get_current();
  if (! d)
     return;

  QStringList sl = d->textEdit->textCursor().selectedText().split (QChar::ParagraphSeparator);

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

  d->textEdit->textCursor().insertText (x);
}


void rvln::fn_filter_delete_before_sep()
{
  last_action = qobject_cast<QAction *>(sender());

  fn_filter_delete_by_sep (true);
}


void rvln::fn_filter_delete_after_sep()
{
  last_action = qobject_cast<QAction *>(sender());

  fn_filter_delete_by_sep (false);
}


void rvln::fman_mk_gallery()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  if (! file_exists (d->file_name))
     return;

  int side = settings->value ("ed_side_size", 110).toInt();
  int thumbs_per_row = settings->value ("ed_thumbs_per_row", 4).toInt();;
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
     d->textEdit->textCursor().insertText (table);
}


void rvln::indent_by_first_line()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QStringList sl = d->textEdit->textCursor().selectedText().split (QChar::ParagraphSeparator);
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

  d->textEdit->textCursor().insertText (t);
}


void rvln::search_in_files_results_dclicked (QListWidgetItem *item)
{
  documents->open_file_triplex (item->text());
  main_tab_widget->setCurrentIndex (idx_tab_edit); 
}


void rvln::search_in_files()
{
  last_action = qobject_cast<QAction *>(sender());

  QStringList lresult;
  QString charset = cb_fman_codecs->currentText();
  QString path = fman->dir.path();
  QString text_to_search = fif_get_text();

  CFTypeChecker fc (":/text-data/cm-tf-names", ":/text-data/cm-tf-exts");
  QStringList l = documents->hls.uniqueKeys();
  fc.lexts.append (l);

  log->log (tr ("Getting files list..."));
  qApp->processEvents();

  CFilesList lf;
  lf.get (path);

  log->log (tr ("Searching..."));
  qApp->processEvents();

  pb_status->show();
  pb_status->setRange (0, lf.list.size());
  pb_status->setFormat (tr ("%p% completed"));
  pb_status->setTextVisible (true);

  for (int i = 0; i < lf.list.size(); i++)
      {
       if (i % 100 == 0)
           qApp->processEvents();

       pb_status->setValue (i); 

       QString fileName = lf.list[i];
       if (! fc.check (fileName))
          continue;

       CTio *tio = documents->tio_handler.get_for_fname (fileName);
       tio->charset = charset;

       if (! tio->load (fileName))
           log->log (tr ("cannot open %1 because of: %2")
                         .arg (fileName)
                         .arg (tio->error_string));

       Qt::CaseSensitivity cs = Qt::CaseInsensitive;
       if (menu_find_case->isChecked())
          cs = Qt::CaseSensitive; 

       int index = tio->data.indexOf (text_to_search, 0, cs);
       if (index != -1)
          lresult.append (fileName + "," + charset + "," + QString::number (index)); 
      }


  pb_status->hide();

  CTextListWnd *w = new CTextListWnd (tr ("Search results"), tr ("Files"));

  w->list->addItems (lresult);
     
  connect (w->list, SIGNAL(itemDoubleClicked ( QListWidgetItem *)), 
           this, SLOT(search_in_files_results_dclicked ( QListWidgetItem *)));

  w->resize (width() - 10, (int) height() / 2);
  w->show();
}


void rvln::view_use_profile()
{
  last_action = qobject_cast<QAction *>(sender());

  QAction *a = qobject_cast<QAction *>(sender());
  QSettings s (a->data().toString(), QSettings::IniFormat);

  QPoint pos = s.value ("pos", QPoint (1, 200)).toPoint();
  QSize size = s.value ("size", QSize (600, 420)).toSize();

  mainSplitter->restoreState (s.value ("splitterSizes").toByteArray());
  resize (size);
  move (pos);

  settings->setValue ("word_wrap", s.value ("word_wrap", "2").toInt());
  settings->setValue ("show_linenums", s.value ("show_linenums", "0").toInt());
  settings->setValue ("additional_hl", s.value ("additional_hl", "0").toInt()); 
  settings->setValue ("show_margin", s.value ("show_margin", "0").toInt()); 

  cb_wordwrap->setCheckState (Qt::CheckState (settings->value ("word_wrap", "2").toInt()));
  cb_show_linenums->setCheckState (Qt::CheckState (settings->value ("show_linenums", "0").toInt()));
  cb_hl_current_line->setCheckState (Qt::CheckState (settings->value ("additional_hl", "0").toInt()));
  cb_show_margin->setCheckState (Qt::CheckState (settings->value ("show_margin", "0").toInt()));

  settings->setValue ("editor_font_name", s.value ("editor_font_name", "Monospace").toString());
  settings->setValue ("editor_font_size", s.value ("editor_font_size", "12").toInt());

  documents->apply_settings();  
}


void rvln::profile_save_as()
{
  last_action = qobject_cast<QAction *>(sender());

  bool ok;
  QString name = QInputDialog::getText (this, tr ("Enter the name"),
                                              tr ("Name:"), QLineEdit::Normal,
                                              tr ("new_profile"), &ok);
  if (! ok || name.isEmpty())
     return;

  QString fname (dir_profiles);
  fname.append ("/").append (name);

  QSettings s (fname, QSettings::IniFormat);

  s.setValue ("word_wrap", settings->value ("word_wrap", "2").toInt());
  s.setValue ("show_linenums", settings->value ("show_linenums", "0").toInt());
  s.setValue ("additional_hl", settings->value ("additional_hl", "0").toInt()); 

  s.setValue ("pos", pos());
  s.setValue ("size", size());
  s.setValue ("splitterSizes", mainSplitter->saveState());

  s.setValue ("editor_font_name", settings->value ("editor_font_name", "Monospace").toString());
  s.setValue ("editor_font_size", settings->value ("editor_font_size", "12").toInt());

  s.sync();
  
  update_profiles();
  shortcuts->load_from_file (shortcuts->fname);
}


void rvln::update_profiles()
{
  menu_view_profiles->clear();
  create_menu_from_dir (this,
                        menu_view_profiles,
                        dir_profiles,
                        SLOT (view_use_profile())
                       );
}


void rvln::fman_items_select_by_regexp (bool mode)
{
  QString ft = fif_get_text();
  if (ft.isEmpty()) 
      return;  
     
  l_fman_find = fman->mymodel->findItems (ft, Qt::MatchRegExp);

  if (l_fman_find.size() < 1)
     return; 
   
  QItemSelectionModel *m = fman->selectionModel();
  for (int i = 0; i < l_fman_find.size(); i++)
      if (mode) 
         m->select (fman->mymodel->indexFromItem (l_fman_find[i]), QItemSelectionModel::Select | QItemSelectionModel::Rows);
      else   
          m->select (fman->mymodel->indexFromItem (l_fman_find[i]), QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
}


void rvln::fman_select_by_regexp()
{
  last_action = qobject_cast<QAction *>(sender());

  fman_items_select_by_regexp (true);
}


void rvln::fman_deselect_by_regexp()
{
  last_action = qobject_cast<QAction *>(sender());

  fman_items_select_by_regexp (false);
}


void rvln::fman_count_lines_in_selected_files()
{
  last_action = qobject_cast<QAction *>(sender());

  QString ft = fif_get_text();
  if (ft.isEmpty()) 
      return;  
          
  QStringList sl = fman->get_sel_fnames();   
  
  if (sl.size() < 1)
     return; 
  
  long int sum = 0;
  
  foreach (QString fname, sl)
          {
           QByteArray f = file_load (fname);
           sum += f.count ('\n');
          }  
          
  log->log (tr ("There are %1 lines at %2 files").arg (sum).arg (sl.size()));        
}


void rvln::set_eol_unix()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (!d)
     return;

  d->eol = "\n";
}


void rvln::set_eol_win()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (!d)
     return;

  d->eol = "\r\n";
}


void rvln::set_eol_mac()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (!d)
     return;

  d->eol = "\r";
}


void rvln::guess_enc()
{
 CCharsetMagic cm;

 QString fn = fman->get_sel_fname();
 QString enc = cm.guess_for_file (fn); 
 cb_fman_codecs->setCurrentIndex (cb_fman_codecs->findText (enc));
}


void rvln::ed_comment()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;
  
  if (! d->highlighter)
     return;
    
  if (d->highlighter->cm_mult.isEmpty() && d->highlighter->cm_single.isEmpty())
     return;    
     
  QString t = d->get_selected_text();   
  QString result;   
    
  bool is_multiline = true;

  int sep_pos = t.indexOf (QChar::ParagraphSeparator);
  if (sep_pos == -1 || sep_pos == t.size() - 1)
     is_multiline = false; 
   
  if (is_multiline)
      result = d->highlighter->cm_mult;
  else
      result = d->highlighter->cm_single;
 
 
  if (is_multiline && result.isEmpty())
     {
      QStringList sl = t.split (QChar::ParagraphSeparator);
      for (int i = 0; i < sl.size(); i++)
          {
           QString x = d->highlighter->cm_single;
           sl[i] = x.replace ("%s", sl[i]);
          }

      QString z = sl.join("\n");
      d->textEdit->textCursor().insertText (z);
          
      return;
     }
 
  d->textEdit->textCursor().insertText (result.replace ("%s", t));
}


void rvln::calendar_clicked (const QDate &date)
{
/*  qDebug() << date.toString ("yyyy-MM-dd");
    
//  QDate d = QDate::currentDate();

  qDebug() << "lunar day by moon_phase_trig2: " << moon_phase_trig2 (date.year(), date.month(), date.day());
  qDebug() << "lunar day by moon_phase_trig1: " << moon_phase_trig1 (date.year(), date.month(), date.day());
  qDebug() << "lunar day by moon_phase_simple: " << moon_phase_simple (date.year(), date.month(), date.day());
  qDebug() << "lunar day by moon_phase_conway: " << moon_phase_conway (date.year(), date.month(), date.day());
  qDebug() << "lunar day by moon_phase_leueshkanov : " << moon_phase_leueshkanov (date.year(), date.month(), date.day());
  */

  QString fname = dir_days + "/" + date.toString ("yyyy-MM-dd");

  //bool fresh = false;

  if (file_exists (fname))
    {
     QString s = qstring_load (fname);
     log->log (s);
    }
    
 // int jd = date.toJulianDay(); 
  //qDebug() << "day: " << jd;
  
}


void rvln::calendar_activated (const QDate &date)
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
  
  if (fresh)
     d->textEdit->selectAll();
  
  main_tab_widget->setCurrentIndex (idx_tab_edit); 
}


void rvln::createCalendar()
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
  
  connect (calendar, SIGNAL(clicked (const QDate &)), this, SLOT(calendar_clicked (const QDate &)));
  connect (calendar, SIGNAL(activated (const QDate &)), this, SLOT(calendar_activated (const QDate &)));
  connect (calendar, SIGNAL(currentPageChanged (int, int)), this, SLOT(calendar_currentPageChanged (int, int)));

  idx_tab_calendar = main_tab_widget->addTab (calendar, tr ("dates"));
}


void rvln::calendar_currentPageChanged (int year, int month)
{
  calendar_update();
} 


void rvln::calendar_update()
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
    
  int days_count = dbase.daysInMonth();

  for (int day = 1; day <= days_count; day++)
      {  
       QDate date (year, month, day);
       QString sdate;
       sdate = sdate.sprintf ("%02d-%02d-%02d", year, month, day);
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

  
void rvln::idx_tab_edit_activate()
{
  fileMenu->menuAction()->setVisible (true);
  editMenu->menuAction()->setVisible (true);
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


void rvln::idx_tab_calendar_activate()
{
  fileMenu->menuAction()->setVisible (true);
  editMenu->menuAction()->setVisible (false);
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


void rvln::idx_tab_tune_activate()
{
  opt_update_keyb();

  fileMenu->menuAction()->setVisible (true);
  editMenu->menuAction()->setVisible (false);
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


void rvln::idx_tab_fman_activate()
{
  fileMenu->menuAction()->setVisible (true);
  editMenu->menuAction()->setVisible (false);
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


void rvln::idx_tab_learn_activate()
{
  fileMenu->menuAction()->setVisible (true);
  editMenu->menuAction()->setVisible (false);
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


void rvln::cal_add_days()
{
  QDate selected = calendar->selectedDate();
  selected = selected.addDays (fif_get_text().toInt());
  calendar->setSelectedDate (selected);
}


void rvln::cal_add_months()
{
  QDate selected = calendar->selectedDate();
  selected = selected.addMonths (fif_get_text().toInt());
  calendar->setSelectedDate (selected);
}


void rvln::cal_add_years()
{
  QDate selected = calendar->selectedDate();
  selected = selected.addYears (fif_get_text().toInt());
  calendar->setSelectedDate (selected);
}


void rvln::cal_set_date_a()
{
  date1 = calendar->selectedDate();
}


void rvln::cal_set_date_b()
{
  date2 = calendar->selectedDate();
}


void rvln::cal_diff_days()
{
  int days = date2.daysTo (date1);
  if (days < 0)
      days = ~ days;

  log->log (QString::number (days));
}


void rvln::cal_remove()
{
  QString fname = dir_days + "/" + calendar->selectedDate().toString ("yyyy-MM-dd");
  QFile::remove (fname);
  calendar_update();
}


void rvln::rename_selected()
{
  CDocument *d = documents->get_current();
  if (! d)
     return;

  if (!d->textEdit->textCursor().hasSelection())
     {
      log->log (tr ("Select the file name first!"));
      return;
     }

  QString fname = d->get_filename_at_cursor();

  if (fname.isEmpty())
     return;

  QString newname = fif_get_text();
  if (newname.isEmpty())
     return;

  QFileInfo fi (fname);
  if (! fi.exists() && ! fi.isWritable())
     return;

  QString newfpath (fi.path());
  newfpath.append ("/").append (newname);
  QFile::rename (fname, newfpath);
  update_dyn_menus();
  fman->refresh();

  QDir dir (d->file_name);
  QString new_name = dir.relativeFilePath (newfpath);

  if (new_name.startsWith (".."))
     new_name = new_name.remove (0, 1);

  if (d->textEdit->textCursor().selectedText().startsWith ("./") && ! new_name.startsWith ("./"))
     new_name = "./" + new_name;

  if (! d->textEdit->textCursor().selectedText().startsWith ("./") && new_name.startsWith ("./"))
     new_name = new_name.remove (0, 2);

  if (d->textEdit->textCursor().hasSelection())
     d->set_selected_text (new_name.trimmed());
}


void rvln::update_labels_menu()
{
  menu_labels->clear();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  create_menu_from_list (this, menu_labels, d->labels, SLOT(select_label()));
}


void rvln::update_labels_list()
{
  CDocument *d = documents->get_current();
  if (! d)
     return;

  d->update_labels();
  update_labels_menu();
}


void rvln::select_label()
{
  last_action = qobject_cast<QAction *>(sender());

  QAction *Act = qobject_cast<QAction *>(sender());
  //qDebug() << Act->text();

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QTextCursor cr;

  QString text_to_find = settings->value ("label_start", "[?").toString()
                         + Act->text()
                         + settings->value ("label_end", "?]").toString();

  qDebug() << text_to_find;

  cr = d->textEdit->document()->find (text_to_find);

  if (! cr.isNull())
     d->textEdit->setTextCursor (cr);
}

/*
void rvln::add_user_font()
{
  QString fontfname = QFileDialog::getOpenFileName (this,
                        tr ("Select font"), "",
                        tr("Fonts (*.ttf *.otf)"));


  QStringList sl;

  if (file_exists (fname_userfonts))
     sl = qstring_load (fname_userfonts).split ("\n");

  sl.append (fontfname);
  qstring_save (fname_userfonts, sl.join("\n").trimmed());
  //load_userfonts();
}
*/

/*
void rvln::load_userfonts()
{
  font_database->removeAllApplicationFonts();

  userfont_ids.clear();

  if (! file_exists (fname_userfonts))
     return;

  QStringList sl = qstring_load (fname_userfonts).split ("\n");

  for (int i = 0; i < sl.count(); i++)
      {
       qDebug() << "FONT: " << sl[i];
       if (file_exists (sl[i]))
          userfont_ids.append (font_database->addApplicationFont (sl[i]));
      }
}
*/

void rvln::fn_insert_cpp()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (d)
     d->textEdit->textCursor().insertText (qstring_load (":/text-data/tpl_cpp.cpp"));
}


void rvln::fn_insert_c()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (d)
     d->textEdit->textCursor().insertText (qstring_load (":/text-data/tpl_c.c"));
}


inline bool CFSizeFNameLessThan (CFSizeFName *v1, CFSizeFName *v2)
{
  return v1->size > v2->size;
}


void rvln::mrkup_document_weight()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QString result;
  QStringList l = html_get_by_patt (d->textEdit->toPlainText(), "src=\"");

  QFileInfo f (d->file_name);
  QUrl baseUrl (d->file_name);

//  result += tr ("%1 %2 kbytes<br>").arg (d->file_name).arg (QString::number (f.size() / 1024));

  QList <CFSizeFName*> lst;
  lst.append (new CFSizeFName (f.size(), d->file_name));

  int size_total = 0;
  int files_total = 1;

  foreach (QString url, l)
          {
           QUrl relativeUrl (url);
           QString resolved = baseUrl.resolved (relativeUrl).toString();
           QFileInfo info (resolved);
           if (! info.exists())
              lst.append (new CFSizeFName (info.size(), tr ("%1 is not found<br>").arg (resolved)));
           else
               {
                lst.append (new CFSizeFName (info.size(), resolved));
                size_total += info.size();
                ++files_total;
               }
           }

  qSort (lst.begin(), lst.end(), CFSizeFNameLessThan);


  for (int i = 0; i < lst.size(); i++)
     {
      result += tr ("%1 kbytes %2 <br>").arg (QString::number (lst[i]->size / 1024)).arg (lst[i]->fname);
      delete lst[i];
     }


  result.prepend (tr ("Total size = %1 kbytes in %2 files<br>").arg (QString::number (size_total / 1024))
                                                               .arg (QString::number (files_total)));

  log->log (result);
}

/*
void rvln::fman_unpack_zip()
{
  QString fn = fman->get_sel_fname();
  if (fn.isEmpty())
     return;

  CZipper z;

  z.unzip (fman->get_sel_fname(), fman->dir.path());
}
*/

void rvln::fman_unpack_zip()
{
  last_action = qobject_cast<QAction *>(sender());

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

  foreach (QString fname, li)
          {
           z.unzip (fname, fman->dir.path());
           log->log (fname + tr (" is unpacked"));

          }

}

void rvln::fman_zip_info()
{
  last_action = qobject_cast<QAction *>(sender());

  QString fn = fman->get_sel_fname();
  if (fn.isEmpty())
     return;

  CZipper z;

  //QString s = z.unzip_info (fman->get_sel_fname());
  
  QStringList sl = z.unzip_list (fman->get_sel_fname());
  
  for (int i = 0; i < sl.size(); i++)
      sl[i] = sl[i].append ("<br>");
    
  log->log (sl.join("\n"));
}


void rvln::cmb_tea_icons_currentIndexChanged (const QString &text)
{
  settings->setValue ("icon_fname", text);

  QString icon_fname = ":/icons/tea-icon-v3-0" + text + ".png";
  
  qApp->setWindowIcon (QIcon (icon_fname));
}


void rvln::cmb_icon_sizes_currentIndexChanged (const QString &text)
{
  settings->setValue ("icon_size", text);
  
  icon_size = settings->value ("icon_size", "32").toInt();

  setIconSize (QSize (text.toInt(), text.toInt()));
  tb_fman_dir->setIconSize (QSize (text.toInt(), text.toInt()));
  filesToolBar->setIconSize (QSize (text.toInt(), text.toInt()));
}


void rvln::cmb_ui_tabs_currentIndexChanged (int i)
{
  main_tab_widget->setTabPosition (int_to_tabpos (i));
  settings->setValue ("ui_tabs_align", i);
}


void rvln::cmb_docs_tabs_currentIndexChanged (int i)
{
  tab_widget->setTabPosition (int_to_tabpos (i));
  
  settings->setValue ("docs_tabs_align", i);

  qDebug() << "cmb_docs_tabs_currentIndexChanged " << i;
}


void rvln::cal_set_to_current()
{
  calendar->showToday();
}


void rvln::clipboard_dataChanged()
{
  if (! capture_to_storage_file)
     return;

  CDocument *ddest = documents->get_document_by_fname (fname_storage_file);
  if (ddest)
     {
      QString t = QApplication::clipboard()->text();
      
      QString tpl = "%s\n";
      
      QString ftemplate = dir_config.append ("/cliptpl.txt");
      if (file_exists (ftemplate))
         tpl = qstring_load (ftemplate);
         
//      qDebug() << tpl;   
    
      tpl = tpl.replace ("%time", QTime::currentTime().toString (settings->value("time_format", "hh:mm:ss").toString()));
      tpl = tpl.replace ("%date", QDate::currentDate().toString (settings->value("date_format", "dd/MM/yyyy").toString()));
      
      QString text_to_insert = tpl.replace ("%s", t);
      
      ddest->textEdit->textCursor().insertText (text_to_insert);
      //ddest->textEdit->textCursor().insertText ("\n");
     }
}


void rvln::fn_remove_by_regexp()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QString t = d->textEdit->textCursor().selectedText();
  t.remove (QRegExp (fif_get_text()));
  d->textEdit->textCursor().insertText (t);
}


void rvln::capture_clipboard_to_storage_file()
{
  last_action = qobject_cast<QAction *>(sender());

  capture_to_storage_file = ! capture_to_storage_file;
}


void rvln::set_as_storage_file()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  fname_storage_file = d->file_name;
}


void rvln::copy_to_storage_file()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *dsource = documents->get_current();
  if (! dsource)
     return;

  CDocument *ddest = documents->get_document_by_fname (fname_storage_file);
  if (ddest)
     {
      QString t = dsource->textEdit->textCursor().selectedText();
      ddest->textEdit->textCursor().insertText (t);
      ddest->textEdit->textCursor().insertText ("\n");
     }
}


void rvln::cal_moon_mode()
{
  calendar->moon_mode = ! calendar->moon_mode;
  calendar->do_update();

  settings->setValue ("moon_mode", calendar->moon_mode);
}


void rvln::create_moon_phase_algos()
{
  moon_phase_algos.insert (MOON_PHASE_TRIG2, tr ("Trigonometric 2"));
  moon_phase_algos.insert (MOON_PHASE_TRIG1, tr ("Trigonometric 1"));
  moon_phase_algos.insert (MOON_PHASE_CONWAY, tr ("Conway"));
//  moon_phase_algos.insert (MOON_PHASE_SIMPLE, tr ("Simple1"));
  moon_phase_algos.insert (MOON_PHASE_LEUESHKANOV, tr ("Leueshkanov"));
}


void rvln::cal_gen_mooncal()
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
  nd->textEdit->textCursor().insertText (s);
  main_tab_widget->setCurrentIndex (idx_tab_edit);
}


void rvln::leaving_tune()
{
//  qDebug() << "Leaving Tune";

  settings->setValue ("date_format", ed_date_format->text());
  settings->setValue ("time_format", ed_time_format->text());

  settings->setValue ("override_locale_val", ed_locale_override->text());

  settings->setValue ("img_viewer_override_command", ed_img_viewer_override->text());


  settings->setValue ("wasd", cb_wasd->checkState());
  settings->setValue ("full_path_at_window_title", cb_full_path_at_window_title->checkState());
  
  
  settings->setValue ("word_wrap", cb_wordwrap->checkState());
  //settings->setValue ("right_to_left", cb_right_to_left->checkState());
  
#if QT_VERSION >= 0x050000
    
  settings->setValue ("qregexpsyntaxhl", cb_use_qregexpsyntaxhl->checkState());

#endif  
  
  settings->setValue ("additional_hl", cb_hl_current_line->checkState());

  settings->setValue ("session_restore", cb_session_restore->checkState());

  settings->setValue ("show_linenums", cb_show_linenums->checkState());
  settings->setValue ("use_hl_wrap", cb_use_hl_wrap->checkState());
  settings->setValue ("hl_brackets", cb_hl_brackets->checkState());
  settings->setValue ("auto_indent", cb_auto_indent->checkState());


  settings->setValue ("spaces_instead_of_tabs", cb_spaces_instead_of_tabs->checkState());
  settings->setValue ("cursor_xy_visible", cb_cursor_xy_visible->checkState());
  settings->setValue ("tab_sp_width", spb_tab_sp_width->value());

  settings->setValue ("center_on_scroll", cb_center_on_cursor->checkState());
  settings->setValue ("show_margin", cb_show_margin->checkState());
  settings->setValue ("margin_pos", spb_margin_pos->value());
  settings->setValue ("b_preview", cb_auto_img_preview->checkState());

  settings->setValue ("cursor_blink_time", spb_cursor_blink_time->value());
  
  cursor_blink_time = spb_cursor_blink_time->value();
  
  qApp->setCursorFlashTime (spb_cursor_blink_time->value());
 
  settings->setValue ("cursor_width", spb_cursor_width->value());
 
  settings->setValue ("override_img_viewer", cb_override_img_viewer->checkState());
  
  settings->setValue ("override_locale", cb_override_locale->checkState());
  settings->setValue ("use_trad_dialogs", cb_use_trad_dialogs->checkState());
  settings->setValue ("start_week_on_sunday", cb_start_on_sunday->checkState());

  settings->setValue ("northern_hemisphere", cb_northern_hemisphere->checkState());

  calendar->northern_hemisphere = bool (cb_northern_hemisphere->checkState());

  int i = moon_phase_algos.key (cmb_moon_phase_algos->currentText());
  settings->setValue ("moon_phase_algo", i);
  calendar->moon_phase_algo = i;

  settings->setValue ("zip_charset_in", cmb_zip_charset_in->currentText());
  settings->setValue ("zip_charset_out", cmb_zip_charset_out->currentText());
  settings->setValue ("cmdline_default_charset", cmb_cmdline_default_charset->currentText());
  
  
  settings->setValue ("label_end", ed_label_end->text());
  settings->setValue ("label_start", ed_label_start->text());

  settings->setValue ("output_image_fmt", cmb_output_image_fmt->currentText());

  settings->setValue ("img_filter", cb_output_image_flt->checkState());


  settings->setValue("fuzzy_q", spb_fuzzy_q->value());


  settings->setValue("img_quality", spb_img_quality->value());
  settings->setValue ("img_post_proc", cb_zip_after_scale->checkState());

  settings->setValue ("cb_exif_rotate", cb_exif_rotate->checkState());

  settings->setValue ("zor_use_exif_orientation", cb_zor_use_exif->checkState());

  settings->setValue ("ed_side_size", ed_side_size->text());

  settings->setValue ("ed_link_options", ed_link_options->text());

  settings->setValue ("ed_cols_per_row", ed_cols_per_row->text());

  calendar->do_update();
  documents->apply_settings();
}



QAction* rvln::add_to_menu (QMenu *menu,
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




void rvln::cmb_ui_langs_currentIndexChanged (const QString &text)
{
 qDebug() << "void rvln::cmb_ui_langs_currentIndexChanged (const QString &text)";

//  settings->setValue ("ui_lang", text);
  if (text == "en")
     {
      qApp->removeTranslator (&myappTranslator);
      qApp->removeTranslator (&qtTranslator);
     }
  else
      {

      qApp->removeTranslator (&myappTranslator);
      qApp->removeTranslator (&qtTranslator);

      QString ts = text;
      if (ts.length() != 2)
         ts = "en";

      qtTranslator.load (QString ("qt_%1").arg (ts),
                         QLibraryInfo::location (QLibraryInfo::TranslationsPath));
      qApp->installTranslator (&qtTranslator);

      myappTranslator.load (":/translations/tea_" + ts);
      qApp->installTranslator (&myappTranslator);
      }
}


void rvln::tab_options_pageChanged (int index)
{
  if (index == idx_tab_keyboard)
     {
    //  opt_update_keyb();
     }
}


void rvln::search_fuzzy_mode()
{
  menu_find_whole_words->setChecked (false);
  menu_find_regexp->setChecked (false);;
}


void rvln::search_regexp_mode()
{
  menu_find_fuzzy->setChecked (false);
}


void rvln::search_whole_words_mode()
{
  menu_find_fuzzy->setChecked (false);
}


void rvln::search_from_cursor_mode()
{
  settings->setValue ("find_from_cursor", menu_find_from_cursor->isChecked());
}


void rvln::instr_font_gallery()
{
  last_action = qobject_cast<QAction *>(sender());

  CFontBox *fb = new CFontBox; 
  fb->show(); 
}
  

void CDarkerWindow::closeEvent (QCloseEvent *event)
{
  event->accept();
}
  
  
CDarkerWindow::CDarkerWindow()
{
  setAttribute (Qt::WA_DeleteOnClose);
  
  setWindowFlags (Qt::Tool/* | Qt::FramelessWindowHint*/);

  setWindowTitle (tr("Darker palette"));
  
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
  int corrected_val = value + 100;
  settings->setValue ("darker_val", corrected_val);
  
  documents->apply_settings();
  mainWindow->update_stylesheet (mainWindow->fname_stylesheet);
}


void rvln::darker()
{
  last_action = qobject_cast<QAction *>(sender());

  CDarkerWindow *wd = new CDarkerWindow; 
  wd->show(); 
}


void rvln::fn_stat_words_lengths()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  unsigned long lengths[33] = { };
  
  QStringList w = d->get_words();
  
  foreach (QString s, w)  
          {
           int len = s.length();
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
     nd->textEdit->textCursor().insertText (l.join("\n"));
}


void rvln::file_notes()
{
  last_action = qobject_cast<QAction *>(sender());

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


void rvln::update_stylesheet (const QString &f)
{
qDebug() << "update_stylesheet";
//Update paletted

  int darker_val = settings->value ("darker_val", 100).toInt();


  QFontInfo fi = QFontInfo (qApp->font());

  QString fontsize = "font-size:" + settings->value ("app_font_size", fi.pointSize()).toString() + "pt;";  
  QString fontfamily = "font-family:" + settings->value ("app_font_name", qApp->font().family()).toString() + ";";  
  QString edfontsize = "font-size:" + settings->value ("editor_font_size", "16").toString() + "pt;";  
  QString edfontfamily = "font-family:" + settings->value ("editor_font_name", "Monospace").toString() + ";";  
 
  QString stylesheet; 
  
  stylesheet = "QWidget, QWidget * {" + fontfamily + fontsize + "}\n";
  
  stylesheet += "QPlainTextEdit, QPlainTextEdit * {" + edfontfamily + edfontsize + "}\n";
  
  stylesheet += "QTextEdit {" + edfontfamily + edfontsize + "}\n";

  stylesheet += "CLogMemo {" + fontsize + "}\n";

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


//Update themed


  QString cssfile = qstring_load (f);

  QString css_path = get_file_path (f) + "/"; 
  //theme_dir = css_path;


  cssfile = cssfile.replace ("./", css_path); 

  cssfile += stylesheet;
  

  qApp->setStyleSheet ("");
  qApp->setStyleSheet (cssfile);
  
  //qDebug() << "css_path " << css_path;

}


QIcon rvln::get_theme_icon (const QString &name)
{
  //if (file_get_ext (fname) == "svg")


  QString fname = theme_dir + "icons/" + name;
  
  if (file_exists (fname))
     return QIcon (fname);
  else     
      return QIcon (":/icons/" + name);
}


QString rvln::get_theme_icon_fname (const QString &name)
{
  QString fname = theme_dir + "icons/" + name;
  
  if (file_exists (fname))
     return fname;
  else     
      return ":/icons/" + name;
}


void rvln::view_use_theme()
{
  last_action = qobject_cast<QAction *>(sender());

  QAction *a = qobject_cast<QAction *>(sender());
  QString css_fname = a->data().toString() + "/" + "stylesheet.css";
  
 // qDebug() << css_fname;
  
  if (! file_exists (css_fname))
     {
      log->log (tr ("There is no stylesheet file"));
      return;
    }

  update_stylesheet (css_fname);

  fname_stylesheet = css_fname;

  settings->setValue ("fname_stylesheet", fname_stylesheet);
}


bool has_css_file (const QString &path)
{
  QDir d (path);
  QStringList l = d.entryList();
  
  for (int i = 0; i < l.size(); i++)
     {
      if (l[i].endsWith(".css"))
          return true;
     }
 
  return false;
}

//uses dir name as menuitem, no recursion
void create_menu_from_themes (QObject *handler,
                               QMenu *menu,
                               const QString &dir,
                               const char *method
                               )
{
  menu->setTearOffEnabled (true);
  QDir d (dir);
  QFileInfoList lst_fi = d.entryInfoList (QDir::NoDotAndDotDot | QDir::Dirs,
                                          QDir::IgnoreCase | QDir::LocaleAware | QDir::Name);

  
  foreach (QFileInfo fi, lst_fi)
         {
          if (fi.isDir())
             {
              if (has_css_file (fi.absoluteFilePath()))   
                 {
                  QAction *act = new QAction (fi.fileName(), menu->parentWidget());
                  act->setData (fi.filePath());
                  handler->connect (act, SIGNAL(triggered()), handler, method);
                  menu->addAction (act);
                }
             else
                 {
                  QMenu *mni_temp = menu->addMenu (fi.fileName());
                  create_menu_from_themes (handler, mni_temp,
                                           fi.filePath(), method);

                 }
             }
         }
             
}


void rvln::update_themes()
{
  menu_view_themes->clear();
  
/*
  menu_view_palettes->clear();

  QStringList l1 = read_dir_entries (dir_palettes);
  QStringList l2 = read_dir_entries (":/palettes");
  l1 += l2;

  create_menu_from_list (this, menu_view_palettes,
                         l1,
                         SLOT (file_use_palette()));

*/
 
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


#if QT_VERSION >= 0x050000


void rvln::fn_use_plugin()
{
  last_action = qobject_cast<QAction *>(sender());

  if (! qml_engine)
     qDebug() << "! qml_engine";
 
  QAction *a = qobject_cast<QAction *>(sender());
  
  QString qml_fname = a->data().toString() + "/" + "main.qml";
  
  
   qDebug() << "qml_fname " << qml_fname;
  
  
  if (! file_exists (qml_fname))
     {
      log->log (tr ("There is no plugin file"));
      return;
    }

    
  QQmlComponent *component = new QQmlComponent (qml_engine, QUrl::fromLocalFile (qml_fname));
  
  if (! component->isReady())
    {
     if (component->isError())
        {
         //qDebug() << "Error: " << component->errorString();
         log->log (tr ("<b>Error:</b> ") + component->errorString());
         return; 
        }
    }
  
  CQQuickWindow *window = new CQQuickWindow;

  QObject::connect((QObject*)qml_engine, SIGNAL(quit()), window, SLOT(close()));
  
  QQuickItem *item = qobject_cast<QQuickItem*>(component->create());
  item->setParentItem (window->contentItem());

  window->id = qml_fname;
  
 
  plugins_list.append (new CPluginListItem (qml_fname, window));

  QVariant v = item->property ("close_on_complete"); 
  if (v.isValid() && v.toBool())
     window->close(); 
  else
      {
       window->resize (item->width(), item->height());
       window->show();
      } 
     
  delete component;
}


bool CQQuickWindow::event(QEvent *event)
{
  if (event->type() == QEvent::Close)
    {
     //qDebug() << "CLOSE EVENT IN QML WINDOW";
 
     for (int i = 0; i < plugins_list.size(); ++i) 
        {
         if (plugins_list[i]->id == id)
            {
             delete plugins_list[i];
             plugins_list.removeAt (i);
  //        qDebug() << "removed at " << i;
             break;
            } 
        
        }   
     }
  
  return QQuickWindow::event (event);
}


bool has_qml_file (const QString &path)
{
  QDir d (path);
  QStringList l = d.entryList();
  
  for (int i = 0; i < l.size(); i++)
     {
      if (l[i].endsWith(".qml"))
          return true;
     }
 
  return false;
}

//uses dir name as menuitem, no recursion
void create_menu_from_plugins (QObject *handler,
                               QMenu *menu,
                               const QString &dir,
                               const char *method
                               )
{
  menu->setTearOffEnabled (true);
  QDir d (dir);
  QFileInfoList lst_fi = d.entryInfoList (QDir::NoDotAndDotDot | QDir::Dirs,
                                          QDir::IgnoreCase | QDir::LocaleAware | QDir::Name);

  
  foreach (QFileInfo fi, lst_fi)
         {
          if (fi.isDir())
             {
              if (has_qml_file (fi.absoluteFilePath()))   
                 {
                  QAction *act = new QAction (fi.fileName(), menu->parentWidget());
                  act->setData (fi.filePath());
                  handler->connect (act, SIGNAL(triggered()), handler, method);
                  menu->addAction (act);
                }
             else
                 {
                  QMenu *mni_temp = menu->addMenu (fi.fileName());
                  create_menu_from_plugins (handler, mni_temp,
                                     fi.filePath(), method);

                 }
             }
         }
             
}


void rvln::update_plugins()
{
  menu_fn_plugins->clear();
  
  create_menu_from_plugins (this,
                            menu_fn_plugins,
                            dir_plugins,
                            SLOT (fn_use_plugin())
                            );
  
}


void rvln::plugins_init()
{
  qDebug() << "rvln::plugins_init()";

  qml_engine = new QQmlEngine;
  
//    qmlRegisterInterface<CDocument>("CDocument");

  qmlRegisterType<CDocument>("semiletov.tea.qmlcomponents", 1, 0, "CDocument");
  qmlRegisterType<CDocument>("semiletov.tea.qmlcomponents", 1, 0, "CLogMemo");
  qmlRegisterType<CDocument>("semiletov.tea.qmlcomponents", 1, 0, "CTEAEdit");

  
  
  qml_engine->rootContext()->setContextProperty("documents", documents); 
  qml_engine->rootContext()->setContextProperty("log", log); 
  qml_engine->rootContext()->setContextProperty("tea", this); 
  qml_engine->rootContext()->setContextProperty("settings", settings); 
  qml_engine->rootContext()->setContextProperty("hs_path", hs_path); 
}


void rvln::plugins_done()
{
 qDebug() << "rvln::plugins_done()";
//закрыть все плагины из списка (при созд. плагина добавляем указатель в список)
//и потом 
 
  for (int i = 0; i < plugins_list.size(); ++i) 
      {
       plugins_list[i]->window->close();
         
      }   
 
  delete qml_engine;
}


CPluginListItem::CPluginListItem (const QString &plid, CQQuickWindow *wnd)
{
  id = plid;
  window = wnd;
}

#endif


void rvln::receiveMessage (const QString &msg)
{
  if (msg.isEmpty()) 
     return;
     
  //qDebug() << msg;
  CDocument *d = documents->open_file (msg, "UTF-8"); 
}


int latex_table_sort_col; 


bool latex_table_sort_fn (const QStringList &l1, const QStringList &l2)
{
  return l1[latex_table_sort_col] < l2[latex_table_sort_col];
}

 
void rvln::fn_sort_latex_table_by_col_abc()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;
  
  QStringList fiftxt = fif_get_text().split("~");
  
  if (fiftxt.size() < 2)
     return;
  
  
  latex_table_sort_col = fiftxt[1].toInt();
  
  
  QString sep = fiftxt[0];
  
  //if (fiftxt.size() == 2)
    
  QString t = d->textEdit->textCursor().selectedText();
  
  if (t.isEmpty())
     return;
  
  if (t.indexOf (sep) == -1)
	  return;
  
  QStringList sl_temp = t.split (QChar::ParagraphSeparator); 

  qDebug() << "sl_temp.size " << sl_temp.size();
  
  QList <QStringList> output;
  
  foreach (QString v, sl_temp)  
          {
	   if (! v.isEmpty())
	      {
               QStringList sl_parsed = v.split (sep);
               if (latex_table_sort_col + 1 <= sl_parsed.size())
            	   output.append (sl_parsed);
	      }
          }
  
 
  qSort (output.begin(), output.end(), latex_table_sort_fn);
    
  sl_temp.clear();    
  
  for (int i = 0; i < output.size(); i++)
      {
       sl_temp.append (output[i].join (sep));
      }
  
  t = sl_temp.join ("\n");
  
  d->textEdit->textCursor().insertText (t);
}



void rvln::fn_table_swap_cells()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
      return;
	  
  QStringList fiftxt = fif_get_text().split("~");
	  
  if (fiftxt.size() < 3)
	 return;
	    
  int col1 = fiftxt[1].toInt();
  int col2 = fiftxt[2].toInt();
	   
  QString sep = fiftxt[0];  
	   
  QString t = d->textEdit->textCursor().selectedText();
	  
  if (t.isEmpty())
	     return;
	  
  if (t.indexOf (sep) == -1)
	  return;
	
  int imax = fmax (col1, col2);

  QStringList sl_temp = t.split (QChar::ParagraphSeparator); 
    
  QList <QStringList> output;
  
  foreach (QString v, sl_temp)  
          {
	  if (! v.isEmpty())
	     {
              QStringList sl_parsed = v.split (sep);
              if (imax + 1 <= sl_parsed.size())
                 {
            	  sl_parsed.swap (col1, col2);	 
                  output.append (sl_parsed);
                 }
	     }
          }
     
  sl_temp.clear();    
  
  for (int i = 0; i < output.size(); i++)
      {
	  
       sl_temp.append (output[i].join (sep));
      }
  
  t = sl_temp.join ("\n");
  
  d->textEdit->textCursor().insertText (t);
}


void rvln::fn_table_delete_cells()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
      return;
	  
  QStringList fiftxt = fif_get_text().split("~");
	  
  if (fiftxt.size() < 2)
	 return;
	    
  int col1 = fiftxt[1].toInt();
  //int col2 = col1;
  
  /*
  if (fiftxt.size() == 3)
     col2 = fiftxt[2].toInt();
	*/  
  QString sep = fiftxt[0];  
	  
 
  QString t = d->textEdit->textCursor().selectedText();
	  
  if (t.isEmpty())
	     return;
	  
  if (t.indexOf (sep) == -1)
	  return;
	
  
  QStringList sl_temp = t.split (QChar::ParagraphSeparator); 
    
  QList <QStringList> output;
  
  foreach (QString v, sl_temp)  
          {
	   if (! v.isEmpty())
	      {
               QStringList sl_parsed = v.split (sep);
               if (col1 + 1 <= sl_parsed.size())
                  {
            	   sl_parsed.removeAt (col1);	 
                   output.append (sl_parsed);
                  }
	      }
          }
   
  
  sl_temp.clear();    
  
  for (int i = 0; i < output.size(); i++)
      {
       sl_temp.append (output[i].join (sep));
      }
  
  t = sl_temp.join ("\n");
  
  d->textEdit->textCursor().insertText (t);
}


void rvln::fn_table_copy_cells()
{
  last_action = qobject_cast<QAction *>(sender());

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
     
 
  QString t = d->textEdit->textCursor().selectedText();
	  
  if (t.isEmpty())
	     return;
	  
  if (t.indexOf (sep) == -1)
	  return;
  
  QStringList sl_temp = t.split (QChar::ParagraphSeparator); 
    
  QList <QStringList> output;  
  
  if (col2 > 0)
  foreach (QString v, sl_temp)  
          {
	   if (! v.isEmpty())
	      {
               QStringList sl_parsed = v.split (sep);
               if (col2 + 1 <= sl_parsed.size())
                  {
                   QStringList tl = sl_parsed.mid (col1, col2 - col1 + 1); 
                   output.append (tl);
                  }
              }
          }
  else
      foreach (QString v, sl_temp)  
	      {
	       if (! v.isEmpty())
	  	  {
	           QStringList sl_parsed = v.split (sep);
	           if (col1 + 1 <= sl_parsed.size())
	              {
	               QStringList tl = sl_parsed.mid (col1, 1); 
	               output.append (tl);
	               }
	  	   }
               }
  
  sl_temp.clear();    
  
  for (int i = 0; i < output.size(); i++)
      {
       sl_temp.append (output[i].join (sep));
      }
  
  t = sl_temp.join ("\n");
  
  QApplication::clipboard()->setText (t);  
}


MyProxyStyle::MyProxyStyle (QStyle * style): QProxyStyle (style)
{
     
}


/*
MyProxyStyle::MyProxyStyle (const QString & key)
{
  QProxyStyle::QProxyStyle (key);  
   
}
*/



/*
void rvln::keyPressEvent (QKeyEvent *event)
{
   if (event->key() == Qt::Key_F10)
      //fileMenu->setFocus(Qt::PopupFocusReason);
      qDebug() << "F10";
      
      
      //menuBar()->setFocus(Qt::MenuBarFocusReason);
      //menuBar()->setFocus(Qt::OtherFocusReason);
      //menuBar()->hovered (filesAct);
   else 
       QMainWindow::keyPressEvent (event);
}
*/


void rvln::test()
{


     
/*  CDocument *d = documents->get_current();
  if (! d)
     return;

    QTime time_start;
    time_start.start();

  pb_status->show();
  pb_status->setRange (0, d->textEdit->toPlainText().size() - 1);
  pb_status->setFormat (tr ("%p% completed"));
  pb_status->setTextVisible (true);

   int i = 0;

   QString fiftxt = fif_get_text();

   d->text_to_search = fiftxt;

   QTextCursor cr = d->textEdit->document()->find (d->text_to_search, 0, get_search_options());
   cr.movePosition (QTextCursor::Right, QTextCursor::KeepAnchor, d->text_to_search.size());
8779797

    do
      {

          {
           f = cr.blockCharFormat();
           f.setUnderlineStyle (QTextCharFormat::SpellCheckUnderline);
           f.setUnderlineColor (QColor (hash_get_val (documents->palette, "error", "red")));
           cr.mergeCharFormat (f);
          }

        pb_status->setValue (i++);
       }
    while (cr.movePosition (QTextCursor::NextWord));

    cr.setPosition (pos);
    d->textEdit->setTextCursor (cr);
    d->textEdit->document()->setModified (false);

    pb_status->hide();

    log->log (tr("elapsed milliseconds: %1").arg (time_start.elapsed()));
//  CDocument *d = documents->get_current();
//  if (! d)
  //   return;


//  QLabel *l_font_demo = new QLabel (d->get_all_text());
//  l_font_demo->setFont (QFont (sl[0]));
//  l_font_demo->show();


*/

/*  CDocument *d = documents->get_current();
  if (! d)
     return;

  QStringList l = text_get_bookmarks (d->textEdit->toPlainText());
  qstring_list_print (l);
*/


/*
  QRegExp e ("([0-1][0-9]|2[0-3]):([0-5][0-9])");
  QString s ("aaa 22:13 bbb");
  qDebug() << s.indexOf (e);
  */
/*
  QString source = "aaa 22:13 bbb 44:56";
  QRegExp e ("([0-1][0-9]|2[0-3]):([0-5][0-9])");
  
  QString sl_parsed;
 
  int i = 0;
  int x = 0;
  
  while (x != -1)
       {
        qDebug() << "i=" << i;
        x = source.indexOf (e, i);
        qDebug() << "x=" << x; 
        if (x != -1)   
           i += (x + 4);
        //else   
          //  i += 4;       
       }
*/  

//    WId wid = QxtWindowSystem::findWindow("Mail - Kontact");
  //  QPixmap screenshot = QPixmap::grabWindow(wid);
  
//full desktop:
//originalPixmap = QPixmap::grabWindow(QApplication::desktop()->winId());
  
}



void rvln::fn_sum_by_last_col()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
      return;
      

  QString t = d->textEdit->textCursor().selectedText();
  t = t.replace (",", ".");	
	  
  if (t.isEmpty())
	 return;
  
  double sum = 0.0f;
  
  QStringList l = t.split (QChar::ParagraphSeparator); 
  
  
  for (int i = 0; i < l.size(); i++)
      {
       QStringList t = l[i].split (" ");
       if (t.size() > 0)
          {
           //qDebug() << t[t.size() - 1];
           sum += t[t.size() - 1].toDouble();
          }
      }
    
  log->log (tr ("sum: %1").arg (sum));

      
}      

/*
void rvln::ed_paste_from_charset()
{
  CTextListWindow w (tr ("Select"), tr ("Available effects"));

  w.list->addItems (sl_charsets);

  int result = w.exec();

  if (result != QDialog::Accepted)
      return;

  //AFx *f = avail_fx->find_by_name (w.list->currentItem()->text());

  QClipboard *clipboard = QApplication::clipboard();
  const QMimeData *mimeData = clipboard->mimeData();
  
  if (!mimeData->hasText())
     {
      qDebug() << "! text";
      return;
     
     }
  
 // QString t = clipboard->text();

  QTextCodec *codec = QTextCodec::codecForName (w.list->currentItem()->text().toLatin1());
  
  
  QString s = codec->toUnicode (mimeData->data("text/plain"));
  
  qDebug() << s.toUtf8().data();

}
*/

void rvln::search_unmark()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  int darker_val = settings->value ("darker_val", 100).toInt();
 
  QString text_color = hash_get_val (global_palette, "text", "black");
  QString back_color = hash_get_val (global_palette, "background", "white");


  QString t_text_color = QColor (text_color).darker(darker_val).name(); 
  QString t_back_color = QColor (back_color).darker(darker_val).name(); 

  
  QTextCursor cr;

     
  d->textEdit->selectAll();
     
          QTextCharFormat f =  d->textEdit->currentCharFormat();
          f.setBackground (QColor (t_back_color));
          f.setForeground (QColor (t_text_color));
  d->textEdit->mergeCurrentCharFormat (f);
  
  d->textEdit->textCursor().clearSelection();
}


void rvln::search_mark_all()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  int darker_val = settings->value ("darker_val", 100).toInt();
 
  QString text_color = hash_get_val (global_palette, "text", "black");
  QString back_color = hash_get_val (global_palette, "background", "white");
  QString t_text_color = QColor (text_color).darker(darker_val).name(); 
  QString t_back_color = QColor (back_color).darker(darker_val).name(); 

  bool cont_search = true;
  
  QTextCursor cr;

  int pos_save = d->textEdit->textCursor().position();
     
  d->textEdit->selectAll();
     
  QTextCharFormat f = d->textEdit->currentCharFormat();
  f.setBackground (QColor (t_back_color));
  f.setForeground (QColor (t_text_color));
  d->textEdit->mergeCurrentCharFormat (f);
  
  d->textEdit->textCursor().clearSelection();

  int from;
      
  if (settings->value ("find_from_cursor", "1").toBool())
      from = d->textEdit->textCursor().position();
  else
      from = 0;

  d->text_to_search = fif_get_text();

  while (cont_search)
      {
      if (menu_find_regexp->isChecked())
         cr = d->textEdit->document()->find (QRegExp (d->text_to_search), from, get_search_options());
      else
          if (menu_find_fuzzy->isChecked())
             {
              int pos = str_fuzzy_search (d->textEdit->toPlainText(), d->text_to_search, from, settings->value ("fuzzy_q", "60").toInt());
              if (pos != -1)
                 {
                  from = pos + d->text_to_search.length() - 1;
                  //set selection:
                  cr = d->textEdit->textCursor();
                  cr.setPosition (pos, QTextCursor::MoveAnchor);
                  cr.movePosition (QTextCursor::Right, QTextCursor::KeepAnchor, d->text_to_search.length());

                  if (! cr.isNull())
                      d->textEdit->setTextCursor (cr);
                 }
              else 
                  cont_search = false; 
             }
      else //normal search
          cr = d->textEdit->document()->find (d->text_to_search, from, get_search_options());


      if (! cr.isNull())  
         {
          d->textEdit->setTextCursor (cr);
          QTextCharFormat f = cr.blockCharFormat();
          f.setBackground (QColor (hash_get_val (global_palette, "backgroundmark", "red")));
          f.setForeground (QColor (hash_get_val (global_palette, "foregroundmark", "blue")));

          cr.mergeCharFormat (f);
         } 
      else 
          cont_search = false; 
          
       from = d->textEdit->textCursor().position();   
      
     }

   d->textEdit->document()->setModified (false);

  d->goto_pos (pos_save);
}


void rvln::scale_image()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  QString fname = d->get_filename_at_cursor();


  if (fname.isEmpty())
     return;

  if (! is_image (fname))
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

    
 // qDebug() << fnameout;
  
  bool scale_by_side = true;
  
  if (params[1].indexOf("%") != -1)   
     scale_by_side = false;
     
  int side;
  int percent;
  
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

  QImage source (fname);
  if (source.isNull())
     return;
                 
                  
 if (settings->value ("cb_exif_rotate", 0).toInt()) 
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
    
    if (scale_by_side)
       dest = image_scale_by (source, true, side, transformMode);
    else   
        dest = image_scale_by (source, false, percent, transformMode);

    QString fmt (settings->value ("output_image_fmt", "jpg").toString());

    fnameout = change_file_ext (fnameout, fmt);

    if (! dest.save (fnameout, fmt.toLatin1().constData(), quality))
        log->log (tr("Cannot save: %1").arg (fnameout));    
    else
        log->log (tr("Saved: %1").arg (fnameout));    
        
  //    qDebug() << fnameout;     
}


void rvln::recentoff()
{
  last_action = qobject_cast<QAction *>(sender());

  b_recent_off = ! b_recent_off;
  menu_recent_off->setChecked (b_recent_off);
}


void rvln::repeat()
{
  if (last_action)
      last_action->trigger();
}


void rvln::ed_block_start()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;
  
  int x = d->textEdit->textCursor().position() - d->textEdit->textCursor().block().position();
  int y = d->textEdit->textCursor().block().blockNumber();
  
  
  d->textEdit->rect_sel_start.setX (x);
  d->textEdit->rect_sel_start.setY (y);
  
  d->textEdit->update_ext_selections();
}


void rvln::ed_block_end()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;
  
  int x = d->textEdit->textCursor().position() - d->textEdit->textCursor().block().position();
  int y = d->textEdit->textCursor().block().blockNumber();
  
  d->textEdit->rect_sel_end.setX (x);
  d->textEdit->rect_sel_end.setY (y);
  
  d->textEdit->update_ext_selections();
}


void rvln::ed_block_copy()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  if (! d->textEdit->has_rect_selection())
     return;
     
  QApplication::clipboard()->setText (d->textEdit->get_rect_sel());   
}


void rvln::ed_block_paste()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  d->textEdit->rect_sel_replace (QApplication::clipboard()->text());
   
  //QApplication::clipboard()->setText (d->textEdit->get_rect_sel());   
}


void rvln::ed_block_cut()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
     return;

  //if (! d->textEdit->has_rect_selection())
    // return;

  d->textEdit->rect_sel_cut();

     
  //QApplication::clipboard()->setText (d->textEdit->get_rect_sel());   
}


void rvln::fman_zeropad()
{
  QString fiftxt = fif_get_text();
  int finalsize = fiftxt.toInt();
  if (finalsize < 1)
     finalsize = 10;

  QStringList sl = fman->get_sel_fnames();   
  
  if (sl.size() < 1)
     return; 

  for (int i = 0; i < sl.size(); i++) 
  //foreach (QString fname, sl)
          {
           QString fname = sl[i];
           QFileInfo fi (fname);
           if (fi.exists() && fi.isWritable())
              {
               //int countzeroes = fi.baseName().count('0');
               
               //int zeroes_to_add = zeroes - countzeroes;
               int zeroes_to_add = finalsize - fi.baseName().length();
                                             
               QString newname = fi.baseName();
               QString ext = file_get_ext (fname);
               
               newname.remove(QRegExp("[a-zA-Z\\s]"));
               
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
               
               //qDebug() << newfpath;
              }
          }  
 
  update_dyn_menus();
  fman->refresh();
}


void rvln::fman_del_n_first_chars()
{
  QString fiftxt = fif_get_text();
  int todel = fiftxt.toInt();
  if (todel < 1)
     todel = 1;

  QStringList sl = fman->get_sel_fnames();   
  
  if (sl.size() < 1)
     return; 
  
  foreach (QString fname, sl)
          {
           QFileInfo fi (fname);
           if (fi.exists() && fi.isWritable())
              {
               QString newname = fi.fileName();
               //QString ext = file_get_ext (fname);
                              
               newname = newname.mid (todel);
               
             //  if (newname.size() > (ext.size() + 1))
                  {
                   QString newfpath (fi.path());
                   newfpath.append ("/").append (newname);
                   QFile::rename (fname, newfpath);
                   
                   //qDebug() << newfpath;
                  }
              }
          }  
 
  update_dyn_menus();
  fman->refresh();
}


void rvln::fman_multreplace()
{
  //QString fiftxt = fif_get_text();
  
  QStringList l = fif_get_text().split ("~");
  if (l.size() < 2)
     return;

  QStringList sl = fman->get_sel_fnames();   
  
  if (sl.size() < 1)
     return; 
  
  foreach (QString fname, sl)
          {
           QFileInfo fi (fname);
           if (fi.exists() && fi.isWritable())
              {
               QString newname = fi.fileName();
    //           QString ext = file_get_ext (fname);
               
               newname = newname.replace (l[0], l[1]);
               
               QString newfpath (fi.path());
               newfpath.append ("/").append (newname);
               QFile::rename (fname, newfpath);
               //   qDebug() << newfpath;
              }    
          }  
 
  update_dyn_menus();
  fman->refresh();
}


void rvln::fman_apply_template()
{
  QString fiftxt = fif_get_text();
  
  QStringList sl = fman->get_sel_fnames();   
  
  if (sl.size() < 1)
     return; 
  
  foreach (QString fname, sl)
          {
           QFileInfo fi (fname);
           if (fi.exists() && fi.isWritable())
              {
               QString ext = file_get_ext (fname);
               QString newname = fiftxt; 
               newname = newname.replace ("%filename", fi.fileName()); 
               newname = newname.replace ("%ext", ext); 
               newname = newname.replace ("%basename", fi.baseName()); 
             
               
               QString newfpath (fi.path());
               newfpath.append ("/").append (newname);
               QFile::rename (fname, newfpath);
               //   qDebug() << newfpath;
              }    
          }  
 
  update_dyn_menus();
  fman->refresh();
}


//UTF-16BE, UTF-32BE 	 	
//′
#define UQS 8242 	
//″
#define UQD 8243 //0x00B3 
//°
#define UQDG 176

//degrees minutes seconds: 40° 26′ 46″ N 79° 58′ 56″ W
//to
//decimal degrees: 40.446° N 79.982° W
void rvln::fn_number_dms2dc()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
      return;
  
  QString t = d->textEdit->textCursor().selectedText();
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

  qDebug() << "latitude " << latitude; 
  
  qDebug() << "longtitude " << longtitude; 
 
  int iqdg = latitude.indexOf(QChar (UQDG));
  int iqs = latitude.indexOf(QChar (UQS));
  int iqd = latitude.indexOf(QChar (UQD));
  
/*  qDebug() << "iqdg : " << iqdg;
  qDebug() << "iqs : " << iqs;
  qDebug() << "iqd : " << iqd;
  */ 
  QString degrees1 = latitude.left (iqdg);
  QString minutes1 = latitude.mid (iqdg + 1, iqs - iqdg - 1);
  QString seconds1 = latitude.mid (iqs + 1, iqd - iqs - 1);

/*
  qDebug() << "degrees1 : " << degrees1;
  qDebug() << "minutes1 : " << minutes1;
  qDebug() << "seconds1 : " << seconds1;
*/
  double lat_decimal_degrees = degrees1.toDouble() + (double) (minutes1.toDouble() / 60) + (double) (seconds1.toDouble() / 3600);
  QString lat_decimal_degrees_N = QString::number (lat_decimal_degrees, 'f', 3) + QChar (UQDG) + north_or_south;
  
  iqdg = longtitude.indexOf (QChar (UQDG));
  iqs = longtitude.indexOf (QChar (UQS));
  iqd = longtitude.indexOf (QChar (UQD));
 
   qDebug() << "iqdg : " << iqdg;
  qDebug() << "iqs : " << iqs;
  qDebug() << "iqd : " << iqd;
 
 
  degrees1 = longtitude.left (iqdg);
  minutes1 = longtitude.mid (iqdg + 1, iqs - iqdg - 1);
  seconds1 = longtitude.mid (iqs + 1, iqd - iqs - 1);
 
  double longt_decimal_degrees = degrees1.toDouble() + (double) (minutes1.toDouble() / 60) + (double) (seconds1.toDouble() / 3600);
  
  QString longt_decimal_degrees_N = QString::number (longt_decimal_degrees, 'f', 3) + QChar (UQDG) + east_or_west;
 
 
  log->log (lat_decimal_degrees_N + " " + longt_decimal_degrees_N);
//  qDebug() << "decimal_degrees " << decimal_degrees; 
//  qDebug() << "decimal_degrees_N " << decimal_degrees_N; 

//     d->textEdit->textCursor().insertText (int_to_binary (d->textEdit->textCursor().selectedText().toInt()));
}



/*

degrees = floor (decimal_degrees)

minutes = floor (60 * (decimal_degrees - degrees))

seconds = 3600 * (decimal_degrees - degrees) - 60 * minites

*/

//decimal degrees: 40.446° N 79.982° W
//to
//degrees minutes seconds: 40° 26′ 46″ N 79° 58′ 56″ W

void rvln::fn_number_dd2dms()
{
  last_action = qobject_cast<QAction *>(sender());

  CDocument *d = documents->get_current();
  if (! d)
      return;
  
  QString t = d->textEdit->textCursor().selectedText();
  t = t.remove (" ");
  t = t.remove (UQDG);
  
  QChar north_or_south = ('N');
  if (t.contains ('S'))
     north_or_south = 'S';
  
  QChar east_or_west = ('E');
  if (t.contains ('W'))
     east_or_west = 'W';
    
  QStringList l = t.split (north_or_south);
  
  QString latitude = l[0];
  QString longtitude = l[1].remove(QRegExp("[a-zA-Z\\s]"));

//  qDebug() << "latitude " << latitude; 
//  qDebug() << "longtitude " << longtitude; 
 
  double degrees = floor (latitude.toDouble());
  double minutes = floor (60 * (latitude.toDouble() - degrees));
  double seconds = round (3600 * (latitude.toDouble() - degrees) - 60 * minutes);

  //qDebug() << "degrees : " << degrees;
  //qDebug() << "minutes : " << minutes;
  //qDebug() << "seconds : " << seconds;

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

  
/*
  double lat_decimal_degrees = degrees1.toDouble() + (double) (minutes1.toDouble() / 60) + (double) (seconds1.toDouble() / 3600);
  QString lat_decimal_degrees_N = QString::number (lat_decimal_degrees, 'f', 3) + QChar (UQDG) + north_or_south;
  
  iqdg = longtitude.indexOf (QChar (UQDG));
  iqs = longtitude.indexOf (QChar (UQS));
  iqd = longtitude.indexOf (QChar (UQD));
 
   qDebug() << "iqdg : " << iqdg;
  qDebug() << "iqs : " << iqs;
  qDebug() << "iqd : " << iqd;
 
 
  degrees1 = longtitude.left (iqdg);
  minutes1 = longtitude.mid (iqdg + 1, iqs - iqdg - 1);
  seconds1 = longtitude.mid (iqs + 1, iqd - iqs - 1);
 
  double longt_decimal_degrees = degrees1.toDouble() + (double) (minutes1.toDouble() / 60) + (double) (seconds1.toDouble() / 3600);
  
  QString longt_decimal_degrees_N = QString::number (longt_decimal_degrees, 'f', 3) + QChar (UQDG) + east_or_west;
 
 
  log->log (lat_decimal_degrees_N + " " + longt_decimal_degrees_N);
//  qDebug() << "decimal_degrees " << decimal_degrees; 
//  qDebug() << "decimal_degrees_N " << decimal_degrees_N; 
*/
//     d->textEdit->textCursor().insertText (int_to_binary (d->textEdit->textCursor().selectedText().toInt()));
}



void rvln::receiveMessageShared(const QStringList &msg)
{
  for (int i = 0; i < msg.size(); i++)
      CDocument *d = documents->open_file (msg[i], "UTF-8"); 
      
  show();
  activateWindow();
  raise();
}  

/*
void rvln::slot_commitDataRequest (QSessionManager &m) 
{
  QString fname (dir_sessions);
  fname.append ("/").append ("shutdownsession");
  documents->save_to_session (fname);
}


void rvln::app_commit_data()
{
  QString fname (dir_sessions);
  fname.append ("/").append ("shutdownsession");
  documents->save_to_session (fname);

  //QString get_triplex();


 //if (documents->recent_files.size() > 0)
   //  {
     // documents->open_file_triplex (documents->recent_files[0]);
      //documents->recent_files.removeAt (0);
      //documents->update_recent_menu();
      //main_tab_widget->setCurrentIndex (idx_tab_edit); 
     //}  

}
*/
