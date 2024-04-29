//#include <iostream>
#include <cstring>
#include <map>

#include <QStringList>
//typedef unsigned short ushort;

#include "enc.h"



//ok
UTF16TEXT cp1251_to_utf16[256] = {
    // 0x00 - 0x7F: Соответствуют символам ASCII
    0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
    0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F,
    0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017,
    0x0018, 0x0019, 0x001A, 0x001B, 0x001C, 0x001D, 0x001E, 0x001F,
    0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
    0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
    0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
    0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
    0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
    0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
    0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
    0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
    0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
    0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
    0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F,
    // 0x80 - 0xFF: Символы CP1251
    0x0402, 0x0403, 0x201A, 0x0453, 0x201E, 0x2026, 0x2020, 0x2021,
    0x20AC, 0x2030, 0x0409, 0x2039, 0x040A, 0x040C, 0x040B, 0x040F,
    0x0452, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
    0x0020, 0x2122, 0x0459, 0x203A, 0x045A, 0x045C, 0x045B, 0x045F,
    0x00A0, 0x040E, 0x045E, 0x0408, 0x00A4, 0x0490, 0x00A6, 0x00A7,
    0x0401, 0x00A9, 0x0404, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x0407,
    0x00B0, 0x00B1, 0x0406, 0x0456, 0x0491, 0x00B5, 0x00B6, 0x00B7,
    0x0451, 0x2116, 0x0454, 0x00BB, 0x0458, 0x0405, 0x0455, 0x0457,
    0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417,
    0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F,
    0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427,
    0x0428, 0x0429, 0x042A, 0x042B, 0x042C, 0x042D, 0x042E, 0x042F,
    0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437,
    0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E, 0x043F,
    0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447,
    0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D, 0x044E, 0x044F
};

