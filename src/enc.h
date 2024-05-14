#ifndef ENC_H
#define ENC_H

#include <qglobal.h>

//#define char16_t uint16_t
/*
#if QT_VERSION >= 0x050000
#include <uchar.h>
#define UTF16TEXT char16_t
#else
#define UTF16TEXT unsigned short
#endif
*/

#if (__cplusplus < 201103L)
//C++11 specific stuff here
#include <stdint.h>
#define char16_t uint16_t
#endif

#define UTF16TEXT char16_t


class CTextConverter
{
public:

  static UTF16TEXT* ConvertFromCP1251ToUTF16 (const char* cp1251Text); //ok
  static char* ConvertFromUTF16ToCP1251 (const UTF16TEXT* utf16Text); //ok

  static UTF16TEXT* ConvertFromDOS866ToUTF16 (const char* dos866Text); //ok

static char* ConvertFromUTF16ToCP866(const UTF16TEXT* utf16Text); //НОЛЬ
static UTF16TEXT* ConvertFromKOI8RToUTF16(const char* koi8rText); //ok
static char* ConvertFromUTF16ToKOI8R(const UTF16TEXT* utf16Text); //ok

static UTF16TEXT* ConvertFromUTF8ToUTF16(const char* utf8Text);

static QStringList get_charsets();


};

std::string ConvertUTF16ToUTF8(const UTF16TEXT* utf16_text);


#endif

