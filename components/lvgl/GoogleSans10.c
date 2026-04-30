/*******************************************************************************
 * Size: 10 px
 * Bpp: 1
 * Opts: --bpp 1 --size 10 --no-compress --stride 1 --align 1 --font GoogleSans-VariableFont_GRAD,opsz,wght.ttf --range 32-255 --format lvgl -o GoogleSans10.c
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



#ifndef GOOGLESANS10
#define GOOGLESANS10 1
#endif

#if GOOGLESANS10

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xfa,

    /* U+0022 "\"" */
    0xf0,

    /* U+0023 "#" */
    0x28, 0xa7, 0xca, 0xfd, 0x45, 0x0,

    /* U+0024 "$" */
    0x23, 0xab, 0x46, 0x1e, 0xae, 0x20,

    /* U+0025 "%" */
    0x60, 0x94, 0x68, 0x10, 0x16, 0x29, 0x46,

    /* U+0026 "&" */
    0x31, 0x4, 0x38, 0x97, 0x27, 0x40,

    /* U+0027 "'" */
    0xc0,

    /* U+0028 "(" */
    0x4a, 0x49, 0x12,

    /* U+0029 ")" */
    0x48, 0x92, 0x52,

    /* U+002A "*" */
    0x5e, 0x80,

    /* U+002B "+" */
    0x21, 0x3e, 0x40,

    /* U+002C "," */
    0x60,

    /* U+002D "-" */
    0xe0,

    /* U+002E "." */
    0x80,

    /* U+002F "/" */
    0x25, 0x25, 0x20,

    /* U+0030 "0" */
    0x7b, 0x28, 0x61, 0x87, 0x37, 0x80,

    /* U+0031 "1" */
    0x3c, 0x92, 0x48,

    /* U+0032 "2" */
    0xe9, 0x12, 0x48, 0xf0,

    /* U+0033 "3" */
    0xe9, 0x16, 0x19, 0xe0,

    /* U+0034 "4" */
    0x8, 0x62, 0x8a, 0x4b, 0xf0, 0x80,

    /* U+0035 "5" */
    0x74, 0x3c, 0x10, 0xc5, 0xc0,

    /* U+0036 "6" */
    0x1, 0x10, 0xe8, 0xc6, 0x2e,

    /* U+0037 "7" */
    0xf1, 0x22, 0x44, 0x80,

    /* U+0038 "8" */
    0x64, 0xa4, 0xc9, 0xc5, 0xc0,

    /* U+0039 "9" */
    0x74, 0x63, 0x17, 0x8, 0x80,

    /* U+003A ":" */
    0x88,

    /* U+003B ";" */
    0x40, 0x40,

    /* U+003C "<" */
    0x7, 0x87,

    /* U+003D "=" */
    0xf0, 0xf0,

    /* U+003E ">" */
    0xe, 0x1e,

    /* U+003F "?" */
    0x69, 0x12, 0x40, 0x40,

    /* U+0040 "@" */
    0x3c, 0x42, 0x9d, 0xa5, 0xa5, 0x9e, 0x40, 0x3c,

    /* U+0041 "A" */
    0x10, 0x60, 0xa2, 0x47, 0xc8, 0xa1, 0x0,

    /* U+0042 "B" */
    0xf4, 0x63, 0xe8, 0xc7, 0xc0,

    /* U+0043 "C" */
    0x3c, 0x8a, 0x4, 0x8, 0x8, 0x4f, 0x0,

    /* U+0044 "D" */
    0xf2, 0x28, 0x61, 0x86, 0x2f, 0x0,

    /* U+0045 "E" */
    0xf8, 0x8f, 0x88, 0xf0,

    /* U+0046 "F" */
    0xf8, 0x8f, 0x88, 0x80,

    /* U+0047 "G" */
    0x38, 0x8a, 0x4, 0x78, 0x28, 0x8e, 0x0,

    /* U+0048 "H" */
    0x8c, 0x63, 0xf8, 0xc6, 0x20,

    /* U+0049 "I" */
    0xfe,

    /* U+004A "J" */
    0x8, 0x42, 0x10, 0xa5, 0xc0,

    /* U+004B "K" */
    0x8a, 0x4a, 0x38, 0x92, 0x48, 0x80,

    /* U+004C "L" */
    0x88, 0x88, 0x88, 0xf0,

    /* U+004D "M" */
    0xc7, 0x8f, 0x1d, 0x5a, 0xb6, 0x64, 0x80,

    /* U+004E "N" */
    0xc7, 0x1a, 0x65, 0x96, 0x38, 0x40,

    /* U+004F "O" */
    0x38, 0x8a, 0xc, 0x18, 0x28, 0x8e, 0x0,

    /* U+0050 "P" */
    0xf4, 0x63, 0xe8, 0x42, 0x0,

    /* U+0051 "Q" */
    0x38, 0x8a, 0xc, 0x18, 0xa9, 0x8f, 0x0,

    /* U+0052 "R" */
    0xf4, 0x63, 0xe9, 0x4a, 0x20,

    /* U+0053 "S" */
    0x74, 0x60, 0xe0, 0xc5, 0xc0,

    /* U+0054 "T" */
    0xf9, 0x8, 0x42, 0x10, 0x80,

    /* U+0055 "U" */
    0x8c, 0x63, 0x18, 0xc5, 0xc0,

    /* U+0056 "V" */
    0x85, 0x14, 0x92, 0x28, 0xc3, 0x0,

    /* U+0057 "W" */
    0x88, 0xc6, 0x55, 0x2a, 0xa5, 0x31, 0x18, 0x88,

    /* U+0058 "X" */
    0x45, 0x23, 0x4, 0x31, 0x28, 0x40,

    /* U+0059 "Y" */
    0x8e, 0x94, 0x42, 0x10, 0x80,

    /* U+005A "Z" */
    0xf8, 0x44, 0x44, 0x43, 0xe0,

    /* U+005B "[" */
    0xea, 0xaa, 0xc0,

    /* U+005C "\\" */
    0x91, 0x24, 0x48,

    /* U+005D "]" */
    0xd5, 0x55, 0xc0,

    /* U+005E "^" */
    0x22, 0xa4,

    /* U+005F "_" */
    0xf8,

    /* U+0060 "`" */
    0x80,

    /* U+0061 "a" */
    0x70, 0x5f, 0x17, 0x80,

    /* U+0062 "b" */
    0x84, 0x3d, 0x18, 0xc7, 0xc0,

    /* U+0063 "c" */
    0x74, 0x61, 0x17, 0x0,

    /* U+0064 "d" */
    0x8, 0x5f, 0x18, 0xc5, 0xe0,

    /* U+0065 "e" */
    0x74, 0x7f, 0x7, 0x0,

    /* U+0066 "f" */
    0x74, 0xe4, 0x44, 0x40,

    /* U+0067 "g" */
    0x7c, 0x63, 0x17, 0xc5, 0xc0,

    /* U+0068 "h" */
    0x88, 0xf9, 0x99, 0x90,

    /* U+0069 "i" */
    0xc5, 0x54,

    /* U+006A "j" */
    0xc5, 0x55, 0xc0,

    /* U+006B "k" */
    0x84, 0x25, 0x4e, 0x5a, 0x40,

    /* U+006C "l" */
    0xfe,

    /* U+006D "m" */
    0xef, 0x26, 0x4c, 0x99, 0x20,

    /* U+006E "n" */
    0xf9, 0x99, 0x90,

    /* U+006F "o" */
    0x74, 0x63, 0x17, 0x0,

    /* U+0070 "p" */
    0xf4, 0x63, 0x1f, 0x42, 0x0,

    /* U+0071 "q" */
    0x6c, 0x63, 0x17, 0x84, 0x20,

    /* U+0072 "r" */
    0xf2, 0x48,

    /* U+0073 "s" */
    0xe8, 0x69, 0xf0,

    /* U+0074 "t" */
    0x4b, 0xa4, 0x98,

    /* U+0075 "u" */
    0x99, 0x99, 0xf0,

    /* U+0076 "v" */
    0x8a, 0x94, 0xa2, 0x0,

    /* U+0077 "w" */
    0x91, 0x9a, 0x6a, 0x6a, 0x24,

    /* U+0078 "x" */
    0x92, 0x88, 0xa9, 0x0,

    /* U+0079 "y" */
    0x8a, 0x94, 0x42, 0x11, 0x0,

    /* U+007A "z" */
    0xf2, 0x64, 0xf0,

    /* U+007B "{" */
    0x69, 0x28, 0x92, 0x60,

    /* U+007C "|" */
    0xff, 0x80,

    /* U+007D "}" */
    0xc9, 0x22, 0x92, 0xc0,

    /* U+007E "~" */
    0x6c, 0xc0,

    /* U+00A0 " " */
    0x0,

    /* U+00A1 "¡" */
    0xbe,

    /* U+00A2 "¢" */
    0x23, 0xab, 0x4a, 0xb8, 0x80,

    /* U+00A3 "£" */
    0x64, 0xa0, 0x8e, 0x23, 0xc0,

    /* U+00A4 "¤" */
    0xf9, 0x99, 0xf0,

    /* U+00A5 "¥" */
    0x8a, 0x95, 0xff, 0x90, 0x80,

    /* U+00A6 "¦" */
    0xef,

    /* U+00A7 "§" */
    0x69, 0xcb, 0x97, 0x19, 0x60,

    /* U+00A8 "¨" */
    0xc0,

    /* U+00A9 "©" */
    0x77, 0x73, 0xd7, 0x0,

    /* U+00AA "ª" */
    0x61, 0x79, 0x70,

    /* U+00AB "«" */
    0x2a, 0xa8, 0xa0,

    /* U+00AC "¬" */
    0xf8, 0x42,

    /* U+00AD "­" */
    0x0,

    /* U+00AE "®" */
    0x75, 0x6b, 0x17, 0x0,

    /* U+00AF "¯" */
    0xc0,

    /* U+00B0 "°" */
    0xdb, 0x0,

    /* U+00B1 "±" */
    0x27, 0xc8, 0xf, 0x80,

    /* U+00B2 "²" */
    0xdc,

    /* U+00B3 "³" */
    0x6d, 0x80,

    /* U+00B4 "´" */
    0x80,

    /* U+00B5 "µ" */
    0x94, 0xa5, 0x2f, 0xc2, 0x0,

    /* U+00B6 "¶" */
    0x7d, 0xd7, 0x5d, 0x14, 0x51, 0x45, 0x14,

    /* U+00B7 "·" */
    0x80,

    /* U+00B8 "¸" */
    0xb0,

    /* U+00B9 "¹" */
    0xd4,

    /* U+00BA "º" */
    0x74, 0x63, 0x17, 0x0,

    /* U+00BB "»" */
    0xa, 0x5a,

    /* U+00BC "¼" */
    0xc8, 0x91, 0x41, 0x23, 0xc9, 0xe1, 0x0,

    /* U+00BD "½" */
    0xc4, 0x91, 0x20, 0xb2, 0x28, 0x91, 0x80,

    /* U+00BE "¾" */
    0xc0, 0x8b, 0x26, 0x81, 0x65, 0xd0, 0x80,

    /* U+00BF "¿" */
    0x20, 0x8, 0x88, 0x45, 0xc0,

    /* U+00C0 "À" */
    0x20, 0x20, 0xc1, 0x44, 0x8f, 0x91, 0x42,

    /* U+00C1 "Á" */
    0x10, 0x20, 0xc1, 0x44, 0x8f, 0x91, 0x42,

    /* U+00C2 "Â" */
    0x10, 0x50, 0x41, 0x82, 0x89, 0x1f, 0x22, 0x84,

    /* U+00C3 "Ã" */
    0x28, 0xb0, 0x41, 0x82, 0x89, 0x1f, 0x22, 0x84,

    /* U+00C4 "Ä" */
    0x28, 0x0, 0x41, 0x82, 0x89, 0x1f, 0x22, 0x84,

    /* U+00C5 "Å" */
    0x30, 0x60, 0xc1, 0x44, 0x8f, 0x91, 0x42,

    /* U+00C6 "Æ" */
    0xf, 0x86, 0x5, 0x4, 0xf3, 0xc2, 0x23, 0x1e,

    /* U+00C7 "Ç" */
    0x3c, 0x82, 0x4, 0x8, 0x8, 0x4f, 0xc, 0x18,

    /* U+00C8 "È" */
    0x4f, 0x88, 0xf8, 0x8f,

    /* U+00C9 "É" */
    0x2f, 0x88, 0xf8, 0x8f,

    /* U+00CA "Ê" */
    0x25, 0xf8, 0x8f, 0x88, 0xf0,

    /* U+00CB "Ë" */
    0xe0, 0xf8, 0x8f, 0x88, 0xf0,

    /* U+00CC "Ì" */
    0x55, 0x55,

    /* U+00CD "Í" */
    0xaa, 0xaa,

    /* U+00CE "Î" */
    0x55, 0x24, 0x92, 0x40,

    /* U+00CF "Ï" */
    0xc5, 0x55, 0x40,

    /* U+00D0 "Ð" */
    0x78, 0x89, 0xf, 0x94, 0x28, 0x9e, 0x0,

    /* U+00D1 "Ñ" */
    0x29, 0x4c, 0x71, 0xa6, 0x59, 0x63, 0x84,

    /* U+00D2 "Ò" */
    0x10, 0x0, 0xe2, 0x28, 0x30, 0x60, 0xa2, 0x38,

    /* U+00D3 "Ó" */
    0x10, 0x0, 0xe2, 0x28, 0x30, 0x60, 0xa2, 0x38,

    /* U+00D4 "Ô" */
    0x10, 0x50, 0xe2, 0x28, 0x30, 0x60, 0xa2, 0x38,

    /* U+00D5 "Õ" */
    0x38, 0x70, 0xe2, 0x28, 0x30, 0x60, 0xa2, 0x38,

    /* U+00D6 "Ö" */
    0x18, 0x0, 0xe2, 0x28, 0x30, 0x60, 0xa2, 0x38,

    /* U+00D7 "×" */
    0xa4, 0xa0,

    /* U+00D8 "Ø" */
    0x3c, 0x9a, 0x4c, 0x9a, 0x28, 0x9e, 0x0,

    /* U+00D9 "Ù" */
    0x20, 0x23, 0x18, 0xc6, 0x31, 0x70,

    /* U+00DA "Ú" */
    0x20, 0x23, 0x18, 0xc6, 0x31, 0x70,

    /* U+00DB "Û" */
    0x22, 0xa3, 0x18, 0xc6, 0x31, 0x70,

    /* U+00DC "Ü" */
    0x50, 0x23, 0x18, 0xc6, 0x31, 0x70,

    /* U+00DD "Ý" */
    0x24, 0x74, 0xa2, 0x10, 0x84,

    /* U+00DE "Þ" */
    0x87, 0xa3, 0x18, 0xfa, 0x0,

    /* U+00DF "ß" */
    0x64, 0xa5, 0x49, 0xc6, 0xc0,

    /* U+00E0 "à" */
    0x20, 0x1c, 0x17, 0xc5, 0xe0,

    /* U+00E1 "á" */
    0x20, 0x1c, 0x17, 0xc5, 0xe0,

    /* U+00E2 "â" */
    0x22, 0x9c, 0x17, 0xc5, 0xe0,

    /* U+00E3 "ã" */
    0x70, 0x1c, 0x17, 0xc5, 0xe0,

    /* U+00E4 "ä" */
    0x50, 0x1c, 0x17, 0xc5, 0xe0,

    /* U+00E5 "å" */
    0x31, 0x80, 0xe0, 0xbe, 0x2f,

    /* U+00E6 "æ" */
    0x77, 0x4, 0x9f, 0xd3, 0xe, 0x70,

    /* U+00E7 "ç" */
    0x74, 0x61, 0x17, 0x10, 0x80,

    /* U+00E8 "è" */
    0x20, 0x1d, 0x1f, 0xc1, 0xc0,

    /* U+00E9 "é" */
    0x20, 0x1d, 0x1f, 0xc1, 0xc0,

    /* U+00EA "ê" */
    0x20, 0x9d, 0x1f, 0xc1, 0xc0,

    /* U+00EB "ë" */
    0x70, 0x1d, 0x1f, 0xc1, 0xc0,

    /* U+00EC "ì" */
    0x45, 0x54,

    /* U+00ED "í" */
    0x8a, 0xa8,

    /* U+00EE "î" */
    0x41, 0x24, 0x90,

    /* U+00EF "ï" */
    0xc5, 0x54,

    /* U+00F0 "ð" */
    0x70, 0x9f, 0x18, 0xc5, 0xc0,

    /* U+00F1 "ñ" */
    0x60, 0xf9, 0x99, 0x90,

    /* U+00F2 "ò" */
    0x20, 0x1d, 0x18, 0xc5, 0xc0,

    /* U+00F3 "ó" */
    0x20, 0x1d, 0x18, 0xc5, 0xc0,

    /* U+00F4 "ô" */
    0x20, 0x1d, 0x18, 0xc5, 0xc0,

    /* U+00F5 "õ" */
    0x70, 0x1d, 0x18, 0xc5, 0xc0,

    /* U+00F6 "ö" */
    0x30, 0x1d, 0x18, 0xc5, 0xc0,

    /* U+00F7 "÷" */
    0x40, 0xf0, 0x40,

    /* U+00F8 "ø" */
    0x75, 0x6b, 0x97, 0x0,

    /* U+00F9 "ù" */
    0x40, 0x99, 0x99, 0xf0,

    /* U+00FA "ú" */
    0x20, 0x99, 0x99, 0xf0,

    /* U+00FB "û" */
    0x60, 0x99, 0x99, 0xf0,

    /* U+00FC "ü" */
    0xa0, 0x99, 0x99, 0xf0,

    /* U+00FD "ý" */
    0x20, 0x22, 0xa5, 0x10, 0x84, 0x40,

    /* U+00FE "þ" */
    0x84, 0x3d, 0x18, 0xc7, 0xd0, 0x80,

    /* U+00FF "ÿ" */
    0x50, 0x22, 0xa5, 0x10, 0x84, 0x40
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 37, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 38, .box_w = 1, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 2, .adv_w = 51, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 3, .adv_w = 102, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 9, .adv_w = 86, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 15, .adv_w = 132, .box_w = 8, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 22, .adv_w = 100, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 28, .adv_w = 28, .box_w = 1, .box_h = 2, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 29, .adv_w = 51, .box_w = 3, .box_h = 8, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 32, .adv_w = 51, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 35, .adv_w = 68, .box_w = 3, .box_h = 3, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 37, .adv_w = 89, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 40, .adv_w = 38, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 41, .adv_w = 70, .box_w = 3, .box_h = 1, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 42, .adv_w = 38, .box_w = 1, .box_h = 1, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 43, .adv_w = 48, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 46, .adv_w = 103, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 52, .adv_w = 69, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 55, .adv_w = 84, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 59, .adv_w = 85, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 63, .adv_w = 95, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 69, .adv_w = 89, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 74, .adv_w = 89, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 79, .adv_w = 84, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 83, .adv_w = 88, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 88, .adv_w = 89, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 93, .adv_w = 38, .box_w = 1, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 94, .adv_w = 38, .box_w = 2, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 96, .adv_w = 79, .box_w = 4, .box_h = 4, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 98, .adv_w = 91, .box_w = 4, .box_h = 3, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 100, .adv_w = 79, .box_w = 4, .box_h = 4, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 102, .adv_w = 78, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 106, .adv_w = 141, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 114, .adv_w = 107, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 121, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 126, .adv_w = 118, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 133, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 139, .adv_w = 87, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 143, .adv_w = 85, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 147, .adv_w = 129, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 154, .adv_w = 111, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 159, .adv_w = 39, .box_w = 1, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 160, .adv_w = 85, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 165, .adv_w = 100, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 171, .adv_w = 81, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 175, .adv_w = 138, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 182, .adv_w = 113, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 188, .adv_w = 132, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 195, .adv_w = 92, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 200, .adv_w = 132, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 207, .adv_w = 95, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 212, .adv_w = 89, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 217, .adv_w = 85, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 222, .adv_w = 107, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 227, .adv_w = 101, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 233, .adv_w = 151, .box_w = 9, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 241, .adv_w = 99, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 247, .adv_w = 94, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 252, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 257, .adv_w = 51, .box_w = 2, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 260, .adv_w = 48, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 263, .adv_w = 51, .box_w = 2, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 266, .adv_w = 74, .box_w = 5, .box_h = 3, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 268, .adv_w = 76, .box_w = 5, .box_h = 1, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 269, .adv_w = 64, .box_w = 1, .box_h = 1, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 270, .adv_w = 85, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 274, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 279, .adv_w = 86, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 283, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 288, .adv_w = 90, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 292, .adv_w = 58, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 296, .adv_w = 95, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 301, .adv_w = 90, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 305, .adv_w = 33, .box_w = 2, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 307, .adv_w = 33, .box_w = 2, .box_h = 9, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 310, .adv_w = 81, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 315, .adv_w = 33, .box_w = 1, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 316, .adv_w = 140, .box_w = 7, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 321, .adv_w = 89, .box_w = 4, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 324, .adv_w = 95, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 328, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 333, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 338, .adv_w = 59, .box_w = 3, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 340, .adv_w = 75, .box_w = 4, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 343, .adv_w = 58, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 346, .adv_w = 89, .box_w = 4, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 349, .adv_w = 81, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 353, .adv_w = 124, .box_w = 8, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 358, .adv_w = 76, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 362, .adv_w = 81, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 367, .adv_w = 78, .box_w = 4, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 370, .adv_w = 50, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 374, .adv_w = 38, .box_w = 1, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 376, .adv_w = 50, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 380, .adv_w = 85, .box_w = 5, .box_h = 2, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 382, .adv_w = 37, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 383, .adv_w = 38, .box_w = 1, .box_h = 7, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 384, .adv_w = 85, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 389, .adv_w = 84, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 394, .adv_w = 97, .box_w = 4, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 397, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 402, .adv_w = 38, .box_w = 1, .box_h = 8, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 403, .adv_w = 87, .box_w = 4, .box_h = 9, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 408, .adv_w = 63, .box_w = 2, .box_h = 1, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 409, .adv_w = 85, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 413, .adv_w = 76, .box_w = 4, .box_h = 5, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 416, .adv_w = 87, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 419, .adv_w = 91, .box_w = 5, .box_h = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 421, .adv_w = 0, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 422, .adv_w = 85, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 426, .adv_w = 64, .box_w = 2, .box_h = 1, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 427, .adv_w = 52, .box_w = 3, .box_h = 3, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 429, .adv_w = 87, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 433, .adv_w = 48, .box_w = 2, .box_h = 3, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 434, .adv_w = 51, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 436, .adv_w = 64, .box_w = 1, .box_h = 1, .ofs_x = 2, .ofs_y = 6},
    {.bitmap_index = 437, .adv_w = 101, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 442, .adv_w = 98, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 449, .adv_w = 38, .box_w = 1, .box_h = 1, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 450, .adv_w = 64, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 451, .adv_w = 37, .box_w = 2, .box_h = 3, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 452, .adv_w = 82, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 456, .adv_w = 87, .box_w = 4, .box_h = 4, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 458, .adv_w = 108, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 465, .adv_w = 115, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 472, .adv_w = 123, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 479, .adv_w = 74, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 484, .adv_w = 107, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 491, .adv_w = 107, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 498, .adv_w = 107, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 506, .adv_w = 107, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 514, .adv_w = 107, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 522, .adv_w = 107, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 529, .adv_w = 152, .box_w = 9, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 537, .adv_w = 118, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 545, .adv_w = 87, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 549, .adv_w = 87, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 553, .adv_w = 87, .box_w = 4, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 558, .adv_w = 87, .box_w = 4, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 563, .adv_w = 39, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 565, .adv_w = 39, .box_w = 2, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 567, .adv_w = 39, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 571, .adv_w = 39, .box_w = 2, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 574, .adv_w = 114, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 581, .adv_w = 113, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 588, .adv_w = 132, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 596, .adv_w = 132, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 604, .adv_w = 132, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 612, .adv_w = 132, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 620, .adv_w = 132, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 628, .adv_w = 87, .box_w = 4, .box_h = 3, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 630, .adv_w = 132, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 637, .adv_w = 107, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 643, .adv_w = 107, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 649, .adv_w = 107, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 655, .adv_w = 107, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 661, .adv_w = 94, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 666, .adv_w = 92, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 671, .adv_w = 88, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 676, .adv_w = 85, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 681, .adv_w = 85, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 686, .adv_w = 85, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 691, .adv_w = 85, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 696, .adv_w = 85, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 701, .adv_w = 85, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 706, .adv_w = 147, .box_w = 9, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 712, .adv_w = 86, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 717, .adv_w = 90, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 722, .adv_w = 90, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 727, .adv_w = 90, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 732, .adv_w = 90, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 737, .adv_w = 33, .box_w = 2, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 739, .adv_w = 33, .box_w = 2, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 741, .adv_w = 33, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 744, .adv_w = 33, .box_w = 2, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 746, .adv_w = 91, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 751, .adv_w = 89, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 755, .adv_w = 95, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 760, .adv_w = 95, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 765, .adv_w = 95, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 770, .adv_w = 95, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 775, .adv_w = 95, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 780, .adv_w = 87, .box_w = 4, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 783, .adv_w = 95, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 787, .adv_w = 89, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 791, .adv_w = 89, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 795, .adv_w = 89, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 799, .adv_w = 89, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 803, .adv_w = 81, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 809, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 815, .adv_w = 81, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = -2}
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
    -10, -10, -6, -5, -10, -10, -10, -6,
    -5, -2, -3, -5, -10, -10, -6, -3,
    -5, -10, -10, -6, -3, -3, -3, -3,
    -3, -3, -10, -6, 3, -2, -2, -3,
    -2, -2, -3, -3, -2, -2, -2, -8,
    -8, -3, -6, -6, -3, -5, -3, -3,
    -5, -3, -3, -3, -6, -6, -5, -2,
    -2, -5, -5, -10, -3, -10, -5, -11,
    -3, -2, -6, -8, -6, -10, -3, -3,
    -3, -18, -18, -11, -5, -3, -3, -11,
    -10, -3, -3, -3, -2, -2, -2, -8,
    -8, -3, -3, -5, -3, -2, -5, -5,
    -2, -3, 3, -8, -3, -2, -3, -2,
    -3, -8
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
const lv_font_t GoogleSans10 = {
#else
lv_font_t GoogleSans10 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 11,          /*The maximum line height required by the font*/
    .base_line = 2,             /*Baseline measured from the bottom of the line*/
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



#endif /*#if GOOGLESANS10*/