//ok
std::map<int, char> UTF16_to_CP1251 = {
    {0x0020, 0x20},  // SPACE
    {0x0021, 0x21},  // EXCLAMATION MARK
    {0x0022, 0x22},  // QUOTATION MARK
    {0x0023, 0x23},  // NUMBER SIGN
    {0x0024, 0x24},  // DOLLAR SIGN
    {0x0025, 0x25},  // PERCENT SIGN
    {0x0026, 0x26},  // AMPERSAND
    {0x0027, 0x27},  // APOSTROPHE
    {0x0028, 0x28},  // LEFT PARENTHESIS
    {0x0029, 0x29},  // RIGHT PARENTHESIS
    {0x002A, 0x2A},  // ASTERISK
    {0x002B, 0x2B},  // PLUS SIGN
    {0x002C, 0x2C},  // COMMA
    {0x002D, 0x2D},  // HYPHEN-MINUS
    {0x002E, 0x2E},  // FULL STOP
    {0x002F, 0x2F},  // SOLIDUS
    {0x0030, 0x30},  // DIGIT ZERO
    {0x0031, 0x31},  // DIGIT ONE
    {0x0032, 0x32},  // DIGIT TWO
    {0x0033, 0x33},  // DIGIT THREE
    {0x0034, 0x34},  // DIGIT FOUR
    {0x0035, 0x35},  // DIGIT FIVE
    {0x0036, 0x36},  // DIGIT SIX
    {0x0037, 0x37},  // DIGIT SEVEN
    {0x0038, 0x38},  // DIGIT EIGHT
    {0x0039, 0x39},  // DIGIT NINE
    {0x003A, 0x3A},  // COLON
    {0x003B, 0x3B},  // SEMICOLON
    {0x003C, 0x3C},  // LESS-THAN SIGN
    {0x003D, 0x3D},  // EQUALS SIGN
    {0x003E, 0x3E},  // GREATER-THAN SIGN
    {0x003F, 0x3F},  // QUESTION MARK
    {0x0040, 0x40},  // COMMERCIAL AT
    {0x0041, 0x41},  // LATIN CAPITAL LETTER A
    {0x0042, 0x42},  // LATIN CAPITAL LETTER B
    {0x0043, 0x43},  // LATIN CAPITAL LETTER C
    {0x0044, 0x44},  // LATIN CAPITAL LETTER D
    {0x0045, 0x45},  // LATIN CAPITAL LETTER E
    {0x0046, 0x46},  // LATIN CAPITAL LETTER F
    {0x0047, 0x47},  // LATIN CAPITAL LETTER G
    {0x0048, 0x48},  // LATIN CAPITAL LETTER H
    {0x0049, 0x49},  // LATIN CAPITAL LETTER I
    {0x004A, 0x4A},  // LATIN CAPITAL LETTER J
    {0x004B, 0x4B},  // LATIN CAPITAL LETTER K
    {0x004C, 0x4C},  // LATIN CAPITAL LETTER L
    {0x004D, 0x4D},  // LATIN CAPITAL LETTER M
    {0x004E, 0x4E},  // LATIN CAPITAL LETTER N
    {0x004F, 0x4F},  // LATIN CAPITAL LETTER O
    {0x0050, 0x50},  // LATIN CAPITAL LETTER P
    {0x0051, 0x51},  // LATIN CAPITAL LETTER Q
    {0x0052, 0x52},  // LATIN CAPITAL LETTER R
    {0x0053, 0x53},  // LATIN CAPITAL LETTER S
    {0x0054, 0x54},  // LATIN CAPITAL LETTER T
    {0x0055, 0x55},  // LATIN CAPITAL LETTER U
    {0x0056, 0x56},  // LATIN CAPITAL LETTER V
    {0x0057, 0x57},  // LATIN CAPITAL LETTER W
    {0x0058, 0x58},  // LATIN CAPITAL LETTER X
    {0x0059, 0x59},  // LATIN CAPITAL LETTER Y
    {0x005A, 0x5A},  // LATIN CAPITAL LETTER Z
    {0x005B, 0x5B},  // LEFT SQUARE BRACKET
    {0x005C, 0x5C},  // REVERSE SOLIDUS
    {0x005D, 0x5D},  // RIGHT SQUARE BRACKET
    {0x005E, 0x5E},  // CIRCUMFLEX ACCENT
    {0x005F, 0x5F},  // LOW LINE
    {0x0060, 0x60},  // GRAVE ACCENT
    {0x0061, 0x61},  // LATIN SMALL LETTER A
    {0x0062, 0x62},  // LATIN SMALL LETTER B
    {0x0063, 0x63},  // LATIN SMALL LETTER C
    {0x0064, 0x64},  // LATIN SMALL LETTER D
    {0x0065, 0x65},  // LATIN SMALL LETTER E
    {0x0066, 0x66},  // LATIN SMALL LETTER F
    {0x0067, 0x67},  // LATIN SMALL LETTER G
    {0x0068, 0x68},  // LATIN SMALL LETTER H
    {0x0069, 0x69},  // LATIN SMALL LETTER I
    {0x006A, 0x6A},  // LATIN SMALL LETTER J
    {0x006B, 0x6B},  // LATIN SMALL LETTER K
    {0x006C, 0x6C},  // LATIN SMALL LETTER L
    {0x006D, 0x6D},  // LATIN SMALL LETTER M
    {0x006E, 0x6E},  // LATIN SMALL LETTER N
    {0x006F, 0x6F},  // LATIN SMALL LETTER O
    {0x0070, 0x70},  // LATIN SMALL LETTER P
    {0x0071, 0x71},  // LATIN SMALL LETTER Q
    {0x0072, 0x72},  // LATIN SMALL LETTER R
    {0x0073, 0x73},  // LATIN SMALL LETTER S
    {0x0074, 0x74},  // LATIN SMALL LETTER T
    {0x0075, 0x75},  // LATIN SMALL LETTER U
    {0x0076, 0x76},  // LATIN SMALL LETTER V
    {0x0077, 0x77},  // LATIN SMALL LETTER W
    {0x0078, 0x78},  // LATIN SMALL LETTER X
    {0x0079, 0x79},  // LATIN SMALL LETTER Y
    {0x007A, 0x7A},  // LATIN SMALL LETTER Z
    {0x007B, 0x7B},  // LEFT CURLY BRACKET
    {0x007C, 0x7C},  // VERTICAL LINE
    {0x007D, 0x7D},  // RIGHT CURLY BRACKET
    {0x007E, 0x7E},  // TILDE
    {0x0401, 0xF0},  // CYRILLIC CAPITAL LETTER IO
    {0x0410, 0xC0},  // CYRILLIC CAPITAL LETTER A
    {0x0411, 0xC1},  // CYRILLIC CAPITAL LETTER BE
    {0x0412, 0xC2},  // CYRILLIC CAPITAL LETTER VE
    {0x0413, 0xC3},  // CYRILLIC CAPITAL LETTER GHE
    {0x0414, 0xC4},  // CYRILLIC CAPITAL LETTER DE
    {0x0415, 0xC5},  // CYRILLIC CAPITAL LETTER IE
    {0x0416, 0xC6},  // CYRILLIC CAPITAL LETTER ZHE
    {0x0417, 0xC7},  // CYRILLIC CAPITAL LETTER ZE
    {0x0418, 0xC8},  // CYRILLIC CAPITAL LETTER I
    {0x0419, 0xC9},  // CYRILLIC CAPITAL LETTER SHORT I
    {0x041A, 0xCA},  // CYRILLIC CAPITAL LETTER KA
    {0x041B, 0xCB},  // CYRILLIC CAPITAL LETTER EL
    {0x041C, 0xCC},  // CYRILLIC CAPITAL LETTER EM
    {0x041D, 0xCD},  // CYRILLIC CAPITAL LETTER EN
    {0x041E, 0xCE},  // CYRILLIC CAPITAL LETTER O
    {0x041F, 0xCF},  // CYRILLIC CAPITAL LETTER PE
    {0x0420, 0xD0},  // CYRILLIC CAPITAL LETTER ER
    {0x0421, 0xD1},  // CYRILLIC CAPITAL LETTER ES
    {0x0422, 0xD2},  // CYRILLIC CAPITAL LETTER TE
    {0x0423, 0xD3},  // CYRILLIC CAPITAL LETTER U
    {0x0424, 0xD4},  // CYRILLIC CAPITAL LETTER EF
    {0x0425, 0xD5},  // CYRILLIC CAPITAL LETTER HA
    {0x0426, 0xD6},  // CYRILLIC CAPITAL LETTER TSE
    {0x0427, 0xD7},  // CYRILLIC CAPITAL LETTER CHE
    {0x0428, 0xD8},  // CYRILLIC CAPITAL LETTER SHA
    {0x0429, 0xD9},  // CYRILLIC CAPITAL LETTER SHCHA
    {0x042A, 0xDA},  // CYRILLIC CAPITAL LETTER HARD SIGN
    {0x042B, 0xDB},  // CYRILLIC CAPITAL LETTER YERU
    {0x042C, 0xDC},  // CYRILLIC CAPITAL LETTER SOFT SIGN
    {0x042D, 0xDD},  // CYRILLIC CAPITAL LETTER E
    {0x042E, 0xDE},  // CYRILLIC CAPITAL LETTER YU
    {0x042F, 0xDF},  // CYRILLIC CAPITAL LETTER YA
    {0x0430, 0xE0},  // CYRILLIC SMALL LETTER A
    {0x0431, 0xE1},  // CYRILLIC SMALL LETTER BE
    {0x0432, 0xE2},  // CYRILLIC SMALL LETTER VE
    {0x0433, 0xE3},  // CYRILLIC SMALL LETTER GHE
    {0x0434, 0xE4},  // CYRILLIC SMALL LETTER DE
    {0x0435, 0xE5},  // CYRILLIC SMALL LETTER IE
    {0x0436, 0xE6},  // CYRILLIC SMALL LETTER ZHE
    {0x0437, 0xE7},  // CYRILLIC SMALL LETTER ZE
    {0x0438, 0xE8},  // CYRILLIC SMALL LETTER I
    {0x0439, 0xE9},  // CYRILLIC SMALL LETTER SHORT I
    {0x043A, 0xEA},  // CYRILLIC SMALL LETTER KA
    {0x043B, 0xEB},  // CYRILLIC SMALL LETTER EL
    {0x043C, 0xEC},  // CYRILLIC SMALL LETTER EM
    {0x043D, 0xED},  // CYRILLIC SMALL LETTER EN
    {0x043E, 0xEE},  // CYRILLIC SMALL LETTER O
    {0x043F, 0xEF},  // CYRILLIC SMALL LETTER PE
    {0x0440, 0xF0},  // CYRILLIC SMALL LETTER ER
    {0x0441, 0xF1},  // CYRILLIC SMALL LETTER ES
    {0x0442, 0xF2},  // CYRILLIC SMALL LETTER TE
    {0x0443, 0xF3},  // CYRILLIC SMALL LETTER U
    {0x0444, 0xF4},  // CYRILLIC SMALL LETTER EF
    {0x0445, 0xF5},  // CYRILLIC SMALL LETTER HA
    {0x0446, 0xF6},  // CYRILLIC SMALL LETTER TSE
    {0x0447, 0xF7},  // CYRILLIC SMALL LETTER CHE
    {0x0448, 0xF8},  // CYRILLIC SMALL LETTER SHA
    {0x0449, 0xF9},  // CYRILLIC SMALL LETTER SHCHA
    {0x044A, 0xFA},  // CYRILLIC SMALL LETTER HARD SIGN
    {0x044B, 0xFB},  // CYRILLIC SMALL LETTER YERU
    {0x044C, 0xFC},  // CYRILLIC SMALL LETTER SOFT SIGN
    {0x044D, 0xFD},  // CYRILLIC SMALL LETTER E
    {0x044E, 0xFE},  // CYRILLIC SMALL LETTER YU
    {0x044F, 0xFF},  // CYRILLIC SMALL LETTER YA
     { u'\n', '\n' }, // Keep newline character
    { u'\r', '\r' },  // Keep carriage return character
};



