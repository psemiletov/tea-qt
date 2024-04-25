#ifndef ENC_H
#define ENC_H


class TextConverter {
public:
    static ushort* ConvertFromCP1251ToUTF16(const char* cp1251Text);
    static ushort* ConvertFromUTF16ToCP1251(const ushort* utf16Text);
    static ushort* ConvertFromDOS866ToUTF16(const char* dos866Text);
    static ushort* ConvertFromUTF16ToDOS866(const ushort* utf16Text);
    static ushort* ConvertFromKOI8RToUTF16(const char* koi8rText);
    static ushort* ConvertFromUTF16ToKOI8R(const ushort* utf16Text);
};

#endif

