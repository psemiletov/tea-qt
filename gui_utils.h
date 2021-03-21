#ifndef GUI_UTILS_H
#define GUI_UTILS_H

#include <QObject>
#include <QMenu>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QBoxLayout>
#include <QDir>


void create_menu_from_list (QObject *handler,
                            QMenu *menu,
                            const QStringList &list,
                            const char *method
                           );


void create_menu_from_themes (QObject *handler,
                              QMenu *menu,
                              const QString &dir,
                              const char *method
                              );

void create_menu_from_dir (QObject *handler,
                           QMenu *menu,
                           const QString &dir,
                           const char *method
                           );

QImage image_scale_by (const QImage &source,
                       bool by_side,
                       int value,
                       Qt::TransformationMode mode);

QLineEdit* new_line_edit (QBoxLayout *layout, const QString &label, const QString &def_value);

QSpinBox* new_spin_box (QBoxLayout *layout, const QString &label, int min, int max, int value, int step = 1);

QComboBox* new_combobox (QBoxLayout *layout,
                         const QString &label,
                         const QStringList &items,
                         const QString &def_value);

QComboBox* new_combobox (QBoxLayout *layout,
                         const QString &label,
                         const QStringList &items,
                         int index);

#endif // GUI_UTILS_H

