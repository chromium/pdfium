// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fpdfapi/fpdf_page.h"
#include "../../../include/fxge/fx_freetype.h"
extern FX_WCHAR PDF_UnicodeFromAdobeName(const FX_CHAR*);
const struct _UnicodeAlt {
    FX_WORD	m_Unicode;
    const FX_CHAR* m_Alter;
}
UnicodeAlts[] = {
    {0x00a0, " "}, {0x00a1, "!"}, {0x00a2, "c"}, {0x00a3, "P"}, {0x00a4, "o"},
    {0x00a5, "Y"}, {0x00a6, "|"}, {0x00a7, "S"}, {0x00a9, "(C)"}, {0x00aa, "a"},
    {0x00ab, "<<"}, {0x00ac, "-|"}, {0x00ae, "(R)"}, {0x00af, "-"},
    {0x00b0, "o"}, {0x00b1, "+/-"}, {0x00b2, "^2"}, { 0x00b3, "^3"},
    {0x00b4, "'"}, {0x00b5, "u"}, {0x00b6, "P"}, {0x00b7, "."},
    {0x00b9, "^1"}, {0x00ba, "o"}, {0x00bb, ">>"}, {0x00bc, "1/4"},
    {0x00bd, "1/2"}, {0x00be, "3/4"},  {0x00bf, "?"}, {0x00c0, "A"},
    {0x00c1, "A"}, {0x00c2, "A"}, {0x00c3, "A"}, {0x00c4, "A"},
    {0x00c5, "A"}, {0x00c6, "AE"}, {0x00c7, "C"}, {0x00c8, "E"},
    {0x00c9, "E"}, {0x00ca, "E"}, {0x00cb, "E"}, {0x00cc, "I"},
    {0x00cd, "I"}, {0x00ce, "I"}, {0x00cf, "I"},
    {0x00d1, "N"}, {0x00d2, "O"}, {0x00d3, "O"}, {0x00d4, "O"},
    {0x00d5, "O"}, {0x00d6, "O"}, {0x00d7, "x"}, {0x00d8, "O"},
    {0x00d9, "U"}, {0x00da, "U"}, {0x00db, "U"}, {0x00dc, "U"},
    {0x00dd, "Y"}, {0x00df, "S"}, {0x00e0, "a"},
    {0x00e1, "a"}, {0x00e2, "a"}, {0x00e3, "a"}, {0x00e4, "a"},
    {0x00e5, "a"}, {0x00e6, "ae"}, {0x00e7, "c"}, {0x00e8, "e"},
    {0x00e9, "e"}, {0x00ea, "e"}, {0x00eb, "e"}, {0x00ec, "i"},
    {0x00ed, "i"}, {0x00ee, "i"}, {0x00ef, "i"},
    {0x00f1, "n"}, {0x00f2, "o"}, {0x00f3, "o"}, {0x00f4, "o"},
    {0x00f5, "o"}, {0x00f6, "o"}, {0x00f7, "/"}, {0x00f8, "o"},
    {0x00f9, "u"}, {0x00fa, "u"}, {0x00fb, "u"}, {0x00fc, "u"},
    {0x00fd, "y"}, {0x00ff, "y"},
    {0x02b0, "h"}, {0x02b2, "j"}, {0x02b3, "r"}, {0x02b7, "w"},
    {0x02b8, "y"}, {0x02b9, "'"}, {0x02ba, "\""}, {0x02bb, "'"},
    {0x02bc, "'"}, {0x02bd, "'"}, {0x02be, "'"}, {0x02bf, "'"},
    {0x02c2, "<"}, {0x02c3, ">"}, {0x02c4, "^"}, {0x02c5, "v"},
    {0x02c6, "^"}, {0x02c7, "v"}, {0x02c8, "'"}, {0x02c9, "-"},
    {0x02ca, "'"}, {0x02cb, "'"}, {0x02cc, "."}, {0x02cd, "_"},
    {0x2010, "-"}, {0x2012, "-"}, {0x2013, "-"}, {0x2014, "--"},
    {0x2015, "--"}, {0x2016, "|"}, {0x2017, "_"},
    {0x2018, "'"}, {0x2019, "'"}, {0x201a, ","}, {0x201b, "'"},
    {0x201c, "\""}, {0x201d, "\""}, {0x201e, ","}, {0x201f, "'"},
    {0x2020, "+"}, {0x2021, "+"}, {0x2022, "*"}, {0x2023, ">"},
    {0x2024, "."}, {0x2025, ".."}, {0x2027, "."}, {0x2032, "'"},
    {0x2033, "\""}, {0x2035, "'"}, {0x2036, "\""}, {0x2038, "^"},
    {0x2039, "<"}, {0x203a, ">"}, {0x203b, "*"}, {0x203c, "!!"},
    {0x203d, "?!"}, {0x203e, "-"}, {0x2044, "/"}, {0x2047, "??"},
    {0x2048, "?!"}, {0x2049, "!?"}, {0x204e, "*"}, {0x2052, "%"},
    {0x2122, "(TM)"},
    {0x2212, "-"}, {0x2215, "/"}, {0x2216, "\\"}, {0x2217, "*"},
    {0x2218, "*"}, {0x2219, "*"}, {0x2223, "|"}, {0x22c5, "."},
    {0x266f, "#"},
    {0XF6D9, "(C)"}, {0XF6DA, "(C)"}, {0XF6DB, "(TM)"},
    {0XF8E8, "(C)"}, {0xf8e9, "(C)"}, {0XF8EA, "(TM)"},

    {0xfb01, "fi"}, {0xfb02, "fl"}
};
const FX_CHAR* FCS_GetAltStr(FX_WCHAR unicode)
{
    int begin = 0;
    int end = sizeof UnicodeAlts / sizeof(struct _UnicodeAlt) - 1;
    while (begin <= end) {
        int middle = (begin + end) / 2;
        FX_WORD middlecode = UnicodeAlts[middle].m_Unicode;
        if (middlecode > unicode) {
            end = middle - 1;
        } else if (middlecode < unicode) {
            begin = middle + 1;
        } else {
            return UnicodeAlts[middle].m_Alter;
        }
    }
    return NULL;
}
const FX_WORD StandardEncoding[256] = {
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x2019,
    0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f, 0x0030, 0x0031,
    0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b,
    0x003c, 0x003d, 0x003e, 0x003f, 0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045,
    0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
    0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059,
    0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f, 0x2018, 0x0061, 0x0062, 0x0063,
    0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d,
    0x006e, 0x006f, 0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
    0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x00a1, 0x00a2, 0x00a3, 0x2044, 0x00a5, 0x0192, 0x00a7, 0x00a4, 0x0027,
    0x201c, 0x00ab, 0x2039, 0x203a, 0xfb01, 0xfb02, 0x0000, 0x2013, 0x2020, 0x2021,
    0x00b7, 0x0000, 0x00b6, 0x2022, 0x201a, 0x201e, 0x201d, 0x00bb, 0x2026, 0x2030,
    0x0000, 0x00bf, 0x0000, 0x0060, 0x00b4, 0x02c6, 0x02dc, 0x00af, 0x02d8, 0x02d9,
    0x00a8, 0x0000, 0x02da, 0x00b8, 0x0000, 0x02dd, 0x02db, 0x02c7, 0x2014, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x00c6, 0x0000, 0x00aa, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0141, 0x00d8, 0x0152, 0x00ba, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x00e6, 0x0000, 0x0000, 0x0000, 0x0131, 0x0000, 0x0000, 0x0142, 0x00f8,
    0x0153, 0x00df, 0x0000, 0x0000, 0x0000, 0x0000
};
const FX_WORD MacRomanEncoding[256] = {
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
    0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f, 0x0030, 0x0031,
    0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b,
    0x003c, 0x003d, 0x003e, 0x003f, 0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045,
    0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
    0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059,
    0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f, 0x0060, 0x0061, 0x0062, 0x0063,
    0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d,
    0x006e, 0x006f, 0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
    0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x0000, 0x00c4, 0x00c5,
    0x00c7, 0x00c9, 0x00d1, 0x00d6, 0x00dc, 0x00e1, 0x00e0, 0x00e2, 0x00e4, 0x00e3,
    0x00e5, 0x00e7, 0x00e9, 0x00e8, 0x00ea, 0x00eb, 0x00ed, 0x00ec, 0x00ee, 0x00ef,
    0x00f1, 0x00f3, 0x00f2, 0x00f4, 0x00f6, 0x00f5, 0x00fa, 0x00f9, 0x00fb, 0x00fc,
    0x2020, 0x00b0, 0x00a2, 0x00a3, 0x00a7, 0x2022, 0x00b6, 0x00df, 0x00ae, 0x00a9,
    0x2122, 0x00b4, 0x00a8, 0x0000, 0x00c6, 0x00d8, 0x0000, 0x00b1, 0x0000, 0x0000,
    0x00a5, 0x00b5, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x00aa, 0x00ba, 0x0000,
    0x00e6, 0x00f8, 0x00bf, 0x00a3, 0x00ac, 0x0000, 0x0192, 0x0000, 0x0000, 0x00ab,
    0x00bb, 0x2026, 0x0020, 0x00c0, 0x00c3, 0x00d5, 0x0152, 0x0153, 0x2013, 0x2014,
    0x201c, 0x201d, 0x2018, 0x2019, 0x00f7, 0x0000, 0x00ff, 0x0178, 0x2044, 0x00a4,
    0x2039, 0x203a, 0xfb01, 0xfb02, 0x2021, 0x00b7, 0x201a, 0x201e, 0x2030, 0x00c2,
    0x00ca, 0x00c1, 0x00cb, 0x00c8, 0x00cd, 0x00ce, 0x00cf, 0x00cc, 0x00d3, 0x00d4,
    0x0000, 0x00d2, 0x00da, 0x00db, 0x00d9, 0x0131, 0x02c6, 0x02dc, 0x00af, 0x02d8,
    0x02d9, 0x02da, 0x00b8, 0x02dd, 0x02db, 0x02c7
};
const FX_WORD AdobeWinAnsiEncoding[256] = {
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
    0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
    0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
    0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
    0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
    0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
    0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
    0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
    0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
    0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
    0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x2022,
    0x20ac, 0x2022, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021,
    0x02c6, 0x2030, 0x0160, 0x2039,	0x0152, 0x2022, 0x017d, 0x2022,
    0x2022, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,
    0x02dc, 0x2122, 0x0161, 0x203a, 0x0153, 0x2022, 0x017e, 0x0178,
    0x0020, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7,
    0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x002d, 0x00ae, 0x00af,
    0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7,
    0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf,
    0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7,
    0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
    0x00d0, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7,
    0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 0x00df,
    0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7,
    0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
    0x00f0, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7,
    0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x00ff
};
extern const FX_WORD PDFDocEncoding[256];
const FX_WORD MacExpertEncoding[256] = {
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0020, 0xf721, 0xf6f8, 0xf7a2, 0xf724, 0xf6e4, 0xf726, 0xf7b4,
    0x207d, 0x207e, 0x2025, 0x2024, 0x002c, 0x002d, 0x002e, 0x2044, 0xf730, 0xf731,
    0xf732, 0xf733, 0xf734, 0xf735, 0xf736, 0xf737, 0xf738, 0xf739, 0x003a, 0x003b,
    0x0000, 0xf6de, 0x0000, 0xf73f, 0x0000, 0x0000, 0x0000, 0x0000, 0xf7f0, 0x0000,
    0x0000, 0x00bc, 0x00bd, 0x00be, 0x215b, 0x215c, 0x215d, 0x215e, 0x2153, 0x2154,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xfb00, 0xfb01, 0xfb02, 0xfb03,
    0xfb04, 0x208d, 0x0000, 0x208e, 0xf6f6, 0xf6e5, 0xf760, 0xf761, 0xf762, 0xf763,
    0xf764, 0xf765, 0xf766, 0xf767, 0xf768, 0xf769, 0xf76a, 0xf76b, 0xf76c, 0xf76d,
    0xf76e, 0xf76f, 0xf770, 0xf771, 0xf772, 0xf773, 0xf774, 0xf775, 0xf776, 0xf777,
    0xf778, 0xf779, 0xf77a, 0x20a1, 0xf6dc, 0xf6dd, 0xf6fe, 0x0000, 0x0000, 0xf6e9,
    0xf6e0, 0x0000, 0x0000, 0x0000, 0x0000, 0xf7e1, 0xf7e0, 0xf7e2, 0xf7e4, 0xf7e3,
    0xf7e5, 0xf7e7, 0xf7e9, 0xf7e8, 0xf7ea, 0xf7eb, 0xf7ed, 0xf7ec, 0xf7ee, 0xf7ef,
    0xf7f1, 0xf7f3, 0xf7f2, 0xf7f4, 0xf7f6, 0xf7f5, 0xf7fa, 0xf7f9, 0xf7fb, 0xf7fc,
    0x0000, 0x2078, 0x2084, 0x2083, 0x2086, 0x2088, 0x2087, 0xf6fd, 0x0000, 0xf6df,
    0x2082, 0x0000, 0xf7a8, 0x0000, 0xf6f5, 0xf6fd, 0x2085, 0x0000, 0xf6e1, 0xf6e7,
    0xf7fd, 0x0000, 0xf6e3, 0x0000, 0x0000, 0xf7fe, 0x0000, 0x2089, 0x2080, 0xf6ff,
    0xf7e6, 0xf7f8, 0xf7bf, 0x2081, 0xf6e9, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0xf7b8, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xf6fa, 0x2012, 0xf6e6,
    0x0000, 0x0000, 0x0000, 0x0000, 0xf7a1, 0x0000, 0xf7ff, 0x0000, 0x00b9, 0x00b2,
    0x00b3, 0x2074, 0x2075, 0x2076, 0x2077, 0x2079, 0x2070, 0x0000, 0xf6ec, 0xf6f1,
    0x0000, 0x0000, 0x0000, 0xf6ed, 0xf6f2, 0xf6eb, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0xf6ee, 0xf6fb, 0xf6f4, 0xf7af, 0xf6ea, 0x207f, 0xf6ef, 0xf6e2, 0xf6e8,
    0xf6f7, 0xf6fc, 0x0000, 0x0000, 0x0000, 0x0000
};
const FX_WORD AdobeSymbolEncoding[256] = {
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0020, 0x0021, 0x2200, 0x0023, 0x2203, 0x0025, 0x0026, 0x220B,
    0x0028, 0x0029, 0x2217, 0x002B, 0x002C, 0x2212, 0x002E, 0x002F,
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
    0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
    0x2245, 0x0391, 0x0392, 0x03A7, 0x0394, 0x0395, 0x03A6, 0x0393,
    0x0397, 0x0399, 0x03D1, 0x039A, 0x039B, 0x039C, 0x039D, 0x039F,
    0x03A0, 0x0398, 0x03A1, 0x03A3, 0x03A4, 0x03A5, 0x03C2, 0x03A9,
    0x039E, 0x03A8, 0x0396, 0x005B, 0x2234, 0x005D, 0x22A5, 0x005F,
    0xF8E5, 0x03B1, 0x03B2, 0x03C7, 0x03B4, 0x03B5, 0x03C6, 0x03B3,
    0x03B7, 0x03B9, 0x03D5, 0x03BA, 0x03BB, 0x03BC, 0x03BD, 0x03BF,
    0x03C0, 0x03B8, 0x03C1, 0x03C3, 0x03C4, 0x03C5, 0x03D6, 0x03C9,
    0x03BE, 0x03C8, 0x03B6, 0x007B, 0x007C, 0x007D, 0x223C, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x20AC, 0x03D2, 0x2032, 0x2264, 0x2044, 0x221E, 0x0192, 0x2663,
    0x2666, 0x2665, 0x2660, 0x2194, 0x2190, 0x2191, 0x2192, 0x2193,
    0x00B0, 0x00B1, 0x2033, 0x2265, 0x00D7, 0x221D, 0x2202, 0x2022,
    0x00F7, 0x2260, 0x2261, 0x2248, 0x2026, 0xF8E6, 0xF8E7, 0x21B5,
    0x2135, 0x2111, 0x211C, 0x2118, 0x2297, 0x2295, 0x2205, 0x2229,
    0x222A, 0x2283, 0x2287, 0x2284, 0x2282, 0x2286, 0x2208, 0x2209,
    0x2220, 0x2207, 0xF6DA, 0xF6D9, 0xF6DB, 0x220F, 0x221A, 0x22C5,
    0x00AC, 0x2227, 0x2228, 0x21D4, 0x21D0, 0x21D1, 0x21D2, 0x21D3,
    0x25CA, 0x2329, 0xF8E8, 0xF8E9, 0xF8EA, 0x2211, 0xF8EB, 0xF8EC,
    0xF8ED, 0xF8EE, 0xF8EF, 0xF8F0, 0xF8F1, 0xF8F2, 0xF8F3, 0xF8F4,
    0x0000, 0x232A, 0x222B, 0x2320, 0xF8F5, 0x2321, 0xF8F6, 0xF8F7,
    0xF8F8, 0xF8F9, 0xF8FA, 0xF8FB, 0xF8FC, 0xF8FD, 0xF8FE, 0x0000,
};
const FX_WORD ZapfEncoding[256] = {
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0020, 0x2701, 0x2702, 0x2703, 0x2704, 0x260E, 0x2706, 0x2707,
    0x2708, 0x2709, 0x261B, 0x261E, 0x270C, 0x270D, 0x270E, 0x270F,
    0x2710, 0x2711, 0x2712, 0x2713, 0x2714, 0x2715, 0x2716, 0x2717,
    0x2718, 0x2719, 0x271A, 0x271B, 0x271C, 0x271D, 0x271E, 0x271F,
    0x2720, 0x2721, 0x2722, 0x2723, 0x2724, 0x2725, 0x2726, 0x2727,
    0x2605, 0x2729, 0x272A, 0x272B, 0x272C, 0x272D, 0x272E, 0x272F,
    0x2730, 0x2731, 0x2732, 0x2733, 0x2734, 0x2735, 0x2736, 0x2737,
    0x2738, 0x2739, 0x273A, 0x273B, 0x273C, 0x273D, 0x273E, 0x273F,
    0x2740, 0x2741, 0x2742, 0x2743, 0x2744, 0x2745, 0x2746, 0x2747,
    0x2748, 0x2749, 0x274A, 0x274B, 0x25CF, 0x274D, 0x25A0, 0x274F,
    0x2750, 0x2751, 0x2752, 0x25B2, 0x25BC, 0x25C6, 0x2756, 0x25D7,
    0x2758, 0x2759, 0x275A, 0x275B, 0x275C, 0x275D, 0x275E, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x2761, 0x2762, 0x2763, 0x2764, 0x2765, 0x2766, 0x2767,
    0x2663, 0x2666, 0x2665, 0x2660, 0x2460, 0x2461, 0x2462, 0x2463,
    0x2464, 0x2465, 0x2466, 0x2467, 0x2468, 0x2469, 0x2776, 0x2777,
    0x2778, 0x2779, 0x277A, 0x277B, 0x277C, 0x277D, 0x277E, 0x277F,
    0x2780, 0x2781, 0x2782, 0x2783, 0x2784, 0x2785, 0x2786, 0x2787,
    0x2788, 0x2789, 0x278A, 0x278B, 0x278C, 0x278D, 0x278E, 0x278F,
    0x2790, 0x2791, 0x2792, 0x2793, 0x2794, 0x2192, 0x2194, 0x2195,
    0x2798, 0x2799, 0x279A, 0x279B, 0x279C, 0x279D, 0x279E, 0x279F,
    0x27A0, 0x27A1, 0x27A2, 0x27A3, 0x27A4, 0x27A5, 0x27A6, 0x27A7,
    0x27A8, 0x27A9, 0x27AA, 0x27AB, 0x27AC, 0x27AD, 0x27AE, 0x27AF,
    0x0000, 0x27B1, 0x27B2, 0x27B3, 0x27B4, 0x27B5, 0x27B6, 0x27B7,
    0x27B8, 0x27B9, 0x27BA, 0x27BB, 0x27BC, 0x27BD, 0x27BE, 0x0000,
};
const FX_LPCSTR StandardEncodingNames[224] = {
    "space", "exclam", "quotedbl", "numbersign", "dollar", "percent", "ampersand", "quoteright",
    "parenleft", "parenright", "asterisk", "plus", "comma", "hyphen", "period", "slash",
    "zero", "one", "two", "three", "four", "five", "six", "seven",
    "eight", "nine", "colon", "semicolon", "less", "equal", "greater", "question",
    "at", "A", "B", "C", "D", "E", "F", "G",
    "H", "I", "J", "K", "L", "M", "N", "O",
    "P", "Q", "R", "S", "T", "U", "V", "W",
    "X", "Y", "Z", "bracketleft", "backslash", "bracketright", "asciicircum", "underscore",
    "quoteleft", "a", "b", "c", "d", "e", "f", "g",
    "h", "i", "j", "k", "l", "m", "n", "o",
    "p", "q", "r", "s", "t", "u", "v", "w",
    "x", "y", "z", "braceleft", "bar", "braceright", "asciitilde", NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, "exclamdown", "cent", "sterling", "fraction", "yen", "florin", "section",
    "currency", "quotesingle", "quotedblleft", "guillemotleft", "guilsinglleft", "guilsinglright", "fi", "fl",
    NULL, "endash", "dagger", "daggerdbl", "periodcentered", NULL, "paragraph", "bullet",
    "quotesinglbase", "quotedblbase", "quotedblright", "guillemotright", "ellipsis", "perthousand", NULL, "questiondown",
    NULL, "grave", "acute", "circumflex", "tilde", "macron", "breve", "dotaccent",
    "dieresis", NULL, "ring", "cedilla", NULL, "hungarumlaut", "ogonek", "caron",
    "emdash", NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, "AE", NULL, "ordfeminine", NULL, NULL, NULL, NULL,
    "Lslash", "Oslash", "OE", "ordmasculine", NULL, NULL, NULL, NULL,
    NULL, "ae", NULL, NULL, NULL, "dotlessi", NULL, NULL,
    "lslash", "oslash", "oe", "germandbls", NULL, NULL, NULL, NULL,
};
const FX_LPCSTR AdobeWinAnsiEncodingNames[224] = {
    "space", "exclam", "quotedbl", "numbersign", "dollar", "percent", "ampersand", "quotesingle",
    "parenleft", "parenright", "asterisk", "plus", "comma", "hyphen", "period", "slash",
    "zero", "one", "two", "three", "four", "five", "six", "seven",
    "eight", "nine", "colon", "semicolon", "less", "equal", "greater", "question",
    "at", "A", "B", "C", "D", "E", "F", "G",
    "H", "I", "J", "K", "L", "M", "N", "O",
    "P", "Q", "R", "S", "T", "U", "V", "W",
    "X", "Y", "Z", "bracketleft", "backslash", "bracketright", "asciicircum", "underscore",
    "grave", "a", "b", "c", "d", "e", "f", "g",
    "h", "i", "j", "k", "l", "m", "n", "o",
    "p", "q", "r", "s", "t", "u", "v", "w",
    "x", "y", "z", "braceleft", "bar", "braceright", "asciitilde", "bullet",
    "Euro", "bullet", "quotesinglbase", "florin", "quotedblbase", "ellipsis", "dagger", "daggerdbl",
    "circumflex", "perthousand", "Scaron", "guilsinglleft", "OE", "bullet", "Zcaron", "bullet",
    "bullet", "quoteleft", "quoteright", "quotedblleft", "quotedblright", "bullet", "endash", "emdash",
    "tilde", "trademark", "scaron", "guilsinglright", "oe", "bullet", "zcaron", "Ydieresis",
    "space", "exclamdown", "cent", "sterling", "currency", "yen", "brokenbar", "section",
    "dieresis", "copyright", "ordfeminine", "guillemotleft", "logicalnot", "hyphen", "registered", "macron",
    "degree", "plusminus", "twosuperior", "threesuperior", "acute", "mu", "paragraph", "periodcentered",
    "cedilla", "onesuperior", "ordmasculine", "guillemotright", "onequarter", "onehalf", "threequarters", "questiondown",
    "Agrave", "Aacute", "Acircumflex", "Atilde", "Adieresis", "Aring", "AE", "Ccedilla",
    "Egrave", "Eacute", "Ecircumflex", "Edieresis", "Igrave", "Iacute", "Icircumflex", "Idieresis",
    "Eth", "Ntilde", "Ograve", "Oacute", "Ocircumflex", "Otilde", "Odieresis", "multiply",
    "Oslash", "Ugrave", "Uacute", "Ucircumflex", "Udieresis", "Yacute", "Thorn", "germandbls",
    "agrave", "aacute", "acircumflex", "atilde", "adieresis", "aring", "ae", "ccedilla",
    "egrave", "eacute", "ecircumflex", "edieresis", "igrave", "iacute", "icircumflex", "idieresis",
    "eth", "ntilde", "ograve", "oacute", "ocircumflex", "otilde", "odieresis", "divide",
    "oslash", "ugrave", "uacute", "ucircumflex", "udieresis", "yacute", "thorn", "ydieresis",
};
const FX_LPCSTR  MacRomanEncodingNames[224] = {
    "space", "exclam", "quotedbl", "numbersign", "dollar", "percent", "ampersand", "quotesingle",
    "parenleft", "parenright", "asterisk", "plus", "comma", "hyphen", "period", "slash",
    "zero", "one", "two", "three", "four", "five", "six", "seven",
    "eight", "nine", "colon", "semicolon", "less", "equal", "greater", "question",
    "at", "A", "B", "C", "D", "E", "F", "G",
    "H", "I", "J", "K", "L", "M", "N", "O",
    "P", "Q", "R", "S", "T", "U", "V", "W",
    "X", "Y", "Z", "bracketleft", "backslash", "bracketright", "asciicircum", "underscore",
    "grave", "a", "b", "c", "d", "e", "f", "g",
    "h", "i", "j", "k", "l", "m", "n", "o",
    "p", "q", "r", "s", "t", "u", "v", "w",
    "x", "y", "z", "braceleft", "bar", "braceright", "asciitilde", NULL,
    "Adieresis", "Aring", "Ccedilla", "Eacute", "Ntilde", "Odieresis", "Udieresis", "aacute",
    "agrave", "acircumflex", "adieresis", "atilde", "aring", "ccedilla", "eacute", "egrave",
    "ecircumflex", "edieresis", "iacute", "igrave", "icircumflex", "idieresis", "ntilde", "oacute",
    "ograve", "ocircumflex", "odieresis", "otilde", "uacute", "ugrave", "ucircumflex", "udieresis",
    "dagger", "degree", "cent", "sterling", "section", "bullet", "paragraph", "germandbls",
    "registered", "copyright", "trademark", "acute", "dieresis", "notequal", "AE", "Oslash",
    "infinity", "plusminus", "lessequal", "greaterequal", "yen", "mu", "partialdiff", "summation",
    "product", "pi", "integral", "ordfeminine", "ordmasculine", "Omega", "ae", "oslash",
    "questiondown", "exclamdown", "logicalnot", "radical", "florin", "approxequal", "Delta", "guillemotleft",
    "guillemotright", "ellipsis", "space", "Agrave", "Atilde", "Otilde", "OE", "oe",
    "endash", "emdash", "quotedblleft", "quotedblright", "quoteleft", "quoteright", "divide", "lozenge",
    "ydieresis", "Ydieresis", "fraction", "currency", "guilsinglleft", "guilsinglright", "fi", "fl",
    "daggerdbl", "periodcentered", "quotesinglbase", "quotedblbase", "perthousand", "Acircumflex", "Ecircumflex", "Aacute",
    "Edieresis", "Egrave", "Iacute", "Icircumflex", "Idieresis", "Igrave", "Oacute", "Ocircumflex",
    "apple", "Ograve", "Uacute", "Ucircumflex", "Ugrave", "dotlessi", "circumflex", "tilde",
    "macron", "breve", "dotaccent", "ring", "cedilla", "hungarumlaut", "ogonek", "caron",
};
const FX_LPCSTR  MacExpertEncodingNames[224] = {
    "space", "exclamsmall", "Hungarumlautsmall", "centoldstyle", "dollaroldstyle", "dollarsuperior", "ampersandsmall", "Acutesmall",
    "parenleftsuperior", "parenrightsuperior", "twodotenleader", "onedotenleader", "comma", "hyphen", "period", "fraction",
    "zerooldstyle", "oneoldstyle", "twooldstyle", "threeoldstyle", "fouroldstyle", "fiveoldstyle", "sixoldstyle", "sevenoldstyle",
    "eightoldstyle", "nineoldstyle", "colon", "semicolon", NULL, "threequartersemdash", NULL, "questionsmall",
    NULL, NULL, NULL, NULL, "Ethsmall", NULL, NULL, "onequarter",
    "onehalf", "threequarters", "oneeighth", "threeeighths", "fiveeighths", "seveneighths", "onethird", "twothirds",
    NULL, NULL, NULL, NULL, NULL, NULL, "ff", "fi",
    "fl", "ffi", "ffl", "parenleftinferior", NULL, "parenrightinferior", "Circumflexsmall", "hypheninferior",
    "Gravesmall", "Asmall", "Bsmall", "Csmall", "Dsmall", "Esmall", "Fsmall", "Gsmall",
    "Hsmall", "Ismall", "Jsmall", "Ksmall", "Lsmall", "Msmall", "Nsmall", "Osmall",
    "Psmall", "Qsmall", "Rsmall", "Ssmall", "Tsmall", "Usmall", "Vsmall", "Wsmall",
    "Xsmall", "Ysmall", "Zsmall", "colonmonetary", "onefitted", "rupiah", "Tildesmall", NULL,
    NULL, "asuperior", "centsuperior", NULL, NULL, NULL, NULL, "Aacutesmall",
    "Agravesmall", "Acircumflexsmall", "Adieresissmall", "Atildesmall", "Aringsmall", "Ccedillasmall", "Eacutesmall", "Egravesmall",
    "Ecircumflexsmall", "Edieresissmall", "Iacutesmall", "Igravesmall", "Icircumflexsmall", "Idieresissmall", "Ntildesmall", "Oacutesmall",
    "Ogravesmall", "Ocircumflexsmall", "Odieresissmall", "Otildesmall", "Uacutesmall", "Ugravesmall", "Ucircumflexsmall", "Udieresissmall",
    NULL, "eightsuperior", "fourinferior", "threeinferior", "sixinferior", "eightinferior", "seveninferior", "Scaronsmall",
    NULL, "centinferior", "twoinferior", NULL, "Dieresissmall", NULL, "Caronsmall", "Scaronsmall",
    "fiveinferior", NULL, "commainferior", "periodinferior", "Yacutesmall", NULL, "dollarinferior", NULL,
    NULL, "Thornsmall", NULL, "nineinferior", "zeroinferior", "Zcaronsmall", "AEsmall", "Oslashsmall",
    "questiondownsmall", "oneinferior", "asuperior", NULL, NULL, NULL, NULL, NULL,
    NULL, "Cedillasmall", NULL, NULL, NULL, NULL, NULL, "OEsmall",
    "figuredash", "hyphensuperior", NULL, NULL, NULL, NULL, "exclamdownsmall", NULL,
    "Ydieresissmall", NULL, "onesuperior", "twosuperior", "threesuperior", "foursuperior", "fivesuperior", "sixsuperior",
    "sevensuperior", "ninesuperior", "zerosuperior", NULL, "esuperior", "rsuperior", NULL, NULL,
    NULL, "isuperior", "ssuperior", "dsuperior", NULL, NULL, NULL, NULL,
    NULL, "lsuperior", "Ogoneksmall", "Brevesmall", "Macronsmall", "bsuperior", "nsuperior", "msuperior",
    "commasuperior", "periodsuperior", "Dotaccentsmall", "Ringsmall", NULL, NULL, NULL, NULL,
};
const FX_LPCSTR  PDFDocEncodingNames[232] = {
    "breve", "caron", "circumflex", "dotaccent", "hungarumlaut", "ogonek", "ring", "tilde",
    "space", "exclam", "quotedbl", "numbersign", "dollar", "percent", "ampersand", "quotesingle",
    "parenleft", "parenright", "asterisk", "plus", "comma", "hyphen", "period", "slash",
    "zero", "one", "two", "three", "four", "five", "six", "seven",
    "eight", "nine", "colon", "semicolon", "less", "equal", "greater", "question",
    "at", "A", "B", "C", "D", "E", "F", "G",
    "H", "I", "J", "K", "L", "M", "N", "O",
    "P", "Q", "R", "S", "T", "U", "V", "W",
    "X", "Y", "Z", "bracketleft", "backslash", "bracketright", "asciicircum", "underscore",
    "grave", "a", "b", "c", "d", "e", "f", "g",
    "h", "i", "j", "k", "l", "m", "n", "o",
    "p", "q", "r", "s", "t", "u", "v", "w",
    "x", "y", "z", "braceleft", "bar", "braceright", "asciitilde", NULL,
    "bullet3", "dagger", "daggerdbl", "ellipsis", "emdash", "endash", "florin", "fraction",
    "guilsinglleft", "guilsinglright", "minus", "perthousand", "quotedblbase", "quotedblleft", "quotedblright", "quoteleft",
    "quoteright", "quotesinglbase", "trademark", "fi", "fl", "Lslash", "OE", "Scaron",
    "Ydieresis", "Zcaron2", "dotlessi", "lslash", "oe", "scaron", "zcaron2", NULL,
    "Euro", "exclamdown", "cent", "sterling", "currency", "yen", "brokenbar", "section",
    "dieresis", "copyright", "ordfeminine", "guillemotleft4", "logicalnot", NULL, "registered", "macron",
    "degree", "plusminus", "twosuperior", "threesuperior", "acute", "mu", "paragraph", "periodcentered",
    "cedilla", "onesuperior", "ordmasculine", "guillemotright4", "onequarter", "onehalf", "threequarters", "questiondown",
    "Agrave", "Aacute", "Acircumflex", "Atilde", "Adieresis", "Aring", "AE", "Ccedilla",
    "Egrave", "Eacute", "Ecircumflex", "Edieresis", "Igrave", "Iacute", "Icircumflex", "Idieresis",
    "Eth", "Ntilde", "Ograve", "Oacute", "Ocircumflex", "Otilde", "Odieresis", "multiply",
    "Oslash", "Ugrave", "Uacute", "Ucircumflex", "Udieresis", "Yacute", "Thorn", "germandbls",
    "agrave", "aacute", "acircumflex", "atilde", "adieresis", "aring", "ae", "ccedilla",
    "egrave", "eacute", "ecircumflex", "edieresis", "igrave", "iacute", "icircumflex", "idieresis",
    "eth", "ntilde", "ograve", "oacute", "ocircumflex", "otilde", "odieresis", "divide",
    "oslash", "ugrave", "uacute", "ucircumflex", "udieresis", "yacute", "thorn", "ydieresis",
};
const FX_LPCSTR  AdobeSymbolEncodingNames[224] = {
    "space", "exclam", "universal", "numbersign", "existential", "percent", "ampersand", "suchthat",
    "parenleft", "parenright", "asteriskmath", "plus", "comma", "minus", "period", "slash",
    "zero", "one", "two", "three", "four", "five", "six", "seven",
    "eight", "nine", "colon", "semicolon", "less", "equal", "greater", "question",
    "congruent", "Alpha", "Beta", "Chi", "Delta", "Epsilon", "Phi", "Gamma",
    "Eta", "Iota", "theta1", "Kappa", "Lambda", "Mu", "Nu", "Omicron",
    "Pi", "Theta", "Rho", "Sigma", "Tau", "Upsilon", "sigma1", "Omega",
    "Xi", "Psi", "Zeta", "bracketleft", "therefore", "bracketright", "perpendicular", "underscore",
    "radicalex", "alpha", "beta", "chi", "delta", "epsilon", "phi", "gamma",
    "eta", "iota", "phi1", "kappa", "lambda", "mu", "nu", "omicron",
    "pi", "theta", "rho", "sigma", "tau", "upsilon", "omega1", "omega",
    "xi", "psi", "zeta", "braceleft", "bar", "braceright", "similar", NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    "Euro", "Upsilon1", "minute", "lessequal", "fraction", "infinity", "florin", "club",
    "diamond", "heart", "spade", "arrowboth", "arrowleft", "arrowup", "arrowright", "arrowdown",
    "degree", "plusminus", "second", "greaterequal", "multiply", "proportional", "partialdiff", "bullet",
    "divide", "notequal", "equivalence", "approxequal", "ellipsis", "arrowvertex", "arrowhorizex", "carriagereturn",
    "aleph", "Ifraktur", "Rfraktur", "weierstrass", "circlemultiply", "circleplus", "emptyset", "intersection",
    "union", "propersuperset", "reflexsuperset", "notsubset", "propersubset", "reflexsubset", "element", "notelement",
    "angle", "gradient", "registerserif", "copyrightserif", "trademarkserif", "product", "radical", "dotmath",
    "logicalnot", "logicaland", "logicalor", "arrowdblboth", "arrowdblleft", "arrowdblup", "arrowdblright", "arrowdbldown",
    "lozenge", "angleleft", "registersans", "copyrightsans", "trademarksans", "summation", "parenlefttp", "parenleftex",
    "parenleftbt", "bracketlefttp", "bracketleftex", "bracketleftbt", "bracelefttp", "braceleftmid", "braceleftbt", "braceex",
    NULL, "angleright", "integral", "integraltp", "integralex", "integralbt", "parenrighttp", "parenrightex",
    "parenrightbt", "bracketrighttp", "bracketrightex", "bracketrightbt", "bracerighttp", "bracerightmid", "bracerightbt", NULL,
};
const FX_LPCSTR  ZapfEncodingNames[224] = {
    "space", "a1", "a2", "a202", "a3", "a4", "a5", "a119",
    "a118", "a117", "a11", "a12", "a13", "a14", "a15", "a16",
    "a105", "a17", "a18", "a19", "a20", "a21", "a22", "a23",
    "a24", "a25", "a26", "a27", "a28", "a6", "a7", "a8",
    "a9", "a10", "a29", "a30", "a31", "a32", "a33", "a34",
    "a35", "a36", "a37", "a38", "a39", "a40", "a41", "a42",
    "a43", "a44", "a45", "a46", "a47", "a48", "a49", "a50",
    "a51", "a52", "a53", "a54", "a55", "a56", "a57", "a58",
    "a59", "a60", "a61", "a62", "a63", "a64", "a65", "a66",
    "a67", "a68", "a69", "a70", "a71", "a72", "a73", "a74",
    "a203", "a75", "a204", "a76", "a77", "a78", "a79", "a81",
    "a82", "a83", "a84", "a97", "a98", "a99", "a100", NULL,
    "a89", "a90", "a93", "a94", "a91", "a92", "a205", "a85",
    "a206", "a86", "a87", "a88", "a95", "a96", NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, "a101", "a102", "a103", "a104", "a106", "a107", "a108",
    "a112", "a111", "a110", "a109", "a120", "a121", "a122", "a123",
    "a124", "a125", "a126", "a127", "a128", "a129", "a130", "a131",
    "a132", "a133", "a134", "a135", "a136", "a137", "a138", "a139",
    "a140", "a141", "a142", "a143", "a144", "a145", "a146", "a147",
    "a148", "a149", "a150", "a151", "a152", "a153", "a154", "a155",
    "a156", "a157", "a158", "a159", "a160", "a161", "a163", "a164",
    "a196", "a165", "a192", "a166", "a167", "a168", "a169", "a170",
    "a171", "a172", "a173", "a162", "a174", "a175", "a176", "a177",
    "a178", "a179", "a193", "a180", "a199", "a181", "a200", "a182",
    NULL, "a201", "a183", "a184", "a197", "a185", "a194", "a198",
    "a186", "a195", "a187", "a188", "a189", "a190", "a191", NULL
};
const FX_CHAR*  PDF_CharNameFromPredefinedCharSet(int encoding, FX_BYTE charcode)
{
    if (encoding == PDFFONT_ENCODING_PDFDOC) {
        if (charcode < 24) {
            return NULL;
        }
        charcode -= 24;
    } else {
        if (charcode < 32) {
            return NULL;
        }
        charcode -= 32;
    }
    switch (encoding) {
        case PDFFONT_ENCODING_WINANSI:
            return AdobeWinAnsiEncodingNames[charcode];
        case PDFFONT_ENCODING_MACROMAN:
            return MacRomanEncodingNames[charcode];
        case PDFFONT_ENCODING_MACEXPERT:
            return MacExpertEncodingNames[charcode];
        case PDFFONT_ENCODING_STANDARD:
            return StandardEncodingNames[charcode];
        case PDFFONT_ENCODING_ADOBE_SYMBOL:
            return AdobeSymbolEncodingNames[charcode];
        case PDFFONT_ENCODING_ZAPFDINGBATS:
            return ZapfEncodingNames[charcode];
        case PDFFONT_ENCODING_PDFDOC:
            return PDFDocEncodingNames[charcode];
    }
    return NULL;
}
FX_WCHAR FT_UnicodeFromCharCode(int encoding, FX_DWORD charcode)
{
    switch (encoding) {
        case FXFT_ENCODING_UNICODE:
            return (FX_WORD)charcode;
        case FXFT_ENCODING_ADOBE_STANDARD:
            return StandardEncoding[(FX_BYTE)charcode];
        case FXFT_ENCODING_ADOBE_EXPERT:
            return MacExpertEncoding[(FX_BYTE)charcode];
        case FXFT_ENCODING_ADOBE_LATIN_1:
            return AdobeWinAnsiEncoding[(FX_BYTE)charcode];
        case FXFT_ENCODING_APPLE_ROMAN:
            return MacRomanEncoding[(FX_BYTE)charcode];
        case PDFFONT_ENCODING_PDFDOC:
            return PDFDocEncoding[(FX_BYTE)charcode];
    }
    return 0;
}
static FX_DWORD PDF_FindCode(const FX_WORD* pCodes, FX_WORD unicode)
{
    for (FX_DWORD i = 0; i < 256; i ++)
        if (pCodes[i] == unicode) {
            return i;
        }
    return 0;
}
const FX_WORD MSSymbolEncoding[256] = {
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 32, 33, 8704, 35, 8707, 37, 38, 8715,
    40, 41, 8727, 43, 44, 8722, 46, 47, 48, 49,
    50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
    60, 61, 62, 63, 8773, 913, 914, 935, 916, 917,
    934, 915, 919, 921, 977, 922, 923, 924, 925, 927,
    928, 920, 929, 931, 932, 933, 962, 937, 926, 936,
    918, 91, 8756, 93, 8869, 95, 8254, 945, 946, 967,
    948, 949, 966, 947, 951, 953, 981, 954, 955, 956,
    957, 959, 960, 952, 961, 963, 964, 965, 982, 969,
    958, 968, 950, 123, 124, 125, 8764, 0, 0, 0,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 978, 8242, 8804, 8725, 8734, 402, 9827, 9830, 9828,
    9824, 8596, 8592, 8593, 8594, 8595, 176, 177, 8243, 8805,
    215, 8733, 8706, 8729, 247, 8800, 8801, 8776, 8943, 0,
    0, 8629, 0, 8465, 8476, 8472, 8855, 8853, 8709, 8745,
    8746, 8835, 8839, 8836, 8834, 8838, 8712, 8713, 8736, 8711,
    174, 169, 8482, 8719, 8730, 8901, 172, 8743, 8744, 8660,
    8656, 8657, 8658, 8659, 9674, 9001, 0, 0, 0, 8721,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0x0000, 9002, 8747, 8992, 0, 8993, 0, 0, 0, 0,
    0, 0, 0x0000, 0x0000, 0x0000, 0x0000
};
FX_DWORD FT_CharCodeFromUnicode(int encoding, FX_WCHAR unicode)
{
    switch (encoding) {
        case FXFT_ENCODING_UNICODE:
            return unicode;
        case FXFT_ENCODING_ADOBE_STANDARD:
            return PDF_FindCode(StandardEncoding, unicode);
        case FXFT_ENCODING_ADOBE_EXPERT:
            return PDF_FindCode(MacExpertEncoding, unicode);
        case FXFT_ENCODING_ADOBE_LATIN_1:
            return PDF_FindCode(AdobeWinAnsiEncoding, unicode);
        case FXFT_ENCODING_APPLE_ROMAN:
            return PDF_FindCode(MacRomanEncoding, unicode);
        case FXFT_ENCODING_ADOBE_CUSTOM:
            return PDF_FindCode(PDFDocEncoding, unicode);
        case FXFT_ENCODING_MS_SYMBOL:
            return PDF_FindCode(MSSymbolEncoding, unicode);
    }
    return 0;
}
const FX_WORD* PDF_UnicodesForPredefinedCharSet(int encoding)
{
    switch (encoding) {
        case PDFFONT_ENCODING_WINANSI:
            return AdobeWinAnsiEncoding;
        case PDFFONT_ENCODING_MACROMAN:
            return MacRomanEncoding;
        case PDFFONT_ENCODING_MACEXPERT:
            return MacExpertEncoding;
        case PDFFONT_ENCODING_STANDARD:
            return StandardEncoding;
        case PDFFONT_ENCODING_ADOBE_SYMBOL:
            return AdobeSymbolEncoding;
        case PDFFONT_ENCODING_ZAPFDINGBATS:
            return ZapfEncoding;
        case PDFFONT_ENCODING_PDFDOC:
            return PDFDocEncoding;
        case PDFFONT_ENCODING_MS_SYMBOL:
            return MSSymbolEncoding;
    }
    return NULL;
}
FX_DWORD PDF_PredefinedCharCodeFromUnicode(int encoding, FX_WCHAR unicode)
{
    return PDF_FindCode(PDF_UnicodesForPredefinedCharSet(encoding), unicode);
}
#ifdef __cplusplus
extern "C" {
#endif
extern int FXFT_unicode_from_adobe_name(const char* name);
extern void FXFT_adobe_name_from_unicode(char* glyph_name, FX_WCHAR unicode);
#ifdef __cplusplus
}
#endif
FX_WCHAR PDF_UnicodeFromAdobeName(const FX_CHAR* name)
{
    return (FX_WCHAR)(FXFT_unicode_from_adobe_name(name) & 0x7FFFFFFF);
}
CFX_ByteString PDF_AdobeNameFromUnicode(FX_WCHAR unicode)
{
    char glyph_name[64];
    FXFT_adobe_name_from_unicode(glyph_name, unicode);
    return CFX_ByteString(glyph_name, -1);
}
