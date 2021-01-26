/**************************************************************************
 *   2007-2021 by Peter Semiletov                            *
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


#ifndef TEA_H
#define TEA_H

#include <QListWidget>
#include <QComboBox>
#include <QProgressBar>
#include <QCheckBox>
#include <QSpinBox>
#include <QApplication>
#include <QCoreApplication>
#include <QSplitter>
#include <QTextBrowser>
#include <QTranslator>
#include <QProcess>
#include <QProxyStyle>


#ifdef USE_QML_STUFF
#include <QQmlEngine>
#include <QQuickWindow>
#endif


#ifdef PRINTER_ENABLE
#include <QPrinter>
#endif


#include "document.h"
#include "fman.h"
#include "calendar.h"
#include "shortcuts.h"
#include "img_viewer.h"


#include "spellchecker.h"



class MyProxyStyle: public QProxyStyle
{
public:

 static bool b_altmenu;
 static int cursor_blink_time;


  int styleHint (StyleHint hint, const QStyleOption *option = 0,
                 const QWidget *widget = 0, QStyleHintReturn *returnData = 0) const
                {
                 if (hint == QStyle::SH_ItemView_ActivateItemOnSingleClick)
                    return 0;

                 if (! b_altmenu && hint == QStyle::SH_MenuBar_AltKeyNavigation)
                    return 0;

                 return QProxyStyle::styleHint (hint, option, widget, returnData);
                }

   MyProxyStyle (QStyle *style = 0);
};


#if QT_VERSION >= 0x050000
class QStyleHints
{
public:

  int cursorFlashTime() const
      {
       return MyProxyStyle::cursor_blink_time;
      }
};
#endif


#ifdef USE_QML_STUFF
class CQQuickWindow: public QQuickWindow
{
Q_OBJECT

public:

  QString id;

  CQQuickWindow (QWindow *parent = 0): QQuickWindow (parent) {}
  ~CQQuickWindow() {}

protected:

  bool event (QEvent *event);
};


class CPluginListItem: public QObject
{
Q_OBJECT

public:

  QString id;
  CQQuickWindow *window;

  CPluginListItem (const QString &plid, CQQuickWindow *wnd);
};
#endif


class CFSizeFName: public QObject
{
Q_OBJECT

public:

  qint64 size;
  QString fname;

  CFSizeFName (qint64 sz, const QString &fn):
               size (sz),
               fname (fn) {}
};


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

public:

  QLabel *logo;

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
  ~CTextListWnd();

protected:

  void closeEvent (QCloseEvent *event);
};


class CTEA: public QMainWindow
{
Q_OBJECT

public:

  CTEA();
  ~CTEA();

/*
=========================
Variables
=========================
*/

#ifdef USE_QML_STUFF
  QQmlEngine *qml_engine;
#endif

  QString charset;

  QStringList sl_places_bmx;
  QStringList sl_urls;
  QStringList sl_charsets;
  QStringList sl_last_used_charsets;
  QStringList sl_fif_history;

  int fm_entry_mode;

  QDate date1;
  QDate date2;

  bool portable_mode;

  CLogMemo *log;
  QAction *last_action;

  CShortcuts *shortcuts;
  CFMan *fman;
  CImgViewer *img_viewer;

  CSpellchecker *spellchecker;
  QStringList spellcheckers;
  QString cur_spellchecker;

  QString theme_dir;

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
  QString fname_stylesheet;
  QString man_search_value;
  QString markup_mode;

  QHash <QString, CMarkupPair*> hs_markup;
  QHash <QString, QString> programs;
  QHash <QString, QString> places_bookmarks;
  QHash <int, QString> moon_phase_algos;

  QTranslator myappTranslator;
  QTranslator qtTranslator;

  QDir dir_lv;


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

  QString dir_plugins;


  QString dir_snippets;
  QString dir_scripts;
  QString dir_palettes;
  QString dir_tables;
  QString dir_days;

  QString fname_def_palette;


  QString fname_fif;

  QString fname_bookmarks;
  QString fname_programs;
  QString fname_places_bookmarks;

  QString bookmarks;
  QString fname_crapbook;
  QString fname_tempfile;
  QString fname_tempparamfile;




/*
===========================
Main menu items
===========================
*/

  QMenu *menu_file;

  QMenu *menu_file_actions;
  QMenu *menu_file_recent;

  QMenu *menu_file_bookmarks;
  QMenu *menu_file_edit_bookmarks;

  QMenu *menu_file_templates;
  QMenu *menu_file_sessions;
  QMenu *menu_file_configs;
