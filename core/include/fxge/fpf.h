// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_PALTFORM_DEVICE_H_
#define _FX_PALTFORM_DEVICE_H_
class IFPF_DeviceModule;
class IFPF_FontMgr;
class IFPF_Font;
class IFPF_DeviceModule
{
public:
    virtual ~IFPF_DeviceModule() { }
    virtual void				Destroy() = 0;
    virtual IFPF_FontMgr*		GetFontMgr() = 0;
};
IFPF_DeviceModule*	FPF_GetDeviceModule();
#define FPF_MATCHFONT_REPLACEANSI		1
FX_DEFINEHANDLE(FPF_HFONT);
class IFPF_Font
{
public:
    virtual void			Release() = 0;
    virtual IFPF_Font*		Retain() = 0;
    virtual FPF_HFONT		GetHandle() = 0;
    virtual CFX_ByteString	GetFamilyName() = 0;
    virtual CFX_WideString	GetPsName() = 0;
    virtual FX_DWORD		GetFontStyle() const = 0;
    virtual FX_BYTE			GetCharset() const = 0;

    virtual FX_INT32		GetGlyphIndex(FX_WCHAR wUnicode) = 0;
    virtual FX_INT32		GetGlyphWidth(FX_INT32 iGlyphIndex) = 0;

    virtual FX_INT32		GetAscent() const = 0;
    virtual FX_INT32		GetDescent() const = 0;

    virtual FX_BOOL			GetGlyphBBox(FX_INT32 iGlyphIndex, FX_RECT &rtBBox) = 0;
    virtual FX_BOOL			GetBBox(FX_RECT &rtBBox) = 0;

    virtual FX_INT32		GetHeight() const = 0;
    virtual FX_INT32		GetItalicAngle() const = 0;
    virtual FX_DWORD		GetFontData(FX_DWORD dwTable, FX_LPBYTE pBuffer, FX_DWORD dwSize) = 0;

protected:
    ~IFPF_Font() { }
};
class IFPF_FontMgr
{
public:
    virtual ~IFPF_FontMgr() { }
    virtual void			LoadSystemFonts() = 0;
    virtual void			LoadPrivateFont(IFX_FileRead* pFontFile) = 0;
    virtual void			LoadPrivateFont(FX_BSTR bsFileName) = 0;
    virtual void			LoadPrivateFont(FX_LPVOID pBuffer, size_t szBuffer) = 0;

    virtual IFPF_Font*		CreateFont(FX_BSTR bsFamilyname, FX_BYTE charset, FX_DWORD dwStyle, FX_DWORD dwMatch = 0) = 0;
};
#endif