UTF16TEXT utf16_to_dos866[65536] = {
    // Для символов в диапазоне от 0x0000 до 0x007F используем ASCII
    // Остальные символы будут преобразованы в соответствующие символы DOS866
    // Таблица заполнена вручную и может быть дополнена при необходимости
    // Для некоторых символов нет прямого эквивалента в DOS866, поэтому используется пустой символ (0x0000)
    0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, // Символы  !"#$%&'
    0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F, // Символы ()*+,-./
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, // Символы 01234567
    0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F, // Символы 89:;<=>?
    0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, // Символы @ABCDEFG
    0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F, // Символы HIJKLMNO
    0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, // Символы PQRSTUVW
    0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F, // Символы XYZ[\]^_
    0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, // Символы `abcdefg
    0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F, // Символы hijklmno
    0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, // Символы pqrstuvw
    0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F, // Символы xyz{|}~
    // Символы в диапазоне от 0x0080 до 0x00FF будут преобразованы в соответствующие символы DOS866
    // Для некоторых символов нет прямого эквивалента в DOS866, поэтому используется пустой символ (0x0000)
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // Символы диапазона 0x0080-0x0087
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // Символы диапазона 0x0088-0x008F
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // Символы диапазона 0x0090-0x0097
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // Символы диапазона 0x0098-0x009F
    0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, // Символы диапазона 0x00A0-0x00A7
    0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF, // Символы диапазона 0x00A8-0x00AF
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, // Символы диапазона 0x00B0-0x00B7
    0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF, // Символы диапазона 0x00B8-0x00BF
    0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417, // Символы диапазона 0x00C0-0x00C7
    0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F, // Символы диапазона 0x00C8-0x00CF
    0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427, // Символы диапазона 0x00D0-0x00D7
    0x0428, 0x0429, 0x042A, 0x042B, 0x042C, 0x042D, 0x042E, 0x042F, // Символы диапазона 0x00D8-0x00DF
    0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437, // Символы диапазона 0x00E0-0x00E7
    0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E, 0x043F, // Символы диапазона 0x00E8-0x00EF
    0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447, // Символы диапазона 0x00F0-0x00F7
    0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D, 0x044E, 0x044F, // Символы диапазона 0x00F8-0x00FF
};




