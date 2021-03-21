#ifndef IMG_VIEWER_H
#define IMG_VIEWER_H

#include <QObject>
#include <QWidget>
#include <QFileInfoList>
#include <QLabel>
#include <QMovie>


class CGIFWindow: public QLabel
{
Q_OBJECT
    
public:

  QMovie *movie;
  
  CGIFWindow (QWidget *parent = 0);
  ~CGIFWindow();

  void load_image (const QString &fname);

  void keyPressEvent ( QKeyEvent * event);
  void closeEvent (QCloseEvent *event);
};


class CZORWindow : public QWidget
{
Q_OBJECT
    
public:

  QString fname_image;
  QImage source_image;

  CZORWindow (QWidget *parent = 0);
  ~CZORWindow();

  void load_image (const QString &fname);

  void paintEvent (QPaintEvent *event);
  void keyPressEvent (QKeyEvent *event);
  void closeEvent (QCloseEvent *event);
};


class CImgViewer: public QObject
{
Q_OBJECT

public:

  QWidget window_mini; 
  CZORWindow window_full;
  QLabel *img_mini;

  CImgViewer (QObject *parent = 0);
  void set_image_mini (const QString &fname);
  void set_image_full (const QString &fname);
  QString get_the_thumb_name (const QString &img_fname);
};

#endif // IMG_VIEWER_H

