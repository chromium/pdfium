// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_ANDROID_FX_ANDROID_FONT_H_
#define CORE_FXGE_ANDROID_FX_ANDROID_FONT_H_

#include "core/fxcrt/include/fx_system.h"

#if _FX_OS_ == _FX_ANDROID_

#include "core/include/fxge/fx_font.h"

class IFPF_FontMgr;

class CFX_AndroidFontInfo : public IFX_SystemFontInfo {
 public:
  CFX_AndroidFontInfo();
  virtual void Release() { delete this; }

  virtual FX_BOOL EnumFontList(CFX_FontMapper* pMapper);

  virtual void* MapFont(int weight,
                        FX_BOOL bItalic,
                        int charset,
                        int pitch_family,
                        const FX_CHAR* face,
                        int& bExact);

  virtual void* GetFont(const FX_CHAR* face);
  virtual uint32_t GetFontData(void* hFont,
                               uint32_t table,
                               uint8_t* buffer,
                               uint32_t size);
  virtual FX_BOOL GetFaceName(void* hFont, CFX_ByteString& name);
  virtual FX_BOOL GetFontCharset(void* hFont, int& charset);

  virtual void DeleteFont(void* hFont);
  virtual void* RetainFont(void* hFont);
  FX_BOOL Init(IFPF_FontMgr* pFontMgr);

 protected:
  IFPF_FontMgr* m_pFontMgr;
};
#endif

#endif  // CORE_FXGE_ANDROID_FX_ANDROID_FONT_H_
