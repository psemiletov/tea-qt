#ifndef TIO_H
#define TIO_H


#include <QObject>
#include <QList>
#include <QByteArray>
#include <QString>
#include <QStringList>

//try http://qt-project.org/doc/qt-5.0/qtcore/qxmlstreamreader.html for qt5


class CSignaturesList: public QObject
{
  Q_OBJECT

public:
  QString encname;
  QList <QByteArray> words;
};


class CCharsetMagic: public QObject
{
  Q_OBJECT

public:

  QList <CSignaturesList*> signatures;

  CCharsetMagic();
  ~CCharsetMagic();
  
  QString guess_for_file (const QString &fname);
};


class CTio: public QObject
{
  Q_OBJECT

public:

  bool ronly;
  QString data;
  QString charset;
  QString error_string;

  QString eol;

  QStringList extensions;

  virtual bool load (const QString &fname) = 0;
  virtual bool save (const QString &fname) = 0;
};


class CTioPlainText: public CTio
{
  Q_OBJECT

public:

  CTioPlainText();
  bool load (const QString &fname);
  bool save (const QString &fname);
};


class CTioReadOnly: public CTio
{
  Q_OBJECT

public:

  bool save (const QString &fname);
};


class CTioGzip: public CTioReadOnly
{
  Q_OBJECT

public:

  CTioGzip();
  bool load (const QString &fname);
};


class CTioXMLZipped: public CTioReadOnly
{
  Q_OBJECT

public:

  CTioXMLZipped();
  bool load (const QString &fname);
};


class CTioODTSpecial: public CTioReadOnly
{
  Q_OBJECT

public:

  CTioODTSpecial();
  bool load (const QString &fname);
};


class CTioABW: public CTioReadOnly
{
  Q_OBJECT

public:

  CTioABW();
  bool load (const QString &fname);
};


class CTioFB2: public CTioReadOnly
{
  Q_OBJECT

public:

  CTioFB2();
  bool load (const QString &fname);
};


class CTioRTF: public CTioReadOnly
{
  Q_OBJECT

public:

  CTioRTF();
  bool load (const QString &fname);
};



#ifdef POPPLER_ENABLE

class CTioPDF: public CTioReadOnly
{
  Q_OBJECT

public:

  CTioPDF();
  bool load (const QString &fname);
};

#endif




#ifdef DJVU_ENABLE

class CTioDJVU: public CTioReadOnly
{
  Q_OBJECT

public:

  CTioDJVU();
  bool load (const QString &fname);
};

#endif



class CTioHandler: public QObject
{
  Q_OBJECT

public:

  QList <CTio *> list;
  CTioPlainText *default_handler;

  CTioHandler();
  ~CTioHandler();

  CTio* get_for_fname (const QString &fname);
};

#endif // TIO_H
