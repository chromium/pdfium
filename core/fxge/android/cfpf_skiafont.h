// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_ANDROID_CFPF_SKIAFONT_H_
#define CORE_FXGE_ANDROID_CFPF_SKIAFONT_H_

#include <stdint.h>

#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/fx_codepage_forward.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxge/cfx_face.h"

class CFPF_SkiaFontMgr;
class CFPF_SkiaPathFont;

class CFPF_SkiaFont {
 public:
  CFPF_SkiaFont(CFPF_SkiaFontMgr* font_mgr,
                const CFPF_SkiaPathFont* font,
                FX_Charset uCharset);
  ~CFPF_SkiaFont();

  bool IsValid() const { return !!face_; }

  ByteString GetFamilyName();
  FX_Charset GetCharset() const { return charset_; }
  uint32_t GetFontData(uint32_t dwTable, pdfium::span<uint8_t> pBuffer);

 private:
  UnownedPtr<CFPF_SkiaFontMgr> const font_mgr_;
  UnownedPtr<const CFPF_SkiaPathFont> const font_;
  RetainPtr<CFX_Face> const face_;
  const FX_Charset charset_;
};

#endif  // CORE_FXGE_ANDROID_CFPF_SKIAFONT_H_