UTF16TEXT koi8r_to_utf16[256] = {
    // От 0x00 до 0x7F: Соответствуют символам ASCII
    0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
    0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F,
    0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017,
    0x0018, 0x0019, 0x001A, 0x001B, 0x001C, 0x001D, 0x001E, 0x001F,
    0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
    0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
    0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
    0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
    0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
    0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
    0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
    0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
    0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
    0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
    0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F,
    // От 0x80 до 0xFF: Символы KOI8-R
    0x2500, 0x2502, 0x250C, 0x2510, 0x2514, 0x2518, 0x251C, 0x2524, // Символы диапазона 0x80-0x87
    0x252C, 0x2534, 0x253C, 0x2580, 0x2584, 0x2588, 0x258C, 0x2590, // Символы диапазона 0x88-0x8F
    0x2591, 0x2592, 0x2593, 0x2320, 0x25A0, 0x2219, 0x221A, 0x2248, // Символы диапазона 0x90-0x97
    0x2264, 0x2265, 0x00A0, 0x2321, 0x00B0, 0x00B2, 0x00B7, 0x00F7, // Символы диапазона 0x98-0x9F
    0x2550, 0x2551, 0x2552, 0x0451, 0x2553, 0x2554, 0x2555, 0x2556, // Символы диапазона 0xA0-0xA7
    0x2557, 0x2558, 0x2559, 0x255A, 0x255B, 0x255C, 0x255D, 0x255E, // Символы диапазона 0xA8-0xAF
    0x255F, 0x2560, 0x2561, 0x0401, 0x2562, 0x2563, 0x2564, 0x2565, // Символы диапазона 0xB0-0xB7
    0x2566, 0x2567, 0x2568, 0x2569, 0x256A, 0x256B, 0x256C, 0x00A9, // Символы диапазона 0xB8-0xBF
    0x044E, 0x0430, 0x0431, 0x0446, 0x0434, 0x0435, 0x0444, 0x0433, // Символы диапазона 0xC0-0xC7
    0x0445, 0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E, // Символы диапазона 0xC8-0xCF
    0x043F, 0x044F, 0x0440, 0x0441, 0x0442, 0x0443, 0x0436, 0x0432, // Символы диапазона 0xD0-0xD7
    0x044C, 0x044B, 0x0437, 0x0448, 0x044D, 0x0449, 0x0447, 0x044A, // Символы диапазона 0xD8-0xDF
    0x042E, 0x0410, 0x0411, 0x0426, 0x0414, 0x0415, 0x0424, 0x0413, // Символы диапазона 0xE0-0xE7
    0x0425, 0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, // Символы диапазона 0xE8-0xEF
    0x041F, 0x042F, 0x0420, 0x0421, 0x0422, 0x0423, 0x0416, 0x0412, // Символы диапазона 0xF0-0xF7
    0x042C, 0x042B, 0x0417, 0x0428, 0x042D, 0x0429, 0x0427, 0x042A  // Символы диапазона 0xF8-0xFF
};

//ushort
UTF16TEXT utf16_to_koi8r[65536] = {
    // Для символов в диапазоне от 0x0000 до 0x007F используем ASCII
    // Остальные символы будут преобразованы в соответствующие символы KOI8-R
    // Таблица заполнена вручную и может быть дополнена при необходимости
    // Для некоторых символов нет прямого эквивалента в KOI8-R, поэтому используется пустой символ (0x0000)
    0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, // Символы  !"#$%&'
    0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F, // Символы ()*+,-./
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, // Символы 01234567
    0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F, // Символы 89:;<=>?
    0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, // Символы @ABCDEFG
    0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F, // Символы HIJKLMNO
    0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, // Символы PQRSTUVW
    0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F, // Символы XYZ[\]^_
    0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, // Символы `abcdefg
    0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F, // Символы hijklmno
    0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, // Символы pqrstuvw
    0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F, // Символы xyz{|}~
    // Символы в диапазоне от 0x0080 до 0x00FF будут преобразованы в соответствующие символы KOI8-R
    // Для некоторых символов нет прямого эквивалента в KOI8-R, поэтому используется пустой символ (0x0000)
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // Символы диапазона 0x0080-0x0087
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // Символы диапазона 0x0088-0x008F
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // Символы диапазона 0x0090-0x0097
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // Символы диапазона 0x0098-0x009F
    0x2500, 0x2502, 0x250C, 0x2510, 0x2514, 0x2518, 0x251C, 0x2524, // Символы диапазона 0x00A0-0x00A7
    0x252C, 0x2534, 0x253C, 0x2580, 0x2584, 0x2588, 0x258C, 0x2590, // Символы диапазона 0x00A8-0x00AF
    0x2591, 0x2592, 0x2593, 0x2320, 0x25A0, 0x2219, 0x221A, 0x2248, // Символы диапазона 0x00B0-0x00B7
    0x2264, 0x2265, 0x00A0, 0x2321, 0x00B0, 0x00B2, 0x00B7, 0x00F7, // Символы диапазона 0x00B8-0x00BF
    0x2550, 0x2551, 0x2552, 0x0451, 0x2553, 0x2554, 0x2555, 0x2556, // Символы диапазона 0x00C0-0x00C7
    0x2557, 0x2558, 0x2559, 0x255A, 0x255B, 0x255C, 0x255D, 0x255E, // Символы диапазона 0x00C8-0x00CF
    0x255F, 0x2560, 0x2561, 0x0401, 0x2562, 0x2563, 0x2564, 0x2565, // Символы диапазона 0x00D0-0x00D7
    0x2566, 0x2567, 0x2568, 0x2569, 0x256A, 0x256B, 0x256C, 0x00A9, // Символы диапазона 0x00D8-0x00DF
    0x044E, 0x0410, 0x0411, 0x0426, 0x0414, 0x0415, 0x0424, 0x0413, // Символы диапазона 0x00E0-0x00E7
    0x0425, 0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, // Символы диапазона 0x00E8-0x00EF
    0x041F, 0x042F, 0x0420, 0x0421, 0x0422, 0x0423, 0x0416, 0x0412, // Символы диапазона 0x00F0-0x00F7
    0x042C, 0x042B, 0x0417, 0x0428, 0x042D, 0x0429, 0x0427, 0x042A  // Символы диапазона 0x00F8-0x00FF
};



