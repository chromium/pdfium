// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FXGE_FPF_H_
#define CORE_INCLUDE_FXGE_FPF_H_

#include "core/include/fxcrt/fx_coordinates.h"

class IFPF_FontMgr;

class IFPF_DeviceModule {
 public:
  virtual ~IFPF_DeviceModule() {}
  virtual void Destroy() = 0;
  virtual IFPF_FontMgr* GetFontMgr() = 0;
};

IFPF_DeviceModule* FPF_GetDeviceModule();

#define FPF_MATCHFONT_REPLACEANSI 1

typedef struct FPF_HFONT_ { void* pData; } * FPF_HFONT;

class IFPF_Font {
 public:
  virtual void Release() = 0;
  virtual IFPF_Font* Retain() = 0;
  virtual FPF_HFONT GetHandle() = 0;
  virtual CFX_ByteString GetFamilyName() = 0;
  virtual CFX_WideString GetPsName() = 0;
  virtual FX_DWORD GetFontStyle() const = 0;
  virtual uint8_t GetCharset() const = 0;

  virtual int32_t GetGlyphIndex(FX_WCHAR wUnicode) = 0;
  virtual int32_t GetGlyphWidth(int32_t iGlyphIndex) = 0;

  virtual int32_t GetAscent() const = 0;
  virtual int32_t GetDescent() const = 0;

  virtual FX_BOOL GetGlyphBBox(int32_t iGlyphIndex, FX_RECT& rtBBox) = 0;
  virtual FX_BOOL GetBBox(FX_RECT& rtBBox) = 0;

  virtual int32_t GetHeight() const = 0;
  virtual int32_t GetItalicAngle() const = 0;
  virtual FX_DWORD GetFontData(FX_DWORD dwTable,
                               uint8_t* pBuffer,
                               FX_DWORD dwSize) = 0;

 protected:
  virtual ~IFPF_Font() {}
};

class IFPF_FontMgr {
 public:
  virtual ~IFPF_FontMgr() {}
  virtual void LoadSystemFonts() = 0;
  virtual void LoadPrivateFont(IFX_FileRead* pFontFile) = 0;
  virtual void LoadPrivateFont(const CFX_ByteStringC& bsFileName) = 0;
  virtual void LoadPrivateFont(void* pBuffer, size_t szBuffer) = 0;

  virtual IFPF_Font* CreateFont(const CFX_ByteStringC& bsFamilyname,
                                uint8_t charset,
                                FX_DWORD dwStyle,
                                FX_DWORD dwMatch = 0) = 0;
};

#endif  // CORE_INCLUDE_FXGE_FPF_H_
