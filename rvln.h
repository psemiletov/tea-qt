/**************************************************************************
 *   2007-2017 by Peter Semiletov                            *
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


#ifndef RVLN_H
#define RVLN_H


#include "document.h"
#include "fman.h"
#include "calendar.h"
#include "shortcuts.h"
#include "img_viewer.h"


#ifdef SPELLCHECK_ENABLE

#include "spellchecker.h"

#endif


#include <QListWidget>
#include <QComboBox>
#include <QProgressBar>
#include <QCheckBox>
#include <QSpinBox>
//#include <QToolButton>
#include <QFontComboBox>
#include <QSessionManager>
#include <QApplication>
#include <QCoreApplication>
#include <QSplitter>
#include <QTextBrowser>
#include <QTranslator>
#include <QProcess>
//#include <QSessionManager>
//#include <QSystemTrayIcon>


//QML stuff
#ifdef USE_QML_STUFF

#include <QQmlEngine>
#include <QQuickWindow>

#endif

//



#ifdef PRINTER_ENABLE

#include <QPrinter>

#endif



class CMarkupPair: public QObject
{
  Q_OBJECT

public:

  QHash <QString, QString> pattern;
};



class CStrIntPair: public QObject
{
  Q_OBJECT

public:

  QString string_value;
  int int_value;
   
  CStrIntPair (const QString &s, int i);
};  


class CDarkerWindow: public QWidget
{
  Q_OBJECT

  QSlider *slider;
  
public:

  CDarkerWindow();

protected:

  void closeEvent (QCloseEvent *event);
  
public slots:
 
 void slot_valueChanged (int value);
};



class CAboutWindow: public QWidget
{
  Q_OBJECT

  QLabel *logo;
  
public:

  CAboutWindow();

protected:

  void closeEvent (QCloseEvent *event);
  
public slots:

  void update_image();
};


class CTextListWnd: public QWidget
{
  Q_OBJECT

public:

  QListWidget *list;
  
  CTextListWnd (const QString &title, const QString &label_text);

protected:

  void closeEvent (QCloseEvent *event);
};


class rvln: public QMainWindow
{
  Q_OBJECT

public:

  rvln();
  ~rvln();
  
#ifdef USE_QML_STUFF

  QQmlEngine *qml_engine;
#endif

  QAction *last_action;
  
  int fm_entry_mode; 

  QDate date1;
  QDate date2;

  bool portable_mode;

  CLogMemo *log;
  
 
  CShortcuts *shortcuts;
  CFMan *fman;
  CImgViewer *img_viewer;

#ifdef SPELLCHECK_ENABLE

  ASpellchecker *spellchecker;
  QStringList spellcheckers;
  QString cur_spellchecker;

#endif


  QHash <int, QString> moon_phase_algos;
  void create_moon_phase_algos();

  int icon_size;

  bool b_preview;
  bool capture_to_storage_file;

  int idx_tab_calendar;
  int idx_tab_edit;
  int idx_tab_tune;
  int idx_tab_fman;
  int idx_tab_learn;
  
  int idx_tab_keyboard;


  int idx_prev;
  
  int fman_find_idx;
  QList <QStandardItem *> l_fman_find;
  
  bool ui_update;
  QString fname_storage_file;

  QHash <QString, CMarkupPair*> hs_markup;

  QToolBar *tb_fman_dir;
  QLabel *l_fman_preview;
 
  QStringList sl_places_bmx;
  QStringList sl_urls;
  QStringList sl_charsets;
  QStringList sl_last_used_charsets;
  QStringList sl_fif_history;

  QHash <QString, QString> programs;
  QHash <QString, QString> places_bookmarks;

  QTranslator myappTranslator;
  QTranslator qtTranslator;

  QDir dir_lv;

  QComboBox *cmb_fif;
  

  //QString stylesheet; 
  QString fname_stylesheet; 


  QString man_search_value;

  QString markup_mode;

#ifdef PRINTER_ENABLE

  QPrinter printer;

#endif


  QString opt_shortcuts_string_to_find;
  QString fman_fname_to_find;

  QString dir_user_dict;
  QString dir_profiles;  
  QString dir_last;
  QString dir_config;
  QString dir_templates;
  QString dir_sessions;
  QString dir_hls;
  QString dir_themes;


//#if QT_VERSION >= 0x050000

  QString dir_plugins;

//#endif
  
  QString dir_snippets;
  QString dir_scripts;
  QString dir_palettes;
  QString dir_tables;
  QString dir_days;

  QString fname_def_palette;
  QString fname_hls_cache;

 // QString fname_userfonts;
  //QList <int> userfont_ids;


  QString fname_fif;

  QString fname_bookmarks;
  QString fname_programs;
  QString fname_places_bookmarks;

  QString bookmarks;
  QString fname_crapbook;
  QString fname_tempfile;
  QString fname_tempparamfile;

  QLabel *l_status;
  QProgressBar *pb_status;
  QLineEdit *ed_spellcheck_path;
  QLineEdit *ed_aspellcheck_path;
  
  QCheckBox *cb_use_qregexpsyntaxhl;

  QCheckBox *cb_right_to_left;

  QCheckBox *cb_wordwrap;
  QCheckBox *cb_show_linenums;
  QCheckBox *cb_hl_current_line;
  QCheckBox *cb_show_margin;
  QSpinBox *spb_margin_pos;

  QSpinBox *spb_fuzzy_q;

  QLineEdit *ed_label_end;
  QLineEdit *ed_label_start;


  QCheckBox *cb_session_restore;
  QCheckBox *cb_use_hl_wrap;
  QCheckBox *cb_hl_brackets;
  QCheckBox *cb_auto_indent;
  QCheckBox *cb_spaces_instead_of_tabs;
  QCheckBox *cb_cursor_xy_visible;

  QSpinBox *spb_tab_sp_width;
  QCheckBox *cb_center_on_cursor;
  
  QCheckBox *cb_full_path_at_window_title;
  
  
  QCheckBox *cb_use_trad_dialogs;
  QCheckBox *cb_start_on_sunday;
  QCheckBox *cb_northern_hemisphere;
  QComboBox *cmb_moon_phase_algos;
  QComboBox *cmb_output_image_fmt;
  QCheckBox *cb_output_image_flt;

  QComboBox *cmb_tea_icons;
  
  
  QComboBox *cmb_ui_langs;

  QSpinBox *spb_img_quality;
  QCheckBox *cb_zip_after_scale;
  
  QCheckBox *cb_exif_rotate;
  
  QCheckBox *cb_zor_use_exif;


protected:

 // void keyPressEvent (QKeyEvent *event);    

  void closeEvent (QCloseEvent *event);
  bool fman_tv_eventFilter (QObject *obj, QEvent *event);


public slots:



/*************************
main window callbacks
*************************/

 // void app_commit_data();

 // void slot_commitDataRequest(QSessionManager &m); 

  void repeat();

  void receiveMessage(const QString &msg);
  
  void receiveMessageShared(const QStringList& msg);
        
  void view_use_theme();


  void select_label();
  void update_labels_menu();
  void update_labels_list();

  
  void fm_full_info();

  void fm_hashsum_md5();
  void fm_hashsum_md4();
  void fm_hashsum_sha1();

  void fman_unpack_zip();
  void fman_zip_info();


  void fman_refresh();
  void fman_rename();
  void fman_delete();
  void fman_zeropad();
  void fman_del_n_first_chars();
  void fman_multreplace();
  void fman_apply_template();

  
  void fman_drives_changed (const QString & path);
  void fman_current_file_changed (const QString &full_path, const QString &just_name);
  void fman_file_activated (const QString &full_path);
  void fman_dir_changed (const QString &full_path);
  void fman_fname_entry_confirm();
  void fman_create_dir();
  void fman_add_bmk();
  void fman_del_bmk();
  void fman_open();
  void fman_home();
  void fman_places_itemActivated (QListWidgetItem *item);
  void fman_select_by_regexp();
  void fman_deselect_by_regexp();
  void fman_count_lines_in_selected_files();

  void cb_button_saves_as();

  void pageChanged (int index);
  void tab_options_pageChanged (int index);

  void file_print();

  void newFile();
  void open();
  void open_at_cursor();
  void rename_selected();

  void file_reload();
  void file_reload_enc_itemDoubleClicked ( QListWidgetItem *item); 
  void file_reload_enc();
  void file_open_bookmarks_file();
  void file_open_programs_file();
  void file_find_obsolete_paths();


  void file_last_opened();
  void file_open_bookmark();
  void file_open_program();
  void file_add_to_bookmarks();
  void file_crapbook();
  
  void file_notes();
  
  void file_use_template();
  void file_use_snippet();
  void file_use_palette();
  void file_use_hl();
  void file_open_session();
  void file_save_version();
  void file_save_bak();
  void test();

  void clipboard_dataChanged();

  void view_use_profile();

  void cal_set_date_a();
  void cal_set_date_b();
  void cal_set_to_current();
  void cal_moon_mode();
  void cal_gen_mooncal();

  void cal_diff_days();


  void cal_remove();

  void fn_sum_by_last_col();
  

  void fn_insert_cpp();
  void fn_insert_c();

  void fn_insert_template_html5();


  bool save();
  bool saveAs();

  void toggle_wrap();
  void darker();

  void about();
  void close_current();

  void ed_block_start();
  void ed_block_end();
  void ed_block_copy();
  void ed_block_paste();
  void ed_block_cut();
  

  void ed_copy();
  void ed_paste();
  void ed_cut();

  void ed_undo();
  void ed_redo();
  void ed_clear();

  void ed_indent();
  void ed_unindent();
 
  void ed_comment();

