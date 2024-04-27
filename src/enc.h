#ifndef ENC_H
#define ENC_H

#include <qglobal.h>



#if QT_VERSION >= 0x050000
#define UTF16TEXT char16_t
#else
#define UTF16TEXT ushort
#endif


class CTextConverter
{
public:

static    UTF16TEXT* ConvertFromCP1251ToUTF16(const char* cp1251Text);
static    char* ConvertFromUTF16ToCP1251(const UTF16TEXT* utf16Text);

static UTF16TEXT* ConvertFromDOS866ToUTF16(const char* dos866Text);

static char* ConvertFromUTF16ToDOS866(const UTF16TEXT* utf16Text);
static UTF16TEXT* ConvertFromKOI8RToUTF16(const char* koi8rText);
static char* ConvertFromUTF16ToKOI8R(const UTF16TEXT* utf16Text);
static UTF16TEXT* ConvertFromUTF8ToUTF16(const char* utf8Text);

static QStringList get_charsets();


};

#endif