//  QAction *menu_recent_off;


  QMenu *menu_edit;

  QMenu *menu_ide;


  QMenu *menu_cal;

  QMenu *menu_cal_add;
  QMenu *menu_cal_sub;
  QMenu *menu_cal_diff;


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

  QMenu *menu_search;
  QMenu *menu_nav;
  QMenu *menu_instr;

  QMenu *menu_fm;
  QMenu *menu_fm_file_ops;
  QMenu *menu_fm_multi_rename;

  QMenu *menu_fm_file_infos;
  QMenu *menu_fm_img_conv;
  QMenu *menu_fm_zip;
  QMenu *menu_fm_checksums;

  QMenu *menu_view;
  QMenu *menu_spell_langs;
  QMenu *helpMenu;

  QMenu *menu_labels;


  QToolBar *fileToolBar;
  QToolBar *editToolBar;
  QToolBar *filesToolBar;
  QToolBar *statusToolBar;
  QToolBar *fifToolBar;

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







/*
=========================
Main window widgets
=========================
*/

  QSplitter *mainSplitter;
  QTextBrowser *man;
  QTabWidget *main_tab_widget;
  QTabWidget *tab_options;
  QTabWidget *tab_browser;
  QTabWidget *tab_editor;
  QLineEdit *fif;
  QWidget *w_right;
  QLineEdit *ed_fman_fname;
  QComboBox *cb_fman_codecs;
  QComboBox *cb_fman_drives;
  CCalendarWidget *calendar;


/*
==============================
Main tab UI elements
==============================
*/

  QComboBox *cmb_fif;
  QLabel *l_status;
  QProgressBar *pb_status;

/*
==============================
FileManager tab UI elements
==============================
*/


  QLineEdit *ed_fman_path;
  QListWidget *lv_places;
  QSplitter *spl_fman;

  QToolBar *tb_fman_dir;
  QLabel *l_fman_preview;
  QLabel *l_charset;


/*
=============================================
Preferences tab :: Interface page UI elements
=============================================
*/


  QComboBox *cmb_ui_mode;
  QComboBox *cmb_lng;
  QComboBox *cmb_styles;
 
  QCheckBox *cb_fif_at_toolbar;
  QComboBox *cmb_icon_size;
  QComboBox *cmb_tea_icons;

  QCheckBox *cb_show_linenums;
  QCheckBox *cb_wordwrap;
  QCheckBox *cb_hl_enabled;
  QCheckBox *cb_hl_current_line;
  QCheckBox *cb_hl_brackets;
  QCheckBox *cb_auto_indent;
  QCheckBox *cb_spaces_instead_of_tabs;
  QSpinBox *spb_tab_sp_width;
  QCheckBox *cb_cursor_xy_visible;
  QCheckBox *cb_center_on_cursor;
  QSpinBox *spb_cursor_blink_time;
  QSpinBox *spb_cursor_width;
  QCheckBox *cb_show_margin;
  QSpinBox *spb_margin_pos;
  QCheckBox *cb_use_hl_wrap;
  QCheckBox *cb_full_path_at_window_title;


/*
=============================================
Preferences tab :: Common page UI elements
=============================================
*/


  QCheckBox *cb_start_on_sunday;
  QCheckBox *cb_northern_hemisphere;
  QComboBox *cmb_moon_phase_algos;
  QComboBox *cmb_cmdline_default_charset;

  QComboBox *cmb_zip_charset_in;
  QComboBox *cmb_zip_charset_out;

  QCheckBox *cb_altmenu;
  QCheckBox *cb_wasd;

#if defined(JOYSTICK_SUPPORTED)
  QCheckBox *cb_use_joystick;
#endif

  QCheckBox *cb_auto_img_preview;
  QCheckBox *cb_session_restore;
  QCheckBox *cb_use_trad_dialogs;
  QCheckBox *cb_use_enca_for_charset_detection;
  QCheckBox *cb_override_img_viewer;
  QLineEdit *ed_img_viewer_override;



/*
=============================================
Preferences tab :: Functions page UI elements
=============================================
*/

  QLineEdit *ed_label_end;
  QLineEdit *ed_label_start;

  QLineEdit *ed_date_format;
  QLineEdit *ed_time_format;



#if defined (HUNSPELL_ENABLE) || defined (ASPELL_ENABLE)

  QComboBox *cmb_spellcheckers;
  void cmb_spellchecker_currentIndexChanged (int);

#endif // SPELLCHECKERS ENABLED


  QLineEdit *ed_spellcheck_path;
  QLineEdit *ed_aspellcheck_path;

  QSpinBox *spb_fuzzy_q;


 