//  void ed_paste_from_charset();


  void set_as_storage_file();
  void copy_to_storage_file();
  void capture_clipboard_to_storage_file();

  
  void upCase();
  void dnCase();

  void edit_copy_current_fname();

  void fn_count();
  void fn_count_rx();

  void fn_enum();
  void fn_filter_with_regexp();
  void fn_text_stat();
  void fn_antispam_email();
  void fn_apply_to_each_line();
  void fn_reverse();
  void fn_escape();

  void fn_evaluate();
  void fn_sort_casecare();
  void fn_sort_casecareless();

  void fn_sort_casecare_sep();
  

  void fn_sort_latex_table_by_col_abc();
  void fn_table_swap_cells();
  void fn_table_delete_cells();
  void fn_table_copy_cells();
      
  
  void fn_flip_a_list();
  void fn_flip_a_list_sep();

  
  void fn_insert_loremipsum();
  void fn_insert_template_html();
  void fn_run_script();
  void cb_script_finished (int exitCode, QProcess::ExitStatus exitStatus);
  void fn_number_arabic_to_roman();
  void fn_number_roman_to_arabic();
  
  void fn_number_dms2dc();
  void fn_number_dd2dms();

  
  void fn_filter_rm_less_than();
  void fn_filter_rm_greater_than();
  void fn_filter_rm_duplicates();
  void fn_filter_rm_empty();

  void fn_filter_delete_before_sep();
  void fn_filter_delete_after_sep();

  void fn_rm_formatting();
  
  void fn_rm_compress();
 
  void text_compare_two_strings();
  
  void fn_convert_quotes_angle();
  void fn_convert_quotes_curly();
  
  void fn_convert_quotes_tex_curly();

  void fn_convert_quotes_tex_angle_01();
  void fn_convert_quotes_tex_angle_02();
  
  
  void fn_get_words_count();
  void fn_unitaz_abc();
  void fn_extract_words();
  void fn_morse_from_en();
  void fn_morse_to_en();
  void fn_morse_from_ru();
  void fn_morse_to_ru();
  void fn_insert_date();
  void fn_insert_time();
  void fn_rm_formatting_at_each_line();
  void fn_rm_trailing_spaces();

  void fn_strip_html_tags();
  void fn_number_decimal_to_binary();
  void fn_number_flip_bits();
  void fn_binary_to_decimal();
  
  void remove_formatting();