//ok
UTF16TEXT* CTextConverter::ConvertFromCP1251ToUTF16(const char* cp1251Text)
{
  size_t len = strlen (cp1251Text);
//  UTF16TEXT* utf16Text = new UTF16TEXT[2 * len + 1]; // Максимальная длина UTF-16 символа - 2 байта, учитываем нулевой символ
  UTF16TEXT* utf16Text = new UTF16TEXT[len + 1]; // Максимальная длина UTF-16 символа - 2 байта, учитываем нулевой символ

  UTF16TEXT* p = utf16Text;

  while (*cp1251Text)
        {
         unsigned char c = *cp1251Text++;
         *p++ = cp1251_to_utf16[c]; // Преобразование всех символов
        }

  *p = u'\0';
  return utf16Text;
}


//ok
char* CTextConverter::ConvertFromUTF16ToCP1251(const UTF16TEXT *s)
{
  std::string cp1251Text;

  for (const char16_t *p = s; *p != u'\0'; ++p)
      {
       auto it = UTF16_to_CP1251.find (*p);

       if (it != UTF16_to_CP1251.end())
           cp1251Text += it->second;
       else
            // If character not found in mapping table, add placeholder or handle differently
           cp1251Text += '?'; // Placeholder for unknown characters
      }

  char *result = new char[cp1251Text.length() + 1];
  //std::strcpy (result, cp1251Text.c_str());

  std::strncpy (result, cp1251Text.c_str(), cp1251Text.size());

  return result;
}



//NOT TESTED
char* CTextConverter::ConvertFromUTF16ToDOS866(const UTF16TEXT* utf16Text) {
    size_t len = 0;
    const UTF16TEXT* p = utf16Text;
    while (*p++) {
        ++len;
    }

    char* dos866Text = new char[len + 1]; // Учитываем нулевой символ
    char* q = dos866Text;
    while (*utf16Text) {
        UTF16TEXT uc = *utf16Text++;
        char c = 0;
        for (size_t i = 0; i < sizeof(utf16_to_dos866) / sizeof(utf16_to_dos866[0]); ++i) {
            if (utf16_to_dos866[i] == uc) {
                c = static_cast<char>(i);
                break;
            }
        }
        *q++ = c;
    }
    *q = '\0';
    return dos866Text;
}


//NOT TESTED
UTF16TEXT* CTextConverter::ConvertFromKOI8RToUTF16(const char* koi8rText) {
    size_t len = strlen(koi8rText);
    UTF16TEXT* utf16Text = new UTF16TEXT[len + 1]; // Учитываем нулевой символ
    UTF16TEXT* p = utf16Text;
    while (*koi8rText) {
        unsigned char c = *koi8rText++;
        *p++ = koi8r_to_utf16[c];
    }
    *p = u'\0';
    return utf16Text;
}

//NOT TESTED
char* CTextConverter::ConvertFromUTF16ToKOI8R(const UTF16TEXT* utf16Text) {
    size_t len = 0;
    const UTF16TEXT* p = utf16Text;
    while (*p++) {
        ++len;
    }

    char* koi8rText = new char[len + 1]; // Учитываем нулевой символ
    char* q = koi8rText;
    while (*utf16Text) {
        UTF16TEXT uc = *utf16Text++;
        char c = 0;
        for (size_t i = 0; i < sizeof(utf16_to_koi8r) / sizeof(utf16_to_koi8r[0]); ++i) {
            if (utf16_to_koi8r[i] == uc) {
                c = static_cast<char>(i);
                break;
            }
        }
        *q++ = c;
    }
    *q = '\0';
    return koi8rText;
}


