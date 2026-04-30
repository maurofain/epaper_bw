/*******************************************************************************
 * Size: 15 px
 * Bpp: 1
 * Opts: --bpp 1 --size 15 --no-compress --stride 1 --align 1 --font GoogleSans-VariableFont_GRAD,opsz,wght.ttf --range 32-255 --format lvgl -o GoogleSans15.c
 ******************************************************************************/

#ifdef __has_include
    #if __has_include("lvgl.h")
        #ifndef LV_LVGL_H_INCLUDE_SIMPLE
            #define LV_LVGL_H_INCLUDE_SIMPLE
        #endif
    #endif
#endif

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif



#ifndef GOOGLESANS15
#define GOOGLESANS15 1
#endif

#if GOOGLESANS15

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xaa, 0xa8, 0x3c,

    /* U+0022 "\"" */
    0xb6, 0x80,

    /* U+0023 "#" */
    0x11, 0x9, 0x84, 0x8f, 0xf1, 0x20, 0x90, 0xc8,
    0xff, 0x26, 0x12, 0x9, 0x0,

    /* U+0024 "$" */
    0x11, 0xed, 0xe4, 0x91, 0xc3, 0x87, 0x16, 0x5d,
    0xde, 0x10,

    /* U+0025 "%" */
    0x70, 0x91, 0x12, 0x24, 0x45, 0x87, 0x20, 0x8,
    0x3, 0x70, 0x51, 0x12, 0x26, 0x44, 0x87, 0x0,

    /* U+0026 "&" */
    0x38, 0x4c, 0x40, 0x60, 0x60, 0xd0, 0x98, 0x8f,
    0x86, 0x46, 0x3f,

    /* U+0027 "'" */
    0xe0,

    /* U+0028 "(" */
    0x25, 0x29, 0x24, 0x91, 0x26, 0x40,

    /* U+0029 ")" */
    0x91, 0x22, 0x49, 0x25, 0x2d, 0x0,

    /* U+002A "*" */
    0x25, 0x5c, 0xe5, 0x80,

    /* U+002B "+" */
    0x10, 0x20, 0x47, 0xf1, 0x2, 0x4, 0x0,

    /* U+002C "," */
    0xfa,

    /* U+002D "-" */
    0xf8,

    /* U+002E "." */
    0xf0,

    /* U+002F "/" */
    0x11, 0x12, 0x22, 0x64, 0x4c, 0x80,

    /* U+0030 "0" */
    0x3c, 0x66, 0x42, 0x81, 0x81, 0x81, 0x81, 0x81,
    0x42, 0x66, 0x3c,

    /* U+0031 "1" */
    0x3e, 0x92, 0x49, 0x24, 0x80,

    /* U+0032 "2" */
    0x7b, 0x30, 0x41, 0xc, 0x21, 0xc, 0x63, 0xf,
    0xc0,

    /* U+0033 "3" */
    0x39, 0x34, 0x41, 0xc, 0xe0, 0xc1, 0x87, 0x37,
    0x80,

    /* U+0034 "4" */
    0x4, 0x18, 0x70, 0xa2, 0x48, 0xb1, 0x7f, 0x4,
    0x8, 0x10,

    /* U+0035 "5" */
    0x7c, 0x81, 0x2, 0x7, 0xc8, 0x80, 0x81, 0xc2,
    0x88, 0xe0,

    /* U+0036 "6" */
    0x8, 0x30, 0xc1, 0x7, 0x98, 0xa0, 0xc1, 0x82,
    0x88, 0xe0,

    /* U+0037 "7" */
    0xfe, 0xc, 0x10, 0x60, 0x83, 0x4, 0x18, 0x60,
    0x81, 0x0,

    /* U+0038 "8" */
    0x79, 0x9a, 0x14, 0x2c, 0xcf, 0x31, 0x41, 0x83,
    0x8c, 0xe0,

    /* U+0039 "9" */
    0x38, 0x8a, 0xc, 0x18, 0x28, 0xcf, 0x4, 0x18,
    0x60, 0x80,

    /* U+003A ":" */
    0xf0, 0xf,

    /* U+003B ";" */
    0xf0, 0xf, 0xa0,

    /* U+003C "<" */
    0xc, 0xcc, 0x38, 0x38, 0x10,

    /* U+003D "=" */
    0xfe, 0x0, 0x7, 0xf0,

    /* U+003E ">" */
    0xc1, 0xc1, 0xc6, 0x62, 0x0,

    /* U+003F "?" */
    0x7b, 0x10, 0x41, 0x8, 0xc2, 0x8, 0x0, 0xc3,
    0x0,

    /* U+0040 "@" */
    0xf, 0x3, 0xc, 0x40, 0x24, 0xfb, 0x99, 0x99,
    0x9, 0x90, 0x99, 0x99, 0x4e, 0x64, 0x0, 0x30,
    0x0, 0xf8,

    /* U+0041 "A" */
    0xc, 0x3, 0x1, 0xe0, 0x48, 0x13, 0xc, 0x42,
    0x10, 0xfe, 0x60, 0x90, 0x24, 0xc,

    /* U+0042 "B" */
    0xfd, 0xe, 0xc, 0x18, 0x7f, 0xa1, 0xc1, 0x83,
    0xf, 0xf0,

    /* U+0043 "C" */
    0x1f, 0x8, 0x64, 0x2, 0x0, 0x80, 0x20, 0x8,
    0x2, 0x0, 0x40, 0x88, 0x61, 0xf0,

    /* U+0044 "D" */
    0xfc, 0x41, 0x20, 0x50, 0x18, 0xc, 0x6, 0x3,
    0x1, 0x81, 0x41, 0x3f, 0x0,

    /* U+0045 "E" */
    0xfe, 0x8, 0x20, 0x83, 0xf8, 0x20, 0x82, 0xf,
    0xc0,

    /* U+0046 "F" */
    0xfe, 0x8, 0x20, 0x83, 0xf8, 0x20, 0x82, 0x8,
    0x0,

    /* U+0047 "G" */
    0x1f, 0x4, 0x19, 0x0, 0x40, 0x8, 0x1, 0x7,
    0xe0, 0xc, 0x3, 0x40, 0x44, 0x18, 0x7c, 0x0,

    /* U+0048 "H" */
    0x81, 0x81, 0x81, 0x81, 0x81, 0xff, 0x81, 0x81,
    0x81, 0x81, 0x81,

    /* U+0049 "I" */
    0xff, 0xe0,

    /* U+004A "J" */
    0x4, 0x10, 0x41, 0x4, 0x10, 0x41, 0x87, 0x37,
    0x80,

    /* U+004B "K" */
    0x86, 0x84, 0x8c, 0x98, 0xb0, 0xb0, 0xd8, 0x88,
    0x84, 0x86, 0x82,

    /* U+004C "L" */
    0x82, 0x8, 0x20, 0x82, 0x8, 0x20, 0x82, 0xf,
    0xc0,

    /* U+004D "M" */
    0xc0, 0xf0, 0x3e, 0x1e, 0x85, 0xa1, 0x6c, 0xd9,
    0x26, 0x49, 0x8c, 0x63, 0x18, 0xc4,

    /* U+004E "N" */
    0xc1, 0xc1, 0xe1, 0xb1, 0x91, 0x99, 0x8d, 0x85,
    0x87, 0x83, 0x81,

    /* U+004F "O" */
    0x1f, 0x4, 0x11, 0x1, 0x40, 0x18, 0x3, 0x0,
    0x60, 0xc, 0x1, 0x40, 0x44, 0x10, 0x7c, 0x0,

    /* U+0050 "P" */
    0xfd, 0xe, 0xc, 0x18, 0x7f, 0xa0, 0x40, 0x81,
    0x2, 0x0,

    /* U+0051 "Q" */
    0x1f, 0x4, 0x11, 0x1, 0x40, 0x18, 0x3, 0x0,
    0x60, 0xc, 0x19, 0x41, 0x44, 0x10, 0x7f, 0x0,

    /* U+0052 "R" */
    0xfd, 0xe, 0xc, 0x18, 0x7f, 0xa6, 0x44, 0x8d,
    0xe, 0x8,

    /* U+0053 "S" */
    0x79, 0x8e, 0x4, 0x6, 0x7, 0x81, 0x81, 0x83,
    0x8d, 0xe0,

    /* U+0054 "T" */
    0xfe, 0x20, 0x40, 0x81, 0x2, 0x4, 0x8, 0x10,
    0x20, 0x40,

    /* U+0055 "U" */
    0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
    0x81, 0x42, 0x3c,

    /* U+0056 "V" */
    0xc0, 0xa0, 0xd0, 0x4c, 0x22, 0x31, 0x10, 0xc8,
    0x2c, 0x14, 0x6, 0x3, 0x0,

    /* U+0057 "W" */
    0xc3, 0xd, 0xc, 0x34, 0x38, 0x99, 0xa2, 0x64,
    0x98, 0x92, 0x42, 0x4d, 0xb, 0x14, 0x38, 0x70,
    0x61, 0x81, 0x82, 0x0,

    /* U+0058 "X" */
    0x41, 0xb1, 0x8c, 0x82, 0x81, 0xc0, 0x60, 0x50,
    0x64, 0x23, 0x30, 0xb0, 0x60,

    /* U+0059 "Y" */
    0x41, 0x31, 0x88, 0x86, 0x81, 0xc0, 0x40, 0x20,
    0x10, 0x8, 0x4, 0x2, 0x0,

    /* U+005A "Z" */
    0xfe, 0xc, 0x10, 0x41, 0x82, 0xc, 0x30, 0x41,
    0x83, 0xf8,

    /* U+005B "[" */
    0xf2, 0x49, 0x24, 0x92, 0x49, 0xc0,

    /* U+005C "\\" */
    0x8c, 0x44, 0x62, 0x22, 0x11, 0x10,

    /* U+005D "]" */
    0xe4, 0x92, 0x49, 0x24, 0x93, 0xc0,

    /* U+005E "^" */
    0x10, 0x70, 0xa2, 0x2c, 0x40,

    /* U+005F "_" */
    0xfe,

    /* U+0060 "`" */
    0x48,

    /* U+0061 "a" */
    0x7d, 0x8c, 0xb, 0xfc, 0x30, 0x61, 0xbd,

    /* U+0062 "b" */
    0x81, 0x2, 0x5, 0xcc, 0x50, 0x60, 0xc1, 0x83,
    0x8a, 0xe0,

    /* U+0063 "c" */
    0x3c, 0x8e, 0x4, 0x8, 0x10, 0x11, 0x9e,

    /* U+0064 "d" */
    0x2, 0x4, 0x9, 0xd4, 0x70, 0x60, 0xc1, 0x82,
    0x8c, 0xe8,

    /* U+0065 "e" */
    0x3c, 0x46, 0x82, 0xfe, 0x80, 0x80, 0x46, 0x3c,

    /* U+0066 "f" */
    0x3a, 0x11, 0xf4, 0x21, 0x8, 0x42, 0x10,

    /* U+0067 "g" */
    0x3a, 0x8e, 0xc, 0x18, 0x30, 0x51, 0x9d, 0x3,
    0x88, 0xe0,

    /* U+0068 "h" */
    0x82, 0x8, 0x2e, 0xce, 0x18, 0x61, 0x86, 0x18,
    0x40,

    /* U+0069 "i" */
    0xf2, 0xaa, 0xa8,

    /* U+006A "j" */
    0x6c, 0x24, 0x92, 0x49, 0x25, 0x80,

    /* U+006B "k" */
    0x82, 0x8, 0x23, 0x9a, 0xce, 0x34, 0x9a, 0x28,
    0xc0,

    /* U+006C "l" */
    0xff, 0xe0,

    /* U+006D "m" */
    0xb9, 0xd8, 0xc6, 0x10, 0xc2, 0x18, 0x43, 0x8,
    0x61, 0xc, 0x21,

    /* U+006E "n" */
    0xbb, 0x38, 0x61, 0x86, 0x18, 0x61,

    /* U+006F "o" */
    0x3c, 0x42, 0x81, 0x81, 0x81, 0x81, 0x42, 0x3c,

    /* U+0070 "p" */
    0xb9, 0x8a, 0xc, 0x18, 0x30, 0x71, 0x5c, 0x81,
    0x2, 0x0,

    /* U+0071 "q" */
    0x3a, 0x8e, 0xc, 0x18, 0x30, 0x51, 0x9d, 0x2,
    0x4, 0x8,

    /* U+0072 "r" */
    0xbc, 0x88, 0x88, 0x88,

    /* U+0073 "s" */
    0x7a, 0x18, 0x1c, 0x1c, 0x18, 0x5e,

    /* U+0074 "t" */
    0x44, 0xf4, 0x44, 0x44, 0x43,

    /* U+0075 "u" */
    0x86, 0x18, 0x61, 0x86, 0x1c, 0xdd,

    /* U+0076 "v" */
    0xc2, 0x42, 0x46, 0x64, 0x24, 0x28, 0x18, 0x18,

    /* U+0077 "w" */
    0xc6, 0x24, 0x62, 0x4e, 0x24, 0xb4, 0x69, 0x43,
    0x94, 0x31, 0xc3, 0x8,

    /* U+0078 "x" */
    0xc6, 0xd8, 0xa1, 0xc3, 0x85, 0x19, 0x63,

    /* U+0079 "y" */
    0xc2, 0x42, 0x46, 0x24, 0x2c, 0x38, 0x18, 0x10,
    0x10, 0x30, 0x20,

    /* U+007A "z" */
    0xfc, 0x31, 0x84, 0x21, 0x8c, 0x3f,

    /* U+007B "{" */
    0x34, 0x44, 0x44, 0x48, 0x44, 0x44, 0x43,

    /* U+007C "|" */
    0xff, 0xfc,

    /* U+007D "}" */
    0xc1, 0x8, 0x42, 0x10, 0x83, 0x21, 0x8, 0x42,
    0x60,

    /* U+007E "~" */
    0x73, 0x38,

    /* U+00A0 " " */
    0x0,

    /* U+00A1 "¡" */
    0xf2, 0xaa, 0xa8,

    /* U+00A2 "¢" */
    0x10, 0x20, 0xe2, 0xa9, 0x12, 0x24, 0x2b, 0x38,
    0x20, 0x40,

    /* U+00A3 "£" */
    0x79, 0x9a, 0x4, 0xc, 0x8, 0x3e, 0x10, 0x20,
    0xc7, 0xf8,

    /* U+00A4 "¤" */
    0xff, 0x38, 0x61, 0xcf, 0xf0,

    /* U+00A5 "¥" */
    0xc3, 0x62, 0x26, 0x34, 0x14, 0x7e, 0x8, 0x7e,
    0x8, 0x8, 0x8,

    /* U+00A6 "¦" */
    0xfd, 0xf8,

    /* U+00A7 "§" */
    0x7a, 0x38, 0x30, 0x72, 0x68, 0x71, 0x74, 0x60,
    0x61, 0xc5, 0xe0,

    /* U+00A8 "¨" */
    0x90,

    /* U+00A9 "©" */
    0x38, 0x8a, 0xed, 0x1b, 0xa8, 0x8e, 0x0,

    /* U+00AA "ª" */
    0x7b, 0x30, 0x5f, 0x86, 0x37, 0x40,

    /* U+00AB "«" */
    0x25, 0x29, 0x24, 0x48, 0x90,

    /* U+00AC "¬" */
    0xfc, 0x10, 0x41,

    /* U+00AD "­" */
    0x0,

    /* U+00AE "®" */
    0x38, 0x8a, 0xed, 0xdb, 0x28, 0x8e, 0x0,

    /* U+00AF "¯" */
    0xf0,

    /* U+00B0 "°" */
    0x69, 0x96,

    /* U+00B1 "±" */
    0x10, 0x23, 0xf8, 0x81, 0x2, 0x0, 0x7f,

    /* U+00B2 "²" */
    0x69, 0x12, 0x4f,

    /* U+00B3 "³" */
    0x69, 0x12, 0x17,

    /* U+00B4 "´" */
    0x48,

    /* U+00B5 "µ" */
    0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x8c, 0xf3,
    0x80, 0x80, 0x80,

    /* U+00B6 "¶" */
    0x3f, 0xf9, 0xf9, 0xf9, 0xf9, 0x79, 0x9, 0x9,
    0x9, 0x9, 0x9, 0x9, 0x9, 0x9,

    /* U+00B7 "·" */
    0xf0,

    /* U+00B8 "¸" */
    0x8f, 0x80,

    /* U+00B9 "¹" */
    0x75, 0x50,

    /* U+00BA "º" */
    0x38, 0x8a, 0xc, 0x18, 0x28, 0x8e, 0x0,

    /* U+00BB "»" */
    0x90, 0x90, 0x91, 0x24, 0x92, 0x0,

    /* U+00BC "¼" */
    0xc1, 0x61, 0x11, 0x8, 0x84, 0x80, 0xc8, 0x4c,
    0x4a, 0x67, 0xa0, 0xa0, 0x0,

    /* U+00BD "½" */
    0xc1, 0x30, 0xc4, 0x21, 0x10, 0x44, 0x2, 0x61,
    0x24, 0x41, 0x20, 0x90, 0x44, 0x3c,

    /* U+00BE "¾" */
    0x70, 0x42, 0x11, 0x84, 0x8, 0x89, 0x20, 0xc8,
    0x1, 0x30, 0x46, 0x11, 0x42, 0x3c, 0x81, 0x0,

    /* U+00BF "¿" */
    0x30, 0xc0, 0x4, 0x10, 0xc6, 0x20, 0x82, 0x37,
    0x80,

    /* U+00C0 "À" */
    0x8, 0x3, 0x0, 0x0, 0x30, 0xc, 0x7, 0x81,
    0x20, 0x4c, 0x31, 0x8, 0x43, 0xf9, 0x82, 0x40,
    0x90, 0x30,

    /* U+00C1 "Á" */
    0x6, 0x1, 0x0, 0x0, 0x30, 0xc, 0x7, 0x81,
    0x20, 0x4c, 0x31, 0x8, 0x43, 0xf9, 0x82, 0x40,
    0x90, 0x30,

    /* U+00C2 "Â" */
    0x4, 0x3, 0x80, 0x0, 0x30, 0xc, 0x7, 0x81,
    0x20, 0x4c, 0x31, 0x8, 0x43, 0xf9, 0x82, 0x40,
    0x90, 0x30,

    /* U+00C3 "Ã" */
    0x19, 0x5, 0x80, 0x0, 0x30, 0xc, 0x7, 0x81,
    0x20, 0x4c, 0x31, 0x8, 0x43, 0xf9, 0x82, 0x40,
    0x90, 0x30,

    /* U+00C4 "Ä" */
    0x12, 0x0, 0x0, 0xc0, 0x78, 0x1e, 0x4, 0x83,
    0x30, 0xcc, 0x21, 0x1f, 0xe6, 0x19, 0x2, 0xc0,
    0xc0,

    /* U+00C5 "Å" */
    0x1c, 0xa, 0x7, 0x3, 0x81, 0x40, 0xa0, 0x98,
    0x44, 0x63, 0x3f, 0x90, 0x58, 0x38, 0x8,

    /* U+00C6 "Æ" */
    0x3, 0xf8, 0x38, 0x1, 0x40, 0x1a, 0x0, 0x90,
    0x8, 0xfc, 0xc4, 0x7, 0xe0, 0x61, 0x2, 0x8,
    0x30, 0x7e,

    /* U+00C7 "Ç" */
    0x1f, 0x8, 0x64, 0x2, 0x0, 0x80, 0x20, 0x8,
    0x2, 0x0, 0x40, 0x8, 0x61, 0xf0, 0x10, 0x2,
    0x3, 0x80,

    /* U+00C8 "È" */
    0x60, 0x80, 0x3f, 0x82, 0x8, 0x20, 0xfe, 0x8,
    0x20, 0x83, 0xf0,

    /* U+00C9 "É" */
    0x10, 0xc0, 0x3f, 0x82, 0x8, 0x20, 0xfe, 0x8,
    0x20, 0x83, 0xf0,

    /* U+00CA "Ê" */
    0x21, 0xc0, 0x3f, 0x82, 0x8, 0x20, 0xfe, 0x8,
    0x20, 0x83, 0xf0,

    /* U+00CB "Ë" */
    0x48, 0xf, 0xe0, 0x82, 0x8, 0x3f, 0x82, 0x8,
    0x20, 0xfc,

    /* U+00CC "Ì" */
    0x91, 0x55, 0x55, 0x50,

    /* U+00CD "Í" */
    0x62, 0xaa, 0xaa, 0xa0,

    /* U+00CE "Î" */
    0x54, 0x24, 0x92, 0x49, 0x24, 0x80,

    /* U+00CF "Ï" */
    0xa1, 0x24, 0x92, 0x49, 0x24,

    /* U+00D0 "Ð" */
    0x7e, 0x10, 0x44, 0x9, 0x1, 0x40, 0x7e, 0x14,
    0x5, 0x1, 0x40, 0x90, 0x47, 0xe0,

    /* U+00D1 "Ñ" */
    0x36, 0x6c, 0x0, 0xc1, 0xc1, 0xe1, 0xb1, 0x91,
    0x99, 0x8d, 0x85, 0x87, 0x83, 0x81,

    /* U+00D2 "Ò" */
    0x8, 0x0, 0x80, 0x0, 0xf, 0x82, 0x8, 0x80,
    0xa0, 0xc, 0x1, 0x80, 0x30, 0x6, 0x0, 0xa0,
    0x22, 0x8, 0x3e, 0x0,

    /* U+00D3 "Ó" */
    0x2, 0x0, 0x80, 0x0, 0xf, 0x82, 0x8, 0x80,
    0xa0, 0xc, 0x1, 0x80, 0x30, 0x6, 0x0, 0xa0,
    0x22, 0x8, 0x3e, 0x0,

    /* U+00D4 "Ô" */
    0xe, 0x1, 0x40, 0x0, 0xf, 0x82, 0x8, 0x80,
    0xa0, 0xc, 0x1, 0x80, 0x30, 0x6, 0x0, 0xa0,
    0x22, 0x8, 0x3e, 0x0,

    /* U+00D5 "Õ" */
    0x1d, 0x2, 0xe0, 0x0, 0xf, 0x82, 0x8, 0x80,
    0xa0, 0xc, 0x1, 0x80, 0x30, 0x6, 0x0, 0xa0,
    0x22, 0x8, 0x3e, 0x0,

    /* U+00D6 "Ö" */
    0xa, 0x0, 0x0, 0x0, 0xf, 0x82, 0x8, 0x80,
    0xa0, 0xc, 0x1, 0x80, 0x30, 0x6, 0x0, 0xa0,
    0x22, 0x8, 0x3e, 0x0,

    /* U+00D7 "×" */
    0x85, 0x23, 0xc, 0x4a, 0x10,

    /* U+00D8 "Ø" */
    0x1f, 0xc4, 0x11, 0x5, 0x41, 0x98, 0x23, 0x8,
    0x61, 0xc, 0x41, 0x58, 0x46, 0x10, 0xfc, 0x10,
    0x0,

    /* U+00D9 "Ù" */
    0x30, 0x10, 0x0, 0x81, 0x81, 0x81, 0x81, 0x81,
    0x81, 0x81, 0x81, 0x81, 0x42, 0x3c,

    /* U+00DA "Ú" */
    0xc, 0x8, 0x0, 0x81, 0x81, 0x81, 0x81, 0x81,
    0x81, 0x81, 0x81, 0x81, 0x42, 0x3c,

    /* U+00DB "Û" */
    0x18, 0x24, 0x0, 0x81, 0x81, 0x81, 0x81, 0x81,
    0x81, 0x81, 0x81, 0x81, 0x42, 0x3c,

    /* U+00DC "Ü" */
    0x24, 0x0, 0x0, 0x81, 0x81, 0x81, 0x81, 0x81,
    0x81, 0x81, 0x81, 0x81, 0x42, 0x3c,

    /* U+00DD "Ý" */
    0xc, 0x4, 0x0, 0x8, 0x26, 0x31, 0x10, 0xd0,
    0x38, 0x8, 0x4, 0x2, 0x1, 0x0, 0x80, 0x40,

    /* U+00DE "Þ" */
    0x81, 0x3, 0xe4, 0x38, 0x30, 0x61, 0xfe, 0x81,
    0x2, 0x0,

    /* U+00DF "ß" */
    0x79, 0x9a, 0x14, 0x28, 0xd7, 0x21, 0x41, 0x83,
    0xe, 0xe0,

    /* U+00E0 "à" */
    0x30, 0x20, 0x3, 0xec, 0x60, 0x5f, 0xe1, 0x83,
    0xd, 0xe8,

    /* U+00E1 "á" */
    0x8, 0x30, 0x3, 0xec, 0x60, 0x5f, 0xe1, 0x83,
    0xd, 0xe8,

    /* U+00E2 "â" */
    0x38, 0xd8, 0x3, 0xec, 0x60, 0x5f, 0xe1, 0x83,
    0xd, 0xe8,

    /* U+00E3 "ã" */
    0x7c, 0x0, 0x3, 0xec, 0x60, 0x5f, 0xe1, 0x83,
    0xd, 0xe8,

    /* U+00E4 "ä" */
    0x28, 0x0, 0x3, 0xec, 0x60, 0x5f, 0xe1, 0x83,
    0xd, 0xe8,

    /* U+00E5 "å" */
    0x38, 0x50, 0xe0, 0x7, 0xd8, 0xc0, 0xbf, 0xc3,
    0x6, 0x1b, 0xd0,

    /* U+00E6 "æ" */
    0x79, 0xe6, 0x38, 0x81, 0x84, 0xff, 0xec, 0x60,
    0x43, 0x2, 0x3c, 0x4f, 0x3c,

    /* U+00E7 "ç" */
    0x3c, 0x8e, 0x4, 0x8, 0x10, 0x11, 0x9e, 0x10,
    0x10, 0x60,

    /* U+00E8 "è" */
    0x30, 0x10, 0x0, 0x3c, 0x46, 0x82, 0xfe, 0x80,
    0x80, 0x46, 0x3c,

    /* U+00E9 "é" */
    0x8, 0x18, 0x0, 0x3c, 0x46, 0x82, 0xfe, 0x80,
    0x80, 0x46, 0x3c,

    /* U+00EA "ê" */
    0x18, 0x2c, 0x0, 0x3c, 0x46, 0x82, 0xfe, 0x80,
    0x80, 0x46, 0x3c,

    /* U+00EB "ë" */
    0x28, 0x0, 0x0, 0x3c, 0x46, 0x86, 0xfe, 0x80,
    0x80, 0x46, 0x3c,

    /* U+00EC "ì" */
    0xd1, 0x55, 0x54,

    /* U+00ED "í" */
    0x62, 0xaa, 0xa8,

    /* U+00EE "î" */
    0x54, 0x24, 0x92, 0x49, 0x0,

    /* U+00EF "ï" */
    0xa0, 0x24, 0x92, 0x49, 0x0,

    /* U+00F0 "ð" */
    0x0, 0xf8, 0xe0, 0x23, 0xc8, 0xe0, 0xc1, 0x83,
    0x5, 0x11, 0xc0,

    /* U+00F1 "ñ" */
    0x78, 0x0, 0x2e, 0xce, 0x18, 0x61, 0x86, 0x18,
    0x40,

    /* U+00F2 "ò" */
    0x30, 0x10, 0x0, 0x3c, 0x42, 0x81, 0x81, 0x81,
    0x81, 0x42, 0x3c,

    /* U+00F3 "ó" */
    0xc, 0x8, 0x0, 0x3c, 0x42, 0x81, 0x81, 0x81,
    0x81, 0x42, 0x3c,

    /* U+00F4 "ô" */
    0x18, 0x24, 0x0, 0x3c, 0x42, 0x81, 0x81, 0x81,
    0x81, 0x42, 0x3c,

    /* U+00F5 "õ" */
    0x7e, 0x0, 0x0, 0x3c, 0x42, 0x81, 0x81, 0x81,
    0x81, 0x42, 0x3c,

    /* U+00F6 "ö" */
    0x24, 0x0, 0x0, 0x3c, 0x42, 0x81, 0x81, 0x81,
    0x81, 0x42, 0x3c,

    /* U+00F7 "÷" */
    0x30, 0x60, 0x0, 0xf, 0xe0, 0xc, 0x18,

    /* U+00F8 "ø" */
    0x3e, 0x46, 0x8d, 0x89, 0x91, 0xb1, 0x62, 0x7c,

    /* U+00F9 "ù" */
    0x20, 0x80, 0x21, 0x86, 0x18, 0x61, 0x87, 0x37,
    0x40,

    /* U+00FA "ú" */
    0x10, 0x40, 0x21, 0x86, 0x18, 0x61, 0x87, 0x37,
    0x40,

    /* U+00FB "û" */
    0x31, 0x20, 0x21, 0x86, 0x18, 0x61, 0x87, 0x37,
    0x40,

    /* U+00FC "ü" */
    0x28, 0x0, 0x31, 0xc7, 0x1c, 0x71, 0xc7, 0x37,
    0x40,

    /* U+00FD "ý" */
    0x8, 0x18, 0x0, 0xc2, 0x42, 0x46, 0x24, 0x2c,
    0x38, 0x18, 0x10, 0x10, 0x30, 0x20,

    /* U+00FE "þ" */
    0x81, 0x2, 0x5, 0xcc, 0x50, 0x60, 0xc1, 0x83,
    0x8a, 0xe4, 0x8, 0x10, 0x0,

    /* U+00FF "ÿ" */
    0x24, 0x0, 0x0, 0x43, 0x42, 0x66, 0x24, 0x34,
    0x18, 0x18, 0x18, 0x10, 0x30, 0x20
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 56, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 57, .box_w = 2, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 4, .adv_w = 77, .box_w = 3, .box_h = 3, .ofs_x = 1, .ofs_y = 8},
    {.bitmap_index = 6, .adv_w = 154, .box_w = 9, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 19, .adv_w = 130, .box_w = 6, .box_h = 13, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 29, .adv_w = 199, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 45, .adv_w = 150, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 56, .adv_w = 42, .box_w = 1, .box_h = 3, .ofs_x = 1, .ofs_y = 8},
    {.bitmap_index = 57, .adv_w = 77, .box_w = 3, .box_h = 14, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 63, .adv_w = 77, .box_w = 3, .box_h = 14, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 69, .adv_w = 102, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 73, .adv_w = 133, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 80, .adv_w = 57, .box_w = 2, .box_h = 4, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 81, .adv_w = 105, .box_w = 5, .box_h = 1, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 82, .adv_w = 57, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 83, .adv_w = 72, .box_w = 4, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 89, .adv_w = 154, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 100, .adv_w = 103, .box_w = 3, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 105, .adv_w = 126, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 114, .adv_w = 128, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 123, .adv_w = 142, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 133, .adv_w = 134, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 143, .adv_w = 133, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 153, .adv_w = 126, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 163, .adv_w = 132, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 173, .adv_w = 133, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 183, .adv_w = 57, .box_w = 2, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 185, .adv_w = 57, .box_w = 2, .box_h = 10, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 188, .adv_w = 119, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 193, .adv_w = 136, .box_w = 7, .box_h = 4, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 197, .adv_w = 119, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 202, .adv_w = 117, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 211, .adv_w = 212, .box_w = 12, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 229, .adv_w = 161, .box_w = 10, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 243, .adv_w = 144, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 253, .adv_w = 177, .box_w = 10, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 267, .adv_w = 168, .box_w = 9, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 280, .adv_w = 131, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 289, .adv_w = 127, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 298, .adv_w = 193, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 314, .adv_w = 167, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 325, .adv_w = 58, .box_w = 1, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 327, .adv_w = 127, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 336, .adv_w = 150, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 347, .adv_w = 121, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 356, .adv_w = 207, .box_w = 10, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 370, .adv_w = 169, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 381, .adv_w = 197, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 397, .adv_w = 138, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 407, .adv_w = 197, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 423, .adv_w = 142, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 433, .adv_w = 133, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 443, .adv_w = 127, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 453, .adv_w = 160, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 464, .adv_w = 152, .box_w = 9, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 477, .adv_w = 227, .box_w = 14, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 497, .adv_w = 149, .box_w = 9, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 510, .adv_w = 140, .box_w = 9, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 523, .adv_w = 137, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 533, .adv_w = 76, .box_w = 3, .box_h = 14, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 539, .adv_w = 72, .box_w = 4, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 545, .adv_w = 76, .box_w = 3, .box_h = 14, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 551, .adv_w = 111, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 556, .adv_w = 114, .box_w = 7, .box_h = 1, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 557, .adv_w = 95, .box_w = 3, .box_h = 2, .ofs_x = 1, .ofs_y = 9},
    {.bitmap_index = 558, .adv_w = 127, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 565, .adv_w = 144, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 575, .adv_w = 130, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 582, .adv_w = 144, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 592, .adv_w = 135, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 600, .adv_w = 87, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 607, .adv_w = 143, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 617, .adv_w = 135, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 626, .adv_w = 50, .box_w = 2, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 629, .adv_w = 50, .box_w = 3, .box_h = 14, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 635, .adv_w = 121, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 644, .adv_w = 50, .box_w = 1, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 646, .adv_w = 210, .box_w = 11, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 657, .adv_w = 134, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 663, .adv_w = 143, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 671, .adv_w = 144, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 681, .adv_w = 144, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 691, .adv_w = 89, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 695, .adv_w = 113, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 701, .adv_w = 87, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 706, .adv_w = 134, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 712, .adv_w = 122, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 720, .adv_w = 186, .box_w = 12, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 732, .adv_w = 114, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 739, .adv_w = 122, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 750, .adv_w = 117, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 756, .adv_w = 75, .box_w = 4, .box_h = 14, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 763, .adv_w = 56, .box_w = 1, .box_h = 14, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 765, .adv_w = 75, .box_w = 5, .box_h = 14, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 774, .adv_w = 128, .box_w = 7, .box_h = 2, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 776, .adv_w = 56, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 777, .adv_w = 57, .box_w = 2, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 780, .adv_w = 127, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 790, .adv_w = 126, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 800, .adv_w = 146, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 805, .adv_w = 137, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 816, .adv_w = 56, .box_w = 1, .box_h = 13, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 818, .adv_w = 131, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 829, .adv_w = 94, .box_w = 4, .box_h = 1, .ofs_x = 1, .ofs_y = 10},
    {.bitmap_index = 830, .adv_w = 127, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 837, .adv_w = 113, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 843, .adv_w = 131, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 848, .adv_w = 137, .box_w = 6, .box_h = 4, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 851, .adv_w = 0, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 852, .adv_w = 127, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 859, .adv_w = 96, .box_w = 4, .box_h = 1, .ofs_x = 1, .ofs_y = 10},
    {.bitmap_index = 860, .adv_w = 77, .box_w = 4, .box_h = 4, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 862, .adv_w = 130, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 869, .adv_w = 72, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 7},
    {.bitmap_index = 872, .adv_w = 76, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 7},
    {.bitmap_index = 875, .adv_w = 95, .box_w = 3, .box_h = 2, .ofs_x = 2, .ofs_y = 9},
    {.bitmap_index = 876, .adv_w = 151, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 887, .adv_w = 147, .box_w = 8, .box_h = 14, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 901, .adv_w = 57, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 902, .adv_w = 97, .box_w = 3, .box_h = 3, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 904, .adv_w = 56, .box_w = 2, .box_h = 6, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 906, .adv_w = 123, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 913, .adv_w = 131, .box_w = 7, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 919, .adv_w = 162, .box_w = 9, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 932, .adv_w = 172, .box_w = 10, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 946, .adv_w = 185, .box_w = 11, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 962, .adv_w = 111, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 971, .adv_w = 161, .box_w = 10, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 989, .adv_w = 161, .box_w = 10, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1007, .adv_w = 161, .box_w = 10, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1025, .adv_w = 161, .box_w = 10, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1043, .adv_w = 161, .box_w = 10, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1060, .adv_w = 161, .box_w = 9, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1075, .adv_w = 229, .box_w = 13, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1093, .adv_w = 177, .box_w = 10, .box_h = 14, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1111, .adv_w = 131, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1122, .adv_w = 131, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1133, .adv_w = 131, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1144, .adv_w = 131, .box_w = 6, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1154, .adv_w = 58, .box_w = 2, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1158, .adv_w = 58, .box_w = 2, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1162, .adv_w = 58, .box_w = 3, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1168, .adv_w = 58, .box_w = 3, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1173, .adv_w = 171, .box_w = 10, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1187, .adv_w = 169, .box_w = 8, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1201, .adv_w = 197, .box_w = 11, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1221, .adv_w = 197, .box_w = 11, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1241, .adv_w = 197, .box_w = 11, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1261, .adv_w = 197, .box_w = 11, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1281, .adv_w = 197, .box_w = 11, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1301, .adv_w = 131, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 1306, .adv_w = 197, .box_w = 11, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1323, .adv_w = 160, .box_w = 8, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1337, .adv_w = 160, .box_w = 8, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1351, .adv_w = 160, .box_w = 8, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1365, .adv_w = 160, .box_w = 8, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1379, .adv_w = 140, .box_w = 9, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1395, .adv_w = 138, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1405, .adv_w = 133, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1415, .adv_w = 127, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1425, .adv_w = 127, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1435, .adv_w = 127, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1445, .adv_w = 127, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1455, .adv_w = 127, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1465, .adv_w = 127, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1476, .adv_w = 221, .box_w = 13, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1489, .adv_w = 130, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1499, .adv_w = 135, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1510, .adv_w = 135, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1521, .adv_w = 135, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1532, .adv_w = 135, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1543, .adv_w = 50, .box_w = 2, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1546, .adv_w = 50, .box_w = 2, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1549, .adv_w = 50, .box_w = 3, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1554, .adv_w = 50, .box_w = 3, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1559, .adv_w = 136, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1570, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1579, .adv_w = 143, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1590, .adv_w = 143, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1601, .adv_w = 143, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1612, .adv_w = 143, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1623, .adv_w = 143, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1634, .adv_w = 130, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1641, .adv_w = 143, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1649, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1658, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1667, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1676, .adv_w = 134, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1685, .adv_w = 122, .box_w = 8, .box_h = 14, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 1699, .adv_w = 144, .box_w = 7, .box_h = 14, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1712, .adv_w = 122, .box_w = 8, .box_h = 14, .ofs_x = 0, .ofs_y = -3}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 160, .range_length = 96, .glyph_id_start = 96,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
};