//  void fn_variants_callback();

  
#ifdef SPELLCHECK_ENABLE

  void fn_change_spell_lang();
  void fn_spell_check();
  void fn_spell_suggest_callback();
  void fn_spell_add_to_dict();
  void fn_remove_from_dict();

  void fn_spell_suggest();

  void cmb_spellchecker_currentIndexChanged (const QString &text);

#ifdef HUNSPELL_ENABLE
  void pb_choose_hunspell_path_clicked();
#endif


#ifdef ASPELL_ENABLE

#if defined(Q_OS_WIN) || defined (Q_OS_OS2)
   void pb_choose_aspell_path_clicked();
#endif

#endif

#endif

  void scale_image();


  void search_mark_all();
  void search_unmark();


  void search_in_files_results_dclicked (QListWidgetItem *item);
  void search_in_files();

  void calendar_clicked (const QDate &date);
  void calendar_activated (const QDate &date);

  void mrkup_bold();
  void mrkup_italic();
  void mrkup_underline();
  void mrkup_link();
  void mrkup_para();
  void mrkup_preview_color();
  void mrkup_br();
  void mrkup_nbsp();
  void mrkup_tags_to_entities();
  void mrkup_mode_choosed();
  void mrkup_header();
  void mrkup_align();
  
  void mrkup_align_center();
  void mrkup_align_left();
  void mrkup_align_right();
  void mrkup_align_justify();
  
  
  void mrkup_document_weight();
  void markup_ins_image();
  void mrkup_color();
  void mrkup_text_to_html();

  void search_fuzzy_mode();
  void search_regexp_mode();
  void search_whole_words_mode();
  void search_from_cursor_mode();


  void search_find();
  void search_find_next();
  void search_find_prev();
  void search_replace_with();
  void search_replace_all();
  void search_replace_all_at_ofiles();

  void view_preview_in_bro();
  void view_hide_error_marks();
  void view_toggle_fs();
  void view_stay_on_top();

  void instr_font_gallery();
  
  
  void nav_goto_line();
  void nav_goto_right_tab();
  void nav_goto_left_tab();
  void nav_toggle_hs();
  void nav_save_pos();
  void nav_goto_pos();
  void nav_focus_to_fif();
  void nav_focus_to_editor();

  void help_show_gpl();
  void help_show_news();
  void help_show_changelog();
  void help_show_todo();

  void session_save_as();

  void main_tab_page_changed (int index);

  void fn_use_table();

  void calendar_currentPageChanged (int year, int month);
 //void add_user_font();


  void man_find_find();
  void man_find_next();
  void man_find_prev();


  void profile_save_as();
  void fman_mk_gallery();
  void indent_by_first_line();

  void process_readyReadStandardOutput();
  void fn_remove_by_regexp();


  void fman_naventry_confirm();
  void fman_img_conv_by_side();
  void fman_img_conv_by_percent();
  void fman_create_zip();
  void fman_add_to_zip();
  void fman_save_zip();
  void fman_preview_image();

  void set_eol_unix();
  void set_eol_win();
  void set_eol_mac();

  void fn_stat_words_lengths();

  
  void guess_enc();

  void cal_add_days();
  void cal_add_months();
  void cal_add_years();

  void recentoff();
  