//NOT NEEDED
UTF16TEXT* CTextConverter::ConvertFromUTF8ToUTF16(const char* utf8Text) {
    size_t len = strlen(utf8Text);
    UTF16TEXT* utf16Text = new UTF16TEXT[len + 1]; // Учитываем нулевой символ
    UTF16TEXT* p = utf16Text;
    while (*utf8Text) {
        unsigned char c = *utf8Text++;
        if (c < 0x80) {
            *p++ = c;
        } else if ((c & 0xE0) == 0xC0) {
            unsigned char c2 = *utf8Text++;
            *p++ = ((c & 0x1F) << 6) | (c2 & 0x3F);
        } else if ((c & 0xF0) == 0xE0) {
            unsigned char c2 = *utf8Text++;
            unsigned char c3 = *utf8Text++;
            *p++ = ((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
        } else {
            // Некорректный символ, заменяем на '?'
            *p++ = u'?';
        }
    }
    *p = u'\0';
    return utf16Text;
}


//OK
const UTF16TEXT CP866_to_UTF16[] = {
            /* 0x00 */ 0x0000, /* NUL */
    /* 0x01 */ 0x0001, /* SOH */
    /* 0x02 */ 0x0002, /* STX */
    /* 0x03 */ 0x0003, /* ETX */
    /* 0x04 */ 0x0004, /* EOT */
    /* 0x05 */ 0x0005, /* ENQ */
    /* 0x06 */ 0x0006, /* ACK */
    /* 0x07 */ 0x0007, /* BEL */
    /* 0x08 */ 0x0008, /* BS */
    /* 0x09 */ 0x0009, /* TAB */
    /* 0x0A */ 0x000A, /* LF */
    /* 0x0B */ 0x000B, /* VT */
    /* 0x0C */ 0x000C, /* FF */
    /* 0x0D */ 0x000D, /* CR */
    /* 0x0E */ 0x000E, /* SO */
    /* 0x0F */ 0x000F, /* SI */
    /* 0x10 */ 0x0010, /* DLE */
    /* 0x11 */ 0x0011, /* DC1 */
    /* 0x12 */ 0x0012, /* DC2 */
    /* 0x13 */ 0x0013, /* DC3 */
    /* 0x14 */ 0x0014, /* DC4 */
    /* 0x15 */ 0x0015, /* NAK */
    /* 0x16 */ 0x0016, /* SYN */
    /* 0x17 */ 0x0017, /* ETB */
    /* 0x18 */ 0x0018, /* CAN */
    /* 0x19 */ 0x0019, /* EM */
    /* 0x1A */ 0x001A, /* SUB */
    /* 0x1B */ 0x001B, /* ESC */
    /* 0x1C */ 0x001C, /* FS */
    /* 0x1D */ 0x001D, /* GS */
    /* 0x1E */ 0x001E, /* RS */
    /* 0x1F */ 0x001F, /* US */
    /* 0x20 */ 0x0020, /* Space */
    /* 0x21 */ 0x0021, /* ! */
    /* 0x22 */ 0x0022, /* " */
    /* 0x23 */ 0x0023, /* # */
    /* 0x24 */ 0x0024, /* $ */
    /* 0x25 */ 0x0025, /* % */
    /* 0x26 */ 0x0026, /* & */
    /* 0x27 */ 0x0027, /* ' */
    /* 0x28 */ 0x0028, /* ( */
    /* 0x29 */ 0x0029, /* ) */
    /* 0x2A */ 0x002A, /* * */
    /* 0x2B */ 0x002B, /* + */
    /* 0x2C */ 0x002C, /* , */
    /* 0x2D */ 0x002D, /* - */
    /* 0x2E */ 0x002E, /* . */
    /* 0x2F */ 0x002F, /* / */
    /* 0x30 */ 0x0030, /* 0 */
    /* 0x31 */ 0x0031, /* 1 */
    /* 0x32 */ 0x0032, /* 2 */
    /* 0x33 */ 0x0033, /* 3 */
    /* 0x34 */ 0x0034, /* 4 */
    /* 0x35 */ 0x0035, /* 5 */
    /* 0x36 */ 0x0036, /* 6 */
    /* 0x37 */ 0x0037, /* 7 */
    /* 0x38 */ 0x0038, /* 8 */
    /* 0x39 */ 0x0039, /* 9 */
    /* 0x3A */ 0x003A, /* : */
    /* 0x3B */ 0x003B, /* ; */
    /* 0x3C */ 0x003C, /* < */
    /* 0x3D */ 0x003D, /* = */
    /* 0x3E */ 0x003E, /* > */
    /* 0x3F */ 0x003F, /* ? */
    /* 0x40 */ 0x0040, /* @ */
    /* 0x41 */ 0x0041, /* A */
    /* 0x42 */ 0x0042, /* B */
    /* 0x43 */ 0x0043, /* C */
    /* 0x44 */ 0x0044, /* D */
    /* 0x45 */ 0x0045, /* E */
    /* 0x46 */ 0x0046, /* F */
    /* 0x47 */ 0x0047, /* G */
    /* 0x48 */ 0x0048, /* H */
    /* 0x49 */ 0x0049, /* I */
    /* 0x4A */ 0x004A, /* J */
    /* 0x4B */ 0x004B, /* K */
    /* 0x4C */ 0x004C, /* L */
    /* 0x4D */ 0x004D, /* M */
    /* 0x4E */ 0x004E, /* N */
    /* 0x4F */ 0x004F, /* O */
    /* 0x50 */ 0x0050, /* P */
    /* 0x51 */ 0x0051, /* Q */
    /* 0x52 */ 0x0052, /* R */
    /* 0x53 */ 0x0053, /* S */
    /* 0x54 */ 0x0054, /* T */
    /* 0x55 */ 0x0055, /* U */
    /* 0x56 */ 0x0056, /* V */
    /* 0x57 */ 0x0057, /* W */
    /* 0x58 */ 0x0058, /* X */
    /* 0x59 */ 0x0059, /* Y */
    /* 0x5A */ 0x005A, /* Z */
    /* 0x5B */ 0x005B, /* [ */
    /* 0x5C */ 0x005C, /* \ */
    /* 0x5D */ 0x005D, /* ] */
    /* 0x5E */ 0x005E, /* ^ */
    /* 0x5F */ 0x005F, /* _ */
    /* 0x60 */ 0x0060, /* ` */
    /* 0x61 */ 0x0061, /* a */
    /* 0x62 */ 0x0062, /* b */
    /* 0x63 */ 0x0063, /* c */
    /* 0x64 */ 0x0064, /* d */
    /* 0x65 */ 0x0065, /* e */
    /* 0x66 */ 0x0066, /* f */
    /* 0x67 */ 0x0067, /* g */
    /* 0x68 */ 0x0068, /* h */
    /* 0x69 */ 0x0069, /* i */
    /* 0x6A */ 0x006A, /* j */
    /* 0x6B */ 0x006B, /* k */
    /* 0x6C */ 0x006C, /* l */
    /* 0x6D */ 0x006D, /* m */
    /* 0x6E */ 0x006E, /* n */
    /* 0x6F */ 0x006F, /* o */
    /* 0x70 */ 0x0070, /* p */
    /* 0x71 */ 0x0071, /* q */
    /* 0x72 */ 0x0072, /* r */
    /* 0x73 */ 0x0073, /* s */
    /* 0x74 */ 0x0074, /* t */
    /* 0x75 */ 0x0075, /* u */
    /* 0x76 */ 0x0076, /* v */
    /* 0x77 */ 0x0077, /* w */
    /* 0x78 */ 0x0078, /* x */
    /* 0x79 */ 0x0079, /* y */
    /* 0x7A */ 0x007A, /* z */
    /* 0x7B */ 0x007B, /* { */
    /* 0x7C */ 0x007C, /* | */
    /* 0x7D */ 0x007D, /* } */
    /* 0x7E */ 0x007E, /* ~ */
    /* 0x7F */ 0x007F, /* DEL */
    /* 0x80 */ 0x0410, /* А */
    /* 0x81 */ 0x0411, /* Б */
    /* 0x82 */ 0x0412, /* В */
    /* 0x83 */ 0x0413, /* Г */
    /* 0x84 */ 0x0414, /* Д */
    /* 0x85 */ 0x0415, /* Е */
    /* 0x86 */ 0x0416, /* Ж */
    /* 0x87 */ 0x0417, /* З */
    /* 0x88 */ 0x0418, /* И */
    /* 0x89 */ 0x0419, /* Й */
    /* 0x8A */ 0x041A, /* К */
    /* 0x8B */ 0x041B, /* Л */
    /* 0x8C */ 0x041C, /* М */
    /* 0x8D */ 0x041D, /* Н */
    /* 0x8E */ 0x041E, /* О */
    /* 0x8F */ 0x041F, /* П */
    /* 0x90 */ 0x0420, /* Р */
    /* 0x91 */ 0x0421, /* С */
    /* 0x92 */ 0x0422, /* Т */
    /* 0x93 */ 0x0423, /* У */
    /* 0x94 */ 0x0424, /* Ф */
    /* 0x95 */ 0x0425, /* Х */
    /* 0x96 */ 0x0426, /* Ц */
    /* 0x97 */ 0x0427, /* Ч */
    /* 0x98 */ 0x0428, /* Ш */
    /* 0x99 */ 0x0429, /* Щ */
    /* 0x9A */ 0x042A, /* Ъ */
    /* 0x9B */ 0x042B, /* Ы */
    /* 0x9C */ 0x042C, /* Ь */
    /* 0x9D */ 0x042D, /* Э */
    /* 0x9E */ 0x042E, /* Ю */
    /* 0x9F */ 0x042F, /* Я */
    /* 0xA0 */ 0x0430, /* а */
    /* 0xA1 */ 0x0431, /* б */
    /* 0xA2 */ 0x0432, /* в */
    /* 0xA3 */ 0x0433, /* г */
    /* 0xA4 */ 0x0434, /* д */
    /* 0xA5 */ 0x0435, /* е */
    /* 0xA6 */ 0x0436, /* ж */
    /* 0xA7 */ 0x0437, /* з */
    /* 0xA8 */ 0x0438, /* и */
    /* 0xA9 */ 0x0439, /* й */
    /* 0xAA */ 0x043A, /* к */
    /* 0xAB */ 0x043B, /* л */
    /* 0xAC */ 0x043C, /* м */
    /* 0xAD */ 0x043D, /* н */
    /* 0xAE */ 0x043E, /* о */
    /* 0xAF */ 0x043F, /* п */
    /* 0xB0 */ 0x2591, /* ░ */
    /* 0xB1 */ 0x2592, /* ▒ */
    /* 0xB2 */ 0x2593, /* ▓ */
    /* 0xB3 */ 0x2502, /* │ */
    /* 0xB4 */ 0x2524, /* ┤ */
    /* 0xB5 */ 0x2561, /* ╡ */
    /* 0xB6 */ 0x2562, /* ╢ */
    /* 0xB7 */ 0x2556, /* ╖ */
    /* 0xB8 */ 0x2555, /* ╕ */
    /* 0xB9 */ 0x2563, /* ╣ */
    /* 0xBA */ 0x2551, /* ║ */
    /* 0xBB */ 0x2557, /* ╗ */
    /* 0xBC */ 0x255D, /* ╝ */
    /* 0xBD */ 0x255C, /* ╜ */
    /* 0xBE */ 0x255B, /* ╛ */
    /* 0xBF */ 0x2510, /* ┐ */
    /* 0xC0 */ 0x2514, /* └ */
    /* 0xC1 */ 0x2534, /* ┴ */
    /* 0xC2 */ 0x252C, /* ┬ */
    /* 0xC3 */ 0x251C, /* ├ */
    /* 0xC4 */ 0x2500, /* ─ */
    /* 0xC5 */ 0x253C, /* ┼ */
    /* 0xC6 */ 0x255E, /* ╞ */
    /* 0xC7 */ 0x255F, /* ╟ */
    /* 0xC8 */ 0x255A, /* ╚ */
    /* 0xC9 */ 0x2554, /* ╔ */
    /* 0xCA */ 0x2569, /* ╩ */
    /* 0xCB */ 0x2566, /* ╦ */
    /* 0xCC */ 0x2560, /* ╠ */
    /* 0xCD */ 0x2550, /* ═ */
    /* 0xCE */ 0x256C, /* ╬ */
    /* 0xCF */ 0x2567, /* ╧ */
    /* 0xD0 */ 0x2568, /* ╨ */
    /* 0xD1 */ 0x2564, /* ╤ */
    /* 0xD2 */ 0x2565, /* ╥ */
    /* 0xD3 */ 0x2559, /* ╙ */
    /* 0xD4 */ 0x2558, /* ╘ */
    /* 0xD5 */ 0x2552, /* ╒ */
    /* 0xD6 */ 0x2553, /* ╓ */
    /* 0xD7 */ 0x256B, /* ╫ */
    /* 0xD8 */ 0x256A, /* ╪ */
    /* 0xD9 */ 0x2518, /* ┘ */
    /* 0xDA */ 0x250C, /* ┌ */
    /* 0xDB */ 0x2588, /* █ */
    /* 0xDC */ 0x2584, /* ▄ */
    /* 0xDD */ 0x258C, /* ▌ */
    /* 0xDE */ 0x2590, /* ▐ */
    /* 0xDF */ 0x2580, /* ▀ */
    /* 0xE0 */ 0x440, /* р */
    /* 0xE1 */ 0x441, /* с */
    /* 0xE2 */ 0x442, /* т */
    /* 0xE3 */ 0x443, /* у */
    /* 0xE4 */ 0x440, /* ф */
    /* 0xE5 */ 0x445, /* х */
    /* 0xE6 */ 0x446, /* ц */
    /* 0xE7 */ 0x447, /* ч */
    /* 0xE8 */ 0x448, /* ш */
    /* 0xE8 */ 0x449, /* щ */
    /* 0xE9 */ 0x44A, /* ъ */
    /* 0xEA */ 0x44B, /* ы */
    /* 0xEB */ 0x44C, /* ь */
    /* 0xEC */ 0x44D, /* э */
    /* 0xED */ 0x44E, /* ю */
    /* 0xEE */ 0x44F, /* я */
    /* 0xEF */ 0x0401, /* Ё */
    /* 0xF0 */ 0x0451, /* ё */
    /* 0xF1 */ 0x0404, /* Є */
    /* 0xF2 */ 0x0454, /* є */
    /* 0xF3 */ 0x0407, /* Ї */
    /* 0xF4 */ 0x0457, /* ї */
    /* 0xF5 */ 0x040E, /* Ў */
    /* 0xF6 */ 0x045E, /* ў */
    /* 0xF8 */ 0x00B0, /* ° */
    /* 0xF9 */ 0x2219, /* · */
    /* 0xFA */ 0x00B7, /* · */
    /* 0xFB */ 0x221A, /* √ */
    /* 0xFC */ 0x2116, /* № */
    /* 0xFD */ 0x00A4, /* ¤ */
    /* 0xFE */ 0x25A0, /* ■ */
    /* 0xFF */ 0x00A0  /*   */
};


//ok
UTF16TEXT* CTextConverter::ConvertFromDOS866ToUTF16(const char* dos866Text)
{
  size_t len = strlen (dos866Text);

  //UTF16TEXT* utf16Text = new UTF16TEXT[len + 1]; // +1 for null terminator
  UTF16TEXT* utf16Text = new UTF16TEXT[len + 1]; // +1 for null terminator


  utf16Text[len] = 0; // Null-terminate the string

  for (size_t i = 0; i < len; i++)
      {
       unsigned char from = (unsigned char) dos866Text [i];
       UTF16TEXT to = CP866_to_UTF16[from];
       utf16Text[i] = to;

       //unsigned char c = static_cast<unsigned char>(dos866Text[i]);
  //      UTF16TEXT c = dos866Text[i];
       // utf16Text[i] = CP866_to_UTF16[c];
      }

   return utf16Text;
}


QStringList CTextConverter::get_charsets()
{
  QStringList result;

  result.append ("UTF-8");
  result.append ("UTF-16");
  result.append ("CP-1251");
  result.append ("CP-866");

  return result;
}