/*
=============================================
Preferences tab :: Images page UI elements
=============================================
*/

  QComboBox *cmb_output_image_fmt;
  QCheckBox *cb_output_image_flt;
  QSpinBox *spb_img_quality;
  QCheckBox *cb_zip_after_scale;
  QCheckBox *cb_exif_rotate;
  QLineEdit *ed_side_size;
  QLineEdit *ed_link_options;
  QLineEdit *ed_cols_per_row;
  QCheckBox *cb_zor_use_exif;

/*
=============================================
Preferences tab :: Keyboard page UI elements
=============================================
*/

  CShortcutEntry *ent_shtcut;
  QListWidget *lv_menuitems;






/*
====================================
Application stuff inits and updates
====================================
*/


public:

  void init_styles();
  void update_dyn_menus();
  void create_paths();
  void calendar_update();


#ifdef USE_QML_STUFF
  void plugins_init();
  void plugins_done();
#endif


#if defined (HUNSPELL_ENABLE) || defined (ASPELL_ENABLE)
  void setup_spellcheckers();
  void create_spellcheck_menu();
#endif

  void handle_args();
  void create_moon_phase_algos();


  void update_labels_menu();

  void opt_update_keyb();

  void createFman();
  void create_markup_hash();

  void create_main_widget_splitter();
  void create_main_widget_docked();

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


  void update_hls_noncached();

  void update_tables();
  void update_scripts();
  void update_places_bookmarks();
  void update_programs();
  void update_logmemo_palette();

  void update_charsets();
  void update_profiles();


  void createToolBars();
  void readSettings();
  void writeSettings();

  void read_search_options();
  void write_search_options();


/*
===========================
Application misc. methods
===========================
*/


  QHash <QString, QString> load_eclipse_theme_xml (const QString &fname);
  void load_palette (const QString &fileName);
  void fman_convert_images (bool by_side, int value);
  QTextDocument::FindFlags get_search_options();
  Q_INVOKABLE QString fif_get_text();

  QAction* add_to_menu (QMenu *menu,
                        const QString &caption,
                        const char *method,
                        const QString &shortkt = QString(),
                        const QString &iconpath = QString()
                        );


  void update_stylesheet (const QString &f);

  QIcon get_theme_icon (const QString &name);
  QString get_theme_icon_fname (const QString &name);

  void leaving_tune();
  void add_to_last_used_charsets (const QString &s);
  void count_substring (bool use_regexp);
  void run_unitaz (int mode);
  void markup_text (const QString &mode);
  void fman_items_select_by_regexp (bool mode);
  void fn_filter_delete_by_sep (bool mode);

  void fman_find();
  void fman_find_next();
  void fman_find_prev();

  void opt_shortcuts_find();
  void opt_shortcuts_find_next();
  void opt_shortcuts_find_prev();

  void idx_tab_edit_activate();
  void idx_tab_calendar_activate();
  void idx_tab_tune_activate();
  void idx_tab_fman_activate();
  void idx_tab_learn_activate();

protected:

  void closeEvent (QCloseEvent *event);
  bool fman_tv_eventFilter (QObject *obj, QEvent *event);
  void dragEnterEvent (QDragEnterEvent *event);
  void dropEvent (QDropEvent *event);




public slots:


/*
===========================
Main window slots
===========================
*/


  void pageChanged (int index);
  void logmemo_double_click (const QString &txt);
  void receiveMessage (const QString &msg);
  void receiveMessageShared (const QStringList& msg);
  void update_labels_list();


/*
===================
File manager slots
===================
*/

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





/*
===================
Main menu callbacks
===================
*/


/*
===================
File menu callbacks
===================
*/

  void test();

  void file_new();
  void file_open();
  void file_open_at_cursor();
  void file_last_opened();

  void file_crapbook();
  void file_notes();

  bool file_save();
  bool file_save_as();

  void file_save_bak();
  void file_save_version();

  void file_reload();
  void file_reload_enc_itemDoubleClicked (QListWidgetItem *item);
  void file_reload_enc();

  void file_set_eol_unix();
  void file_set_eol_win();
  void file_set_eol_mac();


#ifdef PRINTER_ENABLE
  void file_print();
#endif


  void menu_file_recent_off();

  void file_add_to_bookmarks();
  void file_find_obsolete_paths();


  void file_open_bookmarks_file();
  void file_open_programs_file();
  void file_open_bookmark();

  void file_use_template();
  void file_open_session();



  void file_close();


/*
===================
Edit menu callbacks
===================
*/


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



/*
===================
Markup menu callbacks
===================
*/





/*
===================
Search menu callbacks
===================
*/




