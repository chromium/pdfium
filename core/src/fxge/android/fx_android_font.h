// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_FXGE_ANDROID_FX_ANDROID_FONT_H_
#define CORE_SRC_FXGE_ANDROID_FX_ANDROID_FONT_H_

#if _FX_OS_ == _FX_ANDROID_
class IFPF_FontMgr;
class CFX_AndroidFontInfo : public IFX_SystemFontInfo
{
public:
    CFX_AndroidFontInfo();
    virtual void		Release()
    {
        delete this;
    }

    virtual	FX_BOOL		EnumFontList(CFX_FontMapper* pMapper);

    virtual void*		MapFont(int weight, FX_BOOL bItalic, int charset, int pitch_family, FX_LPCSTR face, FX_BOOL& bExact);

    virtual void*		GetFont(FX_LPCSTR face);
    virtual FX_DWORD	GetFontData(void* hFont, FX_DWORD table, FX_LPBYTE buffer, FX_DWORD size);
    virtual FX_BOOL		GetFaceName(void* hFont, CFX_ByteString& name);
    virtual FX_BOOL		GetFontCharset(void* hFont, int& charset);

    virtual void		DeleteFont(void* hFont);
    virtual void*       RetainFont(void* hFont);
    FX_BOOL				Init(IFPF_FontMgr *pFontMgr);
protected:
    IFPF_FontMgr		*m_pFontMgr;
};
#endif

#endif  // CORE_SRC_FXGE_ANDROID_FX_ANDROID_FONT_H_