/*************************
prefs window callbacks
*************************/

  void cmb_ui_langs_currentIndexChanged (const QString &text);

  void cb_altmenu_stateChanged (int state);

  void cmb_ui_tabs_currentIndexChanged (int i);
  void cmb_docs_tabs_currentIndexChanged (int i);


  void cmb_icon_sizes_currentIndexChanged (const QString &text);
  void cmb_tea_icons_currentIndexChanged  (const QString &text);

  void pb_assign_hotkey_clicked();
  void pb_remove_hotkey_clicked();

  void slot_lv_menuitems_currentItemChanged (QListWidgetItem *current, QListWidgetItem *previous);
  void slot_app_fontname_changed (const QString &text);
  void slot_editor_fontname_changed (const QString &text);
  void slot_font_size_changed (int i);

  void slot_app_font_size_changed (int i);
  void slot_style_currentIndexChanged (const QString &text);

  
#ifdef USE_QML_STUFF
  
  void fn_use_plugin();
#endif
  
public:

/*************************
main window widgets
*************************/

  QSplitter *mainSplitter;
  QTextBrowser *man;
 // QPlainTextEdit *log_memo;
  QString charset;

  QTabWidget *main_tab_widget;
  QTabWidget *tab_options;
  QTabWidget *tab_browser;
  QTabWidget *tab_widget;
  QLineEdit *fif;

  QMenu *fileMenu;
  QMenu *editMenu;
  QMenu *menu_cal;

  QMenu *menu_cal_add;
  QMenu *menu_cal_sub;
  QMenu *menu_cal_diff;


  QMenu *menu_file_edit_bookmarks;

  QMenu *menu_file_configs;
  QMenu *menu_file_sessions;
  QMenu *menu_file_actions;
  QMenu *menu_file_bookmarks;
  QMenu *menu_file_templates;
  QMenu *menu_programs;
  QMenu *menu_fn_snippets;
  QMenu *menu_fn_tables;
  QMenu *menu_view_palettes;
  QMenu *menu_view_hl;
  QMenu *menu_view_profiles;

  QMenu *menu_fn_sessions;
  QMenu *menu_fn_scripts;
  
  