/*
===================
Fn menu callbacks
===================
*/


  void fn_use_snippet();
  void fn_count();
  void fn_count_rx();

  void fn_enum();

  void fn_filter_by_repetitions();

  void fn_filter_with_regexp();
  void fn_text_stat();
  void fn_antispam_email();
  void fn_apply_to_each_line();
  void fn_reverse();
  void fn_escape();

  void fn_evaluate();
  void fn_sort_length();
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
  void fn_unitaz_len();

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

#if defined (HUNSPELL_ENABLE) || defined (ASPELL_ENABLE)

  void fn_change_spell_lang();
  void fn_spell_check();
  void fn_spell_suggest_callback();
  void fn_spell_add_to_dict();
  void fn_remove_from_dict();
  void fn_spell_suggest();


  void fn_sum_by_last_col();
  void fn_anagram();

  void fn_insert_cpp();
  void fn_insert_c();
  void fn_insert_template_html5();
  void fn_insert_template_tea();

  void cmb_spellchecker_currentIndexChanged (int);


#endif // SPELLCHECKERS ENABLED

#ifdef HUNSPELL_ENABLE
  void pb_choose_hunspell_path_clicked();
#endif

#ifdef ASPELL_ENABLE
  void pb_choose_aspell_path_clicked();
#endif


/*
===================
Run menu callbacks
===================
*/



/*
===================
IDE menu callbacks
===================
*/

  void ide_run();
  void ide_build();
  void ide_clean();



/*
===================
Nav menu callbacks
===================
*/




/*
===================
View menu callbacks
===================
*/

  void view_use_profile();
  void view_use_palette();
  void view_use_hl();
  void view_use_theme();


  void rename_selected();




#ifdef USE_QML_STUFF
  void fn_use_plugin();
#endif

  

  void repeat();


  void select_label();

  void fm_full_info();

  void fm_hashsum_md5();
  void fm_hashsum_md4();
  void fm_hashsum_sha1();


#if QT_VERSION >= 0x050000

  void fm_hashsum_sha224();
  void fm_hashsum_sha384();
  void fm_hashsum_sha256();
  void fm_hashsum_sha512();

#endif



  void fm_hashsum_sha3_224();
  void fm_hashsum_sha3_256();
  void fm_hashsum_sha3_384();
  void fm_hashsum_sha3_512();



  void fm_hashsum_keccak_256();
  void fm_hashsum_keccak_224();
  void fm_hashsum_keccak_384();
  void fm_hashsum_keccak_512();


  void fman_unpack_zip();
  void fman_zip_info();

  void fman_refresh();
  void fman_rename();
  void fman_delete();
  void fman_zeropad();
  void fman_del_n_first_chars();
  void fman_multreplace();
  void fman_apply_template();





  void clipboard_dataChanged();


  void cal_set_date_a();
  void cal_set_date_b();
  void cal_set_to_current();
  void cal_moon_mode();
  void cal_gen_mooncal();
  void cal_diff_days();


  void cal_remove();


  void toggle_wrap();
  void darker();

  void about();


  void set_as_storage_file();
  void copy_to_storage_file();
  void capture_clipboard_to_storage_file();

  void upCase();
  void dnCase();

  void edit_copy_current_fname();




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

  void view_hide_error_marks();
  void view_toggle_fs();
  void view_stay_on_top();


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
  void calendar_currentPageChanged (int year, int month);

  void man_find_find();
  void man_find_next();
  void man_find_prev();

  void profile_save_as();
  void fman_mk_gallery();
  void indent_by_first_line();

  void process_readyReadStandardOutput();
  void fn_remove_by_regexp();
  void fn_use_table();

  void fman_naventry_confirm();
  void fman_img_conv_by_side();
  void fman_img_conv_by_percent();
  void fman_create_zip();
  void fman_add_to_zip();
  void fman_save_zip();
  void fman_preview_image();


  void fn_stat_words_lengths();

  void guess_enc();

  void cal_add_days();
  void cal_add_months();
  void cal_add_years();

  void run_program();


/*
====================
Tune page callbacks
====================
*/

#if defined(JOYSTICK_SUPPORTED)
  void cb_use_joystick_stateChanged (int state);
#endif


  void cb_altmenu_stateChanged (int state);

  void cmb_ui_tabs_currentIndexChanged (int i);
  void cmb_docs_tabs_currentIndexChanged (int i);
  void cmb_icon_sizes_currentIndexChanged (int i);
  void cmb_tea_icons_currentIndexChanged  (int i);


  void pb_assign_hotkey_clicked();
  void pb_remove_hotkey_clicked();

  void slot_lv_menuitems_currentItemChanged (QListWidgetItem *current, QListWidgetItem *previous);
  

  void slot_font_logmemo_select();
  void slot_font_interface_select();
  void slot_font_editor_select();


  void slot_style_currentIndexChanged (int);

};


#endif