/*
this code is Public Domain
*/


#include <math.h>

#include <QDateTime>
#include <QDate>
#include <QPainter>
#include <QTextCharFormat>
#include <QDebug>

//#include "utils.h"
#include "calendar.h"


inline int tropical_year (int year)
{
  return 1 + (year % 19);
}


int moon_phase_leueshkanov (int year, int month, int day)
{
  int L = tropical_year (year);
  int phase = L * 11 - 14 + day + month + 3;

  return phase % 30;
}


/*
moon phase - based on Ben Daglish JavaScript code from
based on http://www.ben-daglish.net/moon.shtml
*/

int moon_phase_trig2 (int year, int month, int day)
{
  double n = floor (12.37 * (year - 1900 + ((1.0 * month - 0.5) / 12.0)));
  double RAD = 3.14159265 / 180.0;
  double t = n / 1236.85;
  double t2 = t * t;
  double as = 359.2242 + 29.105356 * n;
  double am = 306.0253 + 385.816918 * n + 0.010730 * t2;
  double xtra = 0.75933 + 1.53058868 * n + ((1.178e-4) - (1.55e-7) * t) * t2;
  xtra += (0.1734 - 3.93e-4 * t) * sin (RAD * as) - 0.4068 * sin (RAD * am);

  double i = (xtra > 0.0 ? floor (xtra) : ceil (xtra - 1.0));
  QDate d (year, month, day);
  int j1 = d.toJulianDay();
  int jd = (2415020 + 28 * n) + i;

  int r = (j1 - jd + 30) % 30;
  if (r == 0)
     r = 30;

  return r;
}


/*This simply mods the difference between the date and a known
new moon date (1970-01-07) by the length of the lunar period.
For this reason, it is only valid from 1970 onwards.*/

int moon_phase_simple (int year, int month, int day)
{
  int lp = 2551443;
  QDateTime now (QDate (year, month - 1, day), QTime (20, 35, 0));
  QDateTime new_moon (QDate (1970, 0, 7), QTime (20, 35, 0));
  double phase = ((now.toMSecsSinceEpoch() - new_moon.toMSecsSinceEpoch()) / 1000) % lp;
  return floor (phase / (24 * 3600)) + 1;
}


/*
Conway
This is based on a 'do it in your head' algorithm by John Conway.
In its current form, it's only valid for the 20th and 21st centuries,
but I'm sure John's got refinements. :)
*/

int moon_phase_conway (int year, int month, int day)
{
  int r = year % 100;
  r %= 19;

  if (r > 9)
     r -= 19;

  r = ((r * 11) % 30) + month + day;

  if (month < 3)
     r += 2;

  r -= ((year < 2000) ? 4 : 8.3);
  r = (int) floor (r + 0.5) % 30;

  return (r < 0) ? r + 30 : r;
}


/*Trig1
This is based on some Basic code by Roger W. Sinnot from Sky & Telescope magazine, March 1985.
I don't pretend to understand it - something to do with a first-order approximation
 to the 'real' calculation of the position of the bodies involved - which I'm still working on... :)
*/

inline double GetFrac (double fr)
{
  return (fr - floor (fr));
}


int moon_phase_trig1 (int year, int month, int day)
{
  QDate d (year, month, day);

  int thisJD = d.toJulianDay();
  double degToRad = 3.14159265 / 180;
  double K0, T, T2, T3, J0, F0, M0, M1, B1;
  int oldJ = 0;

  K0 = floor ((year - 1900) * 12.3685);
  T = (year - 1899.5) / 100;
  T2 = T * T;
  T3 = T * T * T;
  J0 = 2415020 + 29*K0;
  F0 = 0.0001178 * T2 - 0.000000155 * T3 + (0.75933 + 0.53058868 * K0) - (0.000837 * T + 0.000335 * T2);
  M0 = 360 * (GetFrac (K0 * 0.08084821133)) + 359.2242 - 0.0000333  *T2 - 0.00000347 * T3;
  M1 = 360 * (GetFrac (K0 * 0.07171366128)) + 306.0253 + 0.0107306 * T2 + 0.00001236 * T3;
  B1 = 360 * (GetFrac (K0 * 0.08519585128)) + 21.2964 - (0.0016528 * T2) - (0.00000239 * T3);

  int phase = 0;
  int jday = 0;

  while (jday < thisJD)
        {
         double F = F0 + 1.530588 * phase;
         double M5 = (M0 + phase * 29.10535608) * degToRad;
         double M6 = (M1 + phase * 385.81691806) * degToRad;
         double B6 = (B1 + phase * 390.67050646) * degToRad;
         F -= 0.4068 * sin (M6) + (0.1734 - 0.000393 * T) * sin (M5);
         F += 0.0161 * sin (2 * M6) + 0.0104 * sin (2 * B6);
         F -= 0.0074 * sin (M5 - M6) - 0.0051 * sin (M5 + M6);
         F += 0.0021 * sin (2 * M5) + 0.0010 * sin (2 * B6 - M6);
         F += 0.5 / 1440;
         oldJ = jday;
         jday = J0 + 28 * phase + floor(F);
         phase++;
        }

  return (thisJD - oldJ) % 30;
}