#ifdef USE_QML_STUFF
  
  QMenu *menu_fn_plugins;
#endif

  QMenu *menu_view_themes;

  
  QMenu *menu_markup;
  QMenu *menu_functions;
  QMenu *menu_functions_case;

  QMenu *menu_file_recent;
  QMenu *menu_search;
  QMenu *menu_nav;
  QMenu *menu_instr;

  QMenu *menu_fm;
  QMenu *menu_fm_file_ops;
  QMenu *menu_fm_multi_rename;

  QMenu *menu_fm_file_infos;
  QMenu *menu_fm_img_conv;
  QMenu *menu_fm_zip;
  QMenu *menu_view;
  QMenu *menu_spell_langs;
  QMenu *helpMenu;

  QMenu *menu_labels;


  QToolBar *fileToolBar;
  QToolBar *editToolBar;
  QToolBar *filesToolBar;

  QAction *act_labels;

  QAction *act_test;
  QAction *filesAct;
  
  QAction *newAct;
  QAction *openAct;
  QAction *saveAct;
  QAction *saveAsAct;
  QAction *exitAct;
  QAction *cutAct;
  QAction *copyAct;
  QAction *closeAct;
  QAction *undoAct;
  QAction *redoAct;
  QAction *pasteAct;
  QAction *aboutAct;
  QAction *aboutQtAct;
  QAction *menu_find_whole_words;
  QAction *menu_find_from_cursor;
  
  QAction *menu_find_case;
  QAction *menu_find_regexp;
  QAction *menu_find_fuzzy;

  QAction *menu_recent_off;

  QWidget *w_right;
  QLineEdit *ed_fman_fname;
  QComboBox *cb_fman_codecs;
  QComboBox *cb_fman_drives;
  QComboBox *cmb_spellcheckers;
  QComboBox *cmb_styles;
  QComboBox *cmb_icon_size;

  
  CCalendarWidget *calendar;

