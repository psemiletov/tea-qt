/*
this code is Public Domain
*/



#include <QDebug>

#include <QApplication>
#include <QPainter>

#include <QDesktopWidget>

#include <QVBoxLayout>
#include <QCryptographicHash>
#include <QDir>
#include <QImageReader>
#include <QKeyEvent>

#include <QSettings>

#include "img_viewer.h"

#include "exif.h"
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
 // window_full.show_image (fname);
  window_full.load_image (fname);
  //qDebug() << "get_exif_orintation " << get_exif_orintation (fname);
}


QString CImgViewer::get_the_thumb_name (const QString &img_fname)
{
 
//FIXME: add OS2 tweak!
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

/*
CViewerWindow::CViewerWindow (QWidget *parent): QWidget (parent)
{
  current_index = 0; 
  angle = 0.0;
  scale = 100;
  
  img_full = new QLabel (tr ("preview"));
  img_full->setAlignment (Qt::AlignCenter);
  QVBoxLayout *lt = new QVBoxLayout;
  lt->addWidget (img_full);
  setLayout (lt);
}


void CViewerWindow::show_image (const QString &fname)
{
  if (fname.isEmpty())
     return;
  
  if (! file_exists (fname))
      return; 
  
  QPixmap pm (fname);

  if (file_name != fname)
     scale = 100;

  if (scale != 100)
     pm = pm.scaledToWidth (get_value (pm.width(), scale));
  
  resize (pm.width() + 5, pm.height() + 5);
  img_full->setPixmap (pm);

  if (get_file_path (file_name) != get_file_path (fname))
  //re-read dir contents if the new file is at the another dir
  //than the previous one  
     {
      QStringList filters;
  
      QList <QByteArray> a = QImageReader::supportedImageFormats();

      foreach (QByteArray x, a)
              {
               QString t (x.data());
               t.prepend ("*.");
               filters.append (t);
              }; 
  
     QDir dir (get_file_path (fname));
     fi = dir.entryInfoList (filters, 
                             QDir::Files | QDir::Readable, 
                             QDir::Name); 
    }

  file_name = fname;
  
  for (int i = 0; i < fi.size(); i++)
       if (fi.at(i).filePath() == file_name)
          {  
           current_index = i;
           break;
          };
  
  QString s_wnd_title (QFileInfo (fname).fileName());
  s_wnd_title.append (QString (" - %1x%2").arg (pm.width()).arg(pm.height()));
  setWindowTitle (s_wnd_title);
}


void CViewerWindow::show_again()
{
  if (file_name.isEmpty())
     return;
  
  QImage pm (file_name);
  if (scale != 100)
     pm = pm.scaledToWidth (get_value (pm.width(), scale));
                           
  QTransform transform;
  transform.rotate (angle);
    
  QPixmap pixmap = QPixmap::fromImage (pm.transformed (transform));
  
  resize (pixmap.width() + 5, pixmap.height() + 5);
  img_full->setPixmap (pixmap);
  
  QString s_wnd_title (QFileInfo (file_name).fileName());
  s_wnd_title.append (QString (" - %1x%2").arg (pm.width()).arg(pm.height()));
  setWindowTitle (s_wnd_title);
}


void CViewerWindow::keyPressEvent (QKeyEvent *event)    
{
  if (event->key() == Qt::Key_Escape)
     {
      event->accept();
      close();
     }

  if ((event->key() == Qt::Key_Space) ||
      event->key() == Qt::Key_PageDown)
     {
      if (current_index < (fi.size() - 1))
          show_image (fi.at(++current_index).filePath());        
    
      event->accept();
      return;
     }

  if (event->key() == Qt::Key_PageUp)
     {
      if (current_index != 0)
         show_image (fi.at(--current_index).filePath());        
    
      event->accept();
      return;
     }

  if (event->key() == Qt::Key_Home)
     {
      current_index = 0;
      show_image (fi.at(current_index).filePath());        
      event->accept();
      return;
     }

  if (event->key() == Qt::Key_End)
     {
      current_index = fi.size() - 1;
      show_image (fi.at(current_index).filePath());        
      event->accept();
      return;
     }

  if (event->key() == Qt::Key_BracketRight)
     {
      angle += 90.0;
      if (angle >= 360.0)
         angle = 0.0;
      
      show_again();
      event->accept();
      return;
     }
  
  if (event->key() == Qt::Key_BracketLeft)
     {
      angle -= 90.0;
      if (angle <= 0)
         angle = 360;
      
      show_again();
      event->accept();
      return;
     }

  if (event->key() == Qt::Key_Plus)
     {
      scale += 10;
          
      show_again();
      event->accept();
      return;
     }

  if (event->key() == Qt::Key_Minus)
     {
      if (scale > 10)
         scale -= 10;
          
      show_again();
      event->accept();
      return;
     }
  
  QWidget::keyPressEvent (event);
}
*/
/*
void CViewerWindow::show_image_from_clipboard()
{
  
  QClipboard *clipboard = QApplication::clipboard();
  const QMimeData *mimeData = clipboard->mimeData();

  if (! mimeData->hasImage()) 
     return;
          
  QPixmap pm (qvariant_cast<QPixmap>(mimeData->imageData()));
  if (scale != 100)
     pm = pm.scaledToWidth (get_value (pm.width(), scale));
  
  resize (pm.width() + 5, pm.height() + 5);
  img_full->setPixmap (pm);
  
  file_name = "";
  
  QString s_wnd_title (tr ("Clipboard"));
  s_wnd_title.append (QString (" - %1x%2").arg (pm.width()).arg(pm.height()));
  setWindowTitle (s_wnd_title);
}
*/




void CZORWindow::closeEvent (QCloseEvent *event)
{
  event->accept();
}


CZORWindow::CZORWindow (QWidget *parent): QWidget (parent)
{
  /*settings = new QSettings (QSettings::NativeFormat,
                            QSettings::UserScope,
                            "zor",
                            "zor");

*/
  //fname_image = settings->value ("fname_image", "none").toString();

  //if (fname_image != "none")
    //  load_image (fname_image);


 // setWindowTitle (tr ("ZOR. Press F1 for help"));
  //setWindowOpacity (0.5f);

//  setWindowOpacity (1.0f);
  setMinimumSize (16, 16);

  
  //setAttribute (Qt::WA_QuitOnClose,true);
  
//  setWindowFlags (Qt::WindowStaysOnTopHint);


  //QAction *quitAction = new QAction (tr ("E&xit"), this);
  //quitAction->setShortcut (tr ("Ctrl+Q"));
  //connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
  //addAction(quitAction);
}

CZORWindow::~CZORWindow()
{
  //qDebug() << "~CZORWindow()";

  //settings->setValue ("fname_image", fname_image);
  //delete settings;
}


void CZORWindow::paintEvent (QPaintEvent *event)
{
  QPainter painter (this);
  painter.drawImage (0, 0, source_image);
}

/*
void CZORWindow::resizeEvent(QResizeEvent *event)
{
  if (transform.isRotating())
     {
      QImage tmp = source_image.transformed (transform);
      transformed_image = tmp.scaled (size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
     }
  else
  transformed_image = source_image.scaled (size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

  update();
}

*/

void CZORWindow::load_image (const QString &fname)
{
  if (! file_exists (fname))
     return;

 // qDebug() << "CZORWindow::load_image " << fname;
  
  fname_image = fname;
 
  if (! source_image.load (fname))
     qDebug() << "cannot load " << fname;
     
  
  bool orientation_portrait = false; 
  
  int exif_orientation = get_exif_orientation (fname);
 // qDebug() << "exif_orientation: " << exif_orientation;
  
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
      
      qreal angle;
      
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