int moon_phase_by_algo (int v, int year, int month, int day)
{
  int r = 0;

  switch (v)
         {
          case MOON_PHASE_TRIG2:
                                r = moon_phase_trig2 (year, month, day);
                                break;

          case MOON_PHASE_TRIG1:
                                r = moon_phase_trig1 (year, month, day);
                                break;

          case MOON_PHASE_CONWAY:
                                 r = moon_phase_conway (year, month, day);
                                 break;

          case MOON_PHASE_LEUESHKANOV:
                                      r = moon_phase_leueshkanov (year, month, day);
                                      break;

          case MOON_PHASE_SIMPLE:
                                 r = moon_phase_simple (year, month, day);
                                 break;
         };

  return r;
}


CCalendarWidget::CCalendarWidget (QWidget *parent, const QString &a_dir_days): QCalendarWidget (parent)
{
  dir_days = a_dir_days;
  moon_phase_algo = MOON_PHASE_TRIG2;
  moon_mode = false;
  if (! moon_tiles.load (":/images/moon-phases.png"))
     qDebug() << ":/images/moon-phases.png" << " is not loaded";

  northern_hemisphere = true;
  //setHeaderTextFormat (const QTextCharFormat & format);

  QTextCharFormat tformat;
  tformat.setForeground (QBrush (Qt::white));
  tformat.setBackground (QBrush (Qt::black));
  tformat.setFontWeight (QFont::Bold);

  setWeekdayTextFormat (Qt::Monday, tformat);
  setWeekdayTextFormat (Qt::Tuesday, tformat);
  setWeekdayTextFormat (Qt::Wednesday, tformat);
  setWeekdayTextFormat (Qt::Thursday, tformat);
  setWeekdayTextFormat (Qt::Friday, tformat);
  setWeekdayTextFormat (Qt::Saturday, tformat);
  setWeekdayTextFormat (Qt::Sunday, tformat);
}


#if QT_VERSION < 0x060000
void CCalendarWidget::paintCell (QPainter *painter, const QRect &rect, const QDate &date) const
#else
void CCalendarWidget::paintCell (QPainter *painter, const QRect &rect, QDate date) const
#endif
{
  QSize fsize = fontMetrics().size (Qt::TextSingleLine, "A");

  if (moon_mode)
     {
      int moon_day = moon_phase_by_algo (moon_phase_algo, date.year(), date.month(), date.day());

      bool has_image = true;

      if (moon_day == 0 || moon_day == 30 || moon_day == 1)
         has_image = false;

      //вычисляем ряд и колонку
      int cursorOffset = moon_day;

/*
      int off = 0;
      int row = 0;

      while (cursorOffset >= (off + 8))
            {
             off += 7;
             row++;
            }

      int col = cursorOffset - off;
*/

      int row = moon_day / 7;

      if ((moon_day % 7 == 0) && (row != 0))
          row--;

      int col = cursorOffset - (row * 7);

/*
    qDebug() << "moon day = " << moon_day;
    qDebug() << "moon_day / 7 = " << (double) moon_day / 7;
    qDebug() << "trow = " << trow;
    qDebug() << "row = " << row;

    qDebug() << "col = " << col;
    qDebug() << "tcol = " << tcol;
*/

    //вычисляем, откуда копировать

      int pad = 3;
      int x = (col - 1) * 73 + (pad * col) - pad;
      int y = row * 73 + (pad * row);

      QRect r (x, y, 66, 73);
      QImage tile = moon_tiles.copy (r);
      QColor bg_color (Qt::black);

      painter->fillRect (rect, bg_color);

      if (has_image)
         {
          if (northern_hemisphere)
             painter->drawImage (rect.x(), rect.y(), tile);
          else
              painter->drawImage (rect.x(), rect.y(), tile.mirrored (true, false));
         }

      painter->setPen (QPen (Qt::yellow));

      QTextCharFormat tcf = dateTextFormat (date);

      if (tcf.fontStrikeOut())
         painter->setPen (QPen (Qt::magenta));
      else
          if (tcf.fontUnderline())
              painter->setPen (QPen (Qt::red));

      painter->drawText (QPoint (rect.x() + 5, rect.y() + fsize.height()), date.toString("dd") + " / " + QString::number (moon_day));

      if (selectedDate() == date)
         {
          QPen dpen (Qt::yellow);
          dpen.setWidth (5);
          painter->setPen (dpen);
          painter->drawRect (rect);
         }
      }
 else
     QCalendarWidget::paintCell (painter, rect, date);
}


void CCalendarWidget::do_update()
{
  updateCells();
}