/*************************
prefs window widgets
*************************/

  QLineEdit *ed_img_post_proc;

  QLineEdit *ed_date_format;
  QLineEdit *ed_time_format;

  QLineEdit *ed_side_size;
  QLineEdit *ed_link_options;
  QLineEdit *ed_cols_per_row;


  QLineEdit *ed_locale_override;
  QLineEdit *ed_img_viewer_override;
  
  QSpinBox *spb_cursor_blink_time;
  QSpinBox *spb_cursor_width;
  
  CShortcutEntry *ent_shtcut;
  QListWidget *lv_menuitems;

  QFontComboBox *cmb_font_name;
  QSpinBox *spb_font_size;

  QFontComboBox *cmb_app_font_name;
  QSpinBox *spb_app_font_size;

  QLineEdit *ed_fman_path;
  QListWidget *lv_places;
  QSplitter *spl_fman;

  QCheckBox *cb_altmenu;
  QCheckBox *cb_wasd;
  

  QCheckBox *cb_auto_img_preview;
  QCheckBox *cb_override_locale;

  QCheckBox *cb_override_img_viewer;

  QComboBox *cmb_cmdline_default_charset;
  
  
  QComboBox *cmb_zip_charset_in;
  QComboBox *cmb_zip_charset_out;

//main window functions

  QAction* add_to_menu (QMenu *menu,
                        const QString &caption,
                        const char *method,
                        const QString &shortkt = QString(),
                        const QString &iconpath = QString()
                        );


  void update_stylesheet (const QString &f);
  
  QString theme_dir;
  
  QIcon get_theme_icon (const QString &name);
  QString get_theme_icon_fname (const QString &name);

  void update_dyn_menus();
  void create_paths();
  
  void calendar_update();


#ifdef SPELLCHECK_ENABLE

  void setup_spellcheckers();
  void create_spellcheck_menu();

#endif

  void init_styles();

  void handle_args();

  void leaving_tune();

  void create_main_widget();
  void createActions();
  void createMenus();
  void createOptions();
  void createCalendar();

  void createManual();
  void updateFonts();
  void update_bookmarks();
  void update_templates();
  void update_snippets();
  void update_sessions();
  void update_palettes();
  void update_view_hls();
  
#ifdef USE_QML_STUFF
    
  void update_plugins();

#endif
  
  void update_themes();


  
  void update_hls (bool force = false);

  void update_tables();
  void update_scripts();
  void update_places_bookmarks();
  void update_programs();
  void update_logmemo_palette();

  void update_charsets();
  void update_profiles();


  void createToolBars();
  void createStatusBar();
  void readSettings();
  void writeSettings();

  void read_search_options();
  void write_search_options();

  void dragEnterEvent (QDragEnterEvent *event);
  void dropEvent (QDropEvent *event);
  void add_to_last_used_charsets (const QString &s);

  void createFman();
  void create_markup_hash();
  //void create_markup_hash2();

  void count_substring (bool use_regexp);

  void run_unitaz (int mode);
 
 QHash <QString, QString> load_eclipse_theme_xml (const QString &fname);

  void load_palette (const QString &fileName);

 
  QTextDocument::FindFlags get_search_options();

  void fman_convert_images (bool by_side, int value);
 
  Q_INVOKABLE QString fif_get_text();
  
  
  
  
  void fn_filter_delete_by_sep (bool mode);

  void fman_items_select_by_regexp (bool mode);

   
  void fman_find();
  void fman_find_next();
  void fman_find_prev();
 
  void opt_update_keyb();
  
  void opt_shortcuts_find();
  void opt_shortcuts_find_next();
  void opt_shortcuts_find_prev();


  void idx_tab_edit_activate();
  void idx_tab_calendar_activate();
  void idx_tab_tune_activate();
  void idx_tab_fman_activate();
  void idx_tab_learn_activate();

  
 // void load_userfonts();
  
//#if QT_VERSION >= 0x050000
#ifdef USE_QML_STUFF
  
  void plugins_init();
  void plugins_done();
  
#endif  

  void markup_text (const QString &mode);
  
};


#ifdef USE_QML_STUFF

class CQQuickWindow: public QQuickWindow
{
   Q_OBJECT
   
public:
 
 
 CQQuickWindow(QWindow * parent = 0): QQuickWindow (parent)
   {;}
 
 QString id;

protected:

  bool event (QEvent *event);

  
};

#endif

#endif
