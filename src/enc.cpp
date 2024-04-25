#include <iostream>
#include <cstring>

//typedef unsigned short ushort;

#include "enc.h"

ushort* TextConverter::ConvertFromCP1251ToUTF16(const char* cp1251Text) {
    size_t len = strlen(cp1251Text);
    ushort* utf16Text = new ushort[len + 1]; // Учитываем нулевой символ
    ushort* p = utf16Text;
    while (*cp1251Text) {
        unsigned char c = *cp1251Text++;
        if (c < 0x80) {
            *p++ = c;
        } else {
            *p++ = (0x0400 + c); // Добавляем смещение для символов выше 0x7F
        }
    }
    *p = u'\0';
    return utf16Text;
}

ushort* TextConverter::ConvertFromUTF16ToCP1251(const ushort* utf16Text) {
    size_t len = 0;
    const ushort* p = utf16Text;
    while (*p++) {
        len++;
    }
    ushort* cp1251Text = new ushort[len + 1];
    p = utf16Text;
    ushort* q = cp1251Text;
    while (*p) {
        ushort c = *p++;
        if (c < 0x80) {
            *q++ = c;
        } else if (c >= 0x0400 && c <= 0x045F) {
            *q++ = (c - 0x0400); // Удаляем смещение для символов выше 0x7F
        } else {
            // Некорректный символ, заменяем на '?'
            *q++ = u'?';
        }
    }
    *q = u'\0';
    return cp1251Text;
}

ushort* TextConverter::ConvertFromDOS866ToUTF16(const char* dos866Text) {
    // Реализация аналогична ConvertFromCP1251ToUTF16
    size_t len = strlen(dos866Text);
    ushort* utf16Text = new ushort[len + 1]; // Учитываем нулевой символ
    ushort* p = utf16Text;
    while (*dos866Text) {
        unsigned char c = *dos866Text++;
        if (c < 0x80) {
            *p++ = c;
        } else {
            *p++ = (0x0400 + c); // Добавляем смещение для символов выше 0x7F
        }
    }
    *p = u'\0';
    return utf16Text;
}

ushort* TextConverter::ConvertFromUTF16ToDOS866(const ushort* utf16Text) {
    // Реализация аналогична ConvertFromUTF16ToCP1251
    size_t len = 0;
    const ushort* p = utf16Text;
    while (*p++) {
        len++;
    }
    ushort* dos866Text = new ushort[len + 1];
    p = utf16Text;
    ushort* q = dos866Text;
    while (*p) {
        ushort c = *p++;
        if (c < 0x80) {
            *q++ = c;
        } else if (c >= 0x0400 && c <= 0x045F) {
            *q++ = (c - 0x0400); // Удаляем смещение для символов выше 0x7F
        } else {
            // Некорректный символ, заменяем на '?'
            *q++ = u'?';
        }
    }
    *q = u'\0';
    return dos866Text;
}

ushort* TextConverter::ConvertFromKOI8RToUTF16(const char* koi8rText) {
    // Реализация аналогична ConvertFromCP1251ToUTF16
    size_t len = strlen(koi8rText);
    ushort* utf16Text = new ushort[len + 1]; // Учитываем нулевой символ
    ushort* p = utf16Text;
    while (*koi8rText) {
        unsigned char c = *koi8rText++;
        if (c < 0x80) {
            *p++ = c;
        } else {
            *p++ = (0x0400 + c); // Добавляем смещение для символов выше 0x7F
        }
    }
    *p = u'\0';
    return utf16Text;
}

ushort* TextConverter::ConvertFromUTF16ToKOI8R(const ushort* utf16Text) {
    // Реализация аналогична ConvertFromUTF16ToCP1251
    size_t len = 0;
    const ushort* p = utf16Text;
    while (*p++) {
        len++;
    }
    ushort* koi8rText = new ushort[len + 1];
    p = utf16Text;
    ushort* q = koi8rText;
    while (*p) {
        ushort c = *p++;
        if (c < 0x80) {
            *q++ = c;
        } else if (c >= 0x0400 && c <= 0x045F) {
            *q++ = (c - 0x0400); // Удаляем смещение для символов выше 0x7F
        } else {
            // Некорректный символ, заменяем на '?'
            *q++ = u'?';
        }
    }
    *q = u'\0';
    return koi8rText;
}


/*
int main() {
    // Пример использования функций конвертации
    const char* cp1251Text = "Пример текста на русском языке в кодировке CP1251";
    ushort* utf16Text = TextConverter::ConvertFromCP1251ToUTF16(cp1251Text);
    std::wcout << L"UTF-16 текст (из CP1251): " << utf16Text << std::endl;
    delete[] utf16Text;

    const char* dos866Text = "Пример текста на русском языке в кодировке DOS866";
    utf16Text = TextConverter::ConvertFromDOS866ToUTF16(dos866Text);
    std::wcout << L"UTF-16 текст (из DOS866): " << utf16Text << std::endl;
    delete[] utf16Text;

    const char* koi8rText = "Пример текста на русском языке в кодировке KOI8-R";
    utf16Text = TextConverter::ConvertFromKOI8RToUTF16(koi8rText);
    std::wcout << L"UTF-16 текст (из KOI8-R): " << utf16Text << std::endl;
    delete[] utf16Text;

    return 0;
}
*/
