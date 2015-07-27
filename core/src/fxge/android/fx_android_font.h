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

    virtual	bool		EnumFontList(CFX_FontMapper* pMapper);

    virtual void*		MapFont(int weight, bool bItalic, int charset, int pitch_family, const FX_CHAR* face, bool& bExact);

    virtual void*		GetFont(const FX_CHAR* face);
    virtual FX_DWORD	GetFontData(void* hFont, FX_DWORD table, uint8_t* buffer, FX_DWORD size);
    virtual bool		GetFaceName(void* hFont, CFX_ByteString& name);
    virtual bool		GetFontCharset(void* hFont, int& charset);

    virtual void		DeleteFont(void* hFont);
    virtual void*       RetainFont(void* hFont);
    bool				Init(IFPF_FontMgr *pFontMgr);
protected:
    IFPF_FontMgr		*m_pFontMgr;
};
#endif

#endif  // CORE_SRC_FXGE_ANDROID_FX_ANDROID_FONT_H_