/*-----------------
 *    KERNING
 *----------------*/


/*Pair left and right glyphs for kerning*/
static const uint8_t kern_pair_glyph_ids[] =
{
    3, 13,
    3, 15,
    3, 21,
    3, 23,
    7, 61,
    8, 13,
    8, 15,
    8, 21,
    8, 23,
    9, 17,
    9, 21,
    12, 18,
    13, 3,
    13, 8,
    13, 18,
    13, 61,
    14, 18,
    15, 3,
    15, 8,
    15, 18,
    15, 61,
    16, 7,
    16, 13,
    16, 15,
    16, 16,
    16, 17,
    16, 21,
    16, 23,
    16, 61,
    17, 10,
    17, 19,
    17, 24,
    17, 62,
    17, 94,
    19, 21,
    20, 18,
    20, 24,
    20, 26,
    20, 61,
    21, 3,
    21, 8,
    21, 10,
    21, 11,
    21, 18,
    21, 24,
    21, 26,
    21, 62,
    21, 94,
    21, 112,
    21, 121,
    21, 124,
    21, 125,
    22, 11,
    22, 18,
    22, 19,
    22, 20,
    22, 21,
    22, 24,
    22, 26,
    23, 3,
    23, 6,
    23, 8,
    23, 11,
    23, 18,
    23, 19,
    23, 20,
    23, 24,
    23, 26,
    23, 61,
    23, 112,
    23, 121,
    23, 124,
    23, 125,
    24, 13,
    24, 15,
    24, 16,
    24, 17,
    24, 19,
    24, 20,
    24, 21,
    24, 23,
    24, 25,
    24, 107,
    25, 18,
    25, 24,
    25, 26,
    25, 61,
    26, 13,
    26, 15,
    26, 16,
    26, 19,
    26, 21,
    26, 23,
    26, 24,
    31, 18,
    33, 18,
    60, 17,
    60, 21,
    61, 16,
    61, 18,
    61, 61,
    92, 17,
    92, 21,
    101, 21,
    123, 24,
    127, 17
};

