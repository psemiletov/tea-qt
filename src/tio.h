#ifndef TIO_H
#define TIO_H

#include <vector>

#include <QObject>
#include <QList>
#include <QByteArray>
#include <QString>
#include <QStringList>


class CSignaturesList: public QObject
{
  Q_OBJECT

public:

  QString encname;
  QList <QByteArray> words;
};

/*
class CCharsetMagic: public QObject
{
  Q_OBJECT

public:

  std::vector <CSignaturesList*> signatures;

  CCharsetMagic();
  ~CCharsetMagic();
  
  QString guess_for_file (const QString &fname);
};
*/

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
// virtual bool understand (const QString &fname) = 0;

  virtual ~CTio() {};
};


//default handler
class CTioPlainText: public CTio
{
  Q_OBJECT

public:

  CTioPlainText();
  bool load (const QString &fname);
  bool save (const QString &fname);
  //bool understand (const QString &fname) {return false};

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


class CTioEpub: public CTioReadOnly
{
  Q_OBJECT

public:

  CTioEpub();
  bool load (const QString &fn);
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


#if defined(POPPLER_ENABLE)
class CTioPDF: public CTioReadOnly
{
  Q_OBJECT

public:
  CTioPDF();
  bool load (const QString &fname);
};
#endif


#if defined(DJVU_ENABLE)

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

  std::vector <CTio*> list;

  CTioPlainText *default_handler;

  CTioHandler();
  ~CTioHandler();

  CTio* get_for_fname (const QString &fname);
 // QStringList get_supported_exts();
};


/*
class CCharsetConverter
{
  Q_OBJECT

public:

  static virtual QStringList get_charsets() = 0;

  virtual ~CCharsetConverter() {};
};
*/
/*
class CIconvCharsetConverter: public QObject
{
  Q_OBJECT

public:

  QStringList charsets;

  QStringList charsets_for_locale (const QString &loc);

  void load_charsets();

  QString to_utf16 (const QString &enc_from, char *inbuf, int len);
 // char* from_utf_16 (const QString &enc_to, const QString &data);


};
*/


#endif // TIO_H
