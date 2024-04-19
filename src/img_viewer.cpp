/*
this code is Public Domain
*/


#include <QDebug>
#include <QApplication>
#include <QPainter>


#include <QVBoxLayout>
#include <QCryptographicHash>
#include <QDir>
#include <QImageReader>
#include <QKeyEvent>
#include <QSettings>

#include "img_viewer.h"
#include "exif_reader.h"

#include "utils.h"

extern QSettings *settings;


CImgViewer::CImgViewer (QObject *parent): QObject (parent)
{
  window_mini.setWindowFlags (Qt::Tool);
  window_mini.resize (200, 200);
  img_mini = new QLabel (tr ("preview"));
  img_mini->setAlignment (Qt::AlignCenter);
  QVBoxLayout *lt = new QVBoxLayout;
  lt->addWidget (img_mini);
  window_mini.setLayout (lt);
}


void CImgViewer::set_image_mini (const QString &fname)
{
  window_mini.resize (200, 200);
  window_mini.setWindowTitle (QFileInfo (fname).fileName());

  QString fn = get_the_thumb_name (fname);
  if (! fn.isEmpty())
     {
      QPixmap pm (fn);
      img_mini->setPixmap (pm);
     }
  else 
      {
       QPixmap pm (fname);
       if ((pm.width() > (window_mini.width() - 10)) ||
           (pm.height() > (window_mini.height() - 10)))
          img_mini->setPixmap (pm.scaled (190, 190, Qt::KeepAspectRatio));
       else
           img_mini->setPixmap (pm);
      }
}


void CImgViewer::set_image_full (const QString &fname)
{
  window_full.load_image (fname);
}


QString CImgViewer::get_the_thumb_name (const QString &img_fname)
{

#if !defined(Q_OS_WIN) || !defined(Q_OS_OS2)

  QCryptographicHash h (QCryptographicHash::Md5); 

  QString uri (img_fname);
  uri.prepend ("file://");
  h.addData (uri.toUtf8());
  QString digest = h.result().toHex();

  QString fname (QDir::homePath());
  fname.append ("/.thumbnails/large");
  fname.append ("/").append (digest).append (".png");

  if (file_exists (fname))
     return fname;

  fname.clear();
  fname.append (QDir::homePath());
  fname.append ("/.thumbnails/normal");
  fname.append ("/").append (digest).append (".png");

  if (file_exists (fname))
     return fname;

#endif

  return QString();
}


void CZORWindow::closeEvent (QCloseEvent *event)
{
  event->accept();
}


CZORWindow::CZORWindow (QWidget *parent): QWidget (parent)
{
  setMinimumSize (16, 16);
}


CZORWindow::~CZORWindow()
{
}


void CZORWindow::paintEvent (QPaintEvent *event)
{
  QPainter painter (this);
  painter.drawImage (0, 0, source_image);
}


void CZORWindow::load_image (const QString &fname)
{
  if (! file_exists (fname))
     return;
 
  if (! source_image.load (fname))
     return;

  fname_image = fname;
  
  bool orientation_portrait = false; 
  int exif_orientation = get_exif_orientation (fname);

  if (settings->value ("zor_use_exif_orientation", 0).toInt())
      if (exif_orientation == 6 || exif_orientation == 8)
          orientation_portrait = true;
  
  bool need_to_scale = false;
  
  if (source_image.size().height() > source_image.size().height())
     orientation_portrait = true;
  
  if (source_image.size().height() > 600 || source_image.size().width() > 800)
     need_to_scale = true;
  
  if (exif_orientation == 3)
     {
      QTransform transform;
      
      transform.rotate (180);
      
      QImage transformed_image = source_image.transformed (transform);
      transformed_image = transformed_image.scaled (size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
      source_image = transformed_image;
     }
  
  if (orientation_portrait)
     {
      QTransform transform;
      
      qreal angle = 0.0;
      
      if (exif_orientation == 6)
         angle = 90;
      else
      if (exif_orientation == 8) //make clockwise
         angle = 270;
         
      transform.rotate (angle);
      
      QImage transformed_image = source_image.transformed (transform);
      transformed_image = transformed_image.scaled (size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
      source_image = transformed_image;
     }
  
  if (need_to_scale)
     {  
      if (orientation_portrait)
         source_image = source_image.scaled (QSize (600, 800), Qt::KeepAspectRatio, Qt::SmoothTransformation);
      else
         source_image = source_image.scaled (QSize (800, 600), Qt::KeepAspectRatio, Qt::SmoothTransformation);
     }
  
  resize (source_image.size());
    
  QString s_wnd_title (QFileInfo (fname_image).fileName());
  s_wnd_title.append (" ");
  s_wnd_title.append (tr ("scaled to: "));
  s_wnd_title.append (QString ("%1x%2").arg (source_image.width()).arg(source_image.height()));
  setWindowTitle (s_wnd_title);

  update();
  raise();
  activateWindow();
}


void CZORWindow::keyPressEvent (QKeyEvent * event)
{
  if (event->key() == Qt::Key_Escape)
     {
      event->accept();
      close();
     }

  event->accept();
}


void CGIFWindow::keyPressEvent (QKeyEvent * event)
{
  if (event->key() == Qt::Key_Escape)
     {
      event->accept();
      close();
     }

  event->accept();
}


void CGIFWindow::load_image (const QString &fname)
{
  if (! file_exists (fname))
     return;
     
  if (movie)
     delete movie;   
     
  movie = new QMovie (fname);
  setMovie (movie);
  
  movie->jumpToFrame (0);  
  resize (movie->currentImage().size()); 
  
  show();
  
  movie->start(); 
}


CGIFWindow::CGIFWindow (QWidget *parent): QLabel (parent)
{
  movie = 0;
}


CGIFWindow::~CGIFWindow()
{
  if (movie)
    delete movie;
}


void CGIFWindow::closeEvent (QCloseEvent *event)
{
  event->accept();
}