/* Kerning between the respective left and right glyphs
 * 4.4 format which needs to scaled with `kern_scale`*/
static const int8_t kern_pair_values[] =
{
    -14, -14, -10, -7, -14, -14, -14, -10,
    -7, -2, -5, -7, -14, -14, -10, -5,
    -7, -14, -14, -10, -5, -5, -5, -5,
    -5, -5, -14, -10, 5, -2, -2, -5,
    -2, -2, -5, -5, -2, -2, -2, -12,
    -12, -5, -10, -10, -5, -7, -5, -5,
    -7, -5, -5, -5, -10, -10, -7, -2,
    -2, -7, -7, -14, -5, -14, -7, -17,
    -5, -2, -10, -12, -10, -14, -5, -5,
    -5, -26, -26, -17, -7, -5, -5, -17,
    -14, -5, -5, -5, -2, -2, -2, -12,
    -12, -5, -5, -7, -5, -2, -7, -7,
    -2, -5, 5, -12, -5, -2, -5, -2,
    -5, -12
};

/*Collect the kern pair's data in one place*/
static const lv_font_fmt_txt_kern_pair_t kern_pairs =
{
    .glyph_ids = kern_pair_glyph_ids,
    .values = kern_pair_values,
    .pair_cnt = 106,
    .glyph_ids_size = 0
};

/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = &kern_pairs,
    .kern_scale = 16,
    .cmap_num = 2,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif

};

extern const lv_font_t lv_font_montserrat_10;


/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t GoogleSans15 = {
#else
lv_font_t GoogleSans15 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 18,          /*The maximum line height required by the font*/
    .base_line = 4,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -2,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = &lv_font_montserrat_14,
#endif
    .user_data = NULL,
};



#endif /*#if GOOGLESANS15*/
