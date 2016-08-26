#ifndef TEXTPROC_H
#define TEXTPROC_H

enum
    {
     QSTRL_PROC_REMOVE_FORMATTING = 0,
     QSTRL_PROC_APPLY_2_EACH_LINE,
     QSTRL_PROC_FLT_WITH_REGEXP,
     QSTRL_PROC_FLT_WITH_SORTCASECARE,
     QSTRL_PROC_LIST_FLIP,
     QSTRL_PROC_FLT_LESS,
     QSTRL_PROC_FLT_GREATER,
     QSTRL_PROC_FLT_REMOVE_DUPS,
     QSTRL_PROC_FLT_REMOVE_EMPTY,
     QSTRL_PROC_FLT_WITH_SORTNOCASECARE,
     QSTRL_PROC_FLT_WITH_SORTCASECARE_SEP,
     QSTRL_PROC_LIST_FLIP_SEP,
     
    };


int str_fuzzy_search (const QString &s, const QString &text_to_find, int start_pos, double q);
QString apply_table (const QString &s, const QString &fname, bool use_regexp);
QString strip_html (const QString &text);
int romanToDecimal (const char *roman);
QString arabicToRoman (int i);
QString qstringlist_process (const QString &s, const QString &params, int mode);
QString string_reverse (const QString &s);
QString int_to_binary (int n);
unsigned int bin_to_decimal (const QString &s);
QString conv_quotes (const QString &source, const QString &c1, const QString &c2);
QStringList html_get_by_patt (const QString &s, const QString &spatt);


#endif // TEXTPROC_H
