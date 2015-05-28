// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_FXGE_ANDROID_FPF_SKIAFONT_H_
#define CORE_SRC_FXGE_ANDROID_FPF_SKIAFONT_H_

#if _FX_OS_ == _FX_ANDROID_
class CFPF_SkiaFontDescriptor;
class CFPF_SkiaFontMgr;
class SkTypeface;
class CFPF_SkiaFont : public IFPF_Font
{
public:
    CFPF_SkiaFont();
    virtual ~CFPF_SkiaFont();
    virtual void			Release();
    virtual IFPF_Font*		Retain();

    virtual FPF_HFONT		GetHandle();

    virtual CFX_ByteString	GetFamilyName();
    virtual CFX_WideString	GetPsName();

    virtual FX_DWORD		GetFontStyle() const
    {
        return m_dwStyle;
    }
    virtual FX_BYTE			GetCharset() const
    {
        return m_uCharset;
    }

    virtual FX_INT32		GetGlyphIndex(FX_WCHAR wUnicode);
    virtual FX_INT32		GetGlyphWidth(FX_INT32 iGlyphIndex);

    virtual FX_INT32		GetAscent() const;
    virtual FX_INT32		GetDescent() const;

    virtual FX_BOOL			GetGlyphBBox(FX_INT32 iGlyphIndex, FX_RECT &rtBBox);
    virtual FX_BOOL			GetBBox(FX_RECT &rtBBox);

    virtual FX_INT32		GetHeight() const;
    virtual FX_INT32		GetItalicAngle() const;
    virtual FX_DWORD		GetFontData(FX_DWORD dwTable, FX_LPBYTE pBuffer, FX_DWORD dwSize);
    FX_BOOL					InitFont(CFPF_SkiaFontMgr *pFontMgr, CFPF_SkiaFontDescriptor *pFontDes, FX_BSTR bsFamily, FX_DWORD dwStyle, FX_BYTE uCharset);
protected:
    CFPF_SkiaFontMgr		*m_pFontMgr;
    CFPF_SkiaFontDescriptor	*m_pFontDes;
    FXFT_Face				m_Face;
    FX_DWORD				m_dwStyle;
    FX_BYTE					m_uCharset;
    FX_DWORD				m_dwRefCount;
};
#endif

#endif  // CORE_SRC_FXGE_ANDROID_FPF_SKIAFONT_H_
