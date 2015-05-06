// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fpdfapi/fpdf_page.h"
#include "../../../include/fpdfapi/fpdf_module.h"
#include "../fpdf_page/pageint.h"
#include <limits.h>
CPDF_Document::CPDF_Document() : CPDF_IndirectObjects(NULL)
{
    m_pRootDict = NULL;
    m_pInfoDict = NULL;
    m_bLinearized = FALSE;
    m_dwFirstPageNo = 0;
    m_dwFirstPageObjNum = 0;
    m_pDocPage = CPDF_ModuleMgr::Get()->GetPageModule()->CreateDocData(this);
    m_pDocRender = CPDF_ModuleMgr::Get()->GetRenderModule()->CreateDocData(this);
}
void CPDF_Document::CreateNewDoc()
{
    ASSERT(m_pRootDict == NULL && m_pInfoDict == NULL);
    m_pRootDict = new CPDF_Dictionary;
    m_pRootDict->SetAtName("Type", "Catalog");
    int objnum = AddIndirectObject(m_pRootDict);
    CPDF_Dictionary* pPages = new CPDF_Dictionary;
    pPages->SetAtName("Type", "Pages");
    pPages->SetAtNumber("Count", 0);
    pPages->SetAt("Kids", new CPDF_Array);
    objnum = AddIndirectObject(pPages);
    m_pRootDict->SetAtReference("Pages", this, objnum);
    m_pInfoDict = new CPDF_Dictionary;
    AddIndirectObject(m_pInfoDict);
}
static const FX_WCHAR g_FX_CP874Unicodes[128] = {
    0x20AC, 0x0000, 0x0000, 0x0000, 0x0000, 0x2026, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x00A0, 0x0E01, 0x0E02, 0x0E03, 0x0E04, 0x0E05, 0x0E06, 0x0E07,
    0x0E08, 0x0E09, 0x0E0A, 0x0E0B, 0x0E0C, 0x0E0D, 0x0E0E, 0x0E0F,
    0x0E10, 0x0E11, 0x0E12, 0x0E13, 0x0E14, 0x0E15, 0x0E16, 0x0E17,
    0x0E18, 0x0E19, 0x0E1A, 0x0E1B, 0x0E1C, 0x0E1D, 0x0E1E, 0x0E1F,
    0x0E20, 0x0E21, 0x0E22, 0x0E23, 0x0E24, 0x0E25, 0x0E26, 0x0E27,
    0x0E28, 0x0E29, 0x0E2A, 0x0E2B, 0x0E2C, 0x0E2D, 0x0E2E, 0x0E2F,
    0x0E30, 0x0E31, 0x0E32, 0x0E33, 0x0E34, 0x0E35, 0x0E36, 0x0E37,
    0x0E38, 0x0E39, 0x0E3A, 0x0000, 0x0000, 0x0000, 0x0000, 0x0E3F,
    0x0E40, 0x0E41, 0x0E42, 0x0E43, 0x0E44, 0x0E45, 0x0E46, 0x0E47,
    0x0E48, 0x0E49, 0x0E4A, 0x0E4B, 0x0E4C, 0x0E4D, 0x0E4E, 0x0E4F,
    0x0E50, 0x0E51, 0x0E52, 0x0E53, 0x0E54, 0x0E55, 0x0E56, 0x0E57,
    0x0E58, 0x0E59, 0x0E5A, 0x0E5B, 0x0000, 0x0000, 0x0000, 0x0000,
};
static const FX_WCHAR g_FX_CP1250Unicodes[128] = {
    0x20AC, 0x0000, 0x201A, 0x0000, 0x201E, 0x2026, 0x2020, 0x2021,
    0x0000, 0x2030, 0x0160, 0x2039, 0x015A, 0x0164, 0x017D, 0x0179,
    0x0000, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
    0x0000, 0x2122, 0x0161, 0x203A, 0x015B, 0x0165, 0x017E, 0x017A,
    0x00A0, 0x02C7, 0x02D8, 0x0141, 0x00A4, 0x0104, 0x00A6, 0x00A7,
    0x00A8, 0x00A9, 0x015E, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x017B,
    0x00B0, 0x00B1, 0x02DB, 0x0142, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
    0x00B8, 0x0105, 0x015F, 0x00BB, 0x013D, 0x02DD, 0x013E, 0x017C,
    0x0154, 0x00C1, 0x00C2, 0x0102, 0x00C4, 0x0139, 0x0106, 0x00C7,
    0x010C, 0x00C9, 0x0118, 0x00CB, 0x011A, 0x00CD, 0x00CE, 0x010E,
    0x0110, 0x0143, 0x0147, 0x00D3, 0x00D4, 0x0150, 0x00D6, 0x00D7,
    0x0158, 0x016E, 0x00DA, 0x0170, 0x00DC, 0x00DD, 0x0162, 0x00DF,
    0x0155, 0x00E1, 0x00E2, 0x0103, 0x00E4, 0x013A, 0x0107, 0x00E7,
    0x010D, 0x00E9, 0x0119, 0x00EB, 0x011B, 0x00ED, 0x00EE, 0x010F,
    0x0111, 0x0144, 0x0148, 0x00F3, 0x00F4, 0x0151, 0x00F6, 0x00F7,
    0x0159, 0x016F, 0x00FA, 0x0171, 0x00FC, 0x00FD, 0x0163, 0x02D9,
};
static const FX_WCHAR g_FX_CP1251Unicodes[128] = {
    0x0402, 0x0403, 0x201A, 0x0453, 0x201E, 0x2026, 0x2020, 0x2021,
    0x20AC, 0x2030, 0x0409, 0x2039, 0x040A, 0x040C, 0x040B, 0x040F,
    0x0452, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
    0x0000, 0x2122, 0x0459, 0x203A, 0x045A, 0x045C, 0x045B, 0x045F,
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
    0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D, 0x044E, 0x044F,
};
static const FX_WCHAR g_FX_CP1253Unicodes[128] = {
    0x20AC, 0x0000, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
    0x0000, 0x2030, 0x0000, 0x2039, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
    0x0000, 0x2122, 0x0000, 0x203A, 0x0000, 0x0000, 0x0000, 0x0000,
    0x00A0, 0x0385, 0x0386, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7,
    0x00A8, 0x00A9, 0x0000, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x2015,
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x0384, 0x00B5, 0x00B6, 0x00B7,
    0x0388, 0x0389, 0x038A, 0x00BB, 0x038C, 0x00BD, 0x038E, 0x038F,
    0x0390, 0x0391, 0x0392, 0x0393, 0x0394, 0x0395, 0x0396, 0x0397,
    0x0398, 0x0399, 0x039A, 0x039B, 0x039C, 0x039D, 0x039E, 0x039F,
    0x03A0, 0x03A1, 0x0000, 0x03A3, 0x03A4, 0x03A5, 0x03A6, 0x03A7,
    0x03A8, 0x03A9, 0x03AA, 0x03AB, 0x03AC, 0x03AD, 0x03AE, 0x03AF,
    0x03B0, 0x03B1, 0x03B2, 0x03B3, 0x03B4, 0x03B5, 0x03B6, 0x03B7,
    0x03B8, 0x03B9, 0x03BA, 0x03BB, 0x03BC, 0x03BD, 0x03BE, 0x03BF,
    0x03C0, 0x03C1, 0x03C2, 0x03C3, 0x03C4, 0x03C5, 0x03C6, 0x03C7,
    0x03C8, 0x03C9, 0x03CA, 0x03CB, 0x03CC, 0x03CD, 0x03CE, 0x0000,
};
static const FX_WCHAR g_FX_CP1254Unicodes[128] = {
    0x20AC, 0x0000, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
    0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x0000, 0x0000, 0x0000,
    0x0000, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
    0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x0000, 0x0000, 0x0178,
    0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7,
    0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
    0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
    0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7,
    0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
    0x011E, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7,
    0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x0130, 0x015E, 0x00DF,
    0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7,
    0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
    0x011F, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7,
    0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x0131, 0x015F, 0x00FF,
};
static const FX_WCHAR g_FX_CP1255Unicodes[128] = {
    0x20AC, 0x0000, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
    0x02C6, 0x2030, 0x0000, 0x2039, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
    0x02DC, 0x2122, 0x0000, 0x203A, 0x0000, 0x0000, 0x0000, 0x0000,
    0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x20AA, 0x00A5, 0x00A6, 0x00A7,
    0x00A8, 0x00A9, 0x00D7, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
    0x00B8, 0x00B9, 0x00F7, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
    0x05B0, 0x05B1, 0x05B2, 0x05B3, 0x05B4, 0x05B5, 0x05B6, 0x05B7,
    0x05B8, 0x05B9, 0x0000, 0x05BB, 0x05BC, 0x05BD, 0x05BE, 0x05BF,
    0x05C0, 0x05C1, 0x05C2, 0x05C3, 0x05F0, 0x05F1, 0x05F2, 0x05F3,
    0x05F4, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x05D0, 0x05D1, 0x05D2, 0x05D3, 0x05D4, 0x05D5, 0x05D6, 0x05D7,
    0x05D8, 0x05D9, 0x05DA, 0x05DB, 0x05DC, 0x05DD, 0x05DE, 0x05DF,
    0x05E0, 0x05E1, 0x05E2, 0x05E3, 0x05E4, 0x05E5, 0x05E6, 0x05E7,
    0x05E8, 0x05E9, 0x05EA, 0x0000, 0x0000, 0x200E, 0x200F, 0x0000,
};
static const FX_WCHAR g_FX_CP1256Unicodes[128] = {
    0x20AC, 0x067E, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
    0x02C6, 0x2030, 0x0679, 0x2039, 0x0152, 0x0686, 0x0698, 0x0688,
    0x06AF, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
    0x06A9, 0x2122, 0x0691, 0x203A, 0x0153, 0x200C, 0x200D, 0x06BA,
    0x00A0, 0x060C, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7,
    0x00A8, 0x00A9, 0x06BE, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
    0x00B8, 0x00B9, 0x061B, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x061F,
    0x06C1, 0x0621, 0x0622, 0x0623, 0x0624, 0x0625, 0x0626, 0x0627,
    0x0628, 0x0629, 0x062A, 0x062B, 0x062C, 0x062D, 0x062E, 0x062F,
    0x0630, 0x0631, 0x0632, 0x0633, 0x0634, 0x0635, 0x0636, 0x00D7,
    0x0637, 0x0638, 0x0639, 0x063A, 0x0640, 0x0641, 0x0642, 0x0643,
    0x00E0, 0x0644, 0x00E2, 0x0645, 0x0646, 0x0647, 0x0648, 0x00E7,
    0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x0649, 0x064A, 0x00EE, 0x00EF,
    0x064B, 0x064C, 0x064D, 0x064E, 0x00F4, 0x064F, 0x0650, 0x00F7,
    0x0651, 0x00F9, 0x0652, 0x00FB, 0x00FC, 0x200E, 0x200F, 0x06D2,
};
static const FX_WCHAR g_FX_CP1257Unicodes[128] = {
    0x20AC, 0x0000, 0x201A, 0x0000, 0x201E, 0x2026, 0x2020, 0x2021,
    0x0000, 0x2030, 0x0000, 0x2039, 0x0000, 0x00A8, 0x02C7, 0x00B8,
    0x0000, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
    0x0000, 0x2122, 0x0000, 0x203A, 0x0000, 0x00AF, 0x02DB, 0x0000,
    0x00A0, 0x0000, 0x00A2, 0x00A3, 0x00A4, 0x0000, 0x00A6, 0x00A7,
    0x00D8, 0x00A9, 0x0156, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00C6,
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
    0x00F8, 0x00B9, 0x0157, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00E6,
    0x0104, 0x012E, 0x0100, 0x0106, 0x00C4, 0x00C5, 0x0118, 0x0112,
    0x010C, 0x00C9, 0x0179, 0x0116, 0x0122, 0x0136, 0x012A, 0x013B,
    0x0160, 0x0143, 0x0145, 0x00D3, 0x014C, 0x00D5, 0x00D6, 0x00D7,
    0x0172, 0x0141, 0x015A, 0x016A, 0x00DC, 0x017B, 0x017D, 0x00DF,
    0x0105, 0x012F, 0x0101, 0x0107, 0x00E4, 0x00E5, 0x0119, 0x0113,
    0x010D, 0x00E9, 0x017A, 0x0117, 0x0123, 0x0137, 0x012B, 0x013C,
    0x0161, 0x0144, 0x0146, 0x00F3, 0x014D, 0x00F5, 0x00F6, 0x00F7,
    0x0173, 0x0142, 0x015B, 0x016B, 0x00FC, 0x017C, 0x017E, 0x02D9,
};
typedef struct {
    FX_BYTE		m_Charset;
    const FX_WCHAR*	m_pUnicodes;
} FX_CharsetUnicodes;
const FX_CharsetUnicodes g_FX_CharsetUnicodes[] = {
    { FXFONT_THAI_CHARSET, g_FX_CP874Unicodes },
    { FXFONT_EASTEUROPE_CHARSET, g_FX_CP1250Unicodes },
    { FXFONT_RUSSIAN_CHARSET, g_FX_CP1251Unicodes },
    { FXFONT_GREEK_CHARSET, g_FX_CP1253Unicodes },
    { FXFONT_TURKISH_CHARSET, g_FX_CP1254Unicodes },
    { FXFONT_HEBREW_CHARSET, g_FX_CP1255Unicodes },
    { FXFONT_ARABIC_CHARSET, g_FX_CP1256Unicodes },
    { FXFONT_BALTIC_CHARSET, g_FX_CP1257Unicodes },
};
#if (_FX_OS_ == _FX_WIN32_DESKTOP_ || _FX_OS_ == _FX_WIN32_MOBILE_ || _FX_OS_ == _FX_WIN64_)
static void _InsertWidthArray(HDC hDC, int start, int end, CPDF_Array* pWidthArray)
{
    int size = end - start + 1;
    int* widths = FX_Alloc(int, size);
    GetCharWidth(hDC, start, end, widths);
    int i;
    for (i = 1; i < size; i ++)
        if (widths[i] != *widths) {
            break;
        }
    if (i == size) {
        int first = pWidthArray->GetInteger(pWidthArray->GetCount() - 1);
        pWidthArray->AddInteger(first + size - 1);
        pWidthArray->AddInteger(*widths);
    } else {
        CPDF_Array* pWidthArray1 = new CPDF_Array;
        pWidthArray->Add(pWidthArray1);
        for (i = 0; i < size; i ++) {
            pWidthArray1->AddInteger(widths[i]);
        }
    }
    FX_Free(widths);
}
CPDF_Font* CPDF_Document::AddWindowsFont(LOGFONTW* pLogFont, FX_BOOL bVert, FX_BOOL bTranslateName)
{
    LOGFONTA lfa;
    FXSYS_memcpy32(&lfa, pLogFont, (char*)lfa.lfFaceName - (char*)&lfa);
    CFX_ByteString face = CFX_ByteString::FromUnicode(pLogFont->lfFaceName);
    if (face.GetLength() >= LF_FACESIZE) {
        return NULL;
    }
    FXSYS_strcpy(lfa.lfFaceName, face.c_str());
    return AddWindowsFont(&lfa, bVert, bTranslateName);
}
extern CFX_ByteString _FPDF_GetNameFromTT(FX_LPCBYTE name_table, FX_DWORD name);
CFX_ByteString _FPDF_GetPSNameFromTT(HDC hDC)
{
    CFX_ByteString result;
    DWORD size = ::GetFontData(hDC, 'eman', 0, NULL, 0);
    if (size != GDI_ERROR) {
        LPBYTE buffer = FX_Alloc(BYTE, size);
        ::GetFontData(hDC, 'eman', 0, buffer, size);
        result = _FPDF_GetNameFromTT(buffer, 6);
        FX_Free(buffer);
    }
    return result;
}
CPDF_Font* CPDF_Document::AddWindowsFont(LOGFONTA* pLogFont, FX_BOOL bVert, FX_BOOL bTranslateName)
{
    pLogFont->lfHeight = -1000;
    pLogFont->lfWidth = 0;
    HGDIOBJ hFont = CreateFontIndirectA(pLogFont);
    HDC hDC = CreateCompatibleDC(NULL);
    hFont = SelectObject(hDC, hFont);
    int tm_size = GetOutlineTextMetrics(hDC, 0, NULL);
    if (tm_size == 0) {
        hFont = SelectObject(hDC, hFont);
        DeleteObject(hFont);
        DeleteDC(hDC);
        return NULL;
    }
    LPBYTE tm_buf = FX_Alloc(BYTE, tm_size);
    OUTLINETEXTMETRIC* ptm = (OUTLINETEXTMETRIC*)tm_buf;
    GetOutlineTextMetrics(hDC, tm_size, ptm);
    int flags = 0, italicangle, ascend, descend, capheight, bbox[4];
    if (pLogFont->lfItalic) {
        flags |= PDFFONT_ITALIC;
    }
    if ((pLogFont->lfPitchAndFamily & 3) == FIXED_PITCH) {
        flags |= PDFFONT_FIXEDPITCH;
    }
    if ((pLogFont->lfPitchAndFamily & 0xf8) == FF_ROMAN) {
        flags |= PDFFONT_SERIF;
    }
    if ((pLogFont->lfPitchAndFamily & 0xf8) == FF_SCRIPT) {
        flags |= PDFFONT_SCRIPT;
    }
    FX_BOOL bCJK = pLogFont->lfCharSet == CHINESEBIG5_CHARSET || pLogFont->lfCharSet == GB2312_CHARSET ||
                   pLogFont->lfCharSet == HANGEUL_CHARSET || pLogFont->lfCharSet == SHIFTJIS_CHARSET;
    CFX_ByteString basefont;
    if (bTranslateName && bCJK) {
        basefont = _FPDF_GetPSNameFromTT(hDC);
    }
    if (basefont.IsEmpty()) {
        basefont = pLogFont->lfFaceName;
    }
    italicangle = ptm->otmItalicAngle / 10;
    ascend = ptm->otmrcFontBox.top;
    descend = ptm->otmrcFontBox.bottom;
    capheight = ptm->otmsCapEmHeight;
    bbox[0] = ptm->otmrcFontBox.left;
    bbox[1] = ptm->otmrcFontBox.bottom;
    bbox[2] = ptm->otmrcFontBox.right;
    bbox[3] = ptm->otmrcFontBox.top;
    FX_Free(tm_buf);
    basefont.Replace(" ", "");
    CPDF_Dictionary* pBaseDict = new CPDF_Dictionary;
    pBaseDict->SetAtName("Type", "Font");
    CPDF_Dictionary* pFontDict = pBaseDict;
    if (!bCJK) {
        if (pLogFont->lfCharSet == ANSI_CHARSET || pLogFont->lfCharSet == DEFAULT_CHARSET ||
                pLogFont->lfCharSet == SYMBOL_CHARSET) {
            if (pLogFont->lfCharSet == SYMBOL_CHARSET) {
                flags |= PDFFONT_SYMBOLIC;
            } else {
                flags |= PDFFONT_NONSYMBOLIC;
            }
            pBaseDict->SetAtName(FX_BSTRC("Encoding"), "WinAnsiEncoding");
        } else {
            flags |= PDFFONT_NONSYMBOLIC;
            int i;
            for (i = 0; i < sizeof g_FX_CharsetUnicodes / sizeof(FX_CharsetUnicodes); i ++)
                if (g_FX_CharsetUnicodes[i].m_Charset == pLogFont->lfCharSet) {
                    break;
                }
            if (i < sizeof g_FX_CharsetUnicodes / sizeof(FX_CharsetUnicodes)) {
                CPDF_Dictionary* pEncoding = new CPDF_Dictionary;
                pEncoding->SetAtName(FX_BSTRC("BaseEncoding"), "WinAnsiEncoding");
                CPDF_Array* pArray = new CPDF_Array;
                pArray->AddInteger(128);
                const FX_WCHAR* pUnicodes = g_FX_CharsetUnicodes[i].m_pUnicodes;
                for (int j = 0; j < 128; j ++) {
                    CFX_ByteString name = PDF_AdobeNameFromUnicode(pUnicodes[j]);
                    if (name.IsEmpty()) {
                        pArray->AddName(FX_BSTRC(".notdef"));
                    } else {
                        pArray->AddName(name);
                    }
                }
                pEncoding->SetAt(FX_BSTRC("Differences"), pArray);
                AddIndirectObject(pEncoding);
                pBaseDict->SetAtReference(FX_BSTRC("Encoding"), this, pEncoding);
            }
        }
        if (pLogFont->lfWeight > FW_MEDIUM && pLogFont->lfItalic) {
            basefont += ",BoldItalic";
        } else if (pLogFont->lfWeight > FW_MEDIUM) {
            basefont += ",Bold";
        } else if (pLogFont->lfItalic) {
            basefont += ",Italic";
        }
        pBaseDict->SetAtName("Subtype", "TrueType");
        pBaseDict->SetAtName("BaseFont", basefont);
        pBaseDict->SetAtNumber("FirstChar", 32);
        pBaseDict->SetAtNumber("LastChar", 255);
        int char_widths[224];
        GetCharWidth(hDC, 32, 255, char_widths);
        CPDF_Array* pWidths = new CPDF_Array;
        for (int i = 0; i < 224; i ++) {
            pWidths->AddInteger(char_widths[i]);
        }
        pBaseDict->SetAt("Widths", pWidths);
    } else {
        flags |= PDFFONT_NONSYMBOLIC;
        pFontDict = new CPDF_Dictionary;
        CFX_ByteString cmap;
        CFX_ByteString ordering;
        int supplement;
        CPDF_Array* pWidthArray = new CPDF_Array;
        switch (pLogFont->lfCharSet) {
            case CHINESEBIG5_CHARSET:
                cmap = bVert ? "ETenms-B5-V" : "ETenms-B5-H";
                ordering = "CNS1";
                supplement = 4;
                pWidthArray->AddInteger(1);
                _InsertWidthArray(hDC, 0x20, 0x7e, pWidthArray);
                break;
            case GB2312_CHARSET:
                cmap = bVert ? "GBK-EUC-V" : "GBK-EUC-H";
                ordering = "GB1", supplement = 2;
                pWidthArray->AddInteger(7716);
                _InsertWidthArray(hDC, 0x20, 0x20, pWidthArray);
                pWidthArray->AddInteger(814);
                _InsertWidthArray(hDC, 0x21, 0x7e, pWidthArray);
                break;
            case HANGEUL_CHARSET:
                cmap = bVert ? "KSCms-UHC-V" : "KSCms-UHC-H";
                ordering = "Korea1";
                supplement = 2;
                pWidthArray->AddInteger(1);
                _InsertWidthArray(hDC, 0x20, 0x7e, pWidthArray);
                break;
            case SHIFTJIS_CHARSET:
                cmap = bVert ? "90ms-RKSJ-V" : "90ms-RKSJ-H";
                ordering = "Japan1";
                supplement = 5;
                pWidthArray->AddInteger(231);
                _InsertWidthArray(hDC, 0x20, 0x7d, pWidthArray);
                pWidthArray->AddInteger(326);
                _InsertWidthArray(hDC, 0xa0, 0xa0, pWidthArray);
                pWidthArray->AddInteger(327);
                _InsertWidthArray(hDC, 0xa1, 0xdf, pWidthArray);
                pWidthArray->AddInteger(631);
                _InsertWidthArray(hDC, 0x7e, 0x7e, pWidthArray);
                break;
        }
        pBaseDict->SetAtName("Subtype", "Type0");
        pBaseDict->SetAtName("BaseFont", basefont);
        pBaseDict->SetAtName("Encoding", cmap);
        pFontDict->SetAt("W", pWidthArray);
        pFontDict->SetAtName("Type", "Font");
        pFontDict->SetAtName("Subtype", "CIDFontType2");
        pFontDict->SetAtName("BaseFont", basefont);
        CPDF_Dictionary* pCIDSysInfo = new CPDF_Dictionary;
        pCIDSysInfo->SetAtString("Registry", "Adobe");
        pCIDSysInfo->SetAtString("Ordering", ordering);
        pCIDSysInfo->SetAtInteger("Supplement", supplement);
        pFontDict->SetAt("CIDSystemInfo", pCIDSysInfo);
        CPDF_Array* pArray = new CPDF_Array;
        pBaseDict->SetAt("DescendantFonts", pArray);
        AddIndirectObject(pFontDict);
        pArray->AddReference(this, pFontDict);
    }
    AddIndirectObject(pBaseDict);
    CPDF_Dictionary* pFontDesc = new CPDF_Dictionary;
    pFontDesc->SetAtName("Type", "FontDescriptor");
    pFontDesc->SetAtName("FontName", basefont);
    pFontDesc->SetAtInteger("Flags", flags);
    CPDF_Array* pBBox = new CPDF_Array;
    for (int i = 0; i < 4; i ++) {
        pBBox->AddInteger(bbox[i]);
    }
    pFontDesc->SetAt("FontBBox", pBBox);
    pFontDesc->SetAtInteger("ItalicAngle", italicangle);
    pFontDesc->SetAtInteger("Ascent", ascend);
    pFontDesc->SetAtInteger("Descent", descend);
    pFontDesc->SetAtInteger("CapHeight", capheight);
    pFontDesc->SetAtInteger("StemV", pLogFont->lfWeight / 5);
    AddIndirectObject(pFontDesc);
    pFontDict->SetAtReference("FontDescriptor", this, pFontDesc);
    hFont = SelectObject(hDC, hFont);
    DeleteObject(hFont);
    DeleteDC(hDC);
    return LoadFont(pBaseDict);
}
#endif
#if (_FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_)
FX_UINT32 FX_GetLangHashCode( FX_LPCSTR pStr)
{
    FXSYS_assert( pStr != NULL);
    FX_INT32 iLength = FXSYS_strlen(pStr);
    FX_LPCSTR pStrEnd = pStr + iLength;
    FX_UINT32 uHashCode = 0;
    while ( pStr < pStrEnd) {
        uHashCode = 31 * uHashCode + tolower(*pStr++);
    }
    return uHashCode;
}
struct FX_LANG2CS {
    FX_DWORD    uLang;
    int         uCharset;
}*FX_LPLANG2CS;
static const FX_LANG2CS gs_FXLang2CharsetTable[] = {
    {3109, 0},
    {3121, 178},
    {3129, 162},
    {3139, 204},
    {3141, 204},
    {3166, 0},
    {3184, 238},
    {3197, 0},
    {3201, 0},
    {3239, 161},
    {3241, 0},
    {3246, 0},
    {3247, 186},
    {3248, 0},
    {3259, 178},
    {3267, 0},
    {3273, 0},
    {3276, 0},
    {3301, 0},
    {3310, 1},
    {3325, 177},
    {3329, 1},
    {3338, 238},
    {3341, 238},
    {3345, 1},
    {3355, 0},
    {3370, 0},
    {3371, 0},
    {3383, 128},
    {3424, 204},
    {3427, 1},
    {3428, 129},
    {3436, 178},
    {3464, 186},
    {3466, 186},
    {3486, 204},
    {3487, 0},
    {3493, 1},
    {3494, 0},
    {3508, 0},
    {3518, 0},
    {3520, 0},
    {3569, 1},
    {3580, 238},
    {3588, 0},
    {3645, 238},
    {3651, 204},
    {3672, 238},
    {3673, 238},
    {3678, 238},
    {3679, 238},
    {3683, 0},
    {3684, 0},
    {3693, 1},
    {3697, 1},
    {3700, 222},
    {3710, 162},
    {3734, 204},
    {3741, 178},
    {3749, 162},
    {3763, 163},
    {3886, 134},
    {105943, 0},
    {106375, 1},
    {3923451837, 134},
    {3923451838, 136},
};
static FX_WORD FX_GetCsFromLangCode(FX_UINT32 uCode)
{
    FX_INT32 iStart = 0;
    FX_INT32 iEnd = sizeof(gs_FXLang2CharsetTable) / sizeof(FX_LANG2CS) - 1;
    while (iStart <= iEnd) {
        FX_INT32 iMid = (iStart + iEnd) / 2;
        const FX_LANG2CS &charset = gs_FXLang2CharsetTable[iMid];
        if (uCode == charset.uLang) {
            return charset.uCharset;
        } else if (uCode < charset.uLang) {
            iEnd = iMid - 1;
        } else {
            iStart = iMid + 1;
        }
    };
    return 0;
}
static FX_WORD FX_GetCharsetFromLang(FX_LPCSTR pLang, FX_INT32 iLength)
{
    FXSYS_assert(pLang);
    if (iLength < 0) {
        iLength = FXSYS_strlen(pLang);
    }
    FX_UINT32 uHash = FX_GetLangHashCode(pLang);
    return FX_GetCsFromLangCode(uHash);
}
static void _CFString2CFXByteString(CFStringRef src, CFX_ByteString &dest)
{
    SInt32 len =  CFStringGetLength(src);
    CFRange range = CFRangeMake(0, len);
    CFIndex used = 0;
    UInt8* pBuffer = (UInt8*)calloc(len+1, sizeof(UInt8));
    CFStringGetBytes(src, range, kCFStringEncodingASCII, 0, false, pBuffer, len, &used);
    dest = (FX_LPSTR)pBuffer;
    free(pBuffer);
}
FX_BOOL IsHasCharSet(CFArrayRef languages, const CFX_DWordArray &charSets)
{
    int iCount = charSets.GetSize();
    for (int i = 0; i < CFArrayGetCount(languages); ++i) {
        CFStringRef language = (CFStringRef)CFArrayGetValueAtIndex(languages, i);
        FX_DWORD CharSet = FX_GetCharsetFromLang(CFStringGetCStringPtr(language, kCFStringEncodingMacRoman), -1);
        for (int j = 0; j < iCount; ++j) {
            if (CharSet == charSets[j]) {
                return TRUE;
            }
        }
    }
    return FALSE;
}
void FX_GetCharWidth(CTFontRef font, UniChar start, UniChar end, int* width)
{
    CGFloat size = CTFontGetSize(font);
    for (; start <= end; ++start) {
        CGGlyph pGlyph = 0;
        CFIndex count = 1;
        CTFontGetGlyphsForCharacters(font, &start, &pGlyph, count);
        CGSize advances;
        CTFontGetAdvancesForGlyphs(font, kCTFontDefaultOrientation, &pGlyph, &advances, 1);
        *width = (int)(advances.width / size * 1000) ;
        width++;
    }
}
static void _InsertWidthArray(CTFontRef font, int start, int end, CPDF_Array* pWidthArray)
{
    int size = end - start + 1;
    int* widths = FX_Alloc(int, size);
    FX_GetCharWidth(font, start, end, widths);
    int i;
    for (i = 1; i < size; i ++)
        if (widths[i] != *widths) {
            break;
        }
    if (i == size) {
        int first = pWidthArray->GetInteger(pWidthArray->GetCount() - 1);
        pWidthArray->AddInteger(first + size - 1);
        pWidthArray->AddInteger(*widths);
    } else {
        CPDF_Array* pWidthArray1 = new CPDF_Array;
        pWidthArray->Add(pWidthArray1);
        for (i = 0; i < size; i ++) {
            pWidthArray1->AddInteger(widths[i]);
        }
    }
    FX_Free(widths);
}
CPDF_Font* CPDF_Document::AddMacFont(CTFontRef pFont, FX_BOOL bVert, FX_BOOL bTranslateName)
{
    CTFontRef font = (CTFontRef)pFont;
    CTFontDescriptorRef descriptor = CTFontCopyFontDescriptor(font);
    if (descriptor == NULL) {
        return NULL;
    }
    CFX_ByteString basefont;
    FX_BOOL bCJK = FALSE;
    int flags = 0, italicangle = 0, ascend = 0, descend = 0, capheight = 0, bbox[4];
    FXSYS_memset32(bbox, 0, sizeof(int) * 4);
    CFArrayRef languages = (CFArrayRef)CTFontDescriptorCopyAttribute(descriptor, kCTFontLanguagesAttribute);
    if (languages == NULL) {
        CFRelease(descriptor);
        return NULL;
    }
    CFX_DWordArray charSets;
    charSets.Add(FXFONT_CHINESEBIG5_CHARSET);
    charSets.Add(FXFONT_GB2312_CHARSET);
    charSets.Add(FXFONT_HANGEUL_CHARSET);
    charSets.Add(FXFONT_SHIFTJIS_CHARSET);
    if (IsHasCharSet(languages, charSets)) {
        bCJK = TRUE;
    }
    CFRelease(descriptor);
    CFDictionaryRef traits = (CFDictionaryRef)CTFontCopyTraits(font);
    if (traits == NULL) {
        CFRelease(languages);
        return NULL;
    }
    CFNumberRef sybolicTrait = (CFNumberRef)CFDictionaryGetValue(traits, kCTFontSymbolicTrait);
    CTFontSymbolicTraits trait = 0;
    CFNumberGetValue(sybolicTrait, kCFNumberSInt32Type, &trait);
    if (trait & kCTFontItalicTrait) {
        flags |= PDFFONT_ITALIC;
    }
    if (trait & kCTFontMonoSpaceTrait) {
        flags |= PDFFONT_FIXEDPITCH;
    }
    if (trait & kCTFontModernSerifsClass) {
        flags |= PDFFONT_SERIF;
    }
    if (trait & kCTFontScriptsClass) {
        flags |= PDFFONT_SCRIPT;
    }
    CFNumberRef weightTrait = (CFNumberRef)CFDictionaryGetValue(traits, kCTFontWeightTrait);
    Float32 weight = 0;
    CFNumberGetValue(weightTrait, kCFNumberFloat32Type, &weight);
    italicangle = CTFontGetSlantAngle(font);
    ascend      = CTFontGetAscent(font);
    descend     = CTFontGetDescent(font);
    capheight   = CTFontGetCapHeight(font);
    CGRect box  = CTFontGetBoundingBox(font);
    bbox[0]     = box.origin.x;
    bbox[1]     = box.origin.y;
    bbox[2]     = box.origin.x + box.size.width;
    bbox[3]     = box.origin.y + box.size.height;
    if (bTranslateName && bCJK) {
        CFStringRef postName = CTFontCopyPostScriptName(font);
        _CFString2CFXByteString(postName, basefont);
        CFRelease(postName);
    }
    if (basefont.IsEmpty()) {
        CFStringRef fullName = CTFontCopyFullName(font);
        _CFString2CFXByteString(fullName, basefont);
        CFRelease(fullName);
    }
    basefont.Replace(" ", "");
    CPDF_Dictionary* pBaseDict = new CPDF_Dictionary;
    CPDF_Dictionary* pFontDict = pBaseDict;
    if (!bCJK) {
        charSets.RemoveAll();
        charSets.Add(FXFONT_ANSI_CHARSET);
        charSets.Add(FXFONT_DEFAULT_CHARSET);
        charSets.Add(FXFONT_SYMBOL_CHARSET);
        if (IsHasCharSet(languages, charSets)) {
            charSets.RemoveAll();
            charSets.Add(FXFONT_SYMBOL_CHARSET);
            if (IsHasCharSet(languages, charSets)) {
                flags |= PDFFONT_SYMBOLIC;
            } else {
                flags |= PDFFONT_NONSYMBOLIC;
            }
            pBaseDict->SetAtName(FX_BSTRC("Encoding"), "WinAnsiEncoding");
        } else {
            flags |= PDFFONT_NONSYMBOLIC;
            size_t i;
            for (i = 0; i < sizeof g_FX_CharsetUnicodes / sizeof(FX_CharsetUnicodes); i ++) {
                charSets.RemoveAll();
                charSets.Add(g_FX_CharsetUnicodes[i].m_Charset);
                if (IsHasCharSet(languages, charSets)) {
                    break;
                }
            }
            if (i < sizeof g_FX_CharsetUnicodes / sizeof(FX_CharsetUnicodes)) {
                CPDF_Dictionary* pEncoding = new CPDF_Dictionary;
                pEncoding->SetAtName(FX_BSTRC("BaseEncoding"), "WinAnsiEncoding");
                CPDF_Array* pArray = new CPDF_Array;
                pArray->AddInteger(128);
                const FX_WCHAR* pUnicodes = g_FX_CharsetUnicodes[i].m_pUnicodes;
                for (int j = 0; j < 128; j ++) {
                    CFX_ByteString name = PDF_AdobeNameFromUnicode(pUnicodes[j]);
                    if (name.IsEmpty()) {
                        pArray->AddName(FX_BSTRC(".notdef"));
                    } else {
                        pArray->AddName(name);
                    }
                }
                pEncoding->SetAt(FX_BSTRC("Differences"), pArray);
                AddIndirectObject(pEncoding);
                pBaseDict->SetAtReference(FX_BSTRC("Encoding"), this, pEncoding);
            }
        }
        if (weight > 0.0 && trait & kCTFontItalicTrait) {
            basefont += ",BoldItalic";
        } else if (weight > 0.0) {
            basefont += ",Bold";
        } else if (trait & kCTFontItalicTrait) {
            basefont += ",Italic";
        }
        pBaseDict->SetAtName("Subtype", "TrueType");
        pBaseDict->SetAtName("BaseFont", basefont);
        pBaseDict->SetAtNumber("FirstChar", 32);
        pBaseDict->SetAtNumber("LastChar", 255);
        int char_widths[224];
        FX_GetCharWidth(font, 32, 255, char_widths);
        CPDF_Array* pWidths = new CPDF_Array;
        for (int i = 0; i < 224; i ++) {
            pWidths->AddInteger(char_widths[i]);
        }
        pBaseDict->SetAt("Widths", pWidths);
    }  else {
        flags |= PDFFONT_NONSYMBOLIC;
        CPDF_Array* pArray = NULL;
        pFontDict = new CPDF_Dictionary;
        CFX_ByteString cmap;
        CFX_ByteString ordering;
        int supplement;
        FX_BOOL bFound = FALSE;
        CPDF_Array* pWidthArray = new CPDF_Array;
        charSets.RemoveAll();
        charSets.Add(FXFONT_CHINESEBIG5_CHARSET);
        if (IsHasCharSet(languages, charSets)) {
            cmap = bVert ? "ETenms-B5-V" : "ETenms-B5-H";
            ordering = "CNS1";
            supplement = 4;
            pWidthArray->AddInteger(1);
            _InsertWidthArray(font, 0x20, 0x7e, pWidthArray);
            bFound = TRUE;
        }
        charSets.RemoveAll();
        charSets.Add(FXFONT_GB2312_CHARSET);
        if (!bFound && IsHasCharSet(languages, charSets)) {
            cmap = bVert ? "GBK-EUC-V" : "GBK-EUC-H";
            ordering = "GB1", supplement = 2;
            pWidthArray->AddInteger(7716);
            _InsertWidthArray(font, 0x20, 0x20, pWidthArray);
            pWidthArray->AddInteger(814);
            _InsertWidthArray(font, 0x21, 0x7e, pWidthArray);
            bFound = TRUE;
        }
        charSets.RemoveAll();
        charSets.Add(FXFONT_HANGEUL_CHARSET);
        if (!bFound && IsHasCharSet(languages, charSets)) {
            cmap = bVert ? "KSCms-UHC-V" : "KSCms-UHC-H";
            ordering = "Korea1";
            supplement = 2;
            pWidthArray->AddInteger(1);
            _InsertWidthArray(font, 0x20, 0x7e, pWidthArray);
            bFound = TRUE;
        }
        charSets.RemoveAll();
        charSets.Add(FXFONT_SHIFTJIS_CHARSET);
        if (!bFound && IsHasCharSet(languages, charSets)) {
            cmap = bVert ? "90ms-RKSJ-V" : "90ms-RKSJ-H";
            ordering = "Japan1";
            supplement = 5;
            pWidthArray->AddInteger(231);
            _InsertWidthArray(font, 0x20, 0x7d, pWidthArray);
            pWidthArray->AddInteger(326);
            _InsertWidthArray(font, 0xa0, 0xa0, pWidthArray);
            pWidthArray->AddInteger(327);
            _InsertWidthArray(font, 0xa1, 0xdf, pWidthArray);
            pWidthArray->AddInteger(631);
            _InsertWidthArray(font, 0x7e, 0x7e, pWidthArray);
        }
        pBaseDict->SetAtName("Subtype", "Type0");
        pBaseDict->SetAtName("BaseFont", basefont);
        pBaseDict->SetAtName("Encoding", cmap);
        pFontDict->SetAt("W", pWidthArray);
        pFontDict->SetAtName("Type", "Font");
        pFontDict->SetAtName("Subtype", "CIDFontType2");
        pFontDict->SetAtName("BaseFont", basefont);
        CPDF_Dictionary* pCIDSysInfo = new CPDF_Dictionary;
        pCIDSysInfo->SetAtString("Registry", "Adobe");
        pCIDSysInfo->SetAtString("Ordering", ordering);
        pCIDSysInfo->SetAtInteger("Supplement", supplement);
        pFontDict->SetAt("CIDSystemInfo", pCIDSysInfo);
        pArray = new CPDF_Array;
        pBaseDict->SetAt("DescendantFonts", pArray);
        AddIndirectObject(pFontDict);
        pArray->AddReference(this, pFontDict);
    }
    AddIndirectObject(pBaseDict);
    CPDF_Dictionary* pFontDesc = new CPDF_Dictionary;
    pFontDesc->SetAtName("Type", "FontDescriptor");
    pFontDesc->SetAtName("FontName", basefont);
    pFontDesc->SetAtInteger("Flags", flags);
    CPDF_Array* pBBox = new CPDF_Array;
    for (int i = 0; i < 4; i ++) {
        pBBox->AddInteger(bbox[i]);
    }
    pFontDesc->SetAt("FontBBox", pBBox);
    pFontDesc->SetAtInteger("ItalicAngle", italicangle);
    pFontDesc->SetAtInteger("Ascent", ascend);
    pFontDesc->SetAtInteger("Descent", descend);
    pFontDesc->SetAtInteger("CapHeight", capheight);
    CGFloat fStemV = 0;
    int16_t min_width = SHRT_MAX;
    static const UniChar stem_chars[] = {'i', 'I', '!', '1'};
    const size_t count = sizeof(stem_chars) / sizeof(stem_chars[0]);
    CGGlyph glyphs[count];
    CGRect boundingRects[count];
    if (CTFontGetGlyphsForCharacters(font, stem_chars, glyphs, count)) {
        CTFontGetBoundingRectsForGlyphs(font, kCTFontHorizontalOrientation,
                                        glyphs, boundingRects, count);
        for (size_t i = 0; i < count; i++) {
            int16_t width = boundingRects[i].size.width;
            if (width > 0 && width < min_width) {
                min_width = width;
                fStemV = min_width;
            }
        }
    }
    pFontDesc->SetAtInteger("StemV", fStemV);
    AddIndirectObject(pFontDesc);
    pFontDict->SetAtReference("FontDescriptor", this, pFontDesc);
    CFRelease(traits);
    CFRelease(languages);
    return LoadFont(pBaseDict);
}
#endif
static void _InsertWidthArray1(CFX_Font* pFont, IFX_FontEncoding* pEncoding, FX_WCHAR start, FX_WCHAR end, CPDF_Array* pWidthArray)
{
    int size = end - start + 1;
    int* widths = FX_Alloc(int, size);
    int i;
    for (i = 0; i < size; i ++) {
        int glyph_index = pEncoding->GlyphFromCharCode(start + i);
        widths[i] = pFont->GetGlyphWidth(glyph_index);
    }
    for (i = 1; i < size; i ++)
        if (widths[i] != *widths) {
            break;
        }
    if (i == size) {
        int first = pWidthArray->GetInteger(pWidthArray->GetCount() - 1);
        pWidthArray->AddInteger(first + size - 1);
        pWidthArray->AddInteger(*widths);
    } else {
        CPDF_Array* pWidthArray1 = new CPDF_Array;
        pWidthArray->Add(pWidthArray1);
        for (i = 0; i < size; i ++) {
            pWidthArray1->AddInteger(widths[i]);
        }
    }
    FX_Free(widths);
}
CPDF_Font* CPDF_Document::AddFont(CFX_Font* pFont, int charset, FX_BOOL bVert)
{
    if (pFont == NULL) {
        return NULL;
    }
    FX_BOOL bCJK = charset == FXFONT_CHINESEBIG5_CHARSET || charset == FXFONT_GB2312_CHARSET ||
                   charset == FXFONT_HANGEUL_CHARSET || charset == FXFONT_SHIFTJIS_CHARSET;
    CFX_ByteString basefont = pFont->GetFamilyName();
    basefont.Replace(" ", "");
    int flags = 0;
    if (pFont->IsBold()) {
        flags |= PDFFONT_FORCEBOLD;
    }
    if (pFont->IsItalic()) {
        flags |= PDFFONT_ITALIC;
    }
    if (pFont->IsFixedWidth()) {
        flags |= PDFFONT_FIXEDPITCH;
    }
    CPDF_Dictionary* pBaseDict = new CPDF_Dictionary;
    pBaseDict->SetAtName("Type", "Font");
    IFX_FontEncoding* pEncoding = FXGE_CreateUnicodeEncoding(pFont);
    CPDF_Dictionary* pFontDict = pBaseDict;
    if (!bCJK) {
        CPDF_Array* pWidths = new CPDF_Array;
        int charcode;
        for (charcode = 32; charcode < 128; charcode ++) {
            int glyph_index = pEncoding->GlyphFromCharCode(charcode);
            int char_width = pFont->GetGlyphWidth(glyph_index);
            pWidths->AddInteger(char_width);
        }
        if (charset == FXFONT_ANSI_CHARSET || charset == FXFONT_DEFAULT_CHARSET ||
                charset == FXFONT_SYMBOL_CHARSET) {
            if (charset == FXFONT_SYMBOL_CHARSET) {
                flags |= PDFFONT_SYMBOLIC;
            } else {
                flags |= PDFFONT_NONSYMBOLIC;
            }
            pBaseDict->SetAtName(FX_BSTRC("Encoding"), "WinAnsiEncoding");
            for (charcode = 128; charcode <= 255; charcode ++) {
                int glyph_index = pEncoding->GlyphFromCharCode(charcode);
                int char_width = pFont->GetGlyphWidth(glyph_index);
                pWidths->AddInteger(char_width);
            }
        } else {
            flags |= PDFFONT_NONSYMBOLIC;
            size_t i;
            for (i = 0; i < sizeof g_FX_CharsetUnicodes / sizeof(FX_CharsetUnicodes); i ++)
                if (g_FX_CharsetUnicodes[i].m_Charset == charset) {
                    break;
                }
            if (i < sizeof g_FX_CharsetUnicodes / sizeof(FX_CharsetUnicodes)) {
                CPDF_Dictionary* pEncodingDict = new CPDF_Dictionary;
                pEncodingDict->SetAtName(FX_BSTRC("BaseEncoding"), "WinAnsiEncoding");
                CPDF_Array* pArray = new CPDF_Array;
                pArray->AddInteger(128);
                const FX_WCHAR* pUnicodes = g_FX_CharsetUnicodes[i].m_pUnicodes;
                for (int j = 0; j < 128; j ++) {
                    CFX_ByteString name = PDF_AdobeNameFromUnicode(pUnicodes[j]);
                    if (name.IsEmpty()) {
                        pArray->AddName(FX_BSTRC(".notdef"));
                    } else {
                        pArray->AddName(name);
                    }
                    int glyph_index = pEncoding->GlyphFromCharCode(pUnicodes[j]);
                    int char_width = pFont->GetGlyphWidth(glyph_index);
                    pWidths->AddInteger(char_width);
                }
                pEncodingDict->SetAt(FX_BSTRC("Differences"), pArray);
                AddIndirectObject(pEncodingDict);
                pBaseDict->SetAtReference(FX_BSTRC("Encoding"), this, pEncodingDict);
            }
        }
        if (pFont->IsBold() && pFont->IsItalic()) {
            basefont += ",BoldItalic";
        } else if (pFont->IsBold()) {
            basefont += ",Bold";
        } else if (pFont->IsItalic()) {
            basefont += ",Italic";
        }
        pBaseDict->SetAtName("Subtype", "TrueType");
        pBaseDict->SetAtName("BaseFont", basefont);
        pBaseDict->SetAtNumber("FirstChar", 32);
        pBaseDict->SetAtNumber("LastChar", 255);
        pBaseDict->SetAt("Widths", pWidths);
    } else {
        flags |= PDFFONT_NONSYMBOLIC;
        pFontDict = new CPDF_Dictionary;
        CFX_ByteString cmap;
        CFX_ByteString ordering;
        int supplement;
        CPDF_Array* pWidthArray = new CPDF_Array;
        switch (charset) {
            case FXFONT_CHINESEBIG5_CHARSET:
                cmap = bVert ? "ETenms-B5-V" : "ETenms-B5-H";
                ordering = "CNS1";
                supplement = 4;
                pWidthArray->AddInteger(1);
                _InsertWidthArray1(pFont, pEncoding, 0x20, 0x7e, pWidthArray);
                break;
            case FXFONT_GB2312_CHARSET:
                cmap = bVert ? "GBK-EUC-V" : "GBK-EUC-H";
                ordering = "GB1", supplement = 2;
                pWidthArray->AddInteger(7716);
                _InsertWidthArray1(pFont, pEncoding, 0x20, 0x20, pWidthArray);
                pWidthArray->AddInteger(814);
                _InsertWidthArray1(pFont, pEncoding, 0x21, 0x7e, pWidthArray);
                break;
            case FXFONT_HANGEUL_CHARSET:
                cmap = bVert ? "KSCms-UHC-V" : "KSCms-UHC-H";
                ordering = "Korea1";
                supplement = 2;
                pWidthArray->AddInteger(1);
                _InsertWidthArray1(pFont, pEncoding, 0x20, 0x7e, pWidthArray);
                break;
            case FXFONT_SHIFTJIS_CHARSET:
                cmap = bVert ? "90ms-RKSJ-V" : "90ms-RKSJ-H";
                ordering = "Japan1";
                supplement = 5;
                pWidthArray->AddInteger(231);
                _InsertWidthArray1(pFont, pEncoding, 0x20, 0x7d, pWidthArray);
                pWidthArray->AddInteger(326);
                _InsertWidthArray1(pFont, pEncoding, 0xa0, 0xa0, pWidthArray);
                pWidthArray->AddInteger(327);
                _InsertWidthArray1(pFont, pEncoding, 0xa1, 0xdf, pWidthArray);
                pWidthArray->AddInteger(631);
                _InsertWidthArray1(pFont, pEncoding, 0x7e, 0x7e, pWidthArray);
                break;
        }
        pBaseDict->SetAtName("Subtype", "Type0");
        pBaseDict->SetAtName("BaseFont", basefont);
        pBaseDict->SetAtName("Encoding", cmap);
        pFontDict->SetAt("W", pWidthArray);
        pFontDict->SetAtName("Type", "Font");
        pFontDict->SetAtName("Subtype", "CIDFontType2");
        pFontDict->SetAtName("BaseFont", basefont);
        CPDF_Dictionary* pCIDSysInfo = new CPDF_Dictionary;
        pCIDSysInfo->SetAtString("Registry", "Adobe");
        pCIDSysInfo->SetAtString("Ordering", ordering);
        pCIDSysInfo->SetAtInteger("Supplement", supplement);
        pFontDict->SetAt("CIDSystemInfo", pCIDSysInfo);
        CPDF_Array* pArray = new CPDF_Array;
        pBaseDict->SetAt("DescendantFonts", pArray);
        AddIndirectObject(pFontDict);
        pArray->AddReference(this, pFontDict);
    }
    AddIndirectObject(pBaseDict);
    CPDF_Dictionary* pFontDesc = new CPDF_Dictionary;
    pFontDesc->SetAtName("Type", "FontDescriptor");
    pFontDesc->SetAtName("FontName", basefont);
    pFontDesc->SetAtInteger("Flags", flags);
    pFontDesc->SetAtInteger("ItalicAngle", pFont->m_pSubstFont ? pFont->m_pSubstFont->m_ItalicAngle : 0);
    pFontDesc->SetAtInteger("Ascent", pFont->GetAscent());
    pFontDesc->SetAtInteger("Descent", pFont->GetDescent());
    FX_RECT bbox;
    pFont->GetBBox(bbox);
    CPDF_Array* pBBox = new CPDF_Array;
    pBBox->AddInteger(bbox.left);
    pBBox->AddInteger(bbox.bottom);
    pBBox->AddInteger(bbox.right);
    pBBox->AddInteger(bbox.top);
    pFontDesc->SetAt("FontBBox", pBBox);
    FX_INT32 nStemV = 0;
    if (pFont->m_pSubstFont) {
        nStemV = pFont->m_pSubstFont->m_Weight / 5;
    } else {
        static const FX_CHAR stem_chars[] = {'i', 'I', '!', '1'};
        const size_t count = sizeof(stem_chars) / sizeof(stem_chars[0]);
        FX_DWORD glyph = pEncoding->GlyphFromCharCode(stem_chars[0]);
        nStemV = pFont->GetGlyphWidth(glyph);
        for (size_t i = 1; i < count; i++) {
            glyph = pEncoding->GlyphFromCharCode(stem_chars[i]);
            int width = pFont->GetGlyphWidth(glyph);
            if (width > 0 && width < nStemV) {
                nStemV = width;
            }
        }
    }
    if (pEncoding) {
        delete pEncoding;
    }
    pFontDesc->SetAtInteger("StemV", nStemV);
    AddIndirectObject(pFontDesc);
    pFontDict->SetAtReference("FontDescriptor", this, pFontDesc);
    return LoadFont(pBaseDict);
}
static int InsertDeletePDFPage(CPDF_Document* pDoc, CPDF_Dictionary* pPages,
                               int nPagesToGo, CPDF_Dictionary* pPage, FX_BOOL bInsert, CFX_PtrArray& stackList)
{
    CPDF_Array* pKidList = pPages->GetArray("Kids");
    if (!pKidList) {
        return -1;
    }
    int nKids = pKidList->GetCount();
    for (int i = 0; i < nKids; i ++) {
        CPDF_Dictionary* pKid = pKidList->GetDict(i);
        if (pKid->GetString("Type") == FX_BSTRC("Page")) {
            if (nPagesToGo == 0) {
                if (bInsert) {
                    pKidList->InsertAt(i, CPDF_Reference::Create(pDoc, pPage->GetObjNum()));
                    pPage->SetAtReference("Parent", pDoc, pPages->GetObjNum());
                } else {
                    pKidList->RemoveAt(i);
                }
                pPages->SetAtInteger("Count", pPages->GetInteger("Count") + (bInsert ? 1 : -1));
                return 1;
            }
            nPagesToGo --;
        } else {
            int nPages = pKid->GetInteger("Count");
            if (nPagesToGo < nPages) {
                int stackCount = stackList.GetSize();
                for (int j = 0; j < stackCount; ++j) {
                    if (pKid == stackList[j]) {
                        return -1;
                    }
                }
                stackList.Add(pKid);
                if (InsertDeletePDFPage(pDoc, pKid, nPagesToGo, pPage, bInsert, stackList) < 0) {
                    return -1;
                }
                stackList.RemoveAt(stackCount);
                pPages->SetAtInteger("Count", pPages->GetInteger("Count") + (bInsert ? 1 : -1));
                return 1;
            }
            nPagesToGo -= nPages;
        }
    }
    return 0;
}
static int InsertNewPage(CPDF_Document* pDoc, int iPage, CPDF_Dictionary* pPageDict, CFX_DWordArray &pageList)
{
    CPDF_Dictionary* pRoot = pDoc->GetRoot();
    if (!pRoot) {
        return -1;
    }
    CPDF_Dictionary* pPages = pRoot->GetDict(FX_BSTRC("Pages"));
    if (!pPages) {
        return -1;
    }
    int nPages = pDoc->GetPageCount();
    if (iPage < 0 || iPage > nPages) {
        return -1;
    }
    if (iPage == nPages) {
        CPDF_Array* pPagesList = pPages->GetArray(FX_BSTRC("Kids"));
        if (!pPagesList) {
            pPagesList = new CPDF_Array;
            pPages->SetAt(FX_BSTRC("Kids"), pPagesList);
        }
        pPagesList->Add(pPageDict, pDoc);
        pPages->SetAtInteger(FX_BSTRC("Count"), nPages + 1);
        pPageDict->SetAtReference(FX_BSTRC("Parent"), pDoc, pPages->GetObjNum());
    } else {
        CFX_PtrArray stack;
        stack.Add(pPages);
        if (InsertDeletePDFPage(pDoc, pPages, iPage, pPageDict, TRUE, stack) < 0) {
            return -1;
        }
    }
    pageList.InsertAt(iPage, pPageDict->GetObjNum());
    return iPage;
}
CPDF_Dictionary* CPDF_Document::CreateNewPage(int iPage)
{
    CPDF_Dictionary* pDict = new CPDF_Dictionary;
    pDict->SetAtName("Type", "Page");
    FX_DWORD dwObjNum = AddIndirectObject(pDict);
    if (InsertNewPage(this, iPage, pDict, m_PageList) < 0) {
        ReleaseIndirectObject(dwObjNum);
        return NULL;
    }
    return pDict;
}
int _PDF_GetStandardFontName(CFX_ByteString& name);
CPDF_Font* CPDF_Document::AddStandardFont(FX_LPCSTR font, CPDF_FontEncoding* pEncoding)
{
    CFX_ByteString name(font, -1);
    if (_PDF_GetStandardFontName(name) < 0) {
        return NULL;
    }
    return GetPageData()->GetStandardFont(name, pEncoding);
}
void CPDF_Document::DeletePage(int iPage)
{
    CPDF_Dictionary* pRoot = GetRoot();
    if (pRoot == NULL) {
        return;
    }
    CPDF_Dictionary* pPages = pRoot->GetDict("Pages");
    if (pPages == NULL) {
        return;
    }
    int nPages = pPages->GetInteger("Count");
    if (iPage < 0 || iPage >= nPages) {
        return;
    }
    CFX_PtrArray stack;
    stack.Add(pPages);
    if (InsertDeletePDFPage(this, pPages, iPage, NULL, FALSE, stack) < 0) {
        return;
    }
    m_PageList.RemoveAt(iPage);
}
CPDF_Object* FPDFAPI_GetPageAttr(CPDF_Dictionary* pPageDict, FX_BSTR name);
void FPDFAPI_FlatPageAttr(CPDF_Dictionary* pPageDict, FX_BSTR name)
{
    if (pPageDict->KeyExist(name)) {
        return;
    }
    CPDF_Object* pObj = FPDFAPI_GetPageAttr(pPageDict, name);
    if (pObj) {
        pPageDict->SetAt(name, pObj->Clone());
    }
}
