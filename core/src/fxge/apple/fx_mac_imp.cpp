// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fxge/fx_ge.h"
#include "apple_int.h"
#if _FX_OS_ == _FX_MACOSX_
static const struct {
    FX_LPCSTR	m_pName;
    FX_LPCSTR	m_pSubstName;
}
Base14Substs[] = {
    {"Courier", "Courier New"},
    {"Courier-Bold", "Courier New Bold"},
    {"Courier-BoldOblique", "Courier New Bold Italic"},
    {"Courier-Oblique", "Courier New Italic"},
    {"Helvetica", "Arial"},
    {"Helvetica-Bold", "Arial Bold"},
    {"Helvetica-BoldOblique", "Arial Bold Italic"},
    {"Helvetica-Oblique", "Arial Italic"},
    {"Times-Roman", "Times New Roman"},
    {"Times-Bold", "Times New Roman Bold"},
    {"Times-BoldItalic", "Times New Roman Bold Italic"},
    {"Times-Italic", "Times New Roman Italic"},
};
class CFX_MacFontInfo : public CFX_FolderFontInfo
{
public:
    virtual void*		MapFont(int weight, FX_BOOL bItalic, int charset, int pitch_family, FX_LPCSTR family, FX_BOOL& bExact);
};
#define JAPAN_GOTHIC "Hiragino Kaku Gothic Pro W6"
#define JAPAN_MINCHO "Hiragino Mincho Pro W6"
static void GetJapanesePreference(CFX_ByteString& face, int weight, int picth_family)
{
    if (face.Find("Gothic") >= 0) {
        face = JAPAN_GOTHIC;
        return;
    }
    if (!(picth_family & FXFONT_FF_ROMAN) && weight > 400) {
        face = JAPAN_GOTHIC;
    } else {
        face = JAPAN_MINCHO;
    }
}
void* CFX_MacFontInfo::MapFont(int weight, FX_BOOL bItalic, int charset, int pitch_family, FX_LPCSTR cstr_face, FX_BOOL& bExact)
{
    CFX_ByteString face = cstr_face;
    int iBaseFont;
    for (iBaseFont = 0; iBaseFont < 12; iBaseFont ++)
        if (face == CFX_ByteStringC(Base14Substs[iBaseFont].m_pName)) {
            face = Base14Substs[iBaseFont].m_pSubstName;
            bExact = TRUE;
            break;
        }
    if (iBaseFont < 12) {
        return GetFont(face);
    }
    FX_LPVOID p;
    if (m_FontList.Lookup(face, p)) {
        return p;
    }
    if (charset == FXFONT_ANSI_CHARSET && (pitch_family & FXFONT_FF_FIXEDPITCH)) {
        return GetFont("Courier New");
    }
    if (charset == FXFONT_ANSI_CHARSET || charset == FXFONT_SYMBOL_CHARSET) {
        return NULL;
    }
    switch (charset) {
        case FXFONT_SHIFTJIS_CHARSET:
            GetJapanesePreference(face, weight, pitch_family);
            break;
        case FXFONT_GB2312_CHARSET:
            face = "STSong";
            break;
        case FXFONT_HANGEUL_CHARSET:
            face = "AppleMyungjo";
            break;
        case FXFONT_CHINESEBIG5_CHARSET:
            face = "LiSong Pro Light";
    }
    if (m_FontList.Lookup(face, p)) {
        return p;
    }
    return NULL;
}
IFX_SystemFontInfo* IFX_SystemFontInfo::CreateDefault()
{
    CFX_MacFontInfo* pInfo = new CFX_MacFontInfo;
    pInfo->AddPath("~/Library/Fonts");
    pInfo->AddPath("/Library/Fonts");
    pInfo->AddPath("/System/Library/Fonts");
    return pInfo;
}
void CFX_GEModule::InitPlatform()
{
    m_pPlatformData = new CApplePlatform;
    m_pFontMgr->SetSystemFontInfo(IFX_SystemFontInfo::CreateDefault());
}
void CFX_GEModule::DestroyPlatform()
{
    if (m_pPlatformData) {
        delete (CApplePlatform *) m_pPlatformData;
    }
    m_pPlatformData = NULL;
}
#endif
