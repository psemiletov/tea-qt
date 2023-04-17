#ifndef CALENDAR_H
#define CALENDAR_H

#include <QCalendarWidget>


class CCalendarWidget: public QCalendarWidget
{
Q_OBJECT

protected:

#if QT_VERSION < 0x060000
  void paintCell (QPainter *painter, const QRect &rect, const QDate &date) const;
#else
  void paintCell (QPainter *painter, const QRect &rect, QDate date) const;
#endif

public:

  QImage moon_tiles;
  QString dir_days;
  bool moon_mode;
  bool northern_hemisphere;
  int moon_phase_algo;

  CCalendarWidget (QWidget *parent, const QString &a_dir_days);
  void do_update();
};


enum
{
  MOON_PHASE_TRIG2 = 0,
  MOON_PHASE_TRIG1,
  MOON_PHASE_CONWAY,
  MOON_PHASE_LEUESHKANOV,
  MOON_PHASE_SIMPLE
};


int moon_phase_by_algo (int v, int year, int month, int day);
int moon_phase_trig2 (int year, int month, int day);
int moon_phase_simple (int year, int month, int day);
int moon_phase_conway (int year, int month, int day);
int moon_phase_trig1 (int year, int month, int day);
int moon_phase_leueshkanov (int year, int month, int day);

#endif // CALENDAR_H
