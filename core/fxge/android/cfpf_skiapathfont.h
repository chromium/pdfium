// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_ANDROID_CFPF_SKIAPATHFONT_H_
#define CORE_FXGE_ANDROID_CFPF_SKIAPATHFONT_H_

#include "core/fxcrt/fx_system.h"

class CFPF_SkiaPathFont {
 public:
  CFPF_SkiaPathFont();
  ~CFPF_SkiaPathFont();

  void SetPath(const char* pPath);
  void SetFamily(const char* pFamily);

  char* m_pPath = nullptr;
  char* m_pFamily = nullptr;
  uint32_t m_dwStyle = 0;
  int32_t m_iFaceIndex = 0;
  uint32_t m_dwCharsets = 0;
  int32_t m_iGlyphNum = 0;
};

#endif  // CORE_FXGE_ANDROID_CFPF_SKIAPATHFONT_H_
